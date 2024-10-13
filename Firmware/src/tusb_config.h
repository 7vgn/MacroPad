/**
 * \file tusb_config.h
 * Configuration of the TinyUSB library
 */

#ifndef _TUSB_CONFIG_H
#define _TUSB_CONFIG_H

//-----------------------------------------------------------------------------
// Board Configuration (i.e. the platform TinyUSB runs on)
// Most settings are in the board.mk file.

/**
 * \brief Port is full speed
 */
#define BOARD_DEVICE_RHPORT_SPEED OPT_MODE_FULL_SPEED

/**
 * \brief This is a device (as opposed to a host)
 */
#define CFG_TUSB_RHPORT0_MODE (OPT_MODE_DEVICE | BOARD_DEVICE_RHPORT_SPEED)

/**
 * \brief Put TinyUSB data in a specific section of memory
 */
#define CFG_TUSB_MEM_SECTION

/**
 * \brief Align TinyUSB data at 32-bit word borders
 */
#define CFG_TUSB_MEM_ALIGN __attribute__ ((aligned(4)))

//-----------------------------------------------------------------------------
// Device Configuration

/**
 * \brief Maximum packet size for Endpoint 0
 * \details It is important that this be set to 64 (the maximum for USB full
 * speed) since we have feature requests that use all 64 bytes.
 */
#define CFG_TUD_ENDPOINT0_SIZE 64

/**
 * \brief Implementing three HID-class devices
 */
#define CFG_TUD_HID		3

//-----------------------------------------------------------------------------
// HID Interface Configuration

/**
 * \brief Maximum packet size for the interrupt endpoints of each device
 * \details This could probably be smaller but depends on the KRO constant (the
 * keyboard interface is probably the one that creates the largest packets).
 */
#define CFG_TUD_HID_EP_BUFSIZE 64

#endif // _TUSB_CONFIG_H
