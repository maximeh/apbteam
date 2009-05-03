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
#include "modules/twi/twi.h"
#include "io.h"

#include "misc.h"
#include "state.h"

#include "pwm.h"
#include "pos.h"
#include "speed.h"
#include "postrack.h"
#include "traj.h"
#include "aux.h"

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
    /* Handle incoming command. */
    while (twi_sl_poll (buf, sizeof (buf)))
	twi_proto_callback (buf, sizeof (buf));
    /* Update status. */
    u8 status[14];
    status[0] = 0
	| (state_aux[1].blocked << 7)
	| (state_aux[1].finished << 6)
	| (state_aux[0].blocked << 5)
	| (state_aux[0].finished << 4)
	| (speed_theta.cur < 0 ? (1 << 3) : 0)
	| (speed_theta.cur > 0 ? (1 << 2) : 0)
	| (state_main.blocked << 1)
	| (state_main.finished << 0);
    status[1] = twi_proto.seq;
    status[2] = v32_to_v8 (postrack_x, 3);
    status[3] = v32_to_v8 (postrack_x, 2);
    status[4] = v32_to_v8 (postrack_x, 1);
    status[5] = v32_to_v8 (postrack_y, 3);
    status[6] = v32_to_v8 (postrack_y, 2);
    status[7] = v32_to_v8 (postrack_y, 1);
    status[8] = v32_to_v8 (postrack_a, 2);
    status[9] = v32_to_v8 (postrack_a, 1);
    status[10] = v16_to_v8 (aux[0].pos, 1);
    status[11] = v16_to_v8 (aux[0].pos, 0);
    status[12] = v16_to_v8 (aux[1].pos, 1);
    status[13] = v16_to_v8 (aux[1].pos, 0);
    twi_sl_update (status, sizeof (status));
}

/** Handle one command. */
static void
twi_proto_callback (u8 *buf, u8 size)
{
    if (buf[0] == twi_proto.seq)
	return;
#define c(cmd, size) (cmd)
    switch (c (buf[1], 0))
      {
      case c ('z', 0):
	/* Reset. */
	utils_reset ();
	break;
      case c ('w', 0):
	/* Set zero pwm. */
	pos_reset (&pos_theta);
	pos_reset (&pos_alpha);
	state_main.mode = MODE_PWM;
	pwm_set (&pwm_left, 0);
	pwm_set (&pwm_right, 0);
	break;
      case c ('s', 0):
	/* Stop (set zero speed). */
	state_main.mode = MODE_SPEED;
	state_main.variant = 0;
	speed_theta.use_pos = speed_alpha.use_pos = 0;
	speed_theta.cons = 0;
	speed_alpha.cons = 0;
	break;
      case c ('l', 3):
	/* Set linear speed controlled position consign.
	 * - 3b: theta consign offset. */
	speed_theta.use_pos = speed_alpha.use_pos = 1;
	speed_theta.pos_cons = pos_theta.cons;
	if (buf[2] & 0x80)
	    speed_theta.pos_cons += v8_to_v32 (0xff, buf[2], buf[3], buf[4]);
	else
	    speed_theta.pos_cons += v8_to_v32 (0, buf[2], buf[3], buf[4]);
	speed_alpha.pos_cons = pos_alpha.cons;
	state_start (&state_main, MODE_SPEED, 0);
	break;
      case c ('a', 2):
	/* Set angular speed controlled position consign.
	 * - w: angle offset. */
	traj_angle_offset_start (((int32_t) (int16_t) v8_to_v16 (buf[2], buf[3])) << 8, 0);
	break;
      case c ('f', 0):
	/* Go to the wall. */
	traj_ftw_start (0);
	break;
      case c ('F', 0):
	/* Go to the dispenser. */
	traj_gtd_start (0);
	break;
      case c ('x', 6):
	/* Go to position.
	 * - 3b: x position.
	 * - 3b: y position. */
	traj_goto_start (v8_to_v32 (buf[2], buf[3], buf[4], 0),
			 v8_to_v32 (buf[5], buf[6], buf[7], 0),
			 0, 0);
	break;
      case c ('r', 6):
	/* Go to position, backward allowed.
	 * - 3b: x position.
	 * - 3b: y position. */
	traj_goto_start (v8_to_v32 (buf[2], buf[3], buf[4], 0),
			 v8_to_v32 (buf[5], buf[6], buf[7], 0),
			 1, 0);
	break;
      case c ('y', 2):
	/* Go to angle.
	 * - w: angle. */
	traj_goto_angle_start (v8_to_v32 (0, buf[2], buf[3], 0), 0);
	break;
      case c ('X', 8):
	/* Go to position, then angle.
	 * - 3b: x position.
	 * - 3b: y position.
	 * - w: angle. */
	traj_goto_xya_start (v8_to_v32 (buf[2], buf[3], buf[4], 0),
			     v8_to_v32 (buf[5], buf[6], buf[7], 0),
			     v8_to_v32 (0, buf[8], buf[9], 0),
			     0, 0);
	break;
      case c ('R', 8):
	/* Go to position, then angle, backward allowed.
	 * - 3b: x position.
	 * - 3b: y position.
	 * - w: angle. */
	traj_goto_xya_start (v8_to_v32 (buf[2], buf[3], buf[4], 0),
			     v8_to_v32 (buf[5], buf[6], buf[7], 0),
			     v8_to_v32 (0, buf[8], buf[9], 0),
			     1, 0);
	break;
      case c ('b', 3):
	/* Move the aux0.
	 * - w: new position.
	 * - b: speed. */
	speed_aux[0].max = buf[4];
	aux_traj_goto_start (&aux[0], v8_to_v16 (buf[2], buf[3]), 0);
	break;
      case c ('B', 1):
	/* Find the zero position of the arm.
	 * - b: speed. */
	speed_aux[0].max = buf[2];
	aux_traj_find_zero_start (&aux[0], 0);
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
	speed_aux[1].max = buf[2];
	aux_traj_find_zero_start (&aux[1], 0);
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
	  case 'X':
	    /* Set current X position.
	     * - 3b: X position. */
	    if (size < 3)
		return 1;
	    postrack_x = v8_to_v32 (buf[0], buf[1], buf[2], 0);
	    eat = 3;
	    break;
	  case 'Y':
	    /* Set current Y position.
	     * - 3b: Y position. */
	    if (size < 3)
		return 1;
	    postrack_y = v8_to_v32 (buf[0], buf[1], buf[2], 0);
	    eat = 3;
	    break;
	  case 'A':
	    /* Set current angle.
	     * - w: angle. */
	    if (size < 2)
		return 1;
	    postrack_a = v8_to_v32 (0, buf[0], buf[1], 0);
	    eat = 2;
	    break;
	  case 's':
	    /* Set maximum and slow speed.
	     * - b: theta max.
	     * - b: alpha max.
	     * - b: theta slow.
	     * - b: alpha slow. */
	    if (size < 4)
		return 1;
	    speed_theta.max = buf[0];
	    speed_alpha.max = buf[1];
	    speed_theta.slow = buf[2];
	    speed_alpha.slow = buf[3];
	    eat = 4;
	    break;
	  default:
	    return 1;
	  }
	buf += eat;
	size -= eat;
      }
    return 0;
}

