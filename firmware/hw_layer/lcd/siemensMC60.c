/*
 * siemensMC60.c
 *
 *  Created on: 20.05.2014
 *      Author: rus084
 */
#include "main.h"
#include "pin_repository.h"
#include "string.h"
#include "siemensMC60.h"
#include "engine_configuration.h"
#include "hal.h"

#define CMD 	0
#define DAT 	1


#define GREEN       0x1C
#define DARK_GREEN  0x15
#define RED         0xE0
#define BLUE        0x1F
#define DARK_BLUE   0x03
#define YELLOW      0xFC
#define ORANGE      0xEC
#define VIOLET      0xE3
#define WHITE       0xFF
#define BLACK       0x00
#define GREY        0x6D



#define LCD_CS_PORT GPIOE
#define LCD_CS_PIN 10
#define LCD_RESET_PORT GPIOE
#define LCD_RESET_PIN 12
#define LCD_RS_PORT GPIOE
#define LCD_RS_PIN 14
#define LCD_CLK_PORT GPIOB
#define LCD_CLK_PIN 10
#define LCD_DATA_PORT GPIOB
#define LCD_DATA_PIN 12


static const SPIConfig spi2cfg = {
  NULL,
  /* HW dependent part.*/
  GPIOB,
  12,
  SPI_CR1_BR_0 | SPI_CR1_BR_1 | SPI_CR1_BR_2
};

static unsigned char lcd_buff[1];




void lcdClock(void) {
	palSetPad(LCD_CLK_PORT, LCD_CLK_PIN);
	chThdSleepMicroseconds(1);
	palClearPad(LCD_CLK_PORT, LCD_CLK_PIN);
}




void lcdMC_reset(void)
{
 palClearPad(LCD_CS_PORT, LCD_CS_PIN);
 palClearPad(LCD_DATA_PORT, LCD_DATA_PIN);
 palClearPad(LCD_RESET_PORT, LCD_RESET_PIN);
 chThdSleepMilliseconds(100);
 palSetPad(LCD_RESET_PORT, LCD_RESET_PIN);
 chThdSleepMilliseconds(500);
 palSetPad(LCD_CS_PORT, LCD_CS_PIN);
 chThdSleepMicroseconds(15);
 palClearPad(LCD_CS_PORT, LCD_CS_PIN);
}

void Send_to_lcd(unsigned char RS, unsigned char data) {
	lcd_buff[0] = data ;
	palWritePad(LCD_RS_PORT, LCD_RS_PIN, RS);
    spiSelect(&SPID2);
    spiSend(&SPID2, 1, lcd_buff);
    spiUnselect(&SPID2);
}


/*void Send_to_lcd_soft(unsigned char RS, unsigned char data)
{
 //unsigned char count;
	palClearPad(LCD_CLK_PORT, LCD_CLK_PIN);
	palClearPad(LCD_DATA_PORT, LCD_DATA_PIN);

 palSetPad(LCD_CS_PORT, LCD_CS_PIN);
 palWritePad(LCD_RS_PORT, LCD_RS_PIN, RS);
 palClearPad(LCD_CS_PORT, LCD_CS_PIN);

// if (RS == 1)  palSetPad(LCD_DATA_PORT, LCD_DATA_PIN);
 //if (RS == 0) palClearPad(LCD_DATA_PORT, LCD_DATA_PIN);
 //lcdClock();

 palWritePad(LCD_DATA_PORT, LCD_DATA_PIN , data & 0x80 ? 1 : 0) ;
 lcdClock();
 palWritePad(LCD_DATA_PORT, LCD_DATA_PIN , data & 0x40 ? 1 : 0) ;
 lcdClock();
 palWritePad(LCD_DATA_PORT, LCD_DATA_PIN , data & 0x20 ? 1 : 0) ;
 lcdClock();
 palWritePad(LCD_DATA_PORT, LCD_DATA_PIN , data & 0x10 ? 1 : 0) ;
 lcdClock();
 palWritePad(LCD_DATA_PORT, LCD_DATA_PIN , data & 0x08 ? 1 : 0) ;
 lcdClock();
 palWritePad(LCD_DATA_PORT, LCD_DATA_PIN , data & 0x04 ? 1 : 0) ;
 lcdClock();
 palWritePad(LCD_DATA_PORT, LCD_DATA_PIN , data & 0x02 ? 1 : 0) ;
 lcdClock();
 palWritePad(LCD_DATA_PORT, LCD_DATA_PIN , data & 0x01 ? 1 : 0) ;
 lcdClock();


 palClearPad(LCD_DATA_PORT, LCD_DATA_PIN);
 palSetPad(LCD_CS_PORT, LCD_CS_PIN);
}
*/
void LCDmc60_init(void)
{
	  /*
	   * Initializes the SPI driver 2. The SPI2 signals are routed as follow:
	   * PB12 - NSS.
	   * PB13 - SCK.
	   * PB14 - MISO.
	   * PB15 - MOSI.
	   */
	  spiStart(&SPID2, &spi2cfg);
	  palSetPad(GPIOB, 12);
	  palSetPadMode(GPIOB, 12, PAL_MODE_OUTPUT_PUSHPULL |
	                           PAL_STM32_OSPEED_HIGHEST);           /* NSS.     */
	  palSetPadMode(GPIOB, 13, PAL_MODE_ALTERNATE(5) |
	                           PAL_STM32_OSPEED_HIGHEST);           /* SCK.     */
	  palSetPadMode(GPIOB, 14, PAL_MODE_ALTERNATE(5));              /* MISO.    */
	  palSetPadMode(GPIOB, 15, PAL_MODE_ALTERNATE(5) |
	                           PAL_STM32_OSPEED_HIGHEST);           /* MOSI.    */


	//mySetPadMode("lcd CS", LCD_CS_PORT, LCD_CS_PIN, PAL_MODE_OUTPUT_PUSHPULL);
	mySetPadMode("lcd RST", LCD_RESET_PORT, LCD_RESET_PIN, PAL_MODE_OUTPUT_PUSHPULL);
	mySetPadMode("lcd RS", LCD_RS_PORT, LCD_RS_PIN, PAL_MODE_OUTPUT_PUSHPULL);
	//mySetPadMode("lcd CLK", LCD_CLK_PORT, LCD_CLK_PIN, PAL_MODE_OUTPUT_PUSHPULL);
	//mySetPadMode("lcd DAT", LCD_DATA_PORT, LCD_DATA_PIN, PAL_MODE_OUTPUT_PUSHPULL);
 lcdMC_reset ();
 Send_to_lcd(CMD,0x01); //reset sw
 chThdSleepMilliseconds(50);

 Send_to_lcd(CMD,0xc6); //initial escape
 Send_to_lcd(CMD,0xb9); //Refresh set
 Send_to_lcd(DAT,0x00);

 chThdSleepMilliseconds(100);

 Send_to_lcd(CMD,0xb6); //Display control
 Send_to_lcd(DAT,0x80); //
 Send_to_lcd(DAT,0x04); //
 Send_to_lcd(DAT,0x0a); //
 Send_to_lcd(DAT,0x54); //
 Send_to_lcd(DAT,0x45); //
 Send_to_lcd(DAT,0x52); //
 Send_to_lcd(DAT,0x43); //

 Send_to_lcd(CMD,0xb3); //Gray scale position set 0
 Send_to_lcd(DAT,0x02); //
 Send_to_lcd(DAT,0x0a); //
 Send_to_lcd(DAT,0x15); //
 Send_to_lcd(DAT,0x1f); //
 Send_to_lcd(DAT,0x28); //
 Send_to_lcd(DAT,0x30); //
 Send_to_lcd(DAT,0x37); //
 Send_to_lcd(DAT,0x3f); //
 Send_to_lcd(DAT,0x47); //
 Send_to_lcd(DAT,0x4c); //
 Send_to_lcd(DAT,0x54); //
 Send_to_lcd(DAT,0x65); //
 Send_to_lcd(DAT,0x75); //
 Send_to_lcd(DAT,0x80); //
 Send_to_lcd(DAT,0x85); //

 Send_to_lcd(CMD,0xb5); //Gamma curve
 Send_to_lcd(DAT,0x01); //

 Send_to_lcd(CMD,0xbd); //Common driver output select
 Send_to_lcd(DAT,0x00); //
 Send_to_lcd(CMD,0xbe); //Power control
 Send_to_lcd(DAT,0x54); //0x58 before
 Send_to_lcd(CMD,0x11); //sleep out
 chThdSleepMilliseconds(100);
 Send_to_lcd(CMD,0xba); //Voltage control
 Send_to_lcd(DAT,0x2f); //
 Send_to_lcd(DAT,0x03); //

 Send_to_lcd(CMD,0x25); //Write contrast
 Send_to_lcd(DAT,0x60); //

 Send_to_lcd(CMD,0xb7); //Temperature gradient
 Send_to_lcd(DAT,0x00); //
 Send_to_lcd(DAT,0x00); //
 Send_to_lcd(DAT,0x00); //
 Send_to_lcd(DAT,0x00); //
 Send_to_lcd(DAT,0x00); //
 Send_to_lcd(DAT,0x00); //
 Send_to_lcd(DAT,0x00); //
 Send_to_lcd(DAT,0x00); //
 Send_to_lcd(DAT,0x00); //
 Send_to_lcd(DAT,0x00); //
 Send_to_lcd(DAT,0x00); //
 Send_to_lcd(DAT,0x00); //
 Send_to_lcd(DAT,0x00); //
 Send_to_lcd(DAT,0x00); //

 Send_to_lcd(CMD,0x03); //Booster voltage ON
 chThdSleepMilliseconds(100);
 Send_to_lcd(CMD,0x36); //Memory access control
 Send_to_lcd(DAT,0x48); //

 Send_to_lcd(CMD,0x2d); //Color set
 Send_to_lcd(DAT,0x00); //
 Send_to_lcd(DAT,0x03); //
 Send_to_lcd(DAT,0x05); //
 Send_to_lcd(DAT,0x07); //
 Send_to_lcd(DAT,0x09); //
 Send_to_lcd(DAT,0x0b); //
 Send_to_lcd(DAT,0x0d); //
 Send_to_lcd(DAT,0x0f); //
 Send_to_lcd(DAT,0x00); //
 Send_to_lcd(DAT,0x03); //
 Send_to_lcd(DAT,0x05); //
 Send_to_lcd(DAT,0x07); //
 Send_to_lcd(DAT,0x09); //
 Send_to_lcd(DAT,0x0b); //
 Send_to_lcd(DAT,0x0d); //
 Send_to_lcd(DAT,0x0f); //
 Send_to_lcd(DAT,0x00); //
 Send_to_lcd(DAT,0x05); //
 Send_to_lcd(DAT,0x0b); //
 Send_to_lcd(DAT,0x0f); //

 Send_to_lcd(CMD,0x3a); //interface pixel format
 Send_to_lcd(DAT,0x02); // 0x02 for 8-bit 0x03 for 12bit
 chThdSleepMilliseconds(100);

 Send_to_lcd(CMD,0x29); //Display ON
}

void SetArea(char x1, char x2, char y1, char y2) {
 Send_to_lcd( CMD, 0x2A );
 Send_to_lcd( DAT, x1 );
 Send_to_lcd( DAT, x2 );

 Send_to_lcd( CMD, 0x2B );
 Send_to_lcd( DAT, y1+1 );
 Send_to_lcd( DAT, y2+1 );

 Send_to_lcd( CMD, 0x2C );
}


void Put_Pixel(char x, char y, unsigned int color) {

 SetArea(x,x,y,y);
 Send_to_lcd(DAT,color);
}


/*


void Send_Symbol(unsigned char symbol, char x, char y, int t_color, int b_color, char zoom_width, char zoom_height, int rot)
{
 unsigned char temp_symbol, a, b, zw, zh, mask;
 char m, n;
 m=x;
 n=y;
 if (symbol>127) symbol-=64;    //Óáèðàåì îòñóòñòâóþùóþ ÷àñòü òàáëèöû ASCII
 for ( a = 0; a < 5; a++) //Ïåðåáèðàþ 5 áàéò, ñîñòàâëÿþùèõ ñèìâîë
 {
  temp_symbol = font_5x8[symbol-32][a];
  zw = 0;
  while(zw != zoom_width) //Âûâîä áàéòà âûïîëíÿåòñÿ zw ðàç
  {
   switch(rot)
   {
    case 0: case 180: n=y; break;
    case 90: case 270: m=x; break;
   }
   mask=0x01;
   for ( b = 0; b < 8; b++ ) //Öèêë ïåðåáèðàíèÿ 8 áèò áàéòà
   {
    zh = 0; //â zoom_height ðàç óâåëè÷èòñÿ âûñîòà ñèìâîëà
    while(zh != zoom_height) //Âûâîä ïèêñåëÿ âûïîëíÿåòñÿ z ðàç
    {
     switch(rot)
     {
      case 0: case 180:
      if (temp_symbol&mask)
      {
       Put_Pixel (m+zw, n+zh, t_color);
      }
      else
      {
       Put_Pixel (m+zw, n+zh, b_color);
      }
      break;
      case 90: case 270:
      if (temp_symbol&mask)
      {
       Put_Pixel (m+zh, n+zw, t_color);
      }
      else
      {
       Put_Pixel (m+zh, n+zw, b_color);
      }
      break; //Ïîëó÷èòü àäðåñ íà÷àëüíîãî ïèêñåëÿ ïî îñè y äëÿ âûâîäà î÷åðåäíîãî áàéòà
     }
     zh++;
    }
    mask<<=1; //Ñìåùàþ ñîäåðæèìîå mask íà 1 áèò âëåâî;
    switch(rot)
    {
     case 0: case 180: n=n+zoom_height; break;
     case 90: case 270: m=m+zoom_height; break;
    }
   }
   zw++;
  }
  switch(rot)
  {
   case 0: case 180: m=m+zoom_width; break;
   case 90: case 270: n=n+zoom_width; break;
  }
 }
}



void LCD_Putchar(char symbol, char x, char y, int t_color, int b_color, char zoom_width, char zoom_height, int rot)
{
    unsigned char m;
    if(zoom_width == 0)   zoom_width = 1;
    if(zoom_height == 0)  zoom_height = 1;
    switch (rot)
    {
        case 0:  //Íà÷àëüíûé àäðåñ îñåé Õ è Ó - ëåâûé âåðõíèé óãîë äèñïëåÿ
        Send_Symbol( symbol, x, y, t_color, b_color, zoom_width, zoom_height, rot);
        break;
        //================================
        case 90:
        m=y+3; y=x; x=m;
        Send_to_lcd(CMD, 0x36);
        Send_to_lcd(DAT, 0x08); //Íà÷àëüíûé àäðåñ îñåé Õ è Ó - ïðàâûé âåðõíèé óãîë äèñïëåÿ
        Send_Symbol( symbol, x, y, t_color, b_color, zoom_width, zoom_height, rot);
        Send_to_lcd(CMD, 0x36);
        Send_to_lcd(DAT, 0x48);
        break;
        //================================
        case 180:
        x+=3; y+=2;
        Send_to_lcd(CMD, 0x36);
        Send_to_lcd(DAT, 0x88); //Íà÷àëüíûé àäðåñ îñåé Õ è Ó - ïðàâûé íèæíèé óãîë äèñïëåÿ
        Send_Symbol( symbol, x, y, t_color, b_color, zoom_width, zoom_height, rot);
        Send_to_lcd(CMD, 0x36);
        Send_to_lcd(DAT, 0x48);
        break;
        //================================
        case 270:
        m=y; y=x+2; x=m;
        Send_to_lcd(CMD, 0x36);
        Send_to_lcd(DAT, 0xC8); //Íà÷àëüíûé àäðåñ îñåé Õ è Ó - ëåâûé íèæíèé óãîë äèñïëåÿ
        Send_Symbol( symbol, x, y, t_color, b_color, zoom_width, zoom_height, rot);
        Send_to_lcd(CMD, 0x36);
        Send_to_lcd(DAT, 0x48);
        break;
        //================================
        default:
        Send_to_lcd(CMD, 0x36);
        Send_to_lcd(DAT, 0x48); //Íà÷àëüíûé àäðåñ îñåé Õ è Ó - ëåâûé âåðõíèé óãîë äèñïëåÿ
        Send_Symbol( symbol, x, y, t_color, b_color, zoom_width, zoom_height, rot);
        Send_to_lcd(CMD, 0x36);
        Send_to_lcd(DAT, 0x48);
        //=================================
    };
}



void LCD_Puts(char *str, int x, int y,  int t_color, int b_color, char zoom_width, char zoom_height, int rot)
{
 unsigned char i=0;

 if(zoom_width == 0)   zoom_width = 1;
 if(zoom_height == 0)  zoom_height = 1;

 while (str[i]) //x è y - àäðåñ ïèêñåëÿ íà÷àëüíîé ïîçèöèè; ñ óâåëè÷åíèåì ïåðåìåííîé i àäðåñ âûâîäà î÷åðåäíîãî ñèìâîëà ñìåùàåòñÿ íà i*6 (÷åì îðãàíèçóþòñÿ ñòîëáöû äèñïëåÿ)
 {
  LCD_Putchar(str[i], x+(i*6*zoom_width), y, t_color, b_color, zoom_width, zoom_height, rot);
  i++;
 }
}


void Send_Symbol_Shadow(unsigned char symbol, char x, char y, int t_color, char zoom_width, char zoom_height, int rot)
{
 unsigned char temp_symbol, a, b, zw, zh, mask;
 char m, n;
 m=x;
 n=y;
 if (symbol>127) symbol-=64;    //Óáèðàåì îòñóòñòâóþùóþ ÷àñòü òàáëèöû ASCII
 for ( a = 0; a < 5; a++) //Ïåðåáèðàþ 5 áàéò, ñîñòàâëÿþùèõ ñèìâîë
 {
  temp_symbol = font_5x8[symbol-32][a];
  zw = 0;
  while(zw != zoom_width) //Âûâîä áàéòà âûïîëíÿåòñÿ zw ðàç
  {
   switch(rot)
   {
    case 0: case 180: n=y; break;
    case 90: case 270: m=x; break;
   }
   mask=0x01;
   for ( b = 0; b < 8; b++ ) //Öèêë ïåðåáèðàíèÿ 8 áèò áàéòà
   {
    zh = 0; //â zoom_height ðàç óâåëè÷èòñÿ âûñîòà ñèìâîëà
    while(zh != zoom_height) //Âûâîä ïèêñåëÿ âûïîëíÿåòñÿ z ðàç
    {
     switch(rot)
     {
      case 0: case 180:
      if (temp_symbol&mask)
      {
       Put_Pixel (m+zw, n+zh, t_color);
      }
      break;
      case 90: case 270:
      if (temp_symbol&mask)
      {
       Put_Pixel (m+zh, n+zw, t_color);
      }
      break; //Ïîëó÷èòü àäðåñ íà÷àëüíîãî ïèêñåëÿ ïî îñè y äëÿ âûâîäà î÷åðåäíîãî áàéòà
     }
     zh++;
    }
    mask<<=1; //Ñìåùàþ ñîäåðæèìîå mask íà 1 áèò âëåâî;
    switch(rot)
    {
     case 0: case 180: n=n+zoom_height; break;
     case 90: case 270: m=m+zoom_height; break;
    }
   }
   zw++;
  }
  switch(rot)
  {
   case 0: case 180: m=m+zoom_width; break;
   case 90: case 270: n=n+zoom_width; break;
  }
 }
}


void LCD_Putchar_Shadow(char symbol, char x, char y, int t_color, char zoom_width, char zoom_height, int rot)
{
 unsigned char m;
 if(zoom_width == 0)   zoom_width = 1;
 if(zoom_height == 0)  zoom_height = 1;
 switch (rot)
 {
  case 0:  //Íà÷àëüíûé àäðåñ îñåé Õ è Ó - ëåâûé âåðõíèé óãîë äèñïëåÿ
  Send_Symbol_Shadow( symbol, x, y, t_color, zoom_width, zoom_height, rot);
  break;
  //================================
  case 90:
  m=y+3; y=x; x=m;
  Send_to_lcd(CMD, 0x36);
  Send_to_lcd(DAT, 0x08); //Íà÷àëüíûé àäðåñ îñåé Õ è Ó - ïðàâûé âåðõíèé óãîë äèñïëåÿ
  Send_Symbol_Shadow( symbol, x, y, t_color, zoom_width, zoom_height, rot);
  Send_to_lcd(CMD, 0x36);
  Send_to_lcd(DAT, 0x48);
  break;
  //================================
  case 180:
  x+=3; y+=2;
  Send_to_lcd(CMD, 0x36);
  Send_to_lcd(DAT, 0x88); //Íà÷àëüíûé àäðåñ îñåé Õ è Ó - ïðàâûé íèæíèé óãîë äèñïëåÿ
  Send_Symbol_Shadow( symbol, x, y, t_color, zoom_width, zoom_height, rot);
  Send_to_lcd(CMD, 0x36);
  Send_to_lcd(DAT, 0x48);
  break;
  //================================
  case 270:
  m=y; y=x+2; x=m;
  Send_to_lcd(CMD, 0x36);
  Send_to_lcd(DAT, 0xC8); //Íà÷àëüíûé àäðåñ îñåé Õ è Ó - ëåâûé íèæíèé óãîë äèñïëåÿ
  Send_Symbol_Shadow( symbol, x, y, t_color, zoom_width, zoom_height, rot);
  Send_to_lcd(CMD, 0x36);
  Send_to_lcd(DAT, 0x48);
  break;
  //================================
  default:
  Send_to_lcd(CMD, 0x36);
  Send_to_lcd(DAT, 0x48); //Íà÷àëüíûé àäðåñ îñåé Õ è Ó - ëåâûé âåðõíèé óãîë äèñïëåÿ
  Send_Symbol_Shadow( symbol, x, y, t_color, zoom_width, zoom_height, rot);
  Send_to_lcd(CMD, 0x36);
  Send_to_lcd(DAT, 0x48);
  //=================================
 };
}


void LCD_Puts_Shadow(char *str, int x, int y,  int t_color, char zoom_width, char zoom_height, int rot)
{
 unsigned char i=0;

 if(zoom_width == 0)   zoom_width = 1;
 if(zoom_height == 0)  zoom_height = 1;

 while (str[i]) //x è y - àäðåñ ïèêñåëÿ íà÷àëüíîé ïîçèöèè; ñ óâåëè÷åíèåì ïåðåìåííîé i àäðåñ âûâîäà î÷åðåäíîãî ñèìâîëà ñìåùàåòñÿ íà i*6 (÷åì îðãàíèçóþòñÿ ñòîëáöû äèñïëåÿ)
 {
  LCD_Putchar_Shadow(str[i], x+(i*6*zoom_width), y, t_color, zoom_width, zoom_height, rot);
  i++;
 }
}
*/

void LCD_FillScreen(unsigned int color)
{
 unsigned int x;
 SetArea( 0, 100, 0, 80 );   //Îáëàñòü âñåãî ýêðàíà
 for (x = 0; x < 8080; x++)
 {

  Send_to_lcd( DAT, color ); //Äàííûå - çàäàžì öâåò ïèêñåëÿ

 }
}


void LCD_DrawLine(char x1, char y1, char x2, char y2, int color)
{
 short  x, y, d, dx, dy, i, i1, i2, kx, ky;
 signed char flag;

 dx = x2 - x1;
 dy = y2 - y1;
 if (dx == 0 && dy == 0) Put_Pixel(x1, y1, color);  //Òî÷êà
 else      //Ëèíèÿ
 {
  kx = 1;
  ky = 1;
  if( dx < 0 )
  {
   dx = -dx;
   kx = -1;
  }
  else
  if(dx == 0) kx = 0;
  if(dy < 0)
  {
   dy = -dy;
   ky = -1;
  }
  if(dx < dy)
  {
   flag = 0;
   d = dx;
   dx = dy;
   dy = d;
  }
  else flag = 1;
  i1 = dy + dy;
  d = i1 - dx;
  i2 = d - dx;
  x = x1;
  y = y1;

  for(i=0; i < dx; i++)
  {
   Put_Pixel(x, y, color);
   if(flag) x += kx;
   else y += ky;
   if( d < 0 ) d += i1;
   else
   {
    d += i2;
    if(flag) y += ky;
    else x += kx;
   }
  }
  Put_Pixel(x, y, color);
 }
}


void LCD_DrawRect(char x1, char y1, char width, char height, char size, int color)
{
 unsigned int i;
 char x2=x1+(width-1), y2=y1+(height-1); //Êîíå÷íûå ðàçìåðû ðàìêè ïî îñÿì õ è ó
 for( i=1; i<=size; i++)   // size - òîëùèíà ðàìêè
 {
  LCD_DrawLine(x1, y1, x1, y2, color);
  LCD_DrawLine(x2, y1, x2, y2, color);
  LCD_DrawLine(x1, y1, x2, y1, color);
  LCD_DrawLine(x1, y2, x2, y2, color);
  x1++; // Óâåëè÷èâàþ òîëùèíó ðàìêè, åñëè ýòî çàäàíî
  y1++;
  x2--;
  y2--;
 }
}


void LCD_FillRect(char x1, char y1, char width, char height, int color)
{
 unsigned int x, y;

 SetArea( x1, x1+(width-1), y1, y1+(height-1) );
 y = width * height;	        //Êîëè÷åñòâî ïèêñåëåé â ïðÿìîóãîëüíèêå
 for (x = 0; x < y; x++)
 {

  Send_to_lcd( DAT, color ); Send_to_lcd( DAT, color ); //Äàííûå - çàäàžì öâåò ïèêñåëÿ

 }
}


void LCD_DrawCircle(char xcenter, char ycenter, char rad, int color)
{
 signed int tswitch, x1=0, y1;
 char d;

 d = ycenter - xcenter;
 y1 = rad;
 tswitch = 3 - 2 * rad;
 while (x1 <= y1)
 {
  Put_Pixel(xcenter + x1, ycenter + y1, color);
  Put_Pixel(xcenter + x1, ycenter - y1, color);
  Put_Pixel(xcenter - x1, ycenter + y1, color);
  Put_Pixel(xcenter - x1, ycenter - y1, color);
  Put_Pixel(ycenter + y1 - d, ycenter + x1, color);
  Put_Pixel(ycenter + y1 - d, ycenter - x1, color);
  Put_Pixel(ycenter - y1 - d, ycenter + x1, color);
  Put_Pixel(ycenter - y1 - d, ycenter - x1, color);

  if (tswitch < 0) tswitch += (4 * x1 + 6);
  else
  {
   tswitch += (4 * (x1 - y1) + 10);
   y1--;
  }
  x1++;
 }
}


void LCD_FillCircle(char xcenter, char ycenter, char rad, int color)
{
 signed int x1=0, y1, tswitch;
 y1 = rad;
 tswitch = 1 - rad;

 do
 {
  LCD_DrawLine(xcenter-x1, ycenter+y1, xcenter+x1, ycenter+y1, color);
  LCD_DrawLine(xcenter-x1, ycenter-y1, xcenter+x1, ycenter-y1, color);
  LCD_DrawLine(xcenter-y1, ycenter+x1, xcenter+y1, ycenter+x1, color);
  LCD_DrawLine(xcenter-y1, ycenter-x1, xcenter+y1, ycenter-x1, color);

  if(tswitch < 0)
  tswitch+= 3 + 2*x1++;
  else
  tswitch+= 5 + 2*(x1++ - y1--);
 } while(x1 <= y1);
}


//===============================================================


void Send_Image(char x, char y, char width, char height)
{
int rot = 0 ;
unsigned char img[4096] = {
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XF5,0XE8,0XE8,0XEC,0XEC,
		0XEC,0XEC,0XE8,0XE8,0XFA,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XEC,0XEC,0XEC,0XEC,0XE8,0XEC,0XEC,
		0XE8,0XEC,0XE8,0XEC,0XEC,0XEC,0XF0,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XF5,0XEC,0XEC,0XE8,0XE8,0XEC,0XE8,0XEC,0XEC,
		0XEC,0XEC,0XE8,0XEC,0XEC,0XEC,0XE8,0XE8,0XFA,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XEC,0XEC,0XEC,0XCC,0XEC,0XE8,0XEC,0XEC,0XEC,0XE8,
		0XEC,0XE8,0XEC,0XEC,0XEC,0XEC,0XEC,0XEC,0XE8,0XF1,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XEC,0XEC,0XEC,0XEC,0XEC,0XE8,0XEC,0XEC,0XE8,0XEC,0XE8,
		0XE8,0XE8,0XEC,0XEC,0XEC,0XEC,0XEC,0XE8,0XE8,0XEC,0XF1,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XEC,0XEC,0XE8,0XEC,0XEC,0XE8,0XE8,0XEC,0XEC,0XEC,0XEC,0XEC,
		0XEC,0XEC,0XE8,0XEC,0XE8,0XE8,0XE8,0XEC,0XEC,0XEC,0XEC,0XFA,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XEC,0XEC,0XEC,0XEC,0XEC,0XEC,0XEC,0XEC,0XE8,0XEC,0XE8,0XE8,
		0XEC,0XEC,0XE8,0XEC,0XE8,0XEC,0XE8,0XEC,0XEC,0XE8,0XEC,0XE8,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XEC,0XEC,0XE8,0XE8,0XEC,0XEC,0XEC,0XEC,0XE8,0XEC,0XE8,0XEC,0XEC,
		0XE8,0XEC,0XE8,0XEC,0XEC,0XEC,0XE8,0XEC,0XEC,0XE8,0XEC,0XEC,0XEC,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFA,0X92,0X92,0X92,0X92,0XFF,0XFF,0XFF,0XFF,0XFF,0X92,
		0X92,0XFF,0XFF,0XFF,0XFF,0XFF,0XD6,0X92,0XFF,0XFF,0XFF,0XFE,0X44,0X00,0X00,0XBB,
		0XFF,0XFF,0XFB,0XE8,0XEC,0XF1,0XF6,0XFA,0XFA,0XFA,0XF6,0XF6,0XF6,0XF6,0XE8,0XF5,
		0XF6,0XF6,0XF6,0XFA,0XF6,0XFA,0XF6,0XF5,0XE8,0XFA,0XFA,0XF6,0XF6,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X00,0X00,0X00,0X00,0X00,0XBF,0XFF,0XFF,0XFF,0X00,
		0X2A,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X00,0XFF,0XFF,0XAD,0X00,0X00,0X00,0X00,0X00,
		0XFF,0XFF,0XEC,0XEC,0XE8,0XFB,0XFF,0XFF,0XFF,0XFA,0XFA,0XFB,0XFF,0XDF,0XEC,0XFB,
		0XFF,0XFF,0XFF,0XFB,0XFB,0XFA,0XFF,0XFA,0XEC,0XFB,0XFF,0XFF,0XFB,0XF1,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X32,0XFF,0XFF,0XFF,0X00,0X00,0XFF,0XFF,0XFF,0X00,
		0XBB,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X2A,0XFF,0XFF,0X00,0X05,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XEC,0XEC,0XEC,0XEC,0XFF,0XFF,0XF5,0XEC,0XEC,0XEC,0XFB,0XF5,0XEC,0XEC,
		0XFF,0XFF,0XEC,0XEC,0XEC,0XEC,0XFF,0XF0,0XEC,0XEC,0XFF,0XFF,0XEC,0XEC,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0XDF,0XFF,0XFF,0XFF,0X00,0X00,0XFF,0XFF,0XD6,0X00,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0XBF,0XFF,0XFF,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XEC,0XE8,0XEC,0XEC,0XFF,0XFF,0XEC,0XEC,0XE8,0XEC,0XEC,0XE8,0XEC,0XE8,
		0XFF,0XFF,0XE8,0XE8,0XEC,0XEC,0XEC,0XEC,0XEC,0XFA,0XFF,0XFF,0XE8,0XEC,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XF5,0X00,0XFF,0XFF,0XFF,0XFF,0X00,0X53,0XFF,0XFF,0X8C,0X00,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XD6,0X00,0XFF,0XFF,0XFF,0X00,0X09,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFA,0XE8,0XE8,0XEC,0XEC,0XFF,0XFF,0XE8,0XEC,0XE8,0XE8,0XEC,0XEC,0XEC,0XEC,
		0XFF,0XFF,0XE8,0XEC,0XEC,0XEC,0XC8,0XEC,0XEC,0XFF,0XFF,0XFA,0XE8,0XEC,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0X60,0X00,0XFF,0XFF,0X8D,0X00,0X04,0XFF,0XFF,0XFF,0X20,0X00,
		0XFF,0XFF,0XFF,0XFF,0XFF,0X8C,0X00,0XFF,0XFF,0XFF,0X24,0X00,0X04,0XFF,0XFF,0XFF,
		0XFF,0XF5,0XE8,0XEC,0XEC,0XF1,0XFF,0XFF,0XF5,0XF5,0XFF,0XFB,0XE8,0XEC,0XEC,0XF6,
		0XFF,0XFF,0XE8,0XEC,0XFF,0XF6,0XE8,0XEC,0XE8,0XFF,0XFF,0XEC,0XEC,0XEC,0XFA,0XFF,
		0XFF,0XFF,0XFF,0XFF,0X00,0X00,0X00,0X00,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X33,
		0XFF,0XFF,0XFF,0XFF,0XFF,0X20,0X00,0XFF,0XFF,0XFF,0XFF,0X84,0X00,0X00,0XBB,0XFF,
		0XFF,0XF5,0XEC,0XE8,0XE8,0XFA,0XFF,0XFF,0XFF,0XFF,0XFF,0XEC,0XE8,0XE8,0XEC,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XEC,0XE8,0XEC,0XE8,0XFF,0XFF,0XE8,0XEC,0XE8,0XFA,0XFF,
		0XFF,0XFF,0XFF,0XFF,0X00,0X4E,0XFF,0XFF,0X00,0X00,0XFF,0XFF,0XFF,0XFF,0X00,0XBB,
		0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X32,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X00,0XFF,
		0XFF,0XFA,0XE8,0XEC,0XEC,0XFF,0XFF,0XF5,0XEC,0XEC,0XF1,0XEC,0XEC,0XEC,0XEC,0XFF,
		0XFF,0XF1,0XEC,0XE8,0XFA,0XEC,0XE8,0XE8,0XF1,0XFF,0XFF,0XEC,0XEC,0XEC,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0X00,0XBB,0XFF,0XFF,0XD5,0X00,0XDF,0XFF,0XFF,0XFF,0X00,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0XDF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFA,0X00,0XDF,
		0XFF,0XFF,0XE8,0XEC,0XE8,0XFF,0XFF,0XEC,0XEC,0XE8,0XEC,0XE8,0XEC,0XE8,0XEC,0XFF,
		0XFF,0XEC,0XE8,0XEC,0XE8,0XEC,0XE8,0XEC,0XDA,0XFF,0XFF,0XEC,0XEC,0XE8,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XD1,0X00,0XFF,0XFF,0XFF,0XFF,0X00,0X9B,0XFF,0XFF,0XFA,0X00,0XDF,
		0XFF,0XFF,0XFF,0XFF,0X40,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XD1,0X00,0XFF,
		0XFF,0XFF,0XEC,0XEC,0XE8,0XFF,0XFF,0XE8,0XEC,0XE8,0XE8,0XF5,0XEC,0XEC,0XEC,0XFF,
		0XFF,0XE8,0XEC,0XEC,0XEC,0XEC,0XEC,0XEC,0XFF,0XFF,0XFA,0XEC,0XEC,0XE8,0XFF,0XFF,
		0XFF,0XFF,0XFF,0X68,0X00,0XFF,0XFF,0XFF,0XFF,0X00,0X4E,0XFF,0XFF,0XFF,0X00,0X00,
		0XFF,0XFF,0XFF,0X64,0X00,0X77,0XFF,0XFF,0X00,0XFF,0XFF,0XFF,0XFA,0X00,0X00,0XFF,
		0XFF,0XFF,0XE8,0XEC,0XF1,0XFF,0XFF,0XEC,0XEC,0XEC,0XEC,0XFF,0XE8,0XE8,0XF5,0XFF,
		0XFF,0XE8,0XE8,0XEC,0XE8,0XE8,0XEC,0XE8,0XFF,0XFF,0XED,0XE8,0XEC,0XEC,0XFF,0XFF,
		0XFF,0XFF,0XFF,0X20,0X00,0XFF,0XFF,0XFF,0XFF,0X00,0X01,0XFF,0XFF,0XFF,0XAC,0X00,
		0X00,0X00,0X00,0X00,0X77,0XFF,0XFF,0XFF,0X00,0X00,0X00,0X00,0X00,0X00,0XFF,0XFF,
		0XFF,0XFF,0XFA,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XEC,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XE8,0XEC,0XE8,0XEC,0XEC,0XFF,0XFF,0XFF,0XFF,0XF1,0XEC,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XB6,0XB6,0XB7,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XB6,0XB6,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XEC,0XE8,0XEC,0XEC,0XE8,0XE8,0XEC,0XEC,0XEC,0XEC,0XE8,0XEC,0XEC,
		0XEC,0XE8,0XE8,0XEC,0XEC,0XEC,0XEC,0XEC,0XEC,0XE8,0XEC,0XEC,0XEC,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFA,0XEC,0XEC,0XEC,0XEC,0XEC,0XC8,0XEC,0XEC,0XEC,0XE8,0XEC,0XEC,
		0XE8,0XEC,0XEC,0XEC,0XE8,0XEC,0XEC,0XE8,0XEC,0XC8,0XEC,0XE8,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XEC,0XEC,0XE8,0XEC,0XEC,0XE8,0XEC,0XEC,0XE8,0XEC,0XE8,0XEC,
		0XEC,0XEC,0XE8,0XEC,0XEC,0XE8,0XEC,0XEC,0XEC,0XEC,0XE8,0XF1,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XE8,0XEC,0XEC,0XEC,0XEC,0XEC,0XEC,0XEC,0XEC,0XEC,0XE8,
		0XE8,0XEC,0XE8,0XEC,0XE8,0XEC,0XE8,0XEC,0XEC,0XEC,0XEC,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XEC,0XEC,0XEC,0XEC,0XEC,0XE8,0XEC,0XEC,0XEC,0XE8,
		0XEC,0XEC,0XEC,0XE8,0XEC,0XEC,0XE8,0XEC,0XEC,0XEC,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XEC,0XE8,0XE8,0XE8,0XE8,0XE8,0XE8,0XEC,0XEC,
		0XEC,0XEC,0XEC,0XEC,0XEC,0XE8,0XEC,0XEC,0XF0,0XFF,0XFF,0XDF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFA,0XEC,0XE8,0XEC,0XEC,0XE8,0XEC,0XEC,
		0XEC,0XE8,0XEC,0XE8,0XEC,0XE8,0XEC,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFE,0XEC,0XE8,0XEC,0XEC,0XE8,
		0XEC,0XEC,0XEC,0XEC,0XEC,0XFF,0XDF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFA,0XF9,
		0XF6,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XD6,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0X64,0X00,
		0XFF,0XFF,0XFF,0X00,0X00,0X00,0X00,0X00,0X00,0XFF,0XFF,0XFF,0X00,0X00,0X00,0X00,
		0X00,0X00,0XFF,0XFF,0XFF,0XFF,0X00,0X00,0X00,0X00,0X00,0X00,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X64,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X01,
		0XFF,0XFF,0XFF,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0XFF,0XFF,0XFF,
		0XD5,0X00,0X9B,0XFF,0XFF,0XD6,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X01,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X7B,
		0XFF,0XFF,0XAC,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XCC,0X00,0XFF,0XFF,0XFF,
		0XFF,0X00,0X57,0XFF,0XFF,0X68,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X77,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0XDF,
		0XFF,0XFF,0X20,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X20,0X00,0XFF,0XFF,0XFF,
		0XD5,0X00,0XDF,0XFF,0XFF,0X00,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0XFF,
		0XFF,0XFF,0X00,0X09,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X09,0XFF,0XFF,0XFA,
		0X00,0X05,0XFF,0XFF,0XFF,0X00,0X32,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XD1,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0XFF,
		0XFF,0XFF,0X00,0X00,0X00,0X00,0X00,0X2A,0XFF,0XFF,0XFF,0X00,0X00,0X00,0X00,0X00,
		0X6E,0XFF,0XFF,0XFF,0XFF,0X00,0X00,0X00,0X00,0X00,0X72,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X20,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X05,0XFF,
		0XFF,0XFA,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFA,0X00,0XDF,0XFF,0XB2,0X00,
		0X53,0XFF,0XFF,0XFF,0XF5,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X04,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X4E,0XFF,
		0XFF,0XAD,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XAD,0X00,0XFF,0XFF,0XFF,0X20,
		0X00,0XFF,0XFF,0XFF,0X84,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X4E,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0XBF,0XFF,
		0XFF,0X44,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X44,0X00,0XFF,0XFF,0XFF,0X64,
		0X00,0XFF,0XFF,0XFF,0X00,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFA,0X00,0XDF,0XFF,0XFF,0XFF,0XFF,0XD1,0X00,0XFF,0XFF,
		0XFF,0X00,0X01,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X01,0XFF,0XFF,0XFF,0XD5,
		0X00,0XDF,0XFF,0XFF,0X00,0X4E,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XD1,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0X68,0X00,0XFF,0XFF,
		0XFF,0X00,0X00,0X00,0X00,0X00,0X00,0XDF,0XFF,0XFF,0X00,0X57,0XFF,0XFF,0XFF,0XFF,
		0X00,0X97,0XFF,0XFF,0X00,0X00,0X00,0X00,0X00,0X00,0XDF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
		};

 char x1, y1;
 static int counterimage = 0;
 y+=1;
 switch (rot)
 {
  case 0: case 180:
  for(y1=y; y1<(y+height); y1++)
  {
   SetArea( x, x+(width-1), y1, y1 );
   for(x1=x; x1<x+width; x1++)
   {

    Send_to_lcd( DAT, img[counterimage] ); //������ - ����� ���� �������
    counterimage = counterimage + 1;
   }
  }
  break;

  case 90: case 270:
  for(x1=x; x1<x+height; x1++)
  {
   SetArea( x1, x1, y, y+(width-1) );
   for(y1=y; y1<y+width; y1++)
   {

	   Send_to_lcd( DAT, img[counterimage] ); //������ - ����� ���� �������
	   counterimage = counterimage + 1;
   }
  }
  break;
 };
}


void helloImage(void) {
	Send_Image( 1, 1, 64, 64 ) ;
}
