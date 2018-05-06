#include "si5351.h"
#include "Wire.h"
#include "ShiftedLCD.h"
#include <EEPROM.h> // Для работы со встроенной памятью ATmega
#include "function.h"

void setup(){

  Serial.begin(9600);
  
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
    
  Read_Value_EEPROM();
  
  si5351.init(SI5351_CRYSTAL_LOAD_0PF, XTAL*M, Fcorr);

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
  si5351.set_freq(Ftx+IF, SI5351_CLK0); //Set RX
  si5351.set_freq(IF, SI5351_CLK2); //Set RX
 
  lcd.begin(16, 2);  /* Инициализируем дисплей: 2 строки по 16 символов */
  F_print();
  
  if (IF == 50300000) {
    lcd.setCursor(13,0);   lcd.print("USB");
  } else {
    lcd.setCursor(13,0);   lcd.print("LSB");
  }
  
  lcd.setCursor(10,1);   lcd.print("  1kHz  ");

  currentTime = millis();
  loopTime = currentTime; 
}

void loop(){
  currentTime = millis();
//-------------------Проверка каждые 3 мс
  if(currentTime >= (loopTime + 3)) {
    Check_enc();
    test();        
    loopTime = currentTime; // Счетчик прошедшего времени
    SWR_count++;
  }
//-------------------Конец проверка каждые 3 мс

  if (tx_flag) {
    if (SWR_count == 33) {
      uFRD += analogRead(FRD);
      uREV += analogRead(REV);
      AVR_count++;
      SWR_count = 0;
    }

    if (AVR_count == 10) {
      uFRD = uFRD/10;
      uREV = uREV/10;

      lcd.setCursor(0,1);
      lcd.print("P");
      lcd.print("    ");
      lcd.setCursor(1,1);
      lcd.print(uFRD);
      
      lcd.setCursor(5,1);
      lcd.print("F");
      lcd.print("    ");
      lcd.setCursor(6,1);
      lcd.print(uREV);

      lcd.setCursor(9,1);
      lcd.print("SWR");
      lcd.print( (float(uFRD+uREV)/(uFRD-uREV)) );
      lcd.print("    ");
      AVR_count == 0;
    }
  }//End s_meter

  if (step_flag) {
    if(enc_flag || rewrite_flag) {      
      lcd.setCursor(10,1);
      step_count = step_count+enc_move;

      if (step_count > 6){ step_count = 6;};
      if (step_count == 0){ step_count = 1;};

        switch (step_count){
                        case  1: STEP=1;        lcd.print("   1"); break;   //1Hz
                        case  2: STEP=10;       lcd.print("  10"); break;   //10Hz
                        case  3: STEP=100;      lcd.print(" 100"); break;   //100Hz
                        case  4: STEP=1000;     lcd.print("  1k"); break;   //1kHz
                        case  5: STEP=10000;    lcd.print(" 10k"); break;   //10kHz
                        case  6: STEP=100000;   lcd.print("100k"); break;   //100kHz
              }
            lcd.print("Hz");
        if (rewrite_flag){
            step_flag = false;
            rewrite_flag = false;
        }
      enc_flag = false;
    }
  }//End Step flag

    if (setup_flag) {
        F_setup();
    }
}//End loop

void test() {
    // Проверка кнопок
    if (Button_flag == 0)
    {
        if (digitalRead(Button_TX) == 0) {
            Button_flag = true;
            F_tx();
        }//End Button_Tx

        if (digitalRead(Button_STEP) == 0) {
            Button_flag = true;
            step_flag = !step_flag;
            enc_block = !enc_block;
            if(setup_flag){enc_block = true;}
            
        }//End Button_STEP

        if (digitalRead(Button_LSB_USB) == 0) {
            Button_flag = true;
            lsb_usb_flag = !lsb_usb_flag;
            if (lsb_usb_flag) {
              IF = 50000000;
              lcd.setCursor(13,0);
              lcd.print("LSB");
            } 
            if (!lsb_usb_flag) {
              IF = 50300000;
              lcd.setCursor(13,0);
              lcd.print("USB");
            }
          if ( tx_flag ) {
              si5351.set_freq( Ftx + IF, SI5351_CLK2);
              si5351.set_freq( IF , SI5351_CLK0);
          } else {
              si5351.set_freq( Ftx + IF, SI5351_CLK0);
              si5351.set_freq( IF , SI5351_CLK2);
          }
        }//End Button_LSB_USB
        
        if (digitalRead(Button_RIT) == 0){
              Button_flag = true;                 
              rit_flag = !rit_flag;
              menu_count = 0;
              rit_flag && !setup_flag ? tone(tone_pin,Ftone) : noTone(tone_pin);
        }//End Button_RIT
    }

    if (digitalRead(Button_TX) == 1 && digitalRead(Button_STEP) == 1 && digitalRead(Button_RIT) == 1 && digitalRead(Button_LSB_USB) == 1 && Button_flag == true)
    {
            Button_flag = false;
    }

    if ( digitalRead(Button_RIT) == 0 && Button_flag == true && menu_count < 255 ){
            menu_count++;
            if (menu_count == 254){
                if(setup_flag){
                    lcd.clear();
                    enc_block = false;
                    F_print();
                    step_flag = 1;
                    rewrite_flag = 1;
                    setup_flag=false; 
                }
                else{
                    enc_flag = true;
                    F_setup();
                }
        }
    }
  //конец проверок кнопок

    if (enc_flag && enc_block == 0)
    {
          Ftx += (STEP*100)*enc_move;
          if (Ftx < Fmin){ Ftx = Fmin; }
          if (Ftx > Fmax){ Ftx = Fmax; }
          si5351.set_freq(Ftx+IF, SI5351_CLK0);
          si5351.set_freq(IF, SI5351_CLK2);
          enc_flag = false;
        F_print();
    }
}

void F_print(){
  uint16_t mid;
  mid = Ftx/100000;
  lcd.setCursor(0,0);
  lcd.write(' ');
                  
  if ( (mid/1000) > 9) {
    lcd.setCursor(0,0);
  }

  lcd.print(mid/1000);
  lcd.write('.');
  
  mid = (mid%1000);
  lcd.print(mid/100);
  mid = (mid%100);
  lcd.print(mid/10);
  lcd.print( mid%10);
  lcd.write('.');
  
  mid = (Ftx/100)%1000;

  lcd.print( mid/100);
  mid = mid%100;
  lcd.print( mid/10);
  lcd.print( mid%10);
}//end f_print

void Read_Value_EEPROM()
{
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
  29    NULL
  30-33 NULL
  34    ( 1 Byte) x*F
*/
      EEPROM.get(0, IF);     //Первая ПЧ
      if (IF > 4000000000) {
        IF = 0;
        EEPROM.put(0, IF);
      }
      EEPROM.get(4, XTAL);
      if (XTAL > 40 || XTAL < 0) {
        XTAL = 40;
        EEPROM.put(4, XTAL);
      }
      EEPROM.get(16, Fcorr); //Fcorr = 0;
      if (Fcorr > 5000000 || Fcorr < -5000000){
          Fcorr = 0;
          EEPROM.put(16, Fcorr);          
      }
      EEPROM.get(20, Ftx);   //Ftx  = 311000000;
      if (Ftx > 4000000000 || Ftx < 0){
        Ftx = 300000000;
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
      EEPROM.get(34, xF); //xF  = 1; multiplier F
      if (xF > 2){
        xF = 1;
        EEPROM.put(34, xF);
      }
  STEP = 1000;
}// End Read EEPROM

void Check_enc(){

               Enc_state = PIND&B00001100;
               enc_move = 0;
               
               if( Enc_state != Enc_last ){

					switch (Enc_state){
                                                case 4: if (Enc_last == 0) enc_move = 1; if (Enc_last == 12)  enc_move = -1; break;
                                                case 8: if (Enc_last == 12)  enc_move = 1; if (Enc_last == 0)  enc_move = -1; break;
  					}
                  Enc_last = Enc_state;
                  enc_flag = true;
                    
                  Serial.println(IF);
                    
               }//End Проверка состояния encoder

}//End Check Enc

void F_tx(){

  enc_block = !enc_block;
  tx_flag = !tx_flag;
                      if (tx_flag == true){
                          si5351.set_freq(Ftx+IF, SI5351_CLK2);
                          si5351.set_freq(IF, SI5351_CLK0);
                          digitalWrite(TX_out, HIGH);

                          lcd.setCursor(0,1);
                          lcd.print("TX");
                      }
                      else{
                          digitalWrite(TX_out, LOW);
                          si5351.set_freq(Ftx+IF, SI5351_CLK0);
                          si5351.set_freq(IF, SI5351_CLK2);
                          lcd.setCursor(0,1);
                          lcd.print("                ");

                          step_flag = 1;
                          rewrite_flag = 1; 
                      }
}//End F tx

void F_setup(){

      if (setup_flag == false){
  
          enc_block = true;
          setup_flag = true;
  
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Setup");
          delay(400);
          lcd.clear();
      }

    if (enc_flag){
        
        if(rit_flag){
               setup_count = setup_count+enc_move;
               if (setup_count > 14){ setup_count = 14;};
               if (setup_count == 0){ setup_count = 1;};
        }
        else{
	            switch (setup_count){
                             case  1: IF+=(STEP*100)*enc_move; if(IF > 4200000000) IF = 0; if(IF > 4000000000) IF = 4000000000; break;
                             case  2: XTAL+=(STEP)*enc_move; if(XTAL > 40) XTAL=40; break;
                             case  3: if(enc_move == 1) xF = 2; if(enc_move == -1) xF = 1; break; // xF
                             case  4: Fcorr+=(STEP/10)*enc_move; if(Fcorr > 10000000) Fcorr=10000000; if(Fcorr < -10000000) Fcorr=-10000000;  break;
                             case  5: Ftx+=(STEP*100)*enc_move; if(Ftx > Fmax) Ftx=Fmax; if(Ftx < Fmin) Ftx=Fmin; break;
                             case  6: Ftone+=(STEP/10)*enc_move; break;
                             case  8: if(enc_move == 1) { F_eeprom_w(); lcd.setCursor(0,1); lcd.print("    Complite!!! "); delay (1000); }; break; // EEPROM write
                             case  9: if(enc_move == 1) softReset(); break; // soft reboot
                             case  10: SI5351_DRIVE_CLK0+=enc_move*2; if(SI5351_DRIVE_CLK0 < 2) SI5351_DRIVE_CLK0 = 2; if(SI5351_DRIVE_CLK0 > 8) SI5351_DRIVE_CLK0 = 8;   break;
                             case  11: SI5351_DRIVE_CLK1+=enc_move*2; if(SI5351_DRIVE_CLK1 < 2) SI5351_DRIVE_CLK1 = 2; if(SI5351_DRIVE_CLK1 > 8) SI5351_DRIVE_CLK1 = 8;   break;
                             case  12: SI5351_DRIVE_CLK2+=enc_move*2; if(SI5351_DRIVE_CLK2 < 2) SI5351_DRIVE_CLK2 = 2; if(SI5351_DRIVE_CLK2 > 8) SI5351_DRIVE_CLK2 = 8;   break;
                    }
        }
                lcd.clear();
                lcd.setCursor(0,0);
	            switch (setup_count){
                             case  1:    lcd.print("IF");lcd.setCursor(0,1);lcd.print(IF/100); break;
                             case  2:    lcd.print("XTAL");lcd.setCursor(0,1);lcd.print(XTAL);lcd.print("MHz "); break;
                             case  3:    lcd.print("xF"); lcd.setCursor(0,1);lcd.print(xF);break;
                             case  4:    lcd.print("Fcorr");lcd.setCursor(0,1);lcd.print(Fcorr); break;
                             case  5:    lcd.print("Ftx");lcd.setCursor(0,1);lcd.print(Ftx/100); break;
                             case  6:    lcd.print("Ftone");lcd.setCursor(0,1);lcd.print(Ftone); break;
                             case  8:    lcd.print("EEPROM Write");lcd.setCursor(0,1);lcd.print("No/Yes?"); break;
                             case  9:   lcd.print("Reboot No/Yes?"); break;
                             case  10:   lcd.print("DRIVE_CLK0"); lcd.setCursor(0,1);lcd.print(SI5351_DRIVE_CLK0);break;
                             case  11:   lcd.print("DRIVE_CLK1"); lcd.setCursor(0,1);lcd.print(SI5351_DRIVE_CLK1);break;
                             case  12:   lcd.print("DRIVE_CLK2"); lcd.setCursor(0,1);lcd.print(SI5351_DRIVE_CLK2);break;
                    }
       enc_flag = false;
    }

}// End F setup

void F_eeprom_w(){

unsigned long temp=0;
long temp_l=0;

      EEPROM.get(0, temp);     //IF=45000000;        // Расчетная промежуточная частота. 450.000.00
      if (IF != temp){
        EEPROM.put(0, IF);
      }
      
      EEPROM.get(4, temp);
      if (XTAL != temp){
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

      temp = xF;
      EEPROM.get(34, xF); //xF  = 1; multiplier F
      if (xF != temp){
        xF = temp;
        EEPROM.put(34, xF);
      }
}

