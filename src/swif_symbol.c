/*---------------------------------------------------------------------------*/

#include "swif_symbol.h"

/*---------------------------------------------------------------------------*/

#include "swif_table-mul-gf256.c"
static inline uint8_t gf256_mul(uint8_t a, uint8_t b)
{ return gf256_mul_table[a][b]; }

/**
 * @brief Take a symbol and add another symbol multiplied by a 
 *        coefficient, e.g. performs the equivalent of: p1 += coef * p2
 * @param[in,out] p1     First symbol (to which coef*p2 will be added)
 * @param[in]     coef  Coefficient by which the second packet is multiplied
 * @param[in]     p2     Second symbol
 */
void symbol_add_scaled
(void *symbol1, uint8_t coef, void *symbol2, uint32_t symbol_size)
{
    uint8_t *data1 = (uint8_t *) symbol1;
    uint8_t *data2 = (uint8_t *) symbol2; 
    for (uint32_t i=0; i<symbol_size; i++) {
        data1[i] ^= gf256_mul(coef, data2[i]);
    }
}

/*---------------------------------------------------------------------------*/
