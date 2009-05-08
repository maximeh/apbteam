/* eeprom.avr.c */
/* io - Input & Output with Artificial Intelligence (ai) support on AVR. {{{
 *
 * Copyright (C) 2008 Dufour Jérémy
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
#include "eeprom.h"

#include "servo.h"		/* SERVO_NUMBER */
#include "servo_pos.h"
#include "sharp.h"		/* sharp_threshold */

#include <avr/eeprom.h>		/* eeprom_{read,write}_byte */

/**
 * @defgroup EepromPrivate EEPROM module private variables and functions
 * declarations
 * @{
 */

/**
 * What is the address, in the EEPROM, where are stored the parameters values?
 */
#define EEPROM_PARAM_START 0

/**
 * Current version of the parameters organization.
 * @warning: you must update this value every time you change the structure of
 * data stored into the EEPROM.
 */
#define EEPROM_PARAM_KEY 0x05

/** @} */

/* Load parameters from the EEPROM. */
void
eeprom_load_param ()
{
    uint8_t compt;
    uint8_t pos;
    uint16_t *ptr16;

    /* The parameters start at the given address */
    uint8_t *ptr8 = (uint8_t *) EEPROM_PARAM_START;
    /* Check if the key/version is correct */
    if (eeprom_read_byte (ptr8++) != EEPROM_PARAM_KEY)
	/* Error, stop here */
	return;

    /* Load servo pos module data */
    for (compt = 0; compt < SERVO_NUMBER; compt++)
      {
	for (pos = 0; pos < SERVO_POS_NUMBER; pos++)
	    servo_pos_high_time[compt][pos] = eeprom_read_byte(ptr8++);
      }

    /* Load sharp module data */
    ptr16 = (uint16_t *) ptr8;
    /* Threshold values, high and low */
    for (compt = 0; compt < SHARP_NUMBER; compt++)
      {
	sharp_threshold[compt][0] = eeprom_read_word (ptr16++);
	sharp_threshold[compt][1] = eeprom_read_word (ptr16++);
      }
}

/* Store parameters in the EEPROM. */
void
eeprom_save_param ()
{
    uint8_t compt, pos;
    uint16_t *ptr16;

    /* The parameters start at the given address */
    uint8_t *ptr8 = (uint8_t *) EEPROM_PARAM_START;
    /* Store the key */
    eeprom_write_byte (ptr8++, EEPROM_PARAM_KEY);

    /* Store servo pos module data */
    for (compt = 0; compt < SERVO_NUMBER; compt++)
      {
	for (pos = 0; pos < SERVO_POS_NUMBER; pos++)
	    eeprom_write_byte (ptr8++, servo_pos_high_time[compt][pos]);
      }

    /* Store sharp module data */
    ptr16 = (uint16_t *) ptr8;
    /* Threshold values, high and low */
    for (compt = 0; compt < SHARP_NUMBER; compt++)
      {
	eeprom_write_word (ptr16++, sharp_threshold[compt][0]);
	eeprom_write_word (ptr16++, sharp_threshold[compt][1]);
      }
}

/* Clear parameters in the EEPROM by invalidating the key. */
void
eeprom_clear_param ()
{
    /* Write an invalid key */
    eeprom_write_byte ((uint8_t *) EEPROM_PARAM_START, 0xFF);
}
