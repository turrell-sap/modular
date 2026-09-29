/* CRC32C (Castagnoli), reflected, polynomial 0x1EDC6F41 -> 0x82F63B78.
 * Init 0xFFFFFFFF, final XOR 0xFFFFFFFF. RFC 3720 Appendix B.4. */
#include "uaf_wire.h"

static uint32_t g_tab[256];
static int g_ready;

static void build(void)
{
    for (uint32_t i = 0; i < 256u; i++) {
        uint32_t c = i;
        for (int k = 0; k < 8; k++)
            c = (c & 1u) ? (0x82F63B78u ^ (c >> 1)) : (c >> 1);
        g_tab[i] = c;
    }
    g_ready = 1;
}

uint32_t uaf_crc32c(uint32_t crc, const void *buf, size_t len)
{
    const uint8_t *p = (const uint8_t *)buf;
    if (!g_ready) build();
    for (size_t i = 0; i < len; i++)
        crc = g_tab[(crc ^ p[i]) & 0xFFu] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFFu;
}
