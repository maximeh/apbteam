#ifndef simu_host_h
#define simu_host_h
/* simu.host.h - Host simulation. */
/* robospierre - Eurobot 2011 AI. {{{
 *
 * Copyright (C) 2011 Nicolas Schodet
 *
 * APBTeam:
 *        Web: http://apbteam.org/
 *      Email: team AT apbteam DOT org
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
#include "defs.h"

#ifdef HOST

extern uint8_t PORTA, DDRA, PINA, PINE, PINF;

/** Send general purpose positions to indicate computation results.
 * - pos: array of positions to report.
 * - pos_nb: number of elements in the array.
 * - id: identifier so that several unrelated positions could be reported. */
void
simu_send_pos_report (vect_t *pos, uint8_t pos_nb, uint8_t id);

#else /* !defined (HOST) */

#define simu_send_pos_report(pos, pos_nb, id) ((void) 0)

#endif /* !defined (HOST) */

#endif /* simu_host_h */
