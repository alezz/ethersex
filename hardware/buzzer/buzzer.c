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

#include <avr/io.h>
#include "buzzer.h"
#include "protocols/ecmd/ecmd-base.h"

volatile uint8_t ticks;
volatile uint8_t cycle;

void 
buzzer_init(void)
{
	BUZZER_OFF;
}

void
buzzer_beep (uint8_t duration_ticks)
{
	BUZZER_ON;
	ticks = duration_ticks;
}

void
buzzer_periodic(void)
{
  if (ticks>0)
	if (--ticks==0)
	{
	  BUZZER_OFF;
	}
}

int16_t parse_cmd_beep (char *cmd, char *output, uint16_t len)
{
	while (*cmd == ' ')	
		cmd++;
	uint8_t duration = 1;
	if (*cmd)
		duration = atoi(cmd);
	buzzer_beep(duration);
	return ECMD_FINAL_OK;
}

/*
 * -- Ethersex META --
 * header(hardware/buzzer/buzzer.h)
 * init(buzzer_init)
 * timer(1, buzzer_periodic())
 * ecmd_feature(beep, "beep",[DURATION], Make a beep optionally for DURATION ticks)
 */
