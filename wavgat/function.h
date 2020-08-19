void si5351Set() {
  si5351.init(SI5351_CRYSTAL_LOAD_10PF, XTAL*1000*1000, Fcorr);

  switch (SI5351_DRIVE_CLK0){
    case  2: si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_2MA); break;
    case  4: si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_4MA); break;
    case  6: si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_6MA); break;
    case  8: si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA); break;
  }
  switch (SI5351_DRIVE_CLK1){
    case  2: si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_2MA); break;
    case  4: si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_4MA); break;
    case  6: si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_6MA); break;
    case  8: si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_8MA); break;
  }
  switch (SI5351_DRIVE_CLK2){
    case  2: si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_2MA); break;
    case  4: si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_4MA); break;
    case  6: si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_6MA); break;
    case  8: si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_8MA); break;
  }
}

void pinOutConfig() {
  pinMode(pin_A, INPUT);
  pinMode(pin_B, INPUT);
  pinMode(Button_STEP, INPUT);
  pinMode(Button_TX, INPUT);
  pinMode(Button_RIT, INPUT);
  pinMode(Button_LSB_USB, INPUT);
  pinMode(TX_out, OUTPUT);
  pinMode(tone_pin, OUTPUT); //объявляем пин как выход для звука

  digitalWrite(pin_A, HIGH);
  digitalWrite(pin_B, HIGH);
  digitalWrite(Button_STEP, HIGH);
  digitalWrite(Button_LSB_USB, HIGH);
  digitalWrite(Button_TX, HIGH);
  digitalWrite(Button_RIT, HIGH);
  digitalWrite(TX_out, LOW);
}

void F_eeprom_w() {
	uint32_t temp = 0;
	int32_t temp_l = 0;

  temp = EEPROM.read32(0);     //IF=45000000;        // Расчетная промежуточная частота. 450.000.00
  if (IF != temp){
    EEPROM.write32(0, IF);
  }

  temp = XTAL;
  XTAL = EEPROM.read(4);
  if (XTAL != temp) {
    XTAL = temp;
    EEPROM.write(4, XTAL);
  }

  //Fcorr = 0;
  temp_l = EEPROM.read32(16);
  if (Fcorr != temp_l){
    EEPROM.write32(16, Fcorr);
  }
  
  //Ftx  = 311000000;
  temp = EEPROM.read32(20);
  if (Ftx != temp){
    EEPROM.write32(20, Ftx);
  }
  
  temp = Ftone;
  Ftone = EEPROM.read(24);
  if (Ftone != temp){
    Ftone = temp;
    EEPROM.write(24, Ftone);
  }

  temp = SI5351_DRIVE_CLK0;
  SI5351_DRIVE_CLK0 = EEPROM.read(26); // Driver current
  if (SI5351_DRIVE_CLK0 != temp){
    SI5351_DRIVE_CLK0 = temp;
    EEPROM.write(26, SI5351_DRIVE_CLK0);
  }
  
  temp = SI5351_DRIVE_CLK1;
  SI5351_DRIVE_CLK1 = EEPROM.read(27); // Driver current
  if (SI5351_DRIVE_CLK1 != temp){
    SI5351_DRIVE_CLK1 = temp;
    EEPROM.write(27, SI5351_DRIVE_CLK1);
  }
  
  temp = SI5351_DRIVE_CLK2;
  SI5351_DRIVE_CLK2 = EEPROM.read(28); // Driver current
  if (SI5351_DRIVE_CLK2 != temp){
    SI5351_DRIVE_CLK2 = temp;
    EEPROM.write(28, SI5351_DRIVE_CLK2);
  }

  temp = ENC_SPIN;
  ENC_SPIN = EEPROM.read(29);
  if (ENC_SPIN != temp){
    ENC_SPIN = temp;
    EEPROM.write(29, ENC_SPIN);
  }
}

void Read_Value_EEPROM() {
/*
  EEPROM
  0-3   ( 4 Byte) IF
  4     ( 1 Byte) XTAL
  5-7   IF_WIDTH
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
/*
  IF = EEPROM.read(0);     //Первая ПЧ
  Serial.println(IF);
  if (IF > 40000000) {
    IF = 0;
    EEPROM.write(0, IF);
  }

/*
  XTAL = EEPROM.read(4);
  if (XTAL > 40 || XTAL < 0) {
    XTAL = 27;
    EEPROM.write(4, XTAL);
  }
*/
  IF = 503000;
  XTAL = 30;
  Fcorr = 0;
  /*
  Fcorr = EEPROM.read32(16); //Fcorr = 0;
  if (Fcorr > 5000000 || Fcorr < -5000000 || Fcorr == -1){
      Fcorr = 0;
      EEPROM.write(16, Fcorr);
  }
*/
  Ftx = EEPROM.read32(20);   //Ftx  = 311000000;
  if (Ftx > 4000000000 || Ftx < 0){
    Ftx = 2920*1000*100;
    EEPROM.write(20, Ftx);
  }

  Ftone = EEPROM.read(24); //Ftone  = 1000;
  if (Ftone > 20000){
    Ftone = 1000;
    EEPROM.write(24, Ftone);
  }

  SI5351_DRIVE_CLK0 = EEPROM.read(26); // Driver current
  if (SI5351_DRIVE_CLK0 > 8){
    SI5351_DRIVE_CLK0 = 2;
    EEPROM.write(26, SI5351_DRIVE_CLK0);
  }

  SI5351_DRIVE_CLK1 = EEPROM.read(27); // Driver current
  if (SI5351_DRIVE_CLK1 > 8){
    SI5351_DRIVE_CLK1 = 2;
    EEPROM.write(27, SI5351_DRIVE_CLK1);
  }

  SI5351_DRIVE_CLK2 = EEPROM.read(28); // Driver current
  if (SI5351_DRIVE_CLK2 > 8){
    SI5351_DRIVE_CLK2 = 2;
    EEPROM.write(28, SI5351_DRIVE_CLK2);
  }

  ENC_SPIN = EEPROM.read(29);
  if (ENC_SPIN > 1 || ENC_SPIN < -1){
    ENC_SPIN = 1;
    EEPROM.write(29, ENC_SPIN);
  }

  STEP = 1000;
}// End Read EEPROM

char * uintToStr( const uint64_t num){
  char *str;
  uint8_t i = 0;
  uint64_t n = num;
 
  do
    i++;
  while ( n /= 10 );
 
  str[i] = '\0';
  n = num;
 
  do
    str[--i] = ( n % 10 ) + '0';
  while ( n /= 10 );

  return str;
}

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
  VDTCR |= 1<<7;
  VDTCR &= ~(1<<SWR);
}//End soft reset

void Frequency() {
  unsigned long ustime1, ustime2;
  int i, mhzFreq, iMax = 10000;

  Serial.print("F_CPU = ");
  Serial.println(F_CPU);

  ustime1 = micros();

  for(i=iMax; i>0; i--) __asm__("nop\n\t");

  ustime2 = micros();
  mhzFreq = 5 * (long)iMax / (ustime2 - ustime1) + 1;

  Serial.print("Frequency = ");
  Serial.print(mhzFreq);
  Serial.println(" MHz");
  Serial.println();
}
