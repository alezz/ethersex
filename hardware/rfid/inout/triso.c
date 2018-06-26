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
  DDR_CONFIG_IN(INOUT_TRISO_STROBE); 
  DDR_CONFIG_IN(INOUT_TRISO_DATA);
  DDR_CONFIG_IN(INOUT_TRISO_PRES);
  
  bitcount = 9;
  data = 0;
  digit = 0;
  parity = 1;
  

}

ISR(RFID_VECTOR)
{
	INOUT_DEBUG ("ISR - tag present change\n");
	if ( ! PIN_HIGH(INOUT_TRISO_PRES)) {
		INOUT_DEBUG (" - tag present LOW\n");
		bitcount = 9;
		digit = 0;
		data = 0;
		while ((!PIN_HIGH(INOUT_TRISO_PRES)) && digit < 16)
		{
			while (PIN_HIGH(INOUT_TRISO_STROBE));	/*wait for clock to go low */
			if (!PIN_HIGH(INOUT_TRISO_DATA)) data |= 0x10;	/* sample DATA */
			if (--bitcount  == 0)
			{
				if (digit++ > 0)
				{
					/* if parity check */
					buffer[digit] = data & 0xF;  	   		    /* store data bits, go to next digit */
					bitcount = 4;									/* 4 data bits + 1 parity bit */
					INOUT_DEBUG ("data (%d): %x\n", digit, data);
					data = 0;
				}
				else
					INOUT_DEBUG ("got preamble, we hope\n");	/* preamble is not checked */
				if (digit == 16)
				{
					/* LCR check */
					INOUT_DEBUG ("RFID finished, data: %s\n", buffer);
					digit = 0;
					bitcount = 9;
				}
			}
			else
			{
				data >>= 1;									/* shift bits */
			}
			while (!PIN_HIGH(INOUT_TRISO_STROBE));	/*wait for clock to return hi */
		}
	}
}

/*
  -- Ethersex META --
  header(hardware/rfid/inout/triso.h)
  init(inout_triso_init)
*/
