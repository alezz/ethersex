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

#ifndef _TRISO_H
#define _TRISO_H

#include "config.h"

#ifdef INOUT_TRISO_SUPPORT

#if !defined(F_CPU)
#error "F_CPU undefined!"
#endif


#ifdef DEBUG_INOUT_TRISO
    # include "core/debug.h"
    # define INOUT_DEBUG(a...)  debug_printf("RFID: " a)
#else
    # define INOUT_DEBUG(a...)
#endif


void inout_triso_init(void);
void inout_triso_interrupt(void);


#define HOOK_NAME rfid_triso_read
#define HOOK_ARGS (uint8_t data[16])
#include "hook.def"
#undef HOOK_NAME
#undef HOOK_ARGS




#endif /* INOUT_TRISO_SUPPORT */
#endif /* _TRISO_H */
