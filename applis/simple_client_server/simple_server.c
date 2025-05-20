/*
 * Simple demo application on top of the SWIF-codec API.
 *
 * It is inspired from the same application from openFEC
 * (http://openfec.org/downloads.html) modified in order
 * to be used with the appropriate API.
 *
 * Author: Vincent Roca (Inria)
 */

#include "simple_client_server.h"

/* Prototypes */

/**
 * Opens and initializes a UDP socket, ready for receptions.
 */
static SOCKET init_socket(SOCKADDR_IN *dst_host);

/**
 * Dumps len32 32-bit words of a buffer (typically a symbol).
 */
static void dump_buffer_32(void *buf, uint32_t len32);

/**
 * Should the next packet be lost?
 */
static bool should_be_lost(double loss_rate);

/**
 * Callback (not really required).
 */
static void source_symbol_removed_from_coding_window_callback(void *context, esi_t old_symbol_esi);

/*************************************************************************************************/
static void cleanup(
    SOCKET so, swif_encoder_t *ses, void **enc_symbols_tab, uint32_t tot_enc, char *pkt_with_fpi)
{
    if(so != INVALID_SOCKET)
    {
        close(so);
    }
    if(ses)
    {
        swif_encoder_release(ses);
    }
    if(enc_symbols_tab)
    {
        for(uint32_t esi = 0; esi < tot_enc; esi++)
        {
            if(enc_symbols_tab[esi])
            {
                free(enc_symbols_tab[esi]);
            }
        }
        free(enc_symbols_tab);
    }
    if(pkt_with_fpi)
    {
        free(pkt_with_fpi);
    }
}

int main(int argc, char *argv[])
{
    if(argc < 5)
    {
        fprintf(stderr, "Usage: %s <loss_rate> <encoding_window_size> <code_rate> <dt>\n", argv[0]);
        return -1;
    }

    // Parse loss rate
    double loss_rate = atof(argv[1]);
    if(loss_rate < 0.0 || loss_rate > 1.0)
    {
        fprintf(stderr, "Error: invalid loss rate (%f). Must be between 0.0 and 1.0.\n", loss_rate);
        return -1;
    }

    // Parse encoding window size
    uint32_t ew_size = atoi(argv[2]);
    if(ew_size < 2)
    {
        fprintf(stderr, "Error: invalid encoding window size (%u). Must be >= 2.\n", ew_size);
        return -1;
    }

    // Parse code rate
    double code_rate = atof(argv[3]);
    if(code_rate <= 0.0 || code_rate > 1.0)
    {
        fprintf(stderr, "Error: invalid code rate (%f). Must be > 0.0 and <= 1.0.\n", code_rate);
        return -1;
    }

    // Parse dt
    uint32_t dt = atoi(argv[4]);
    if(dt < 0 || dt > 15)
    {
        fprintf(stderr, "Error: invalid dt (%u). Must be >= 0 and <= 15.\n", dt);
        return -1;
    }

    swif_codepoint_t codepoint; /* identifier of the codec to use */
    swif_encoder_t *ses = NULL;
    void **enc_symbols_tab =
        NULL; /* table containing pointers to the encoding (i.e. source + repair) symbols buffers */
    uint32_t tot_src; /* total number of source symbols */
    uint32_t tot_enc; /* total number of encoding symbols (i.e. source + repair) in the session */
    esi_t esi;        /* source symbol id */
    uint32_t idx;     /* index in the source+repair table */
    SOCKET so = INVALID_SOCKET; /* UDP socket for server => client communications */
    char *pkt_with_fpi =
        NULL; /* buffer containing a fixed size packet plus a header consisting only of the FPI */
    fec_oti_t fec_oti; /* FEC Object Transmission Information as sent to the client */
    repair_fpi_t *fpi; /* header (FEC Payload Information) for source and repair symbols */
    SOCKADDR_IN dst_host;
    uint32_t ret = -1;

    if(ew_size < 2)
    {
        fprintf(stderr, "Error: invalid encoding window size (%u). Cannot be < 2.\n", ew_size);
        ret = -1;
        return ret;
    }
    tot_src = 1000;
    tot_enc = (uint32_t)floor((double)tot_src / (double)code_rate);
    if(tot_enc < tot_src)
    {
        fprintf(stderr, "Error initializing tot_enc (%u). Cannot be < tot_src (%u)!\n", tot_enc,
                tot_src);
        ret = -1;
        return ret;
    }
    codepoint = SWIF_CODEPOINT_RLC_GF_256_FULL_DENSITY_CODEC;

    /* first initialize the UDP socket... */
    if((so = init_socket(&dst_host)) == INVALID_SOCKET)
    {
        fprintf(stderr, "Error initializing socket!\n");
        ret = -1;
        return ret;
    }
    printf("First of all, send the FEC OTI to %s/%d\n", DEST_IP, DEST_PORT);
    /* initialize and send the FEC OTI to the client */
    fec_oti.codepoint = htonl(codepoint);
    fec_oti.ew_size = htonl(ew_size);
    fec_oti.tot_src = htonl(tot_src);
    fec_oti.tot_enc = htonl(tot_enc);
    if((ret = sendto(so, (void *)&fec_oti, sizeof(fec_oti), 0, (SOCKADDR *)&dst_host,
                     sizeof(dst_host))) != sizeof(fec_oti))
    {
        fprintf(stderr, "Error while sending the FEC OTI\n");
        ret = -1;
        cleanup(so, NULL, NULL, 0, NULL);
        return ret;
    }
    /* allocate a buffer where we'll copy each symbol plus its simplified FPI.
     * This buffer will be reused during the whole session */
    if((pkt_with_fpi = malloc(sizeof(repair_fpi_t) + SYMBOL_SIZE)) == NULL)
    {
        fprintf(stderr, "no memory (malloc failed for pkt_with_fpi)\n");
        ret = -1;
        cleanup(so, NULL, NULL, 0, NULL);
        return ret;
    }
    /* continue with the SWIF codec */
    printf("\nInitialize a SWIF encoder instance: tot_src=%u src symbols, ew_size=%u, total %u "
           "encoding symbols\n",
           tot_src, ew_size, tot_enc);
    if((ses = swif_encoder_create(codepoint, VERBOSITY, SYMBOL_SIZE, ew_size)) == NULL)
    {
        fprintf(stderr, "Error, swif_encoder_create() failed\n");
        ret = -1;
        cleanup(so, NULL, NULL, 0, pkt_with_fpi);
        return ret;
    }
    if(swif_encoder_set_callback_functions(ses, source_symbol_removed_from_coding_window_callback,
                                           NULL) != SWIF_STATUS_OK)
    {
        fprintf(stderr, "Error, swif_encoder_set_callback_functions() failed\n");
        ret = -1;
        cleanup(so, ses, NULL, 0, pkt_with_fpi);
        return ret;
    }
    /* allocate the table with pointers to source and repair symbols... */
    if((enc_symbols_tab = (void **)calloc(tot_enc, sizeof(void *))) == NULL)
    {
        fprintf(stderr, "Error, no memory (calloc failed for enc_symbols_tab, tot_enc=%u)\n",
                tot_enc);
        ret = -1;
        cleanup(so, ses, NULL, 0, pkt_with_fpi);
        return ret;
    }
    /*
     * main loop, where the application goes through all source symbols and submits them one by one
     * to the codec, asking for a repair symbol from time to time.
     */
    double interval_between_repairs = (double)tot_src / (double)(tot_enc - tot_src);
    double repair_counter = 0.0; // Tracks when to generate a repair symbol
    idx = 0;
    for(esi = 0; esi < tot_src; esi++)
    {
        if((enc_symbols_tab[idx] = malloc(SYMBOL_SIZE)) == NULL)
        {
            fprintf(stderr, "Error, no memory (calloc failed for enc_symbols_tab[%u]/esi=%u)\n",
                    idx, esi);
            ret = -1;
            cleanup(so, ses, enc_symbols_tab, tot_enc, pkt_with_fpi);
            return ret;
        }
        /* in order to detect corruption, the first source symbol is filled with 0x1111..., the
         * second with 0x2222..., etc. NB: the 0x0 value is avoided since it is a neutral element in
         * the target finite fields, i.e. it prevents the detection of symbol corruption */
        memset(enc_symbols_tab[idx], (char)(esi + 1), SYMBOL_SIZE);
        /* add it to the encoding window (no need to do anything else for a source symbol) */
        if(swif_encoder_add_source_symbol_to_coding_window(ses, enc_symbols_tab[idx], esi) !=
           SWIF_STATUS_OK)
        {
            fprintf(stderr,
                    "Error, swif_encoder_add_source_symbol_to_coding_window failed for esi=%u)\n",
                    esi);
            ret = -1;
            cleanup(so, ses, enc_symbols_tab, tot_enc, pkt_with_fpi);
            return ret;
        }
        /* prepend a header in network byte order */
        fpi = (repair_fpi_t *)pkt_with_fpi;
        fpi->is_source = htons(1);
        fpi->repair_key = htons(0);          /* only meaningful in case of a repair */
        uint16_t dt_nss = dt << 12 & 0xF000; // dt is the upper 4 bits
        fpi->dt_nss = htons(dt_nss);
        printf("SENDING dt_nss=%x, dt=%x, nss=%x\n", dt_nss, dt, 0);
        fpi->esi = htonl(esi);
        memcpy(pkt_with_fpi + sizeof(repair_fpi_t), enc_symbols_tab[idx], SYMBOL_SIZE);
        if(should_be_lost(loss_rate))
        {
            printf(" => src symbol %u is lost\n", esi);
        }
        else
        {
            if(VERBOSITY > 1)
            {
                printf("src[%03d]= ", esi);
                dump_buffer_32(pkt_with_fpi, 8);
            }
            else
            {
                printf(" => sending src symbol %u\n", esi);
            }
            if((ret = sendto(so, pkt_with_fpi, sizeof(repair_fpi_t) + SYMBOL_SIZE, 0,
                             (SOCKADDR *)&dst_host, sizeof(dst_host))) == SOCKET_ERROR)
            {
                fprintf(stderr, "Error, sendto() failed!\n");
                ret = -1;
                cleanup(so, ses, enc_symbols_tab, tot_enc, pkt_with_fpi);
                return ret;
            }
        }
        /* perform a short usleep() to slow down transmissions and avoid UDP socket saturation at
         * the receiver. Note that the true solution consists in adding some rate control mechanism
         * here... */
        usleep(300);
        repair_counter += 1.0 / interval_between_repairs;
        idx++;
        while((repair_counter >= 1.0) && (idx < tot_enc))
        {
            repair_counter -= 1.0;
            esi_t first;
            esi_t last;
            uint32_t nss;
            /* the index is the repair_key */
            if(swif_encoder_generate_coding_coefs(ses, idx, dt, 0) != SWIF_STATUS_OK)
            {
                fprintf(stderr,
                        "Error, swif_decoder_generate_coding_coefs() failed for repair_key=%u\n",
                        idx);
                ret = -1;
                cleanup(so, ses, enc_symbols_tab, tot_enc, pkt_with_fpi);
                return ret;
            }
            /* the build_repair allocates memory for enc_symbols_tab[idx] and updates the table
             * itself. */
            if(swif_build_repair_symbol(ses, &enc_symbols_tab[idx]) != SWIF_STATUS_OK)
            {
                fprintf(stderr, "Error, swif_build_repair_symbol() failed for repair_key=%u\n",
                        idx);
                ret = -1;
                cleanup(so, ses, enc_symbols_tab, tot_enc, pkt_with_fpi);
                return ret;
            }
            /* prepend a header in network byte order */
            if(swif_encoder_get_coding_window_information(ses, &first, &last, &nss) !=
               SWIF_STATUS_OK)
            {
                fprintf(stderr,
                        "Error, swif_encoder_get_coding_window_information() failed for "
                        "repair_key=%u\n",
                        idx);
                ret = -1;
                cleanup(so, ses, enc_symbols_tab, tot_enc, pkt_with_fpi);
                return ret;
            }
            /* in our simple case, there is no ESI loop back to zero, so check consistency */
            if(nss != last - first + 1)
            {
                fprintf(stderr,
                        "Error, nss (%u) != last (%u) - first (%u) + 1, it should be equal\n", nss,
                        last, first);
                ret = -1;
                cleanup(so, ses, enc_symbols_tab, tot_enc, pkt_with_fpi);
                return ret;
            }
            fpi = (repair_fpi_t *)pkt_with_fpi;
            fpi->is_source = htons(0);
            fpi->repair_key = htons(idx);

            fpi->dt_nss = htons((dt << 12) | ((nss) & 0x0FFF)); // dt is the upper 4 bits
            printf("SENDING dt_nss=%x, dt=%x, nss=%x\n", fpi->dt_nss, dt, nss);

            fpi->esi = htonl(first);
            memcpy(pkt_with_fpi + sizeof(repair_fpi_t), enc_symbols_tab[idx], SYMBOL_SIZE);
            if(should_be_lost(loss_rate))
            {
                printf(" => repair symbol %u is lost\n", esi);
            }
            else
            {
                if(VERBOSITY > 1)
                {
                    printf("repair[%03d]= ", first);
                    dump_buffer_32(pkt_with_fpi, 8);
                }
                else
                {
                    printf(" => sending repair symbol %u\n", first);
                }
                if((ret = sendto(so, pkt_with_fpi, sizeof(repair_fpi_t) + SYMBOL_SIZE, 0,
                                 (SOCKADDR *)&dst_host, sizeof(dst_host))) == SOCKET_ERROR)
                {
                    fprintf(stderr, "Error, sendto() failed!\n");
                    ret = -1;
                    cleanup(so, ses, enc_symbols_tab, tot_enc, pkt_with_fpi);
                    return ret;
                }
            }
            /* Perform a short usleep() to slow down transmissions and avoid UDP socket saturation
             * at the receiver. Note that the true solution consists in adding some rate control
             * mechanism here, like a leaky or token bucket. */
            usleep(300);
            idx++;
        }
    }
    printf("\nCompleted, %d packets sent successfully.\n", idx);
    ret = 1;
    return ret;
}

/* Initialize our UDP socket */
static SOCKET init_socket(SOCKADDR_IN *dst_host)
{
    SOCKET s;

    if((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        printf("Error, call to socket() failed\n");
        return INVALID_SOCKET;
    }
    dst_host->sin_family = AF_INET;
    dst_host->sin_port = htons((short)DEST_PORT);
    dst_host->sin_addr.s_addr = inet_addr(DEST_IP);
    return s;
}

static void dump_buffer_32(void *buf, uint32_t len32)
{
    uint32_t *ptr;
    uint32_t j = 0;

    printf("0x");
    for(ptr = (uint32_t *)buf; len32 > 0; len32--, ptr++)
    {
        /* convert to big endian format to be sure of byte order */
        printf("%08X", htonl(*ptr));
        if(++j == 10)
        {
            j = 0;
            printf("\n");
        }
    }
    printf("\n");
}

static bool should_be_lost(double loss_rate)
{
    uint32_t lost;

    /* Randomly loose loss_rate fraction of packets. */
    lost = rand() % 100;
    if(lost < (loss_rate * 100))
    {
        return true;
    }
    else
    {
        return false;
    }
}

static void source_symbol_removed_from_coding_window_callback(void *context, esi_t old_symbol_esi)
{
    if(VERBOSITY > 1)
    {
        printf("callback: symbol %u removed\n", old_symbol_esi);
    }
}
