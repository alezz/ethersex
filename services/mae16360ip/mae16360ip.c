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

#include <avr/pgmspace.h>

#include <util/delay.h>
#include <util/atomic.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "mae16360ip.h"

#include "protocols/ecmd/ecmd-base.h"

#include "protocols/uip/uip.h"

uip_conn_t *fobos_conn = NULL;
#define FOBOS_PORT 1636
#define FOBOS_BUFFER_LEN 128
#define FOBOS_VER 0x01

struct fobos_buffer
{
  uint16_t len;
  uint16_t sent;
  uint8_t data[FOBOS_BUFFER_LEN];
};

/*struct fobos_buffer fobos_send_buffer;*/
struct fobos_buffer fobos_recv_buffer;

uint8_t fobos_option;
enum { FOBOS_OPT_DASHES = 0, FOBOS_OPT_NOTIFY_ALSO_DEC=1, FOBOS_OPT_3RDDOT=2, FOBOS_OPT_EXPLICIT_ZERO_GREEN=3,
  FOBOS_OPT_EXPLICIT_ZERO_RED=4, FOBOS_OPT_NOBLINK=5, FOBOS_OPT_NOBEEP=6, FOBOS_OPT_BLINKBEEP_ALSO_GREEN=7 };
uint16_t mae_numbers[16]; /* contain a copy of the values, to make comparisons */
uint8_t mae_blink[16];  /* counters for blinking, if > 0 that number should blink */
uint8_t mae_beep;  /* counter for beeping, if > 0 i should beep */

#define MAE_BLINK_LOOPS 5     /* how many times to blink */
#define MAE_BEEP_LOOPS 2    /* how many times to beep */

void
mae_init(void)
{
  MAE_DEBUG ("MAE_init\n");
  DDRC=0b00011111;
  PORTC=0b00001000;
  DDRA|=(1<<7);
  PORTA&=~(1<<7);
  for (uint8_t i=0; i<16; i++)
  {
    mae_numbers[i]=0;
    mae_blink[i]=0;
  }
  mae_beep=0;
}

void
mae_net_init(void)
{
  MAE_DEBUG ("MAE_NET_init\n");
  uip_listen(HTONS(FOBOS_PORT),fobos_net_handle);
}

void
fobos_net_handle(void)
{
  /*MAE_DEBUG ("MAE TCP HANDLE\n");*/
  if (uip_connected())
  {
    if (fobos_conn == NULL)
    {
      fobos_conn = uip_conn;
      uip_conn->wnd = FOBOS_BUFFER_LEN - 1;
      /* delete the receive buffer */
      fobos_recv_buffer.len = 0;
    }
    else
      /* if we already have a connection, send an error */
      uip_send("ERROR: connection blocked\n", 27);
  }
  else if (uip_acked())
  {
    /* if the peer is not our connection, close it */
    if (fobos_conn != uip_conn)
      uip_close();
    else
    {
      /* data we have sent was acked */
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
      {
        fobos_recv_buffer.len -= fobos_recv_buffer.sent;
        memmove(fobos_recv_buffer.data,
                fobos_recv_buffer.data + fobos_recv_buffer.sent,
                fobos_recv_buffer.len);
      }
    }
  }
  else if (uip_closed() || uip_aborted() || uip_timedout())
  {
    /* if the closed connection was our connection, clean yport_conn */
    if (fobos_conn == uip_conn)
      fobos_conn = NULL;
  }
  else if (uip_newdata())
  {
    MAE_DEBUG ("newdata\n");
    memcpy(fobos_recv_buffer.data,uip_appdata,uip_len);
    fobos_recv_buffer.len=uip_len;
    MAE_DEBUG ("data: %s END\n",fobos_recv_buffer.data);
    MAE_DEBUG ("lenght: %d\n",fobos_recv_buffer.len);
    fobos_newdata();
  }

  /* retransmit last packet */
  if (uip_rexmit() && fobos_conn == uip_conn)
  {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      uip_send(fobos_recv_buffer.data, fobos_recv_buffer.sent);
    }

  }
  else
  {

    /* restart connection */
    if (uip_poll()
        && fobos_conn == uip_conn
        && uip_stopped(fobos_conn)
        //&& fobos_send_buffer.sent == fobos_send_buffer.len
        )
      uip_restart();

  
    /* send data */
    /*
    if ((uip_poll() || uip_acked()) && fobos_conn == uip_conn ) */
        /* receive buffer reached water mark */
    {
      /* we have enough uart data, send it via tcp */
      /*
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
      {
        uip_send(fobos_recv_buffer.data, fobos_recv_buffer.len);
        fobos_recv_buffer.sent = fobos_recv_buffer.len;
      }*/
    }
    
  }
}

void
fobos_newdata(void)
{
  if (fobos_recv_buffer.len<8) return;
  if (fobos_recv_buffer.data[0]!='M') return;
  if (fobos_recv_buffer.data[1]!='a') return;
  if (fobos_recv_buffer.data[2]!='E') return;
  if (fobos_recv_buffer.data[3]!='-') return;
  if (fobos_recv_buffer.data[4]!='C') return;
  if (fobos_recv_buffer.data[5]!='P') return;
  if (fobos_recv_buffer.data[6]<FOBOS_VER) return;
  if (fobos_recv_buffer.data[7]==0x10) /* "refresh all" command */
  {
    if (fobos_recv_buffer.len<42){
      uip_send("too short\n",10);
      return;
    }
    fobos_option = fobos_recv_buffer.data[8];
    //    fobos_dimmer = fobos_recv_buffer.data[9];   /* not implemented by now */
    
    /* data */
    uint8_t i, q;
    uint16_t value;
    
    // firstly, check if something has changed or not
    
    for (i=0, q=41; i<16;i++, q-=2)
    {
      value = (((uint16_t)fobos_recv_buffer.data[q-1])<<8) + fobos_recv_buffer.data[q];
      if (value!=mae_numbers[i]) 
      {
        /* blink notification */
        if (!(fobos_option & (1<<FOBOS_OPT_NOBLINK)))   /* if blink has not been disabled */
          if ((!(i&1))||(fobos_option&(1<<FOBOS_OPT_BLINKBEEP_ALSO_GREEN)))  /* if red or also green */
            if ((fobos_option & (1<<FOBOS_OPT_NOTIFY_ALSO_DEC)) || (value>mae_numbers[i]) )  /* if inc or also dec */
              mae_blink[i]=MAE_BLINK_LOOPS*2;
        /* beep notification */
        if (!(fobos_option & (1<<FOBOS_OPT_NOBEEP)))  /* if beep has not been disabled */
          if ((!(i&1))||(fobos_option&(1<<FOBOS_OPT_BLINKBEEP_ALSO_GREEN)))  /* if red or also green */
            if ((fobos_option & (1<<FOBOS_OPT_NOTIFY_ALSO_DEC)) || (value>mae_numbers[i]) )  /* if inc or also dec */
              mae_beep=MAE_BEEP_LOOPS*2;
        /* change */
        mae_numbers[i]=value;
      }
    }
    
    PORTC |= 1;
    for (i=0; i<16;i++)
    {
      if ((mae_numbers[i]==0) && (
        ((!(fobos_option&(1<<FOBOS_OPT_EXPLICIT_ZERO_GREEN))) && (i&1))||
        ((!(fobos_option&(1<<FOBOS_OPT_EXPLICIT_ZERO_RED))) && (!(i&1)))
        ))
      {
        send_byte((fobos_option&(1<<FOBOS_OPT_3RDDOT)) <<5);
        send_byte(0);
        send_byte(0);
      }
      else
        send_number(mae_numbers[i],fobos_option);
    }
    PORTC &=~ 1;
    uip_send("ok\n",3);
  }
}

void
send_number(uint16_t number, uint8_t option)
{
  MAE_DEBUG("send_number: %d\n",number);
  uint8_t c,d;
  if ((number==0)&&(option & (1<<FOBOS_OPT_DASHES)))
    {
      /* show "---" */
      send_byte(ito7s(0x10)+( (option&(1<<FOBOS_OPT_3RDDOT)) <<5) );
      send_byte(ito7s(0x10));
      send_byte(ito7s(0x10));
    }
  else if (number==0) 
    {
      /* show "  0" */
      send_byte(ito7s(0)+( (option&(1<<FOBOS_OPT_3RDDOT)) <<5) );
      send_byte(0);
      send_byte(0);
    }
  else
    {
      c=number/100;
      d=(number-(c*100))/10;
      send_byte(ito7s(number-(c*100)-(d*10))+( (option&(1<<FOBOS_OPT_3RDDOT)) <<5) );
      if ((c==0) && (d==0))
        send_byte(0);
      else
        send_byte(ito7s(d));
      if (c==0)
        send_byte(0);
      else
        send_byte(ito7s(c));
    }
}

void
show_welcome(void)
{
  uip_ipaddr_t hostaddr;
  uip_gethostaddr(&hostaddr);
  uint8_t *ip = (uint8_t *) hostaddr;
  MAE_DEBUG("host addr: %u.%u.%u.%u\n",ip[0],ip[1],ip[2],ip[3]);
  for (uint8_t i=0;i<16;i++)
  {
    if (i== 6) {
      send_byte(0b01101110);
      send_byte(0b01111011);
      send_byte(0b00111111);
    }
    else if (i==4) {
      send_byte(0b01101110);
      send_byte(0);
      send_byte(0b01101101);
    }
    else if (i==2) {
      send_byte(0b00011000);
      send_byte(0b01101111);
      send_byte(0b01111011);
    }
    else if (i==0) {
      send_byte(0b00111100);
      send_byte(0b01000000);
      send_byte(0b11111011);
    }
    else if (i==15) send_number(ip[0],1<<FOBOS_OPT_3RDDOT);
    else if (i==13) send_number(ip[1],1<<FOBOS_OPT_3RDDOT);
    else if (i==11) send_number(ip[2],1<<FOBOS_OPT_3RDDOT);
    else if (i==9) send_number(ip[3],0);
    else {
      send_byte(0);
      send_byte(0);
      send_byte(0);
    }
    
  }
}

void
send_byte(uint8_t byte)
{
  for (uint8_t i =0; i<8;i++)
  {
    if ((byte & 1)==1)
      PORTC|=(1<<2);
    else
      PORTC&=~(1<<2);
    MAE_DELAY;
    PORTC|=(1<<1);
    MAE_DELAY;
    PORTC&=~(1<<1);
    MAE_DELAY;
    byte=byte>>1;
  }
}

uint8_t
ito7s(uint8_t digit)
{
  switch (digit)
  {
    case 0: return 0b01110111; break;
    case 1: return 0b01000001; break;
    case 2: return 0b00111011; break;
    case 3: return 0b01101011; break;
    case 4: return 0b01001101; break;
    case 5: return 0b01101110; break;
    case 6: return 0b01111110; break;
    case 7: return 0b01000011; break;
    case 8: return 0b01111111; break;
    case 9: return 0b01101111; break;
    case 0x0A: return 0b01011111; break;
    case 0x0B: return 0b01111100; break;
    case 0x0C: return 0b00110110; break;
    case 0x0D: return 0b01110001; break;
    case 0x0E: return 0b00111110; break;
    case 0x0F: return 0b00011110; break;
    case 0x10: return 0b00001000; break;  // '-'
    default: return 0;
  }
}

void
mae_timer(void)
{
  MAE_DEBUG ("tick\n");
  /* called every 25*20ms = 0.5s - ethersex does the timing */
  uint8_t i;
  uint8_t flag=0;
  if (mae_beep>0)
  {
    if (mae_beep & 1) PORTA&=~(1<<7);
    else PORTA |= (1<<7);
    mae_beep--;
  }
  for (i=0; i<16;i++)
  {
    if (mae_blink[i]>0)
    {
      flag=1;
      mae_blink[i]--;
    }
  }
  if (flag) /* refresh only if have to blink */
  {
    PORTC |= 1;
    for (i=0; i<16;i++)
    {
      if (mae_blink[i]&1)  /*odd*/
        {
          send_byte(0);
          send_byte(0);
          send_byte(0);
        }
      else      /*even or zero*/
        send_number(mae_numbers[i],fobos_option);
    }
    PORTC &=~ 1;
  }  
  
}

/*
  -- Ethersex META --
  header(services/mae16360ip/mae16360ip.h)
  init(mae_init)
  net_init(mae_net_init)
  startup(show_welcome)
  timer(25,mae_timer())
*/
