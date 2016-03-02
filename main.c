#include <stdint.h>

/* ARM standalone endianness test */

uint32_t exec_area[1024] = {
    [1] = 1,
    [1023] = 2,
};

uint32_t stack [1024] = {
    [1] = 1,
    [1023] = 2,
};

uint32_t data = 0x11223344;

void doit() {
    uint32_t *da = &data;
    /* adcseq if endianness same for inst and data (LE/BE32),
     * adceq if not (BE8)
     */
    uint32_t inst = 0x00bfaf00;
    uint32_t *ea = exec_area;
    //uint8_t d = *(uint8_t *)&data;
    asm volatile("ldrb r1,[%2]\n" /* R1 = first (as per endian) byte of data */
#ifdef DO_SETEND_TEST
                 "cmp r1, #0x11\n"

                 /* R3 = data with reversed endianness */
                 "setend le\n"
                 "beq is_be\n"
                 "setend be\n"
                 "is_be: ldr r3,[%2]\n"

                 /* R2 = data with original endianness */
                 "setend le\n"
                 "bne is_le\n"
                 "setend be\n"
#else
                 "ldr r3, [%2]\n"
                 "rev r3, r3\n"
#endif
                 "is_le: ldr r2, [%2]\n"

                 "str %0, [%1]\n"
                 "ldr r0, [pc, #8]\n"
                 "str r0, [%1, #4]\n"
                 "movs r0, #1\n"
                 "mov pc, %1\n"
                 "sub pc, #8\n"
                 :: "r" (inst), "r"(ea), "r"(da) : "r0", "r1", "r2", "r3");
    for (;;);
}

__attribute__((naked)) void _start() {
    uint32_t *sp = &stack[512];
    void *f = doit;
    asm volatile ("mov sp, %0" :: "r" (sp) :);
    asm volatile ("mov pc, %0" :: "r" (f) :);
}
