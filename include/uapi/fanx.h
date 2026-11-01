// SPDX-License-Identifier: GPL-2.0
// vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :

#ifndef _UAPI_FANX_H
#define _UAPI_FANX_H

#include <linux/types.h>

#ifndef FANX_DRIVER_NAME
#define FANX_DRIVER_NAME "Fanx"
#endif

#ifndef FANX_DEVICE_NAME
#define FANX_DEVICE_NAME FANX_DRIVER_NAME
#endif

/* Hardware limits */
#define FANX_RPM_MAX 5000
#define FANX_RTIME_MAX 86400

/* Temperature thresholds (millidegrees Celsius) */
#define FANX_TEMP_CRIT 90000
#define FANX_TEMP_HIGH 75000
#define FANX_TEMP_MEDIUM 60000
#define FANX_TEMP_GOOD 40000
#define FANX_TEMP_HYST 5000 /* Hysteresis */

/**
 * enum fanx_mode - Operating modes of the fan
 * @FANX_MODE_MAN:   Manual control mode
 * @FANX_MODE_AUTO:  Automatic thermal regulation mode
 * @FANX_MODE_RTIME: Timer-based operation mode
 */
enum fanx_mode {
    FANX_MODE_MAN = 0,
    FANX_MODE_AUTO = 1,
    FANX_MODE_RTIME = 2,
};

/**
 * enum fanx_temp_level - Thermal severity level evaluated by the module
 * @FANX_LVL_GOOD:   Normal temperature range
 * @FANX_LVL_MEDIUM: Elevated temperature range
 * @FANX_LVL_HIGH:   High temperature range
 * @FANX_LVL_CRIT:   Critical temperature threshold reached
 */
enum fanx_temp_level {
    FANX_LVL_GOOD = 0,
    FANX_LVL_MEDIUM = 1,
    FANX_LVL_HIGH = 2,
    FANX_LVL_CRIT = 3,
};

/**
 * struct fanx_status - Structure holding the urrent status and metrics of the
 *                          fan
 * @rpm:           Current measured fan speed (RPM)
 * @target_rpm:    Target fan speed setpoint (RPM)
 * @timer_total_s: Requested total duration for the timer mode (seconds)
 * @timer_left_s:  Remaining duration calculated by the module (seconds)
 * @uptime_s:      Total operating time since power-on (seconds)
 * @temp_mc:       Current temperature in millidegrees Celsius (m°C)
 * @mode:          Active operating mode (refer to &enum fanx_mode)
 * @level:         Current thermal level (refer to &enum fanx_temp_level)
 * @running:       Running state flag (1 = active, 0 = stopped)
 * @pad:           Explicit padding alignment byte for structure layout
 *                  consistency (to 128 bytes)
 * @profile:       Profile description string (guaranteed NUL-terminated)
 */
struct fanx_ds {
    /* Cacheline 0 */
    __u32 rpm;
    __u32 target_rpm;
    __u32 timer_total_s;
    __u32 timer_left_s;
    __u32 uptime_s;
    __s32 temp_mc;
    char profile[64];

    /* Cacheline 1 */
    __u8 mode;
    __u8 level;
    __u8 running;
    __u8 pad[37];
};

#endif /* _UAPI_FANX_H */
