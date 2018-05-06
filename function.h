#define pin_A          2
#define pin_B          3

#define Button_TX      9 
#define Button_STEP    7 
#define Button_LSB_USB 8

#define Button_RIT     10
#define TX_out         11
#define tone_pin       12

#define Fmin           250000
#define Fmax           4000000000

#define M 1000000

#define FRD            A0
#define REV            A1

Si5351 si5351;

// data_pin(rs)(orange), clk_pin (purpur) , enable_pin (red)
LiquidCrystal lcd(4,5,6);

unsigned long currentTime,loopTime;
unsigned long IF,Ftx,STEP;
long Fcorr;

boolean enc_block=false, enc_flag=false, lsb_usb_flag=false, rit_flag=false, Button_flag=false, tx_flag=false, step_flag=false, rewrite_flag=false, setup_flag=false;

uint8_t SWR_count=0, AVR_count=0, Enc_state, Enc_last, step_count=4, menu_count=0, 
        setup_count=8, xF=1,
        SI5351_DRIVE_CLK0, SI5351_DRIVE_CLK1, SI5351_DRIVE_CLK2, SI5351_CAPACITOR_LOAD; // 0 ..255
int8_t enc_move=0, XTAL = 30;

uint16_t Ftone,uFRD,uREV;

//----------------------------
void softReset(){

  asm volatile ("  jmp 0");

}//End soft reset

