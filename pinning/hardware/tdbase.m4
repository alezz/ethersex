
/* (c) 2018 Alessandro Mauro <alez@maetech.it> */

/* port the enc28j60 is attached to */
pin(SPI_CS_NET, PB4, OUTPUT)

ifdef(`conf_SD_READER', `dnl
  /* port the sd-reader CS is attached to */
  pin(SPI_CS_SD_READER, PB5, OUTPUT)
')dnl

ifdef(`conf_STATUSLED_POWER', `dnl
pin(STATUSLED_POWER, PF1, OUTPUT)
')dnl

ifdef(`conf_STATUSLED_BOOTED', `dnl
pin(STATUSLED_BOOTED, PF1, OUTPUT)
')dnl

ifdef(`conf_STATUSLED_NETLINK', `dnl
pin(STATUSLED_NETLINK, PF2, OUTPUT)
')dnl

ifdef(`conf_STATUSLED_HB_ACT', `dnl
pin(STATUSLED_HB_ACT, PF2, OUTPUT)
')dnl

ifdef(`conf_HD44780', `
  pin(HD44780_RS, PG2)
  pin(HD44780_EN1, PD7)
  pin(HD44780_D0, PC0)
  pin(HD44780_D1, PC1)
  pin(HD44780_D2, PC2)
  pin(HD44780_D3, PC3)
  pin(HD44780_D4, PC4)
  pin(HD44780_D5, PC5)
  pin(HD44780_D6, PC6)
  pin(HD44780_D7, PC7)
')
ifdef(`conf_HD44780_BACKLIGHT', `
  pin(HD44780_BL, PE3, OUTPUT)
')

/* hardware/rfid/inout/triso.c */
/* Strobe and Presence PCINT must be same PCIx */

ifdef(`conf_INOUT_TRISO', `dnl
pin(INOUT_TRISO_STROBE, PK2, INPUT)
pin(INOUT_TRISO_DATA, PK3, INPUT)
pin(INOUT_TRISO_PRES, PK4, INPUT)
RFID_USE_PCINT_STROBE_PRES(18,20)
')dnl
