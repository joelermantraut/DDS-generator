//--------------------------------------------------------------
// File     : stm32_ub_lcd_2x16.c
// Datum    : 15.04.2013
// Version  : 1.0
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F4
// IDE      : CooCox CoIDE 1.7.0
// Module   : GPIO
// Funktion : Text-LCD Funktionen (2x16 Zeichen)
//            im 4Bit-Mode
//            Chip : ST7066U/HD44780/SED1278/KS0066U/S6A0069X
// 
// Hinweis  : Das Display benutzt die CPU-Pins :
//             PE5  -> LCD_RS
//             PE6  -> LCD_E
//             PE7  -> LCD_DB4
//             PE8  -> LCD_DB5
//             PE9  -> LCD_DB6
//             PE10 -> LCD_DB7
//--------------------------------------------------------------

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_lcd_2x16.h"

/* Defines */
#define PORT_DISPLAY_RS GPIOB
#define PORT_DISPLAY_E  GPIOE
#define PORT_DISPLAY_D4 GPIOE
#define PORT_DISPLAY_D5 GPIOE
#define PORT_DISPLAY_D6 GPIOE
#define PORT_DISPLAY_D7 GPIOC
// PUERTOS
#define PIN_DISPLAY_RS GPIO_Pin_8
#define PIN_DISPLAY_E  GPIO_Pin_5
#define PIN_DISPLAY_D4 GPIO_Pin_4
#define PIN_DISPLAY_D5 GPIO_Pin_6
#define PIN_DISPLAY_D6 GPIO_Pin_2
#define PIN_DISPLAY_D7 GPIO_Pin_13
// PINES
#define PERIPH_DISPLAY_RS RCC_AHB1Periph_GPIOB
#define PERIPH_DISPLAY_E  RCC_AHB1Periph_GPIOE
#define PERIPH_DISPLAY_D4 RCC_AHB1Periph_GPIOE
#define PERIPH_DISPLAY_D5 RCC_AHB1Periph_GPIOE
#define PERIPH_DISPLAY_D6 RCC_AHB1Periph_GPIOE
#define PERIPH_DISPLAY_D7 RCC_AHB1Periph_GPIOC
// PERIFERICOS


//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
void P_LCD_2x16_InitIO(void);
void P_LCD_2x16_PinLo(TLCD_NAME_t lcd_pin);
void P_LCD_2x16_PinHi(TLCD_NAME_t lcd_pin);
void P_LCD_2x16_Clk(void);
void P_LCD_2x16_InitSequenz(void);
void P_LCD_2x16_Cmd(uint8_t wert);
void P_LCD_2x16_Data(uint8_t wert);
void P_LCD_2x16_Cursor(uint8_t x, uint8_t y);
void P_LCD_2x16_Delay(volatile uint32_t nCount);
 


//--------------------------------------------------------------
// Definition aller Pins für das Display
// Reihenfolge wie bei TLCD_NAME_t
//
// Init : [Bit_SET,Bit_RESET]
//--------------------------------------------------------------
LCD_2X16_t LCD_2X16[] = {
 // Name   ,PORT , PIN       , CLOCK              , Init
  {TLCD_RS , PORT_DISPLAY_RS , PIN_DISPLAY_RS , PERIPH_DISPLAY_RS , Bit_RESET},
  {TLCD_E  , PORT_DISPLAY_E  , PIN_DISPLAY_E  , PERIPH_DISPLAY_E  , Bit_RESET},
  {TLCD_D4 , PORT_DISPLAY_D4 , PIN_DISPLAY_D4 , PERIPH_DISPLAY_D4 , Bit_RESET},
  {TLCD_D5 , PORT_DISPLAY_D5 , PIN_DISPLAY_D5 , PERIPH_DISPLAY_D5 , Bit_RESET},
  {TLCD_D6 , PORT_DISPLAY_D6 , PIN_DISPLAY_D6 , PERIPH_DISPLAY_D6 , Bit_RESET},
  {TLCD_D7 , PORT_DISPLAY_D7 , PIN_DISPLAY_D7 , PERIPH_DISPLAY_D7 , Bit_RESET},
};
// ESTO ES LO UNICO QUE HACE FALTA CAMBIAR


//--------------------------------------------------------------
// Init vom Text-LCDisplay
//--------------------------------------------------------------
void UB_LCD_2x16_Init(void)
{
  // init aller IO-Pins
  P_LCD_2x16_InitIO();
  // kleine Pause
  P_LCD_2x16_Delay(TLCD_INIT_PAUSE);
  // Init Sequenz starten
  P_LCD_2x16_InitSequenz();
  // LCD-Settings einstellen
  P_LCD_2x16_Cmd(TLCD_CMD_INIT_DISPLAY);
  P_LCD_2x16_Cmd(TLCD_CMD_ENTRY_MODE);
  // Display einschalten
  P_LCD_2x16_Cmd(TLCD_CMD_DISP_M1);
  // Display löschen
  P_LCD_2x16_Cmd(TLCD_CMD_CLEAR);
  // kleine Pause
  P_LCD_2x16_Delay(TLCD_PAUSE);
}


//--------------------------------------------------------------
// Löscht das Text-LCDisplay
//--------------------------------------------------------------
void UB_LCD_2x16_Clear(void)
{
  // Display löschen
  P_LCD_2x16_Cmd(TLCD_CMD_CLEAR);
  // kleine Pause
  P_LCD_2x16_Delay(TLCD_PAUSE);
}


//--------------------------------------------------------------
// Stellt einen Display Mode ein
// mode : [TLCD_OFF, TLCD_ON, TLCD_CURSOR, TLCD_BLINK]
//--------------------------------------------------------------
void UB_LCD_2x16_SetMode(TLCD_MODE_t mode)
{
  if(mode==TLCD_OFF) P_LCD_2x16_Cmd(TLCD_CMD_DISP_M0);
  if(mode==TLCD_ON) P_LCD_2x16_Cmd(TLCD_CMD_DISP_M1);
  if(mode==TLCD_CURSOR) P_LCD_2x16_Cmd(TLCD_CMD_DISP_M2);
  if(mode==TLCD_BLINK) P_LCD_2x16_Cmd(TLCD_CMD_DISP_M3);
}


//--------------------------------------------------------------
// Ausgabe von einem String auf dem Display an x,y Position
// x : 0 bis 15
// y : 0 bis 1
//--------------------------------------------------------------
void UB_LCD_2x16_String(uint8_t x, uint8_t y, char *ptr)
{
  // Cursor setzen
  P_LCD_2x16_Cursor(x,y);
  // kompletten String ausgeben
  while (*ptr != 0) {
    P_LCD_2x16_Data(*ptr);
    ptr++;
  }
}


//--------------------------------------------------------------
// interne Funktion
// init aller IO-Pins
//--------------------------------------------------------------
void P_LCD_2x16_InitIO(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  TLCD_NAME_t lcd_pin;
  
  for(lcd_pin=0;lcd_pin<TLCD_ANZ;lcd_pin++) {
    // Clock Enable
    RCC_AHB1PeriphClockCmd(LCD_2X16[lcd_pin].TLCD_CLK, ENABLE);

    // Config als Digital-Ausgang
    GPIO_InitStructure.GPIO_Pin = LCD_2X16[lcd_pin].TLCD_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LCD_2X16[lcd_pin].TLCD_PORT, &GPIO_InitStructure);

    // Default Wert einstellen
    if(LCD_2X16[lcd_pin].TLCD_INIT==Bit_RESET) {
      P_LCD_2x16_PinLo(lcd_pin);
    }
    else {
      P_LCD_2x16_PinHi(lcd_pin);
    }
  }  
}


//--------------------------------------------------------------
// interne Funktion
// Pin auf Lo setzen
//--------------------------------------------------------------
void P_LCD_2x16_PinLo(TLCD_NAME_t lcd_pin)
{
  LCD_2X16[lcd_pin].TLCD_PORT->BSRRH = LCD_2X16[lcd_pin].TLCD_PIN;
}


//--------------------------------------------------------------
// interne Funktion
// Pin auf Hi setzen
//--------------------------------------------------------------
void P_LCD_2x16_PinHi(TLCD_NAME_t lcd_pin)
{
  LCD_2X16[lcd_pin].TLCD_PORT->BSRRL = LCD_2X16[lcd_pin].TLCD_PIN;
}


//--------------------------------------------------------------
// interne Funktion
// einen Clock Impuls ausgeben
//--------------------------------------------------------------
void P_LCD_2x16_Clk(void)
{
  // Pin-E auf Hi
  P_LCD_2x16_PinHi(TLCD_E);
  // kleine Pause
  P_LCD_2x16_Delay(TLCD_CLK_PAUSE);
  // Pin-E auf Lo
  P_LCD_2x16_PinLo(TLCD_E);
  // kleine Pause
  P_LCD_2x16_Delay(TLCD_CLK_PAUSE);  
}


//--------------------------------------------------------------
// interne Funktion
// init Sequenz für das Display
//--------------------------------------------------------------
void P_LCD_2x16_InitSequenz(void)
{
  // Init Sequenz
  P_LCD_2x16_PinHi(TLCD_D4);
  P_LCD_2x16_PinHi(TLCD_D5);
  P_LCD_2x16_PinLo(TLCD_D6);
  P_LCD_2x16_PinLo(TLCD_D7);
  // Erster Init Impuls
  P_LCD_2x16_Clk();
  P_LCD_2x16_Delay(TLCD_PAUSE);
  // Zweiter Init Impuls
  P_LCD_2x16_Clk();
  P_LCD_2x16_Delay(TLCD_PAUSE);
  // Dritter Init Impuls
  P_LCD_2x16_Clk();
  P_LCD_2x16_Delay(TLCD_PAUSE);
  // LCD-Modus einstellen (4Bit-Mode)
  P_LCD_2x16_PinLo(TLCD_D4);
  P_LCD_2x16_PinHi(TLCD_D5);
  P_LCD_2x16_PinLo(TLCD_D6);
  P_LCD_2x16_PinLo(TLCD_D7);
  P_LCD_2x16_Clk();
  P_LCD_2x16_Delay(TLCD_PAUSE);
}


//--------------------------------------------------------------
// interne Funktion
// Kommando an das Display senden
//--------------------------------------------------------------
void P_LCD_2x16_Cmd(uint8_t wert)
{
  // RS=Lo (Command)
  P_LCD_2x16_PinLo(TLCD_RS);
  // Hi-Nibble ausgeben         
  if((wert&0x80)!=0) P_LCD_2x16_PinHi(TLCD_D7); else P_LCD_2x16_PinLo(TLCD_D7);
  if((wert&0x40)!=0) P_LCD_2x16_PinHi(TLCD_D6); else P_LCD_2x16_PinLo(TLCD_D6);
  if((wert&0x20)!=0) P_LCD_2x16_PinHi(TLCD_D5); else P_LCD_2x16_PinLo(TLCD_D5);
  if((wert&0x10)!=0) P_LCD_2x16_PinHi(TLCD_D4); else P_LCD_2x16_PinLo(TLCD_D4);
  P_LCD_2x16_Clk();
  // Lo-Nibble ausgeben         
  if((wert&0x08)!=0) P_LCD_2x16_PinHi(TLCD_D7); else P_LCD_2x16_PinLo(TLCD_D7);
  if((wert&0x04)!=0) P_LCD_2x16_PinHi(TLCD_D6); else P_LCD_2x16_PinLo(TLCD_D6);
  if((wert&0x02)!=0) P_LCD_2x16_PinHi(TLCD_D5); else P_LCD_2x16_PinLo(TLCD_D5);
  if((wert&0x01)!=0) P_LCD_2x16_PinHi(TLCD_D4); else P_LCD_2x16_PinLo(TLCD_D4);
  P_LCD_2x16_Clk();  
}


//--------------------------------------------------------------
// interne Funktion
// Daten an das Display senden
//--------------------------------------------------------------
void P_LCD_2x16_Data(uint8_t wert)
{
  // RS=Hi (Data)
  P_LCD_2x16_PinHi(TLCD_RS);
  // Hi-Nibble ausgeben          
  if((wert&0x80)!=0) P_LCD_2x16_PinHi(TLCD_D7); else P_LCD_2x16_PinLo(TLCD_D7);
  if((wert&0x40)!=0) P_LCD_2x16_PinHi(TLCD_D6); else P_LCD_2x16_PinLo(TLCD_D6);
  if((wert&0x20)!=0) P_LCD_2x16_PinHi(TLCD_D5); else P_LCD_2x16_PinLo(TLCD_D5);
  if((wert&0x10)!=0) P_LCD_2x16_PinHi(TLCD_D4); else P_LCD_2x16_PinLo(TLCD_D4);
  P_LCD_2x16_Clk();
  // Lo-Nibble ausgeben        
  if((wert&0x08)!=0) P_LCD_2x16_PinHi(TLCD_D7); else P_LCD_2x16_PinLo(TLCD_D7);
  if((wert&0x04)!=0) P_LCD_2x16_PinHi(TLCD_D6); else P_LCD_2x16_PinLo(TLCD_D6);
  if((wert&0x02)!=0) P_LCD_2x16_PinHi(TLCD_D5); else P_LCD_2x16_PinLo(TLCD_D5);
  if((wert&0x01)!=0) P_LCD_2x16_PinHi(TLCD_D4); else P_LCD_2x16_PinLo(TLCD_D4);
  P_LCD_2x16_Clk();  
}


//--------------------------------------------------------------
// interne Funktion
// Cursor auf x,y stellen
//--------------------------------------------------------------
void P_LCD_2x16_Cursor(uint8_t x, uint8_t y)
{
  uint8_t wert;

  if(x>=TLCD_MAXX) x=0;
  if(y>=TLCD_MAXY) y=0;

  wert=(y<<6);
  wert|=x;
  wert|=0x80;
  P_LCD_2x16_Cmd(wert);
}


//--------------------------------------------------------------
// kleine Pause (ohne Timer)
//--------------------------------------------------------------
void P_LCD_2x16_Delay(volatile uint32_t nCount)
{
  while(nCount--)
  {
  }
}
