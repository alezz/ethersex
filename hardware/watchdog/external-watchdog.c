/*
 *   EXTERNAL WATCHDOG CHIP
 * 
 * Copyright (c) 2018 by Enrico Giacomazzi <enrico@giacomazzi.cc>
 * Copyright (c) 2018 by HyperWare Solutions snc <info@hyperware.io>
 * Copyright (c) 2018 by Alessandro Mauro <alez@maetech.it>
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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
 * http://www.gnu.org/copyleft/gpl.html* 
 * 
 */

#include <avr/io.h>
#include "external-watchdog.h"

volatile uint8_t extwdt_ticks;

void
extwdt_init(void){
  extwdt_ticks = EXTERNAL_WATCHDOG_TICKS;
}

void
extwdt_periodic(void){
  //PIN_TOGGLE(EXTERNAL_WATCHDOG_PIN);
  if ( (--extwdt_ticks) == 0 ) 
  {
	extwdt_ticks = EXTERNAL_WATCHDOG_TICKS;
	PIN_TOGGLE(EXTERNAL_WATCHDOG_PIN);
  }
}


/*
  -- Ethersex META --
  header(hardware/watchdog/external-watchdog.h)
  init(extwdt_init)
  timer(1,extwdt_periodic())
*/

