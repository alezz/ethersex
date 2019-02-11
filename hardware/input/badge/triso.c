/*
 *   INOUT srl "TR-ISO" RFID READER MODULE
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
 * http://www.gnu.org/copyleft/gpl.html
 */

/* Enable the hook rfid_triso_read */
#define HOOK_NAME rfid_triso_read
#define HOOK_ARGS (uint8_t data[16])
#define HOOK_COUNT 1
#define HOOK_ARGS_CALL (data)
#define HOOK_IMPLEMENT 1



#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/parity.h>
#include <avr/interrupt.h>

#include "triso.h"
#include "core/bit-macros.h"
#include "core/debug.h"

static volatile uint8_t triso_bitcount;
static volatile uint8_t triso_data;
static volatile uint8_t triso_digit;
static volatile uint8_t triso_parity;
static volatile uint8_t triso_lcr;
static volatile uint8_t triso_badflag;
static volatile uint8_t triso_buffer[16];


void
inout_triso_init(void)
{
  triso_configure_pcint();
  
  //should not be needed
  DDR_CONFIG_IN(INOUT_TRISO_STROBE); /*pcint18*/
  DDR_CONFIG_IN(INOUT_TRISO_DATA);
  DDR_CONFIG_IN(INOUT_TRISO_PRES);  /*pcint20*/
  //pull-up in the case the module is phisically disconnected
  PIN_SET(INOUT_TRISO_STROBE);
  PIN_SET(INOUT_TRISO_DATA);
  PIN_SET(INOUT_TRISO_PRES);
  
  triso_bitcount = 9;
  triso_data = 0;
  triso_digit = 0;
  triso_parity = 0x10;
  triso_lcr=0;
  triso_badflag = 0;

}

void
inout_triso_interrupt(void)
{
	if (PIN_HIGH(INOUT_TRISO_PRES))
	{
		/* no "tag presence" -> reset all */
		triso_bitcount = 9;
 	    triso_data = 0;
		triso_digit = 0;
		triso_parity = 0x10;
		triso_lcr = 0;
		triso_badflag = 0;
	}
	else 				
	{
		/* tag presence */
		if ( ! PIN_HIGH(INOUT_TRISO_STROBE) ) {
			{
				if (!PIN_HIGH(INOUT_TRISO_DATA)) 				/* sample DATA */
				{
					if (triso_bitcount > 1) triso_parity ^= 0x10;
					triso_data |= 0x10;	
				}
				triso_bitcount--;										/* next bit  */
				if (triso_bitcount  == 0)								/* last bit */
				{
					triso_bitcount = 5;
					if (triso_digit == 16)							/* last digit */
					{
						/* parity check onto LCR digit */
						if ( (triso_data & 0x10) == triso_parity )
						{
							triso_data &= 0xF;
						}
						else
							triso_badflag = 1;
							
						if ((triso_lcr == triso_data) && (triso_badflag == 0))
						{
							INOUT_DEBUG ("RFID finished, LCR %d ok\n", triso_data);
							/* storage of "buffer" into global available pool */
							hook_rfid_triso_read_call(triso_buffer);
						}
					#ifdef DEBUG_INOUT_TRISO
						else if (triso_badflag == 0)
						{
							INOUT_DEBUG ("RFID finished, BAD LCR (got %d, expected %d), PE ok !\n", triso_data,triso_lcr);
						}
						else
						{
							INOUT_DEBUG ("RFID finished, some PE wrong!\n", triso_data);
						}
						INOUT_DEBUG ("DATA =");
						for (uint8_t i = 0; i<16; i++)
							printf_P(PSTR(" %x"), triso_buffer[i]);
						printf_P(PSTR("\n"));
					#endif
						triso_digit = 0;
						triso_bitcount = 20;	/*postamble*/
					} 
					else
					{
						if (triso_digit > 0)
						{
							/* if parity check */
							if ( (triso_data & 0x10) == triso_parity )
							{
								triso_data &= 0xF;				/* keep data bits */
								triso_buffer[triso_digit] = triso_data;  	   		    
								triso_lcr ^= triso_data;	
							}
							else
							{
								triso_badflag = 1;
							}
						}
						else
						{
							/* preamble or postable , shall we check it ? */
						}
						triso_digit++;
					}
					triso_data = 0;
					triso_parity=0x10;
				}
				triso_data >>= 1;									    /* shift bits */
			}
		}
	}
}

/*
  -- Ethersex META --
  header(hardware/input/badge/triso.h)
  init(inout_triso_init)
*/
