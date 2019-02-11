/*
 *
 * Copyright (c) 2018 by Enrico Giacomazzi <enrico@giacomazzi.cc>
 * Copyright (c) 2018 by HyperWare Solutions snc <info@hyperware.io>
 * Copyright (c) 2018 by Alessandro Mauro <alez@maetech.it>
 * 
 * Copyright (c) 2012 by Daniel Walter <fordprfkt@googlemail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */

#ifndef MATRIX_BUTTONS_H
#define MATRIX_BUTTONS_H

#include "config.h"
#ifdef MATRIX_BUTTONS_INPUT_SUPPORT

#if !defined(MATRIX_BUTTONS_COUNT) || !defined(MATRIX_BUTTONS_CONFIG)
#error Error in pinning configuration for buttons module. Check your pinning\
 configuration.
#endif

/* FOR THE MOMENT IT IS NOT SUPPORTED
 * To configure the buttons add the following to your pinning configuration:
 * FOR THE MOMENT IT IS NOT SUPPORTED
 * 
ifdef(`conf_MATRIX_BUTTONS_COLUMNS', `
  pin(BTN_C0, PL0, OUTPUT)
  pin(BTN_C1, PL2, OUTPUT)
  pin(BTN_C2, PL4, OUTPUT)
  pin(BTN_C3, PL6, OUTPUT)
  pin(BTN_R0, PL1, INPUT)
  pin(BTN_R1, PL3, INPUT)
  pin(BTN_R2, PL5, INPUT)
  pin(BTN_R3, PL7, INPUT)
  
  #define BUTTONS_COLUMNS 4
  #define BUTTONS_ROWS 4

  #define BUTTONS_COLUMNS_CONF(_x) \
  _x(BTN_C0)\
  _x(BTN_C1)\
  _x(BTN_C2)\
  _x(BTN_C3)

  #define BUTTONS_ROWS_CONF(_x) \
  _x(BTN_R0)\
  _x(BTN_R1)\
  _x(BTN_R2)\
  _x(BTN_R3)

  #define MATRIX_BUTTONS_CONFIG(_x) \
  _x(F1,B1,B2,B3,F2,B4,B5,B6,F3,B7,B8,B9,F4,BC,B0,BE)

')
 *
 */

enum
{
  BUTTON_RELEASE   = 0, // Button is not pressed / released
  BUTTON_PRESS     = 1, // Button pressed
  BUTTON_LONGPRESS = 2, // Long pressed button
  BUTTON_REPEAT    = 3  // Repeat during long presses button
};

/* These macros allow to use the the same configuration macro (in
 * buttons_cfg.h) to initialize the buttons_ButtonsType enum, the
 * button_configType struct and set the pullups. */
#define E(_v) _v,
//#define C(_v) {.portIn = &PIN_CHAR(_v##_PORT), .pin = _v##_PIN},
//#define PULLUP(_v) PIN_SET(_v);

typedef volatile uint8_t * const portPtrType;

/* Enum used in the cyclic function to loop over all buttons */
//typedef enum
//{
  //BUTTONS_COLUMNS_CONF(E)
//} matrix_buttons_ColumnsType;
//typedef enum
//{
  //BUTTONS_ROWS_CONF(E)
//} matrix_buttons_RowsType;



typedef enum
{
  MATRIX_BUTTONS_CONFIG(E)
} matrix_buttons_Buttons;



/* Static configuration data for each button */
//typedef struct
//{
  //portPtrType portIn;
  //const uint8_t pin;
//} buttons_configType;

/* Status information for each button */
typedef struct
{
  uint8_t status :2;    // RELEASE, PRESS, LONGPRESS, REPEAT
  uint8_t curStatus :1; // Current pin value
  uint8_t :5;           // unused
  uint8_t ctr;          // Debounce timer
} matrix_buttons_statusType;

void matrixbuttons_init(void);
void matrixbuttons_periodic(void);

#ifdef DEBUG_MATRIX_BUTTONS
#include "core/debug.h"
#define MATRIX_BUTTONS_DEBUG(a...)  debug_printf("button: " a)
#else
#define MATRIX_BUTTONS_DEBUG(a...)
#endif

#define HOOK_NAME matrix_buttons_input
#define HOOK_ARGS (matrix_buttons_Buttons button, uint8_t status)
#include "hook.def"
#undef HOOK_NAME
#undef HOOK_ARGS

#endif /* BUTTONS_INPUT_SUPPORT */
#endif /* BUTTONS_H */
