#!/usr/bin/env python3
#
# Copyright (C) 2004-2025 Savoir-faire Linux Inc.
#
# Author: Eloi Bail <eloi.bail@savoirfairelinux.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#

import dbus
import time
import sys
import os
from random import randint

class libjamiToggleVideo():
    def start(self):
        bus = dbus.SessionBus()

        videoControlBus = bus.get_object('org.sflphone.SFLphone', '/org/sflphone/SFLphone/VideoControls')
        videoControl = dbus.Interface(videoControlBus, dbus_interface='org.sflphone.SFLphone.VideoControls')

        while True:
            time.sleep(2)
            id = videoControl.openVideoInput("")
            time.sleep(2)
            videoControl.closeVideoInput(id)
