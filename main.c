// #include "io430.h"
#include "msp430.h"
#include "stdbool.h"
#include "stdint.h"

#define B(x)         (0x01 << (x))
#define epdcklow     P2OUT &= (~B(1))
#define epdckhigh    P2OUT |= B(1)
#define epddalow     P2OUT &= (~B(2))
#define epddahigh    P2OUT |= B(2)
#define epdcslow     P1OUT &= (~B(3))
#define epdcshigh    P1OUT |= B(3)

#define epddclow     P1OUT &= (~B(4))
#define epddchigh    P1OUT |= B(4)
#define epdrstlow    P1OUT &= (~B(5))
#define epdrsthigh   P1OUT |= B(5)

#define READ_EPD_BUSY() (P2IN & (B(0)))

/*
#define epdbslow          P3OUT &= 0xfd
#define epdunknowhigh          P3OUT &= 0x80
#define epdon           P2OUT &= 0xbf
#define epdoff          P2OUT |= 0x40
*/

/*
#define rfcklow            P1OUT &= 0xef
#define rfckhigh           P1OUT |= 0x10
#define rfdalow            P1OUT &= 0xfb
#define rfdahigh           P1OUT |= 0x04
#define rfcslow            P1OUT &= 0xf7
#define rfcshigh           P1OUT |= 0x08
#define rfon               P2OUT &= 0x7f
#define rfoff              P2OUT |= 0x80

#define spiromcshigh       P3OUT |= 0x01
#define spiromcslow        P3OUT &= 0xfe
*/

void InitClk(void);
void InitGpio(void);
void InitEpd(void);
void sendbyte(uint8_t sdbyte);
void epd_send_cmd(int cmd);
void epd_sendcmddata(int cmd, int data);
void Delay(unsigned int nCount);
void Delaylong(unsigned int n10Count);
extern const unsigned char img1[];
extern const unsigned char img2[];
extern const unsigned char epdinit[];

#define EPD_Y_RAW_PIXELS 200
#define EPD_X_RAW_PIXELS 200

// const uint8_t GDOControl[] = {0x01, (EPD_Y_RAW_PIXELS - 1) % 256,
// (EPD_Y_RAW_PIXELS - 1) / 256, 0x00}; //for 1.54inch
const uint8_t GDOControl[] = {0x01, 0xF9, 0x00};
const uint8_t EPD_BSSC[] = {0x0c, 0xd7, 0xd6, 0x9d};
const uint8_t EPD_SDLP[] = {0x3a, 0x00}; // 4 dummy line per gate
const uint8_t EPD_SGLW[] = {0x3b, 0x00}; // 2us per line
const uint8_t EPD_DEMS[] = {0x11, 0x03};
const uint8_t EPD_SRAMXASE[] = {0x44, 0x00, 0x0f};
const uint8_t EPD_SRAMYASE[] = {0x45, 0x00, 0xf9};
const uint8_t EPD_VCOMVol[] = {0x2c, 0xA0};
const uint8_t EPD_UNKNOWN[] = {0x02, 0x21, 0x83};

// uint8_t EPD_buffer[500];

/*
const uint8_t LUTDefault_full[] = {
    0x32, // command
    0x50, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x11, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00};
const uint8_t LUTDefault_part[] = {
    0x32, // command
    0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
*/
const uint8_t EPD_LUT[] = {0x33, 0x32, 0x66, 0x66, 0x26, 0x04, 0x55, 0xaa,
                           0x08, 0x91, 0x11, 0x88, 0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x19, 0x19, 0x0a, 0x0a, 0x5e, 0x1e,
                           0x1e, 0x0a, 0x39, 0x14, 0x00, 0x00, 0x00};

void EPD_writeCommandData(const uint8_t *pCD, uint8_t len);

int main(void) {
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;

  InitGpio();
  InitClk();
  InitEpd();

  epd_sendcmddata(0x4e, 0x0);
  epd_sendcmddata(0x4f, 0x0);
  epd_send_cmd(0x24); //  write ram
  epdcshigh;
  epdcslow;
  for (unsigned int s = 0; s < 4000; s++) {
    sendbyte(~img1[s]);
  }
  epdcshigh;
  while (1) {
    epd_sendcmddata(0x22, 0xc7);
    epd_send_cmd(0x20);
    while (READ_EPD_BUSY())
      ;
    Delaylong(100);
  }
}

void EPD_wait() {
  while (READ_EPD_BUSY())
    ;
}

void InitEpd(void) {
  // unsigned char i;
  epd_send_cmd(0x12);
  // Delaylong(1);
  while (READ_EPD_BUSY())
    ;

  /*
  EPD_writeCommandData(GDOControl, sizeof(GDOControl));
  // EPD_writeCommandData(softstart, sizeof(softstart));
  EPD_writeCommandData(EPD_SDLP, sizeof(EPD_SDLP));
  EPD_writeCommandData(EPD_SGLW, sizeof(EPD_SGLW));
  EPD_writeCommandData(EPD_DEMS, sizeof(EPD_DEMS));
  EPD_writeCommandData(EPD_SRAMXASE, sizeof(EPD_SRAMXASE));
  EPD_writeCommandData(EPD_SRAMYASE, sizeof(EPD_SRAMYASE));
  EPD_writeCommandData(EPD_VCOMVol, sizeof(EPD_VCOMVol));
  EPD_writeCommandData(EPD_LUT, sizeof(EPD_LUT));
  // EPD_writeCommandData(EPD_UNKNOWN, sizeof(EPD_UNKNOWN));
  */

  for (uint8_t i = 0; i <= 0x33; i++) {
    if ((i == 0x0) || (i == 0x3) || (i == 0x5) || (i == 0x7) || (i == 0x9) ||
        (i == 0xc) || (i == 0xf) || (i == 0x11) || (i == 0x13) || (i == 0x32)) {
      epddclow;
    }
    epdcslow;
    sendbyte(epdinit[i]);
    epdcshigh;
    epddchigh;
    while (READ_EPD_BUSY())
      ;
  }
}

uint16_t EPD_linebuffer[((EPD_Y_RAW_PIXELS / 16) + 1)];
const uint8_t *EPD_linebuffer8 = (uint8_t *)(&EPD_linebuffer[0]);

const uint8_t _EPD_STRETCH_LUT[] = {B8(00000000), B8(00000011), B8(00001100),
                                    B8(00001111), B8(00110000), B8(00110011),
                                    B8(00111100), B8(00111111), B8(11000000),
                                    B8(11000011), B8(11001100), B8(11001111),
                                    B8(11110000), B8(11110011), B8(11111100),
                                    B8(11111111)}

uint16_t
EPD_stretch(uint8_t byte) {
  uint8_t buf[2];
  buf[0] = _EPD_STRETCH_LUT[(byte & 0x0F)];
  buf[1] = _EPD_STRETCH_LUT[(byte >> 4)];
  return *((uint16_t *)buf);
}

void InitGpio(void) {
  // rfcslow;
  // rfcklow;
  // rfdalow;
  // spiromcslow;
  // epdunknowhigh;
  // epdbslow ;
  // epdon;
  // rfoff;
  P1DIR = B(3) | B(4) | B(5);
  P2DIR = B(1) | B(2);
  P1SEL = 0x0;
  P1SEL2 = 0x0;
  P2SEL = 0x0;
  P2SEL2 = 0x0;
  P3SEL = 0x0;
  P3SEL2 = 0x0;
  Delaylong(1);
  epdrsthigh;
  Delaylong(1);
  epdrstlow;
  Delaylong(1);
  epdrsthigh;
}

void InitClk(void) {
  DCOCTL = 0x0000;
  BCSCTL1 = CALBC1_16MHZ; // Set range
  DCOCTL = CALDCO_16MHZ;

  BCSCTL3 |= LFXT1S_2; /* Mode 2 for LFXT1 : VLO */
  IFG1 &= !(OFIFG);
  Delay(512);
  BCSCTL1 |= DIVA_0; /* ACLK Divider 3: /8 */
}

void epd_sendcmddata(int cmd, int data) {
  epddclow;
  epdcslow;
  sendbyte(cmd);
  epddchigh;
  sendbyte(data);
  epdcshigh;
}

void epd_send_cmd(int cmd) {
  epddclow;
  epdcslow;
  sendbyte(cmd);
  epdcshigh;
  epddchigh;
}

void sendbyte(uint8_t sdbyte) {
  epdcklow;
  if (sdbyte & 0x80) epddahigh; else epddalow;
  epdckhigh;
  epdcklow;
  if (sdbyte & 0x40) epddahigh; else epddalow;
  epdckhigh;
  epdcklow;
  if (sdbyte & 0x20) epddahigh; else epddalow;
  epdckhigh;
  epdcklow;
  if (sdbyte & 0x10) epddahigh; else epddalow;
  epdckhigh;
  epdcklow;
  if (sdbyte & 0x08) epddahigh; else epddalow;
  epdckhigh;
  epdcklow;
  if (sdbyte & 0x04) epddahigh; else epddalow;
  epdckhigh;
  epdcklow;
  if (sdbyte & 0x02) epddahigh; else epddalow;
  epdckhigh;
  epdcklow;
  if (sdbyte & 0x01) epddahigh; else epddalow;
  epdckhigh;
  epdcklow;
}

void EPD_writeCommandData(const uint8_t *pCD, uint8_t len) {
  EPD_wait();
  epddclow;
  epdcslow;
  sendbyte(*pCD++);
  epddchigh;
  for (uint8_t i = 0; i < len - 1; i++)
    sendbyte(*pCD++);
  epdcshigh;
  asm("NOP");
  asm("NOP");
}

void Delay(unsigned int nCount) {
  for (; nCount != 0; nCount--) {
    asm("NOP");
    asm("NOP");
    asm("NOP");
    asm("NOP");
  }
}

void Delaylong(unsigned int n10Count) {
  unsigned char i;
  while (n10Count--) {
    for (i = 0; i < 10; i++) {
      Delay(10000);
    }
  }
}