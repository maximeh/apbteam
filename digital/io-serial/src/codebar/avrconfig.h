#ifndef avrconfig_h
#define avrconfig_h
/* avrconfig.h - Codebar configuration template. */
/* io-serial.codebar - Codebar AVR module. {{{
 *
 * Copyright (C) 2011 Nicolas Schodet
 *
 * Robot APB Team/Efrei 2011.
 *        Web: http://assos.efrei.fr/robot/
 *      Email: robot AT efrei DOT fr
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * }}} */

/* global */                                                                                                                                 
/** AVR Frequency : 1000000, 1843200, 2000000, 3686400, 4000000, 7372800,
 * 8000000, 11059200, 14745600, 16000000, 18432000, 20000000. */
#define AC_FREQ 14745600

/* uart - UART module. */
/** Select hardware uart for primary uart: 0, 1 or -1 to disable. */
#define AC_UART0_PORT 0
/** Baudrate: 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 76800,
 * 115200, 230400, 250000, 500000, 1000000. */
#define AC_UART0_BAUDRATE 9600
/** Send mode:
 *  - POLLING: no interrupts.
 *  - RING: interrupts, ring buffer. */
#define AC_UART0_SEND_MODE RING
/** Recv mode, same as send mode. */
#define AC_UART0_RECV_MODE RING
/** Character size: 5, 6, 7, 8, 9 (only 8 implemented). */
#define AC_UART0_CHAR_SIZE 8
/** Parity : ODD, EVEN, NONE. */
#define AC_UART0_PARITY NONE
/** Stop bits : 1, 2. */
#define AC_UART0_STOP_BITS 1
/** Send buffer size, should be power of 2 for RING mode. */
#define AC_UART0_SEND_BUFFER_SIZE 32
/** Recv buffer size, should be power of 2 for RING mode. */
#define AC_UART0_RECV_BUFFER_SIZE 32
/** If the send buffer is full when putc:
 *  - DROP: drop the new byte.
 *  - WAIT: wait until there is room in the send buffer. */
#define AC_UART0_SEND_BUFFER_FULL WAIT
/** In HOST compilation:
 *  - STDIO: use stdin/out.
 *  - PTS: use pseudo terminal. */
#define AC_UART0_HOST_DRIVER PTS
/** Same thing for secondary port. */
#define AC_UART1_PORT 1
#define AC_UART1_BAUDRATE 9600
#define AC_UART1_SEND_MODE RING
#define AC_UART1_RECV_MODE RING
#define AC_UART1_CHAR_SIZE 8
#define AC_UART1_PARITY NONE
#define AC_UART1_STOP_BITS 1
#define AC_UART1_SEND_BUFFER_SIZE 32
#define AC_UART1_RECV_BUFFER_SIZE 32
#define AC_UART1_SEND_BUFFER_FULL WAIT
#define AC_UART1_HOST_DRIVER PTS

/* twi - TWI module. */
/** Driver to implement TWI: HARD, SOFT, or USI. */
#define AC_TWI_DRIVER HARD
/** Do not use interrupts. */
#define AC_TWI_NO_INTERRUPT 0
/** TWI frequency, should really be 100 kHz. */
#define AC_TWI_FREQ 100000
/** Enable slave part. */
#define AC_TWI_SLAVE_ENABLE 1
/** Enable master part. */
#define AC_TWI_MASTER_ENABLE 0
/** Use polled slave mode: received data is stored in a buffer which can be
 * polled using twi_slave_poll. */
#define AC_TWI_SLAVE_POLLED 1
/** Slave reception callback to be defined by the user when not in polled
 * mode. */
#undef AC_TWI_SLAVE_RECV
/** Use internal pull up. */
#define AC_TWI_PULL_UP 0
/** Slave reception buffer size. */
#define AC_TWI_SLAVE_RECV_BUFFER_SIZE 16
/** Slave transmission buffer size. */
#define AC_TWI_SLAVE_SEND_BUFFER_SIZE 16

#endif /* avrconfig_h */
