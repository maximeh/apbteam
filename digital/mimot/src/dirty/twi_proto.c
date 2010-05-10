/* twi_proto.c - Implement the protocol over TWI. */
/* asserv - Position & speed motor control on AVR. {{{
 *
 * Copyright (C) 2008 Nicolas Schodet
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
#include "common.h"
#include "twi_proto.h"

#include "modules/utils/utils.h"
#include "modules/utils/byte.h"
#include "modules/utils/crc.h"
#include "modules/twi/twi.h"
#include "io.h"

#include "state.h"

#include "pwm.h"
#include "pos.h"
#include "speed.h"
#include "aux.h"

#ifdef HOST
# include "simu.host.h"
#endif

struct twi_proto_t
{
    u8 seq;
};

struct twi_proto_t twi_proto;

static void
twi_proto_callback (u8 *buf, u8 size);

static u8
twi_proto_params (u8 *buf, u8 size);

/** Initialise. */
void
twi_proto_init (void)
{
    twi_init (AC_ASSERV_TWI_ADDRESS);
    twi_proto_update ();
}

/** Handle received commands and update status. */
void
twi_proto_update (void)
{
    u8 buf[AC_TWI_SL_RECV_BUFFER_SIZE];
    u8 read_data;
    /* Handle incoming command. */
    while ((read_data = twi_sl_poll (buf, sizeof (buf))))
	twi_proto_callback (buf, read_data);
    /* Update status. */
    u8 status_with_crc[8];
    u8 *status = &status_with_crc[1];
    status[0] = 0
	| (state_aux[1].blocked << 3)
	| (state_aux[1].finished << 2)
	| (state_aux[0].blocked << 1)
	| (state_aux[0].finished << 0);
    status[1] = PINC;
    status[2] = twi_proto.seq;
    status[3] = v16_to_v8 (aux[0].pos, 1);
    status[4] = v16_to_v8 (aux[0].pos, 0);
    status[5] = v16_to_v8 (aux[1].pos, 1);
    status[6] = v16_to_v8 (aux[1].pos, 0);
    /* Compute CRC. */
    status_with_crc[0] = crc_compute (&status_with_crc[1],
                                      sizeof (status_with_crc) - 1);
    twi_sl_update (status_with_crc, sizeof (status_with_crc));
}

/** Handle one command. */
static void
twi_proto_callback (u8 *buf, u8 size)
{
    /* Check CRC. */
    if (crc_compute (buf + 1, size - 1) != buf[0])
	return;
    else
      {
	/* Remove the CRC of the buffer. */
	buf += 1;
	size -= 1;
      }

    if (buf[0] == twi_proto.seq)
	return;
#define c(cmd, size) (cmd)
    switch (c (buf[1], 0))
      {
      case c ('z', 0):
	/* Reset. */
	utils_reset ();
	break;
      case c ('b', 3):
	/* Move the aux0.
	 * - w: new position.
	 * - b: speed. */
	speed_aux[0].max = buf[4];
	aux_traj_goto_start (&aux[0], v8_to_v16 (buf[2], buf[3]), 0);
	break;
      case c ('B', 1):
	/* Find the zero position of the aux0.
	 * - b: speed. */
	aux_traj_find_limit_start (&aux[0], buf[2], 0);
	break;
      case c ('c', 3):
	/* Move the aux1.
	 * - w: new position.
	 * - b: speed. */
	speed_aux[1].max = buf[4];
	aux_traj_goto_start (&aux[1], v8_to_v16 (buf[2], buf[3]), 0);
	break;
      case c ('C', 1):
	/* Find the zero position of the aux1.
	 * - b: speed. */
	aux_traj_find_limit_start (&aux[1], buf[2], 0);
	break;
      case c ('l', 4):
	/* Clamp.
	 * - b: aux index.
	 * - b: speed.
	 * - w: claming PWM. */
	if (buf[2] < AC_ASSERV_AUX_NB)
	     aux_traj_clamp_start (&aux[buf[2]], buf[3],
				   v8_to_v16 (buf[4], buf[5]), 0);
	else
	    buf[0] = 0;
	break;
      case c ('p', x):
	/* Set parameters. */
	if (twi_proto_params (&buf[2], size - 2) != 0)
	    buf[0] = 0;
	break;
      default:
	buf[0] = 0;
	break;
      }
    /* Acknowledge. */
    twi_proto.seq = buf[0];
}

/* Handle a parameter list of change. */
static u8
twi_proto_params (u8 *buf, u8 size)
{
    u8 eat;
    while (*buf && size)
      {
	size--;
	switch (*buf++)
	  {
	  default:
	    return 1;
	  }
	buf += eat;
	size -= eat;
      }
    return 0;
}

