#ifndef radar_h
#define radar_h
/* radar.h */
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
#include "defs.h"

/**
 * Handle any distance sensors information to extract useful data.  This
 * includes:
 *  - combining several sensors information for a more precise obstacle
 *  position,
 *  - ignoring obstacles not in the playground,
 *  - determining if an obstacle should make the robot stop.
 */

/** Estimated obstacle radius.  As the sensors detect obstacle edge, this is
 * added to position obstacle center. */
#define RADAR_OBSTACLE_RADIUS_MM 150

/** Stop distance. Distance under which an obstacle is considered harmful when
 * moving. */
#define RADAR_STOP_MM 350

/** Clearance distance.  Distance over which an obstacle should be to the side
 * when moving.
 *
 * OK, more explanations: when moving, a rectangle is placed in front of the
 * robot, of length RADAR_STOP_MM and width 2 * (RADAR_CLEARANCE_MM +
 * BOT_SIZE_SIDE).  If an obstacle is inside this rectangle, it is considered
 * in the way. */
#define RADAR_CLEARANCE_MM 100

/** Destination distance near enough so that obstacles could be ignored. */
#define RADAR_EPSILON_MM 70

/** Update radar view.  Return the number of obstacles found.  Obstacles
 * positions are returned in obs_pos. */
uint8_t
radar_update (const position_t *robot_pos, vect_t *obs_pos);

/** Return non zero if there is a blocking obstacle near the robot while going
 * to a destination point. */
uint8_t
radar_blocking (const vect_t *robot_pos, const vect_t *dest_pos,
		const vect_t *obs_pos, uint8_t obs_nb);

#endif /* radar_h */
