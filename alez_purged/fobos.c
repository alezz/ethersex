/*
*
* Copyright (c) 2014 by Alessandro Mauro <alez@maetech.it>
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


#include "config.h"

#include "protocols/uip/uip.h"

void fobos_net_init()
{
    uip_listen(HTONS(FOBOS_TCP_PORT), fobos_net_handler);
}

void fobos_net_handler()
{
	if (!uip_poll()) 
	{
		
	}
	if (uip_connected())
	{
			
	}
	if (uip_acked())
	{
		
	}
	if (uip_newdata()) 
	{
	
	}	
	if(uip_rexmit() ||
		uip_newdata() ||
		uip_acked() ||
		uip_connected() ||
		uip_poll()) {
        if (state->out_len > 0) {
#ifdef DEBUG_FOBOS
            debug_printf("sending %d bytes\n", state->out_len);
#endif
            uip_send(state->outbuf, state->out_len);
        } else if (state->close_requested)
          uip_close();
    }
}


/ *
  - Ethersex META -
  net_init(fobos_net_init)
* /
