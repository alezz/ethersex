/*
 *   INOUT srl "TR-ISO" RFID READER MODULE
 *   
 *   (c) 2018 Alessandro Mauro <alez@maetech.it>
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


#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/parity.h>
#include <avr/interrupt.h>

#include "triso.h"
#include "core/bit-macros.h"
#include "core/debug.h"

#include "hardware/lcd/hd44780.h"

static volatile uint8_t bitcount;
static volatile uint8_t data;
static volatile uint8_t digit;
static volatile uint8_t parity;
static volatile uint8_t buffer[20];


void
inout_triso_init(void)
{
/* macro defined in pinning/internals/header.m4, 
 * invoked by pinning/hardware/tdbase.m4
 * configures Pin Change Interrupt by STROBE line
 * */
   rfid_configure_pcint();
  
  //should not be needed
  DDR_CONFIG_IN(INOUT_TRISO_STROBE); /*pcint18*/
  DDR_CONFIG_IN(INOUT_TRISO_DATA);
  DDR_CONFIG_IN(INOUT_TRISO_PRES);  /*pcint20*/
  
  bitcount = 8;
  data = 0;
  digit = 0;
  parity = 1;
  

}

ISR(RFID_VECTOR)
{
	if (PIN_HIGH(INOUT_TRISO_PRES))
	{
		/* no "tag presence" -> reset all */
		bitcount = 8;
 	    data = 0;
		digit = 0;
		parity = 1;
	}
	else
	{
	//INOUT_DEBUG ("ISR - strobe change\n");
		if ( ! PIN_HIGH(INOUT_TRISO_STROBE) ) {
			//INOUT_DEBUG (" - strobe LOW\n");
			//while ((!PIN_HIGH(INOUT_TRISO_PRES)) && digit < 16)
			{
				if (!PIN_HIGH(INOUT_TRISO_DATA)) data |= 0x10;	/* sample DATA */
				bitcount--;										/* next bit  */
				if (bitcount  == 0)								/* last bit */
				{
					bitcount = 5;
					if (digit == 16)
					{
						/* LCR check */
						INOUT_DEBUG ("RFID finished, LCR: %x\n", data);
						digit = 0;
						bitcount = 8;
					} 
					else
					{
						if (digit > 0)
						{
							/* if parity check */
							//buffer[digit] = data & 0xF;  	   		    /* store data bits */
							//INOUT_DEBUG ("data (%d): %x par %d\n", digit, data & 0xF, ((data & 0x10) == 0x10)?1:0);
							INOUT_DEBUG ("data (%d): %x\n", digit, data );
						}
						else
						{
							INOUT_DEBUG ("got preamble, we hope\n");	/* preamble is not checked */
						}
						digit++;
					}
					data = 0;
				}
				data >>= 1;									    /* shift bits */
			}
		}
	}
}

/*
  -- Ethersex META --
  header(hardware/rfid/inout/triso.h)
  init(inout_triso_init)
*/
