#ifndef avrconfig_h
#define avrconfig_h
/* avrconfig.h */
/* avr.proto - Protocol AVR module. {{{
 *
 * Copyright (C) 2005 Nicolas Schodet
 *
 * Robot APB Team/Efrei 2006.
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

/* proto - Protocol module. */
/** Maximum argument size. */
#define AC_PROTO_ARGS_MAX_SIZE 8
/** Callback function name. */
#define AC_PROTO_CALLBACK proto_callback
/** Putchar function name. */
#define AC_PROTO_PUTC uart0_putc
/** Support for quote parameter. */
#define AC_PROTO_QUOTE 1

#endif /* avrconfig_h */
