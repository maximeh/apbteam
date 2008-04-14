/* asserv.c */
/* io - Input & Output with Artificial Intelligence (ai) support on AVR. {{{
 *
 * Copyright (C) 2008 Dufour J�r�my
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
#include "asserv.h"

#include "modules/twi/twi.h"	/* twi_* */
#include "modules/utils/byte.h"	/* v*_to_v* */
#include "giboulee.h"		/* BOT_* */
#include "io.h"

/**
 * @defgroup AsservPrivate Asserv module private variables and functions
 * declarations and definitions
 * @{
 */

/**
 * Sequence number.
 * It is used for the acknowledge of the command sent to the asserv.
 */
static uint8_t asserv_twi_seq;

/**
 * Shared buffer used to send commands to the asserv.
 */
static uint8_t asserv_twi_buffer[7];

/**
 * Pointer to access the buffer to put the parameters list.
 */
static uint8_t *asserv_twi_buffer_param = &asserv_twi_buffer[2];

/**
 * Status structure maintains by the update command.
 */
typedef struct asserv_struct_s
{
    /** Status flags. */
    uint8_t status;
    /** Sequence number. */
    uint8_t seq;
    /** Bot position. */
    asserv_position_t position;
    /** Arm position. */
    uint16_t arm_position;
} asserv_struct_s;

/**
 * Status variable.
 */
asserv_struct_s asserv_status;

/**
 * Retransmit counter initial value.
 */
#define ASSERV_RETRANSMIT_COUNT 10

/**
 * Retransmit counter.
 */
static uint8_t asserv_retransmit_counter;

/**
 * Length of the last transmitted command.
 */
static uint8_t asserv_retransmit_length;

/**
 * Update TWI module until request (send or receive) is finished.
 * This functions is blocking.
 */
static inline void
asserv_twi_update (void);

/**
 * Send a command to the asserv board using the TWI module.
 * If the command you want to send have some parameters, you need to fill
 * asserv_twi_buffer_param buffer.
 * @param command the command to send.
 * @param length the length of the parameters list (in byte).
 * @return
 *  - 0 when there is no error
 *  - 1 if there is already a command running on the asserv (not acknowledged
 *  or not finished yet).
 */
static inline uint8_t
asserv_twi_send_command (uint8_t command, uint8_t length);

/**
 * Send a prepared command to the asserv board using TWI module.
 * It will reset the counter retransmission value and store the length for
 * future retransmission.
 * In comparison with \a asserv_twi_send_command this function is internal and
 * used by \a asserv_twi_send_command.
 * @param length th length of the complete command with parameters.
 * @return
 *   - 0 if no error occurred.
 *   - 1 if TWI transmission failed.
 */
static inline uint8_t
asserv_twi_send (uint8_t length);
/**
 * Move the arm.
 * A complete rotation correspond to 5000 steps.
 * @param position desired goal position (in step).
 * @param speed speed of the movement.
 */
void
asserv_move_arm_absolute (uint16_t position, uint8_t speed);

/**
 * Current position of the arm.
 * We need to maintain it by ourself as it is more accurate than the one sent
 * by the asserv board.
 */
static uint16_t asserv_arm_current_position;

/* Update TWI module until request (send or receive) is finished. */
static inline void
asserv_twi_update (void)
{
    while (!twi_ms_is_finished ())
	;
}

/* Send a command to the asserv board using the TWI module. */
static inline uint8_t
asserv_twi_send_command (uint8_t command, uint8_t length)
{
    /* Check we are not doing a command ? */
    if (!asserv_last_cmd_ack ())
	return 1;

    /* Put the command into the buffer */
    asserv_twi_buffer[0] = command;
    /* Put the sequence number */
    asserv_twi_buffer[1] = ++asserv_twi_seq;

    /* Send the prepared command */
    return asserv_twi_send (length + 2);
}

/* Send a prepared command to the asserv board using TWI module. */
static inline uint8_t
asserv_twi_send (uint8_t length)
{
    /* Send command to the asserv */
    if (twi_ms_send (AC_ASSERV_TWI_ADDRESS, asserv_twi_buffer, length) != 0)
	return 1;

    /* Update until the command is sent */
    asserv_twi_update ();
    
    /* Reset retransmit counter */
    asserv_retransmit_counter = ASSERV_RETRANSMIT_COUNT;
    /* Store length for retransmission */
    asserv_retransmit_length = length;

    return 0;
}
/** @} */

/* Initialize the asserv control module. */
void
asserv_init (void)
{
    /* Initialize TWI with my (io) address */
    twi_init (AC_IO_TWI_ADDRESS);
    /* We are at first command */
    asserv_twi_seq = asserv_status.seq = 0;
}

/* Update the status of the asserv board seen from the io program. */
void
asserv_update_status (void)
{
    /* Status buffer used to receive data from the asserv */
    static uint8_t status_buffer[AC_ASSERV_STATUS_LENGTH];
    /* Read data from the asserv card */
    twi_ms_read (AC_ASSERV_TWI_ADDRESS, status_buffer , AC_ASSERV_STATUS_LENGTH);
    /* Update until done */
    asserv_twi_update ();
    /* Parse received data and store them */
    asserv_status.status = status_buffer[0];
    asserv_status.seq = status_buffer[1];
    asserv_status.position.x = v8_to_v32 (status_buffer[2], status_buffer[3],
				     status_buffer[4], 0);
    asserv_status.position.y = v8_to_v32 (status_buffer[5], status_buffer[6],
				     status_buffer[7], 0);
    asserv_status.position.a = v8_to_v16 (status_buffer[8], status_buffer[9]);
    asserv_status.arm_position = v8_to_v16 (status_buffer[10], status_buffer[11]);
}

/* Is last command sent to the asserv board is being executed? */
uint8_t
asserv_last_cmd_ack (void)
{
    /* Compare last command sequence number and the one acknowledge by the
     * asserv */
    return (asserv_status.seq == asserv_twi_seq);
}

/**
 * Re-send command if not acknowledged.
 */
uint8_t
asserv_retransmit (void)
{
    /* Check if we have reached the maximum number of check before
     * retransmission */
    if (--asserv_retransmit_counter == 0)
      {
	/* Retransmit! */
	asserv_twi_send (asserv_retransmit_length);
	return 1;
      }
    return 0;
}

/* Is last move class command has successfully ended? */
asserv_status_e
asserv_move_cmd_status (void)
{
    /* Check Motor Finished flag */
    if (asserv_status.status & _BV (0))
	return success;
    /* Check Motor Blocked flag */
    else if (asserv_status.status & _BV (1))
	return failure;
    /* Otherwise, not finished nor failure */
    return none;
}

/* Is last arm class command has successfully ended? */
asserv_status_e
asserv_arm_cmd_status (void)
{
    /* Check Arm Finished flag */
    if (asserv_status.status & _BV (2))
	return success;
    /* Check Arm Blocked flag */
    else if (asserv_status.status & _BV (3))
	return failure;
    /* Otherwise, not finished nor failure */
    return none;
}

/* Get the current position of the bot. */
void
asserv_get_position (asserv_position_t *current_position)
{
    if (current_position)
      {
	/* Copy last received status buffer information to current position */
	current_position->x = asserv_status.position.x;
	current_position->y = asserv_status.position.y;
	current_position->a = asserv_status.position.a;
      }
}

/* Get the arm position. */
uint16_t
asserv_get_arm_position (void)
{
    /* Return the position of the arm of the current status buffer */
    return asserv_status.arm_position;
}

/* Reset the asserv board. */
void
asserv_reset (void)
{
    /* Send the reset command to the asserv board */
    asserv_twi_send_command ('z', 0);
}

/* Free the motors (stop controlling them). */
void
asserv_free_motor (void)
{
    /* Send the free motor command to the asserv board */
    asserv_twi_send_command ('w', 0);
}

/* Stop the motor (and the bot). */
void
asserv_stop_motor (void)
{
    /* Send the stop motor command to the asserv board */
    asserv_twi_send_command ('s', 0);
}

/* Move linearly. */
void
asserv_move_linearly (int32_t distance)
{
    /* Put distance as parameter */
    asserv_twi_buffer_param[0] = v32_to_v8 (distance, 2);
    asserv_twi_buffer_param[1] = v32_to_v8 (distance, 1);
    asserv_twi_buffer_param[2] = v32_to_v8 (distance, 0);
    /* Send the linear move command to the asserv board */
    asserv_twi_send_command ('l', 3);
}

 /* Move angularly (turn). */
void
asserv_move_angularly (int16_t angle)
{
    /* Put angle as parameter */
    asserv_twi_buffer_param[0] = v16_to_v8 (angle, 1);
    asserv_twi_buffer_param[1] = v16_to_v8 (angle, 0);
    /* Send the angular move command to the asserv board */
    asserv_twi_send_command ('a', 2);
}

/* Go to the wall (moving backward). */
void
asserv_go_to_the_wall (void)
{
    /* Send the go the wall command to the asserv board */
    asserv_twi_send_command ('f', 0);
}

/* Move forward to approach a ditributor. */
void
asserv_go_to_distributor (void)
{
    /* Send the go the distributor command to the asserv board */
    asserv_twi_send_command ('F', 0);
}

/* Move the arm. */
void
asserv_move_arm_absolute (uint16_t position, uint8_t speed)
{
    /* Put position and speed as parameters */
    asserv_twi_buffer_param[0] = v16_to_v8 (position, 1);
    asserv_twi_buffer_param[1] = v16_to_v8 (position, 0);
    asserv_twi_buffer_param[2] = speed;
    /* Send the move the arm command to the asserv board */
    asserv_twi_send_command ('b', 3);
}

/* Move the arm to a certain number of steps. */
void
asserv_move_arm (int16_t offset, uint8_t speed)
{
    /* Compute the new desired position with the desired offset */
    asserv_arm_current_position += offset;
    /* Move the arm to the desired position */
    asserv_move_arm_absolute (asserv_arm_current_position, speed);
}

/* Move the arm to close the input hole. */
void
asserv_close_input_hole (void)
{
    /* Move the arm */
    asserv_move_arm (asserv_arm_current_position %
		     BOT_ARM_THIRD_ROUND, BOT_ARM_SPEED);
}

/* Set current X position. */
void
asserv_set_x_position (int32_t x)
{
    /* 'X' subcommand */
    asserv_twi_buffer_param[0] = 'X';
    /* Put x position as parameter */
    asserv_twi_buffer_param[1] = v32_to_v8 (x, 2);
    asserv_twi_buffer_param[2] = v32_to_v8 (x, 1);
    asserv_twi_buffer_param[3] = v32_to_v8 (x, 0);
    /* Send the set X position command to the asserv board */
    asserv_twi_send_command ('p', 4);
}

/* Set current Y position. */
void
asserv_set_y_position (int32_t y)
{
    /* 'Y' subcommand */
    asserv_twi_buffer_param[0] = 'Y';
    /* Put y position as parameter */
    asserv_twi_buffer_param[1] = v32_to_v8 (y, 2);
    asserv_twi_buffer_param[2] = v32_to_v8 (y, 1);
    asserv_twi_buffer_param[3] = v32_to_v8 (y, 0);
    /* Send the set Y position command to the asserv board */
    asserv_twi_send_command ('p', 4);
}

/* Set current angular position. */
void
asserv_set_angle_position (int16_t angle)
{
    /* 'A' subcommand */
    asserv_twi_buffer_param[0] = 'A';
    /* Put angle position as parameter */
    asserv_twi_buffer_param[1] = v32_to_v8 (angle, 1);
    asserv_twi_buffer_param[2] = v32_to_v8 (angle, 0);
    /* Send the set angular position command to the asserv board */
    asserv_twi_send_command ('p', 3);
}

/* Set speeds of movements. */
void
asserv_set_speed (uint8_t linear_high, uint8_t angular_high,
		  uint8_t linear_low, uint8_t angular_low)
{
    /* 's' subcommand */
    asserv_twi_buffer_param[0] = 's';
    /* Put speeds as parameters */
    asserv_twi_buffer_param[1] = linear_high;
    asserv_twi_buffer_param[2] = angular_high;
    asserv_twi_buffer_param[3] = linear_low;
    asserv_twi_buffer_param[4] = angular_low;
    /* Send the set speed of movements command to the asserv board */
    asserv_twi_send_command ('p', 5);
}

/**
 * Go to the position provided in parameters. Those points shall be in the
 * table.
 * @param  x  the x position on the table.
 * @param  y  the y position on the table.
 */
void
asserv_goto (uint32_t x, uint32_t y)
{
}

