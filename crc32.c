#include "crc32.h"

unsigned int calculate_crc32(char *buffer, unsigned int length) {

    unsigned int result = 0xFFFFFFFFUL;

    if(length == 0)
        return result;

    while(length) {
        result = ((result >> 8) & 0x00FFFFFF) ^ crc_32_tab[(result ^ (*buffer++)) & 0xFF ];
        length -= 1;
    }

    return result ^ 0xFFFFFFFFUL;

}
