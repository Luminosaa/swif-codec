#---------------------------------------------------------------------------
# swif-codec - 2019
#---------------------------------------------------------------------------
# Definitions of the "C" structure, functions etc.
# of SWIF includes.
#---------------------------------------------------------------------------

from libc.stdint cimport uint8_t, uint32_t, int64_t, bool
from libc.stdio cimport FILE

cdef extern from "swif_api.h":
    ctypedef enum swif_status_t:
        SWIF_STATUS_OK = 0
        SWIF_STATUS_FAILURE
        SWIF_STATUS_ERROR

    ctypedef enum swif_errno_t:
        SWIF_ERRNO_NULL = 0
        SWIF_ERRNO_UNSUPPORTED_CODEPOINT

    ctypedef enum swif_codepoint_t:
        SWIF_CODEPOINT_NULL = 0
        SWIF_CODEPOINT_RLC_GF_256_FULL_DENSITY_CODEC

    ctypedef uint32_t esi_t

    ctypedef struct swif_encoder_t:
        swif_codepoint_t codepoint;
        swif_errno_t     swif_errno;

    swif_encoder_t *swif_encoder_create (
        swif_codepoint_t codepoint,
        uint32_t        verbosity,
        uint32_t        symbol_size,
        uint32_t        max_coding_window_size);

    swif_status_t swif_encoder_release(swif_encoder_t *enc);

    swif_status_t swif_build_repair_symbol(swif_encoder_t *enc,
                                           uint8_t  *new_buf);

    swif_status_t swif_encoder_add_source_symbol_to_coding_window (
        swif_encoder_t *enc, void *new_src_symbol_buf,
        esi_t new_src_symbol_esi);

    swif_status_t swif_encoder_remove_source_symbol_from_coding_window (
        swif_encoder_t* enc,
        esi_t old_src_symbol_esi);

    swif_status_t swif_encoder_set_coding_coefs_tab (
        swif_encoder_t* enc,
        void* coding_coefs_tab,
        uint32_t nb_coefs_in_tab);

    swif_status_t swif_encoder_generate_coding_coefs (
        swif_encoder_t *enc,
        uint32_t       key,
        uint32_t       add_param);

    swif_status_t swif_encoder_get_coding_coefs_tab (
        swif_encoder_t *enc,
        void           **coding_coefs_tab,
        uint32_t       *nb_coefs_in_tab);


    ctypedef struct swif_decoder_t:
        swif_codepoint_t codepoint;
        swif_errno_t     swif_errno;

    swif_decoder_t *swif_decoder_create (
        swif_codepoint_t codepoint,
        uint32_t        verbosity,
        uint32_t        symbol_size,
        uint32_t        max_coding_window_size,
        uint32_t        max_linear_system_size);

    swif_status_t swif_decoder_add_source_symbol_to_coding_window (
        swif_decoder_t* dec,
        esi_t           new_src_symbol_esi)

#---------------------------------------------------------------------------


#cdef extern from "swif_rlc_cb.h":
#    ctypedef struct swif_encoder_rlc_cb_t:
#        swif_codepoint_t  codepoint
#        swif_errno_t      swif_errno
#        uint32_t          verbosity
#        uint32_t          max_coding_window_size
#        uint32_t          symbol_size
#        uint8_t	          *cc_tab
#        uint32_t          cw_nb_in_list

#     ctypedef struct swif_decoder_rlc_cb_t:
#        swif_codepoint_t  codepoint
#        swif_errno_t      swif_errno
#        uint32_t          verbosity
#       uint32_t          max_coding_window_size
#       uint32_t          symbol_size
#       uint8_t	          *coef_tab
#       uint32_t          nb_id

#---------------------------------------------------------------------------

cdef extern from "swif_symbol.h":
    ctypedef uint32_t symbol_id_t

    uint8_t gf256_inv(uint8_t a)
    uint8_t gf256_add(uint8_t a, uint8_t b)
    uint8_t gf256_sub(uint8_t a, uint8_t b)    
    uint8_t gf256_mul(uint8_t a, uint8_t b)
    uint8_t gf256_div(uint8_t a, uint8_t b)

    void symbol_add_scaled(void *symbol1, uint8_t coef,
                           void *symbol2, uint32_t symbol_size);

    void symbol_add(void *symbol1, void *symbol2,
                    uint32_t symbol_size, uint8_t* result);

    void symbol_sub(void *symbol1, void *symbol2,
                    uint32_t symbol_size, uint8_t* result);

    void symbol_mul(void *symbol1, uint8_t coeff,
                    uint32_t symbol_size, uint8_t* result);

    void symbol_div(void *symbol1, uint32_t symbol_size,
                    uint8_t coeff, uint8_t* result);

#---------------------------------------------------------------------------

cdef extern from "swif_full_symbol.h":
     cdef unsigned int SYMBOL_ID_NONE

     cdef struct s_swif_full_symbol_t:
         uint8_t *coef     
         symbol_id_t first_id
         symbol_id_t last_id
         symbol_id_t first_nonzero_id
         symbol_id_t last_nonzero_id

         uint8_t *data
         uint32_t data_size

     ctypedef s_swif_full_symbol_t swif_full_symbol_t

     swif_full_symbol_t *full_symbol_create_from_source(
         uint32_t full_symbol_id, uint8_t *full_symbol_data,
         uint32_t full_symbol_size);
     
     swif_full_symbol_t *full_symbol_create(
         uint8_t *symbol_coef_table, uint32_t min_symbol_id,
         uint32_t nb_symbol_id,
         uint8_t *symbol_data, uint32_t symbol_size);

     uint32_t full_symbol_get_size(swif_full_symbol_t *full_symbol);

     uint32_t full_symbol_get_min_symbol_id(swif_full_symbol_t *full_symbol);

     uint32_t full_symbol_get_max_symbol_id(swif_full_symbol_t *full_symbol);

     uint32_t full_symbol_get_max_symbol_id(swif_full_symbol_t *full_symbol);

     uint32_t full_symbol_count_coef(swif_full_symbol_t *full_symbol);

     uint8_t  full_symbol_get_coef(swif_full_symbol_t *full_symbol,
                                   uint32_t symbol_id);

     void     full_symbol_set_coef(swif_full_symbol_t *full_symbol,
                                   uint32_t symbol_id, uint8_t coef);
     
     bint full_symbol_is_zero(swif_full_symbol_t *full_symbol);

     void full_symbol_free(swif_full_symbol_t* swif_full_symbol);
     
     swif_full_symbol_t *full_symbol_clone(
         swif_full_symbol_t *swif_full_symbol);

     void full_symbol_get_data(
         swif_full_symbol_t *full_symbol, uint8_t *result_data);

     void full_symbol_dump(swif_full_symbol_t *full_symbol, FILE *out);     

#---------------------------------------------------------------------------


cdef extern from "swif_full_symbol_impl.h":

     ctypedef s_swif_full_symbol_set_t swif_full_symbol_set_t

     ctypedef void (*notify_decoded_func_t)(swif_full_symbol_set_t *set, symbol_id_t symbol_id, void *context);


     cdef struct s_swif_full_symbol_set_t:
         uint32_t size
         uint32_t first_symbol_id
         uint32_t nmbr_packets
         swif_full_symbol_t **full_symbol_tab
         notify_decoded_func_t notify_decoded_func
         void                  *notify_context
     
     ctypedef uint32_t symbol_id_t

     swif_full_symbol_set_t *full_symbol_set_alloc()

     void full_symbol_set_free(swif_full_symbol_set_t *set)

     uint32_t full_symbol_set_add(
         swif_full_symbol_set_t* set, swif_full_symbol_t* full_symbol)

     swif_full_symbol_t *full_symbol_alloc(
         symbol_id_t first_symbol_id, symbol_id_t last_symbol_id, uint32_t symbol_size)

     bint full_symbol_has_sufficient_size(
         swif_full_symbol_t* symbol, symbol_id_t id1, symbol_id_t id2)
     bint full_symbol_includes_id(
         swif_full_symbol_t* symbol, symbol_id_t symbol_id)

     bint full_symbol_adjust_min_coef(swif_full_symbol_t* symbol)

     bint full_symbol_adjust_max_coef(swif_full_symbol_t* symbol)

     bint full_symbol_adjust_min_max_coef(swif_full_symbol_t* symbol)


     void full_symbol_scale(
         void *symbol1, uint8_t coereef)

     void full_symbol_add_base(swif_full_symbol_t *symbol1, swif_full_symbol_t *symbol2, swif_full_symbol_t *symbol_result)

     swif_full_symbol_t* full_symbol_add(
         swif_full_symbol_t *symbol1, swif_full_symbol_t *symbol2)

     void full_symbol_set_dump(swif_full_symbol_set_t *full_symbol_set, FILE *out)
    
     swif_full_symbol_t *full_symbol_set_get_pivot(
         swif_full_symbol_set_t *full_symbol_set, symbol_id_t symbol_id)

     swif_full_symbol_t *full_symbol_set_remove_each_pivot(
         swif_full_symbol_set_t *full_symbol_set, swif_full_symbol_t *new_symbol)

     uint32_t full_symbol_set_add_as_pivot(
        swif_full_symbol_set_t *full_symbol_set, swif_full_symbol_t *new_symbol)

     void full_symbol_add_with_elimination(
        swif_full_symbol_set_t *full_symbol_set, swif_full_symbol_t *new_symbol)


#---------------------------------------------------------------------------
cdef extern from "swif_rlc_api.h":
     swif_status_t   swif_rlc_decoder_decode_with_new_source_symbol (    
                                swif_decoder_t* dec,
                                void* const     new_symbol_buf,
                                esi_t           new_symbol_esi)
    
     swif_status_t   swif_rlc_decoder_decode_with_new_repair_symbol (
                                swif_decoder_t* dec,
                                void* const     new_symbol_buf,
                                esi_t           new_symbol_esi)
    
     swif_status_t   swif_rlc_decoder_generate_coding_coefs (
                                swif_decoder_t* dec,
                                uint32_t        key,
                                uint8_t         dt,
                                uint32_t        add_param)
     swif_status_t   swif_rlc_decoder_reset_coding_window (
                                swif_decoder_t*  dec)
    
     swif_status_t   swif_rlc_decoder_decode_with_new_source_symbol (
                                swif_decoder_t* dec,
                                void* const     new_symbol_buf,
                                esi_t           new_symbol_esi)

