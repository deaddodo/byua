#include "shared.h"

bool check_nth_bit(unsigned char check, int pos) {
    static unsigned char masks[] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
    return ((check&masks[pos]) != 0);
}