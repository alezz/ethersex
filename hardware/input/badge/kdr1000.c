/*
 *   KD ELECTRONICS KDR-1000 MAGNETIC STRIP READER
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

/* Enable the hook msr_kdr1000_read */
#define HOOK_NAME msr_kdr1000_read
#define HOOK_ARGS (uint8_t data[40], uint8_t lenght)
#define HOOK_COUNT 1
#define HOOK_ARGS_CALL (data, lenght)
#define HOOK_IMPLEMENT 1



#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/parity.h>
#include <avr/interrupt.h>

#include "kdr1000.h"
#include "core/bit-macros.h"
#include "core/debug.h"

//#include "hardware/lcd/hd44780.h"

#define KDR1000_BUFFER_SIZE	40

static volatile uint8_t kdr1000_bitcount;
static volatile uint8_t kdr1000_data;
static volatile uint8_t kdr1000_digit;
static volatile uint8_t kdr1000_parity;
static volatile uint8_t kdr1000_lrc;
static volatile uint8_t kdr1000_badflag;
static volatile uint8_t kdr1000_buffer[KDR1000_BUFFER_SIZE];
static volatile uint8_t kdr1000_end;


void
kde_kdr1000_init(void)
{
  kdr1000_configure_pcint();
  
  //should not be needed
  DDR_CONFIG_IN(KDE_KDR1000_DATA);
  DDR_CONFIG_IN(KDE_KDR1000_STROBE);
  DDR_CONFIG_IN(KDE_KDR1000_PRES);
  //pull-up in the case the module is phisically disconnected
  PIN_SET(KDE_KDR1000_DATA);
  PIN_SET(KDE_KDR1000_STROBE);
  PIN_SET(KDE_KDR1000_PRES);
  
  kdr1000_bitcount = 5;
  kdr1000_data = 0;
  kdr1000_digit = 0;
  kdr1000_parity = 0x10;
  kdr1000_lrc=0;
  kdr1000_badflag = 0;
  kdr1000_end=0;
}

void
kde_kdr1000_interrupt(void)
{
	if (PIN_HIGH(KDE_KDR1000_PRES))
	{
		/* idle = reset all */
		kdr1000_bitcount = 5;
 	    kdr1000_data = 0;
		kdr1000_digit = 0;
		kdr1000_parity = 0x10;
		kdr1000_lrc = 0;
		kdr1000_badflag = 0;
		kdr1000_end = 0;
	}
	else 				
	{
		/* badge presence */
		if ( ! PIN_HIGH(KDE_KDR1000_STROBE) ) {
			{												/* sample DATA */
				if (PIN_HIGH(KDE_KDR1000_DATA))
				{
					/* at the beginning data stays hi for some time, 
					 * we have to ignore this */
					 if ((kdr1000_digit == 0) && (kdr1000_bitcount == 5)) return;
				}
				else 				/* data LO means bit=1 */
				{
					if (kdr1000_bitcount > 1) kdr1000_parity ^= 0x10;
					kdr1000_data |= 0x10;
				}
				kdr1000_bitcount--;										/* next bit  */
				if (kdr1000_bitcount  == 0)								/* last bit */
				{
					kdr1000_bitcount = 5;
					/* parity check */
					if ( (kdr1000_data & 0x10) == kdr1000_parity )
					{
						kdr1000_data &= 0xF;				/* keep data bits */
						if (kdr1000_digit < KDR1000_BUFFER_SIZE )	/* prevent out-of-boundary */
							kdr1000_buffer[kdr1000_digit] = kdr1000_data;
						if (kdr1000_data == 0xF)		/* end sentinel */
						{
							kdr1000_end = 1;
						} 
						else if (kdr1000_end == 1)
						{
						/* the next byte after end sentinel is the LRC */
						#ifdef DEBUG_KDE_KDR1000
							KDE_DEBUG ("MSR: end sentinel 0xF then LRC\n");
							KDE_DEBUG ("DATA (%d) =", kdr1000_digit+1);
							for (uint8_t i = 0; i<=kdr1000_digit; i++)
								printf_P(PSTR(" %x"), kdr1000_buffer[i]);
							printf_P(PSTR("\n"));
							KDE_DEBUG ("LCR calcolato = %x, letto = %x", kdr1000_lrc, kdr1000_data );
							KDE_DEBUG ("badflag (parity check) = %u\n", kdr1000_badflag);
						#endif
							if ((kdr1000_lrc == kdr1000_data) && (kdr1000_badflag == 0))
							{
								KDE_DEBUG ("MSR callback\n");
								hook_msr_kdr1000_read_call(kdr1000_buffer, kdr1000_digit);
							}
							kdr1000_end = 0;
						}
						kdr1000_lrc ^= kdr1000_data;
					}
					else
					{
						kdr1000_badflag = 1;
					}
					kdr1000_digit++;
					kdr1000_data = 0;
					kdr1000_parity=0x10;
				}
				kdr1000_data >>= 1;									    /* shift bits */
			}
		}
	}
}

/*
  -- Ethersex META --
  header(hardware/input/badge/kdr1000.h)
  init(kde_kdr1000_init)
*/
