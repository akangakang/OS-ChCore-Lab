/*
   MIT License

   Copyright (c) 2018 Sergey Matyukevich

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

/*
 * ChCore refers to
 * https://github.com/s-matyukevich/raspberry-pi-os/blob/master/docs/lesson01/rpi-os.md
 * for the min-uart init process.
 */

#pragma once

#include <common/vars.h>

/* This peripheral mapped offset is specific to BCM2837 */
#define PHYSADDR_OFFSET     (KBASE + 0x3F000000UL)

/* BCM2835 and BCM2837 define the same offsets */
#define GPFSEL1            (PHYSADDR_OFFSET + 0x00200004)
#define GPSET0             (PHYSADDR_OFFSET + 0x0020001C)
#define GPCLR0             (PHYSADDR_OFFSET + 0x00200028)
#define GPPUD              (PHYSADDR_OFFSET + 0x00200094)
#define GPPUDCLK0          (PHYSADDR_OFFSET + 0x00200098)

#define AUX_ENABLES        (PHYSADDR_OFFSET + 0x00215004)
#define AUX_MU_IO_REG      (PHYSADDR_OFFSET + 0x00215040)
#define AUX_MU_IER_REG     (PHYSADDR_OFFSET + 0x00215044)
#define AUX_MU_IIR_REG     (PHYSADDR_OFFSET + 0x00215048)
#define AUX_MU_LCR_REG     (PHYSADDR_OFFSET + 0x0021504C)
#define AUX_MU_MCR_REG     (PHYSADDR_OFFSET + 0x00215050)
#define AUX_MU_LSR_REG     (PHYSADDR_OFFSET + 0x00215054)
#define AUX_MU_MSR_REG     (PHYSADDR_OFFSET + 0x00215058)
#define AUX_MU_SCRATCH     (PHYSADDR_OFFSET + 0x0021505C)
#define AUX_MU_CNTL_REG    (PHYSADDR_OFFSET + 0x00215060)
#define AUX_MU_STAT_REG    (PHYSADDR_OFFSET + 0x00215064)
#define AUX_MU_BAUD_REG    (PHYSADDR_OFFSET + 0x00215068)
