#include <kernel/Bios.h>
#include <kernel/memory/MemLayout.h>
#include <lib/Color.h>
#include <lib/libc/printf.h>
#include <kernel/KernelSymbols.h>
#include "BlueScreen.h"

void BlueScreen::initialize() {
    BC_params->AX = 0x03;
    Bios::Int(0x10);

    BC_params->AX = 0x0100;
    BC_params->CX = 0x2607;
    Bios::Int(0x10);

    auto *dest = (uint64_t *) VIRT_CGA_START;

    uint64_t end = 80 * 25 * 2 / sizeof(uint64_t);

    for(uint64_t i = 0; i < end; i++) {
        dest[i] = 0x1000100010001000;
    }

    stdout = this;
}

void BlueScreen::print(InterruptFrame &frame) {

    printf("\n\n  [PANIC] %s\n\n", Cpu::getExceptionName(frame.interrupt));

    uint32_t *ebp = (uint32_t*) frame.ebp;

    uint32_t eip = frame.eip;

    uint32_t i = 0;

    while (eip) {

        printf("     #%02d 0x%08x --- %s\n", i, eip, KernelSymbols::get(eip));

        eip = ebp[1];

        ebp = (uint32_t*) ebp[0];

        if ((uint32_t ) ebp < KERNEL_START) {

            break;
        }

        i++;
    }

    printf("\n\n");
    printf("     eax=0x%08x  ebx=0x%08x  ecx=0x%08x  edx=0x%08x\n", frame.eax, frame.ebx, frame.ecx, frame.edx);
    printf("     esp=0x%08x  ebp=0x%08x  esi=0x%08x  edi=0x%08x\n\n", frame.esp, frame.ebp, frame.esi, frame.edi);
    printf("     eflags=0x%08x", frame.eflags);
}

void BlueScreen::puts(const char *s, uint32_t n) {
    for(uint32_t i = 0; i < n; i++) {
        putc(s[i]);
    }
}

void BlueScreen::putc(const char c) {

    if(y >= ROWS) {
        return;
    }

    if(c == '\n') {
        x = 0;
        y++;
    } else {
        show(x, y, c);
        x++;
    }

    if(x >= COLUMNS) {
        x = 0;
        y++;
    }
}

void BlueScreen::show(uint16_t x, uint16_t y, const char c) {

    if (x < 0 || x >= COLUMNS || y < 0 || y > ROWS)
        return;

    uint16_t pos = (y * COLUMNS + x) * (uint16_t) 2;

    *((uint8_t *) (CGA_START + pos)) = static_cast<uint8_t>(c);
    *((uint8_t *) (CGA_START + pos + 1)) = ATTRIBUTE;
}

void BlueScreen::flush() {
    puts(StringBuffer::buffer, StringBuffer::pos);
    pos = 0;
}

