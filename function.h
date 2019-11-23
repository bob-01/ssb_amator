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

#define FRD            A0
#define REV            A1

Si5351 si5351;

// data_pin(rs)(orange), clk_pin (purpur) , enable_pin (red)
LiquidCrystal lcd(4,5,6);

unsigned long currentTime, loopTime;
unsigned long IF, Ftx, STEP;
long Fcorr = 0;

boolean enc_block=false, enc_flag=false, lsb_usb_flag=false,
        rit_flag=false, Button_flag=false, tx_flag=false,
        step_flag=false, rewrite_flag=false, setup_flag=false;

uint8_t SWR_count=0, AVR_count=0,
        Enc_state, Enc_last,
        step_count=4, menu_count=0, setup_count=8, xF=1,
        SI5351_DRIVE_CLK0, SI5351_DRIVE_CLK1, SI5351_DRIVE_CLK2; // 0 ..255
int8_t enc_move=0, XTAL = 30;
byte ENC_SPIN = 1;

uint16_t Ftone, uFRD, uREV;

void F_eeprom_w() {

	unsigned long temp=0;
	long temp_l=0;

      EEPROM.get(0, temp);     //IF=45000000;        // Расчетная промежуточная частота. 450.000.00
      if (IF != temp){
        EEPROM.put(0, IF);
      }
      
			temp = XTAL;
      EEPROM.get(4, XTAL);
      if (XTAL != temp) {
								XTAL = temp;
        EEPROM.put(4, XTAL);
      }
      
      EEPROM.get(16, temp_l); //Fcorr = 0;
      if (Fcorr != temp_l){
        EEPROM.put(16, Fcorr);
      }
      
      EEPROM.get(20, temp);   //Ftx  = 311000000;
      if (Ftx != temp){
        EEPROM.put(20, Ftx);
      }
      
      temp = Ftone;
      EEPROM.get(24, Ftone); //Ftone  = 1000;
      if (Ftone != temp){
        Ftone = temp;
        EEPROM.put(24, Ftone);
      }

      temp = SI5351_DRIVE_CLK0;
      EEPROM.get(26, SI5351_DRIVE_CLK0); // Driver current
      if (SI5351_DRIVE_CLK0 != temp){
        SI5351_DRIVE_CLK0 = temp;
        EEPROM.put(26, SI5351_DRIVE_CLK0);
      }
      
      temp = SI5351_DRIVE_CLK1;
      EEPROM.get(27, SI5351_DRIVE_CLK1); // Driver current
      if (SI5351_DRIVE_CLK1 != temp){
        SI5351_DRIVE_CLK1 = temp;
        EEPROM.put(27, SI5351_DRIVE_CLK1);
      }
      
      temp = SI5351_DRIVE_CLK2;
      EEPROM.get(28, SI5351_DRIVE_CLK2); // Driver current
      if (SI5351_DRIVE_CLK2 != temp){
        SI5351_DRIVE_CLK2 = temp;
        EEPROM.put(28, SI5351_DRIVE_CLK2);
      }

      temp = ENC_SPIN;
      EEPROM.get(29, ENC_SPIN);
      if (ENC_SPIN != temp){
        ENC_SPIN = temp;
        EEPROM.put(29, ENC_SPIN);
      }

      temp = xF;
      EEPROM.get(34, xF); //xF  = 1; multiplier F
      if (xF != temp){
        xF = temp;
        EEPROM.put(34, xF);
      }
}

void Read_Value_EEPROM() {
/*
  EEPROM
  0-3   ( 4 Byte) IF
  4     ( 1 Byte) XTAL
  5-7   NULL
  8-11  NULL
  12-15 NULL
  16-19 ( 4 Byte) Fcorr
  20-23 ( 4 Byte) Ftx
  24-25 ( 2 Byte) Ftone
  26    ( 1 Byte) SI5351_DRIVE_CLK0   2MA 4MA 6MA 8MA 
  27    ( 1 Byte) SI5351_DRIVE_CLK1
  28    ( 1 Byte) SI5351_DRIVE_CLK2
  29    ( 1 Byte) ENC_SPIN
  30-33 NULL
  34    ( 1 Byte) x*F
*/
      EEPROM.get(0, IF);     //Первая ПЧ
      if (IF > 40000000) {
        IF = 500000;
        EEPROM.put(0, IF);
      }
      EEPROM.get(4, XTAL);
      if (XTAL > 40 || XTAL < 0) {
        XTAL = 30;
        EEPROM.put(4, XTAL);
      }
      EEPROM.get(16, Fcorr); //Fcorr = 0;
      if (Fcorr > 5000000 || Fcorr < -5000000 || Fcorr == -1){
          Fcorr = 0;
          EEPROM.put(16, Fcorr);
      }
      EEPROM.get(20, Ftx);   //Ftx  = 311000000;
      if (Ftx > 4000000000 || Ftx < 0){
        Ftx = 292000000;
        EEPROM.put(20, Ftx);
      }
      EEPROM.get(24, Ftone); //Ftone  = 1000;
      if (Ftone > 60000){
        Ftone = 1000;
        EEPROM.put(24, Ftone);
      }
      EEPROM.get(26, SI5351_DRIVE_CLK0); // Driver current
      if (SI5351_DRIVE_CLK0 > 8){
        SI5351_DRIVE_CLK0 = 2;
        EEPROM.put(26, SI5351_DRIVE_CLK0);
      }
      EEPROM.get(27, SI5351_DRIVE_CLK1); // Driver current
      if (SI5351_DRIVE_CLK1 > 8){
        SI5351_DRIVE_CLK1 = 2;
        EEPROM.put(27, SI5351_DRIVE_CLK1);
      }
      EEPROM.get(28, SI5351_DRIVE_CLK2); // Driver current
      if (SI5351_DRIVE_CLK2 > 8){
        SI5351_DRIVE_CLK2 = 2;
        EEPROM.put(28, SI5351_DRIVE_CLK2);
      }

      EEPROM.get(29, ENC_SPIN); // Driver current
      if (ENC_SPIN > 1 || ENC_SPIN < -1){
        ENC_SPIN = 1;
        EEPROM.put(29, ENC_SPIN);
      }
            
      EEPROM.get(34, xF); //xF  = 1; multiplier F
      if (xF > 2){
        xF = 1;
        EEPROM.put(34, xF);
      }
  STEP = 1000;
}// End Read EEPROM

void SWR_Print() {
  lcd.setCursor(0,1);
  lcd.print("P");
  lcd.print("    ");
  lcd.setCursor(1,1);
  lcd.print(uFRD);
  
  lcd.setCursor(5,1);
  lcd.print("F");
  lcd.print("   ");
  lcd.setCursor(6,1);
  lcd.print(uREV);

  lcd.setCursor(9,1);
  lcd.print("SWR");
  lcd.print( (float(uFRD+uREV)/(uFRD-uREV)) );
  lcd.print("   ");
}//End SWR_Print()

void softReset(){

  asm volatile ("  jmp 0");

}//End soft reset

