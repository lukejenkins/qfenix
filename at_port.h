/* SPDX-License-Identifier: BSD-3-Clause */
#ifndef __AT_PORT_H__
#define __AT_PORT_H__

#include <stddef.h>

/*
 * Auto-detect the AT command port for a Qualcomm modem.
 *
 * On macOS: uses qcseriald to find /dev/tty.qcserial-at0
 * On Linux: scans /sys/bus/usb/devices, probes with AT\r
 * On Windows: scans COM ports via SetupAPI
 *
 * If serial is non-NULL, filters by USB serial number.
 * Returns 1 if found (path written to buf), 0 otherwise.
 */
int at_detect_port(char *buf, size_t size, const char *serial);

#endif /* __AT_PORT_H__ */
