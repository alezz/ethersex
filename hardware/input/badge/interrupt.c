/*
 *   RFID/MSR BADGE INTERRUPT SUPPORT
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


#include <avr/io.h>
#include <avr/interrupt.h>

#include "triso.h"
#include "kdr1000.h"
#include "config.h"

//void
//badge_interrupt_init(void)
//{
///* macro defined in pinning/internals/header.m4, 
 //* invoked by pinning/hardware/tdbase.m4
 //* configures Pin Change Interrupt 
 //* */
   //badge_configure_pcint();
//}

ISR(BADGE_VECTOR)
{
#ifdef INOUT_TRISO_SUPPORT
	inout_triso_interrupt();
#endif
#ifdef KDE_KDR1000_SUPPORT
	kde_kdr1000_interrupt();
#endif
}

/*
  -- Ethersex META --
*/
