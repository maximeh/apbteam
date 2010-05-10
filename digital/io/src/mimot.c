/* mimot.c */
/* io - Input & Output with Artificial Intelligence (ai) support on AVR. {{{
 *
 * Copyright (C) 2010 Nicolas Schodet
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
#include "mimot.h"

#include "twi_master.h"

#include "modules/utils/byte.h"
#include "io.h"

/** Flag bit positions in mimot status byte. */
enum mimot_status_flag_t
{
    /** Motor0 movement finished with success. */
    mimot_status_flag_motor0_succeed = 0,
    /** Motor0 movement finished with failure. */
    mimot_status_flag_motor0_failed = 1,
    /** Motor1 movement finished with success. */
    mimot_status_flag_motor1_succeed = 2,
    /** Motor1 movement finished with failure. */
    mimot_status_flag_motor1_failed = 3,
};

/** Status structure. */
struct mimot_status_t
{
    /** Status flags. */
    uint8_t status;
    /** Mimot input port. */
    uint8_t input_port;
    /** Motor0 position. */
    uint16_t motor0_position;
    /** Motor1 position. */
    uint16_t motor1_position;
};

/** Current mimot status. */
struct mimot_status_t mimot_status;

void
mimot_init (void)
{
    /* Nothing to do. */
}

void
mimot_status_cb (uint8_t *status)
{
    /* Parse received data and store them. */
    mimot_status.status = status[0];
    mimot_status.input_port = status[1];
    mimot_status.motor0_position = v8_to_v16 (status[3], status[4]);
    mimot_status.motor1_position = v8_to_v16 (status[5], status[6]);
}

asserv_status_e
mimot_motor0_cmd_status (void)
{
    if (mimot_status.status & _BV (mimot_status_flag_motor0_succeed))
	return success;
    else if (mimot_status.status & _BV (mimot_status_flag_motor0_failed))
	return failure;
    else
	return none;
}

asserv_status_e
mimot_motor1_cmd_status (void)
{
    if (mimot_status.status & _BV (mimot_status_flag_motor1_succeed))
	return success;
    else if (mimot_status.status & _BV (mimot_status_flag_motor1_failed))
	return failure;
    else
	return none;
}

uint16_t
mimot_get_motor0_position (void)
{
    return mimot_status.motor0_position;
}

uint16_t
mimot_get_motor1_position (void)
{
    return mimot_status.motor1_position;
}

void
mimot_reset (void)
{
    uint8_t *buffer = twi_master_get_buffer (MIMOT_SLAVE);
    buffer[0] = 'z';
    twi_master_send (1);
}

void
mimot_move_motor0_absolute (uint16_t position, uint8_t speed)
{
    uint8_t *buffer = twi_master_get_buffer (MIMOT_SLAVE);
    buffer[0] = 'b';
    buffer[1] = v16_to_v8 (position, 1);
    buffer[2] = v16_to_v8 (position, 0);
    buffer[3] = speed;
    twi_master_send (4);
}

void
mimot_move_motor1_absolute (uint16_t position, uint8_t speed)
{
    uint8_t *buffer = twi_master_get_buffer (MIMOT_SLAVE);
    buffer[0] = 'c';
    buffer[1] = v16_to_v8 (position, 1);
    buffer[2] = v16_to_v8 (position, 0);
    buffer[3] = speed;
    twi_master_send (4);
}

void
mimot_motor0_zero_position (int8_t speed)
{
    uint8_t *buffer = twi_master_get_buffer (MIMOT_SLAVE);
    buffer[0] = 'B';
    buffer[1] = speed;
    twi_master_send (2);
}

void
mimot_motor1_zero_position (int8_t speed)
{
    uint8_t *buffer = twi_master_get_buffer (MIMOT_SLAVE);
    buffer[0] = 'C';
    buffer[1] = speed;
    twi_master_send (2);
}

void
mimot_motor0_clamp (int8_t speed, int16_t pwm)
{
    uint8_t *buffer = twi_master_get_buffer (MIMOT_SLAVE);
    buffer[0] = 'l';
    buffer[1] = 0;
    buffer[2] = speed;
    buffer[3] = v16_to_v8 (pwm, 1);
    buffer[4] = v16_to_v8 (pwm, 0);
    twi_master_send (5);
}

void
mimot_motor1_clamp (int8_t speed, int16_t pwm)
{
    uint8_t *buffer = twi_master_get_buffer (MIMOT_SLAVE);
    buffer[0] = 'l';
    buffer[1] = 1;
    buffer[2] = speed;
    buffer[3] = v16_to_v8 (pwm, 1);
    buffer[4] = v16_to_v8 (pwm, 0);
    twi_master_send (5);
}
