#define pin_A          2
#define pin_B          3

#define Button_TX      9 
#define Button_STEP    7 
#define Button_LSB_USB 8

#define Button_RIT     10
#define TX_out         11
#define tone_pin       12

#define Fmin           8000*100ULL
#define Fmax           40000000*100ULL

#define FRD            A0
#define REV            A1

Si5351 si5351;

// data_pin(rs)(orange), clk_pin (purpur) , enable_pin (red)
LiquidCrystal lcd(4,5,6);

unsigned long currentTime, loopTime;
uint32_t IF = 0, STEP = 1;
uint32_t Ftx = 8;
long Fcorr = 0;

boolean enc_block=false, enc_flag=false, lsb_usb_flag=false,
        rit_flag=false, Button_flag=false, tx_flag=false,
        step_flag=false, rewrite_flag=false, setup_flag = false, ftone_flag = false;;

uint8_t SWR_count = 0, AVR_count = 0, XTAL = 27, Enc_state, Enc_last;
uint8_t step_count = 4, menu_count=0, setup_count = 1;
uint8_t SI5351_DRIVE_CLK0, SI5351_DRIVE_CLK1, SI5351_DRIVE_CLK2; // 0 ..255

int8_t enc_move = 0, ENC_SPIN = 1;

uint16_t Ftone, uFRD, uREV, IF_WIDTH;
