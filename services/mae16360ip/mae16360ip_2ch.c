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
#define FOBOS_BUFFER_LEN 128
#define FOBOS_VER 0x01

#define MAE_PORT PORTC
#define MAE_DDR DDRC
#define MAE_PIN_EN0 (1<<0)
#define MAE_PIN_CLK0 (1<<1)
#define MAE_PIN_DATA0 (1<<2)
#define MAE_PIN_LE0 (1<<3)
#define MAE_PIN_EN1 (1<<4)
#define MAE_PIN_CLK1 (1<<5)
#define MAE_PIN_DATA1 (1<<6)
#define MAE_PIN_LE1 (1<<7)

struct fobos_buffer
{
  uint16_t len;
  uint16_t sent;
  uint8_t data[FOBOS_BUFFER_LEN];
};

/*struct fobos_buffer fobos_send_buffer;*/
struct fobos_buffer fobos_recv_buffer;

uint8_t fobos_option;

void
mae_init(void)
{
  MAE_DEBUG ("MAE_init\n");
  MAE_DDR=0b11111111;
  MAE_PORT=MAE_PIN_EN0 & MAE_PIN_EN1;
}

void
mae_net_init(void)
{
  MAE_DEBUG ("MAE_NET_init\n");
  MAE_DDR=0b00001111;  /* i have to set this again here, dont know why */
  MAE_PORT=MAE_PIN_LE0 & MAE_PIN_LE1; /* enable latches and output */
  show_welcome();
  uip_listen(HTONS(4455),fobos_net_handle);
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
    /*
    if (uip_len <= YPORT_BUFFER_LEN ) &&
        yport_rxstart(uip_appdata, uip_len) != 0)
    {*/
      /* prevent the other side from sending more data via tcp */
      /*uip_stop();
    }
    */
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
    //    fobos_dimmer = fobos_recv_buffer.data[9];
    
    /* data */
    MAE_PORT |= MAE_PIN_EN0 & MAE_PIN_EN1;
    uint8_t i;
    uint16_t value0, value1;
    for (i=41; i>9;i-=4)
    {
      value0 = (((uint16_t)fobos_recv_buffer.data[i-1])<<8) + fobos_recv_buffer.data[i-0];
      value1 = (((uint16_t)fobos_recv_buffer.data[i-3])<<8) + fobos_recv_buffer.data[i-2];
      send_number(value0, value1);
    }
    MAE_PORT &=~ (MAE_PIN_EN0 & MAE_PIN_EN1);
    uip_send("ok\n",3);
  }
}

void
send_number(uint16_t number0, uint16_t number1)
{
  MAE_DEBUG("send_number: %d\n",number);
  uint8_t c,d;
  if ((number==0)&&(option & (1<<0)))
    {
      send_byte(ito7s(0x10)+( (option&(1<<2)) <<5) );
      send_byte(ito7s(0x10));
      send_byte(ito7s(0x10));
    }
    else if ((number==0) && (!(option & (1<<1))))
    {
      send_byte(ito7s(0)+( (option&(1<<2)) <<5) );
      send_byte(0);
      send_byte(0);
    }
    else
    {
      c=number/100;
      d=(number-(c*100))/10;
      send_byte(ito7s(number-(c*100)-(d*10))+( (option&(1<<2)) <<5) );
      if ((c==0) && (d==0) && (!(option & (1<<1))))
        send_byte(0);
      else
        send_byte(ito7s(d));
      if ((c==0) && (!(option & (1<<1))))
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
      send_byte(0b00111111);
      send_byte(0b01111011);
      send_byte(0b01101110);
    }
    else if (i==4) {
      send_byte(0b01101101);
      send_byte(0);
      send_byte(0b01101110);
    }
    else if (i==2) {
      send_byte(0b01111011);
      send_byte(0b01101111);
      send_byte(0b00011000);
    }
    else if (i==0) {
      send_byte(0b11111011);
      send_byte(0b01000000);
      send_byte(0b00111100);
    }
    else if (i==15) send_number(ip[0],4);
    else if (i==13) send_number(ip[1],4);
    else if (i==11) send_number(ip[2],4);
    else if (i==9) send_number(ip[3],0);
    else {
      send_byte(0);
      send_byte(0);
      send_byte(0);
    }
    
  }
}

void
send_2byte(uint8_t byte0, uint8_t byte1)
{
  for (uint8_t i =0; i<8;i++)
  {
    /* prepare data bit */
    if ((byte0 & 1)==1)
      MAE_PORT|=MAE_PIN_DATA0;
    else
      MAE_PORT&=~MAE_PIN_DATA0;
    if ((byte1 & 1)==1)
      MAE_PORT|=MAE_PIN_DATA1;
    else
      MAE_PORT&=~MAE_PIN_DATA1;
    MAE_DELAY;
    /* clock rise */
    MAE_PORT|=MAE_PIN_CLK0 & MAE_PIN_CLK1;
    MAE_DELAY;
    /* clock fall */
    MAE_PORT&=~(MAE_PIN_CLK0 & MAE_PIN_CLK1);
    MAE_DELAY;
    /* next */
    byte0=byte0>>1;
    byte1=byte1>>1;
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

/*
  -- Ethersex META --
  header(services/mae16360ip/mae16360ip.h)
  init(mae_init)
  net_init(mae_net_init)
*/
