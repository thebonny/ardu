 /**
 * \file
 *
 * \brief PIO Alternate Function Example.
 *
 * Copyright (c) 2011-2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

/**
 *  \mainpage PIO Alternate Function Example
 *
 *  \par Purpose
 *
 *  The PIO Alternate Function demonstrates how to alternate ERASE pin between system
 *  I/O mode and PIO mode.
 *
 *  \par Requirements
 *
 *  This package can be used with SAM EKs.
 *
 *  \par Description
 *
 *  The application shows the alternate function of ERASE pin, which extends PIO
 *  from a dedicated system I/O line. It first programs the internal flash with
 *  a walking pattern and then enables ERASE pin in PIO mode. After the erase
 *  operation, it checks the content of flash, which is unchanged due to ERASE in
 *  PIO mode. In the next step, it puts ERASE pin in system I/O mode, which will
 *  enable the erase function. The content of flash should be erased to 0xFF, which
 *  indicates the ERASE pin functioning.
 *
 *  The pins which are used by USB(DDM,DDP), JTAG(TCK,TMS,TDO,TDI) and Oscillator(XIN,
 *  XOUT,XIN32,XOUT32) could also be alternated between general purpose I/O and system
 *  I/O in the similar way.
 *
 *  \par Usage
 *
 *  -# Build the program and download it into the evaluation board.
 *  -# On the computer, open and configure a terminal application
 *     (e.g., HyperTerminal on Microsoft Windows) with these settings:
 *    - 115200 bauds
 *    - 8 bits of data
 *    - No parity
 *    - 1 stop bit
 *    - No flow control
 *  -# Start application.
 *  -# In the terminal window, the
 *     following text should appear (values depend on the board and the chip used):
 *     \code
	-- PIO Alternate Function Example --
	-- xxxxxx-xx --
	-- Compiled: xxx xx xxxx xx:xx:xx --
\endcode
 *  -# Perform erase operation following the application prompt.
 *
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#include "asf.h"
#include "conf_board.h"
#include "conf_clock.h"
#include "conf_example.h"
#include <math.h>
#include <string.h>
// For printf
#include <stdio.h>

/** Buffer size. */
#define BUFFER_SIZE            (IFLASH_PAGE_SIZE / 4)


#define STRING_EOL    "\r"
#define STRING_HEADER "Test double to ascii conversion"

static double PRECISION = 0.00000000000001;
static int MAX_NUMBER_STRING_SIZE = 32;

/**
 * Double to ASCII prototype. Must be declared before usage in main function !!
 */
char * tdtoa(char *s, double n);

/**
 *  Configure UART for debug message output.
 */
static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
#ifdef CONF_UART_CHAR_LENGTH
		.charlength = CONF_UART_CHAR_LENGTH,
#endif
		.paritytype = CONF_UART_PARITY,
#ifdef CONF_UART_STOP_BITS
		.stopbits = CONF_UART_STOP_BITS,
#endif
	};

	/* Configure console UART. */
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
}

/**
 *  \brief pio_alternate_function Application entry point.
 *
 *  Program the test page of internal flash with pattern
 *  0x00000001,0x00000002,...,0x80000000. If the ERASE is in
 *  PIO mode, putting jumper on ERASE has no effect.
 *  That is, this pin could be used as general purpose I/O
 *  line. Otherwise, the content of flash will be erased to
 *  0xFFFFFFFF.
 *
 *  \return Unused (ANSI-C compatibility).
 *
 */

int main(void)
{
	
	
	/* Initialize the system. */
	sysclk_init();
	board_init();

	/* Configure UART for debug message output. */
	configure_console();

	

	/* Output example information. */
	puts(STRING_HEADER);
	
	 int i;
	 char s[MAX_NUMBER_STRING_SIZE];
	 double d[] = {
		 0.0,
		 42.0,
		 1234567.89012345,
		 0.000000000000018,
		 555555.55555555555555555,
		 -888888888888888.8888888,
		 111111111111111111111111.2222222222
	 };
	 for (i = 0; i < 7; i++) {
		 printf("%d: printf: %.14g, dtoa: %s\n", i+1, d[i], tdtoa(s, d[i]));
	 }


	while (1) {
	}
}

/**
 * Double to ASCII
 */
char * tdtoa(char *s, double n) {
    // handle special cases
    if (isnan(n)) {
        strcpy(s, "nan");
    } else if (isinf(n)) {
        strcpy(s, "inf");
    } else if (n == 0.0) {
        strcpy(s, "0");
    } else {
        int digit, m, m1;
        char *c = s;
        int neg = (n < 0);
        if (neg)
            n = -n;
        // calculate magnitude
        m = log10(n);
        int useExp = (m >= 14 || (neg && m >= 9) || m <= -9);
        if (neg)
            *(c++) = '-';
        // set up for scientific notation
        if (useExp) {
            if (m < 0)
               m -= 1.0;
            n = n / pow(10.0, m);
            m1 = m;
            m = 0;
        }
        if (m < 1.0) {
            m = 0;
        }
        // convert the number
        while (n > PRECISION || m >= 0) {
            double weight = pow(10.0, m);
            if (weight > 0 && !isinf(weight)) {
                digit = floor(n / weight);
                n -= (digit * weight);
                *(c++) = '0' + digit;
            }
            if (m == 0 && n > 0)
                *(c++) = '.';
            m--;
        }
        if (useExp) {
            // convert the exponent
            int i, j;
            *(c++) = 'e';
            if (m1 > 0) {
                *(c++) = '+';
            } else {
                *(c++) = '-';
                m1 = -m1;
            }
            m = 0;
            while (m1 > 0) {
                *(c++) = '0' + m1 % 10;
                m1 /= 10;
                m++;
            }
            c -= m;
            for (i = 0, j = m-1; i<j; i++, j--) {
                // swap without temporary
                c[i] ^= c[j];
                c[j] ^= c[i];
                c[i] ^= c[j];
            }
            c += m;
        }
        *(c) = '\0';
    }
    return s;
}
