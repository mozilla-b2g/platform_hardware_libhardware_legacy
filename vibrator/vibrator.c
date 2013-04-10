/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <hardware_legacy/vibrator.h>
#include "qemu.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

static const char *vibrator_sysfs[] = {
    "/sys/class/input/event1/device/device/vibrator_enable",
    "/sys/class/input/event2/device/device/vibrator_enable",
    "/sys/class/timed_output/vibrator/enable",
};

static int vibrator_index = -1;

int vibrator_exists()
{
    int i, fd;

#ifdef QEMU_HARDWARE
    if (qemu_check()) {
        return 1;
    }
#endif

    for (i = 0; i < (sizeof(vibrator_sysfs) / sizeof(const char *)); i++) {
        fd = open(vibrator_sysfs[i], O_RDWR);
        if (fd >= 0) {
			vibrator_index = i;
			close(fd);
			return 1;
		}
	}

    return 0;
}

static int sendit(int timeout_ms)
{
    int nwr, ret, fd;
    char value[20];

#ifdef QEMU_HARDWARE
    if (qemu_check()) {
        return qemu_control_command( "vibrator:%d", timeout_ms );
    }
#endif

    if (vibrator_index < 0)
	return -1;

    fd = open(vibrator_sysfs[vibrator_index], O_RDWR);
    if (fd < 0)
		return errno;

    nwr = sprintf(value, "%d\n", 1);
    ret = write(fd, value, nwr);
    if (ret < 0) {
		close(fd);
		return ret;
    }

    usleep(timeout_ms * 1000);

    nwr = sprintf(value, "%d\n", 0);
    ret = write(fd, value, nwr);
    if (ret < 0) {
	close(fd);
	return ret;
    }

    close(fd);
    return 0;
}

int vibrator_on(int timeout_ms)
{
    /* constant on, up to maximum allowed time */
    return sendit(timeout_ms);
}

int vibrator_off()
{
    return sendit(0);
}
