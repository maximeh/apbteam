/* simu.host.c - Host simulation. */
/* io - Input & Output with Artificial Intelligence (ai) support on AVR. {{{
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
#include "simu.host.h"

#include "servo.h"

#include "modules/utils/utils.h"
#include "modules/host/host.h"
#include "modules/host/mex.h"

enum
{
    MSG_SIMU_IO_JACK = 0xb0,
    MSG_SIMU_IO_COLOR = 0xb1,
    MSG_SIMU_IO_SERVO = 0xb2,
};

/** Requested servo position. */
uint8_t servo_high_time_[SERVO_NUMBER];

/** Current servo position. */
uint8_t servo_high_time_current_[SERVO_NUMBER];

/** Servo speed is about 120 ms for 60 degrees.  This means about 360 ms for
 * the full swing. */
#define SERVO_SPEED (int) ((256 / (360.0 / 4.4444)) + 0.5)

/** Initialise simulation. */
void
simu_init (void)
{
    mex_node_connect ();
}

/** Make a simulation step. */
void
simu_step (void)
{
    int i;
    mex_msg_t *m;
    /* Update servos. */
    for (i = 0; i < SERVO_NUMBER; i++)
      {
	if (UTILS_ABS (servo_high_time_current_[i] - servo_high_time_[i]) <
	    SERVO_SPEED)
	    servo_high_time_current_[i] = servo_high_time_[i];
	else if (servo_high_time_current_[i] < servo_high_time_[i])
	    servo_high_time_current_[i] += SERVO_SPEED;
	else
	    servo_high_time_current_[i] -= SERVO_SPEED;
      }
    /* Send servos. */
    m = mex_msg_new (MSG_SIMU_IO_SERVO);
    for (i = 0; i < SERVO_NUMBER; i++)
	mex_msg_push (m, "B", servo_high_time_current_[i]);
    mex_node_send (m);
}

void
servo_init (void)
{
}

void
servo_set_high_time (uint8_t servo, uint8_t high_time)
{
    servo_high_time_[servo] = high_time;
}

uint8_t
servo_get_high_time (uint8_t servo)
{
    return servo_high_time_[servo];
}

void
switch_init (void)
{
}

uint8_t
switch_get_color (void)
{
    mex_msg_t *m = mex_msg_new (MSG_SIMU_IO_COLOR);
    m = mex_node_request (m);
    uint8_t r;
    mex_msg_pop (m, "B", &r);
    mex_msg_delete (m);
    return r;
}

uint8_t
switch_get_jack (void)
{
    mex_msg_t *m = mex_msg_new (MSG_SIMU_IO_JACK);
    m = mex_node_request (m);
    uint8_t r;
    mex_msg_pop (m, "B", &r);
    mex_msg_delete (m);
    return r;
}

void
switch_update (void)
{
}

void
main_timer_init (void)
{
    simu_init ();
}

void
main_timer_wait (void)
{
    mex_node_wait_date (mex_node_date () + 4);
    simu_step ();
}

void
eeprom_load_param (void)
{
}

void
eeprom_save_param (void)
{
}

void
eeprom_clear_param (void)
{
}

void
chrono_init (void)
{
}

