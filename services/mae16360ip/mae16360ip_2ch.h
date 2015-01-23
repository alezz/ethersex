/*
 * Copyright (c) 2014 by Alessandro Mauro <alez@maetech.it>
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

#ifndef HAVE_MAE16360IP_H
#define HAVE_MAE16360IP_H

void
mae_init(void);

void
mae_net_init(void);

void
fobos_newdata(void);

void
show_welcome(void);

void
send_number(uint16_t number0, uint16_t number1);

void
send_2byte(uint8_t byte0, uint8_t byte1);

uint8_t
ito7s(uint8_t digit);

void
fobos_net_handle(void);

#include <util/delay.h>
/* #define MAE_DELAY _delay_ms(1) */
#define MAE_DELAY 

#include "config.h"
#ifdef DEBUG_MAE16360IP
# include "core/debug.h"
# define MAE_DEBUG(a...)  debug_printf("mae16360ip: " a)
#else
# define MAE_DEBUG(a...)
#endif

#endif  /* HAVE_MAE16360IP_H */
