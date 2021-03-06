# simu - Robot simulation. {{{
#
# Copyright (C) 2010 Nicolas Schodet
#
# APBTeam:
#        Web: http://apbteam.org/
#      Email: team AT apbteam DOT org
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
# }}}
"""Generic ultra sonic distance sensor (use several rays)."""
from simu.inter.drawable import Drawable
from simu.view.distance_sensor import DistanceSensor

class DistanceSensorUS (Drawable):

    def __init__ (self, onto, model):
        Drawable.__init__ (self, onto)
        self.model = model
        self.rays = [ ]
        for r in model.rays:
            self.rays.append (DistanceSensor (onto, r))

