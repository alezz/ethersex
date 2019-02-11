/*
 *   LCD CONTRAST with MCP41xxx (DIGITAL POT. CHIP) 
 *   CONNECTED VIA SPI
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
#include <util/atomic.h>
#include "core/spi.h"
#include "protocols/ecmd/ecmd-base.h"
#include "contrast_mcp41xxx.h"


/* check out this , also check SD card... */

#ifdef RFM12_IP_SUPPORT
/* RFM12 uses interrupts which do SPI interaction, therefore
   we have to disable interrupts if support is enabled */
#  define mcp_select()  uint8_t sreg = SREG; cli(); PIN_CLEAR(SPI_CS_MCP); 
#  define mcp_unselect() PIN_SET(SPI_CS_MCP); SREG = sreg;
#else
#  define mcp_select()  PIN_CLEAR(SPI_CS_MCP)
#  define mcp_unselect() PIN_SET(SPI_CS_MCP)
#endif



static volatile uint8_t contrast_value;



void
mcp_init(void){
	contrast_set(1);
}

uint8_t
contrast_get(void){
	/*  return current contrast value */
	return contrast_value;
}

void
contrast_set(uint8_t value){
	/*  set contrast to value */
	contrast_value = value;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		mcp_select();
		spi_send(0x11);
		spi_send(value);
		mcp_unselect();
	}
}





/*
  -- Ethersex META --
  header(hardware/lcd/contrast_mcp41xxx.h)
  init(mcp_init)
*/
