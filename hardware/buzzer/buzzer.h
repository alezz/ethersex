/*
 * Copyright (c) 2018 by Enrico Giacomazzi <enrico@giacomazzi.cc>
 * Copyright (c) 2018 by HyperWare Solutions snc <info@hyperware.io>
 * Copyright (c) 2018 by Alessandro Mauro <alez@maetech.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
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

#ifndef _BUZZER_H
#define _BUZZER_H

#include "config.h"
#ifdef BUZZER_SUPPORT

#if !defined(BUZZER_LOGIC_HI) && !defined(BUZZER_LOGIC_LOW)
	#error Error in pinning configuration for buttons module. Check your pinning\
 configuration.
#endif

#ifdef BUZZER_LOGIC_LOW
	#define BUZZER_ON	PIN_CLEAR(BUZZER_PIN)
	#define BUZZER_OFF	PIN_SET(BUZZER_PIN)
#else
	#define BUZZER_ON	PIN_SET(BUZZER_PIN)
	#define BUZZER_OFF	PIN_CLEAR(BUZZER_PIN)
#endif

void buzzer_init(void);
void buzzer_periodic(void);
void buzzer_beep (uint8_t duration_ticks);
int16_t parse_cmd_beep (char *cmd, char *output, uint16_t len) ;

#endif

#endif /* _BUZZER_H */
