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


/*
 * 
 * for the moment , there is no menuconfig/m4 buttons configuration
 * 
 * PL0246 COLUMNS (OUTPUT)
 * PL1357 ROWS (INPUT)
 * NO PULLUP
 * SCAN LOW - INPUT LOW
*/

/* Enable the hook buttons_input */
#define HOOK_NAME matrix_buttons_input
#define HOOK_ARGS (matrix_buttons_Buttons button, uint8_t status)
#define HOOK_COUNT 3
#define HOOK_ARGS_CALL (button, status)
#define HOOK_IMPLEMENT 1

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "matrixbuttons.h"
#include "config.h"
#include "protocols/ecmd/ecmd-base.h"

/* This driver uses the E6 hook mechanism to notify of events. (see hook.def
 * for details)
 * To get notified in your application of a button press:
 * 1) #include "buttons.h"
 *
 * 2) Define a handler function in your application:
 *    void hook_button_handler(buttons_ButtonsType button, uint8_t status) {
 *      debug_printf("Button %d Status: %d\n",button, status);
 *    }
 *
 * 3) Register it for callback (in the Init Function of your app):
 *    hook_buttons_input_register(hook_buttons_handler);
 *
 *    Upon a button press, the function will be called with the following
 *    parameters:
 *    button           = The button that was pressed.
 *    status           = One of the following values:
 *      BUTTON_PRESS     = Button was pressed
 *                         <CONF_BUTTONS_DEBOUNCE_TIME> * 20ms (default: 80ms)
 *      BUTTON_LONGPRESS = Button was pressed for
 *                         <CONF_BUTTONS_LONGPRESS_TIME> * 20ms (default: 2s)
 *      BUTTON_REPEAT    = Button was pressed for
 *                         <CONF_BUTTONS_REPEAT_DELAY> * 20ms (default 3.5s),
 *                         this event is then repeated every
 *                         <CONF_BUTTONS_REPEAT_RATE> * 20ms (default 0.5s)
 *                         until the button is released.
 *      BUTTON_RELEASE   = Button released.
 */

#ifdef MATRIX_BUTTONS_INPUT_SUPPORT

#ifdef DEBUG_MATRIX_BUTTONS
  /*  For providing the actual name of the buttons for debug output */
  #define STR(_v)  const char _v##_str[] PROGMEM = #_v;
  #define STRLIST(_v) _v##_str,
  #define BUTTONS_GET_NAME(i) ((PGM_P)pgm_read_word(&buttonNames[i]))

  /* This creates an array of string in ROM which hold the button names. */
  MATRIX_BUTTONS_CONFIG(STR);
  PGM_P const buttonNames[MATRIX_BUTTONS_COUNT] PROGMEM = { MATRIX_BUTTONS_CONFIG(STRLIST) };
#endif

uint8_t column;

//const buttons_configType buttonConfig[MATRIX_BUTTONS_COUNT] PROGMEM =
  //{ MATRIX_BUTTONS_CONFIG(C) };

matrix_buttons_statusType buttonStatus[MATRIX_BUTTONS_COUNT];

/**
 * @brief Initializes the module after startup
 *
 * Resets all buttons to BUTTON_RELEASE.
 *
 * @param void
 * @returns void
 */
void
matrixbuttons_init(void)
{
	//MATRIX_BUTTONS_DEBUG("Init\n");

	//#ifdef CONF_BUTTONS_USE_PULLUPS
		//MATRIX_BUTTONS_CONFIG(PULLUP);
	//#endif

	//PL0246 COLUMNS (OUTPUT)
	//PL1357 ROWS (INPUT)
	// NO PULLUP
	// SCAN LOW - INPUT LOW
	
	DDRL =  0b01010101;
	PORTL = 0b01010100;	//scan column 0
	column = 0;
}

/**
 * @brief Updates the button status every 20ms
 *
 * Reads in the button pins and runs the simple state machine for
 * short/longpress and repeat.
 *
 * @param void
 * @returns void
 */
void
matrixbuttons_periodic(void)
{
	for (uint8_t r=0; r< 4;r++)
	{
		uint8_t i = column*4 + r;
		uint8_t a = _BV(r*2 + 1);
		uint8_t curState = (( PINL & a ) == a) ? 0 : 1;
		if (buttonStatus[i].curStatus == curState)
		{
		  /* If the current button state is different from the last stable state,
		   * run the debounce timer. Also keep the debounce timer running if the
		   * button is pressed, because we need it for long press/repeat
		   * recognition */
		  if ((buttonStatus[i].curStatus != buttonStatus[i].status) ||
			  (BUTTON_RELEASE != buttonStatus[i].status))
		  {
			buttonStatus[i].ctr++;
		  }
		}
		else
		{
		  /* Actual state has changed since the last read. Restart the debounce
		   * timer */
		  buttonStatus[i].ctr = 0;
		  buttonStatus[i].curStatus = curState;
		}
		/* Button was stable for DEBOUNCE_TIME * 20 ms */
		if (CONF_MATRIX_BUTTONS_DEBOUNCE_DELAY <= buttonStatus[i].ctr)
		{
			/* Button is pressed.. */
			if (1 == buttonStatus[i].curStatus)
			{
				switch (buttonStatus[i].status)
				{
				  /* ..and was not pressed before. Send the PRESS event */
				  case BUTTON_RELEASE:
					buttonStatus[i].status = BUTTON_PRESS;
					MATRIX_BUTTONS_DEBUG("Pressed %S\n", BUTTONS_GET_NAME(i));
					hook_matrix_buttons_input_call(i, buttonStatus[i].status);
					break;

				  /* ..and was pressed before. Wait for long press. */
				  case BUTTON_PRESS:
					if (CONF_MATRIX_BUTTONS_LONGPRESS_DELAY <= buttonStatus[i].ctr)
					{
					  /* Long press time reached. Send LONGPRESS event. */
					  buttonStatus[i].status = BUTTON_LONGPRESS;
					  MATRIX_BUTTONS_DEBUG("Long press %S\n", BUTTONS_GET_NAME(i));
					  hook_matrix_buttons_input_call(i, buttonStatus[i].status);
					}
					break;

				  case BUTTON_LONGPRESS:
					/* Wait for button release */
					break;

				  default:
					MATRIX_BUTTONS_DEBUG("Oops! Invalid state.\n");
					break;
				}
			}
			else
			{
				/* Button is not pressed anymore. Send RELEASE. */
				buttonStatus[i].status = BUTTON_RELEASE;
				MATRIX_BUTTONS_DEBUG("Released %S\n", BUTTONS_GET_NAME(i));
				buttonStatus[i].ctr = 0;
				hook_matrix_buttons_input_call(i, buttonStatus[i].status);
			}
		}
	}
	if (++column>3) column=0;
	PORTL = 0b01010101 & ~_BV(column*2);
}

/*
 * -- Ethersex META --
 * header(hardware/input/matrixbuttons/matrixbuttons.h)
 * timer(1, matrixbuttons_periodic())
 * init(matrixbuttons_init)
 */

#endif //MATRIX_BUTTONS_INPUT_SUPPORT
