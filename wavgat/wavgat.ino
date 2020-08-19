#include "lgtx8p.h"
#include "si5351.h"
#include "Wire.h"
#include "ShiftedLCD.h"
#include <EEPROM.h>
#include "variables.h"
#include "function.h"

void setup() {
  Serial.begin(115200);
  Serial.println("Hi, my name is Amator-SA !!!");
  
  Frequency();

  pinOutConfig();

  Read_Value_EEPROM();
  si5351Set();
  
  si5351.set_freq(Ftx + (IF*100), SI5351_CLK0); //Set RX
  si5351.set_freq(IF*100, SI5351_CLK2); //Set RX
 
  lcd.begin(16, 2);  /* Инициализируем дисплей: 2 строки по 16 символов */
  F_print();
  
  if (IF == 503000) {
    lcd.setCursor(13,0);   lcd.print("USB");
  } else {
    lcd.setCursor(13,0);   lcd.print("LSB");
  }
  
  lcd.setCursor(10,1);   lcd.print("  1kHz  ");

  currentTime = millis();
  loopTime = currentTime; 
}

void loop() {
  while(1) {
    currentTime = millis();
    //-------------------Проверка каждые 5 мс
    if(currentTime >= (loopTime + 7)) {
      Check_enc();
      test();        
      loopTime = currentTime; // Счетчик прошедшего времени
      SWR_count++;
    }

    if ((SWR_count > 16) and tx_flag) {
      SWR_count = 1;
      AVR_count++;  
      uFRD += analogRead(FRD);
      uREV += analogRead(REV);

      if (AVR_count > 10) {
        uFRD = uFRD/AVR_count;
        uREV = uREV/AVR_count;
        SWR_Print();
        AVR_count = 1;
      }//End  if (count_avr > 3) 
    }//End s_meter
  
    if (step_flag) {
      if(enc_flag || rewrite_flag) {
        lcd.setCursor(10,1);
        step_count = step_count+enc_move;
        if (step_count > 6) { step_count = 6;};
        if (step_count == 0) { step_count = 1;};

        switch (step_count) {
          case  1: STEP=10;       lcd.print("  10"); break;   //10Hz
          case  2: STEP=100;      lcd.print(" 100"); break;   //100Hz
          case  3: STEP=1000;     lcd.print("  1k"); break;   //1kHz
          case  4: STEP=5000;     lcd.print("  5k"); break;
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
  }
}

void test() {
    // Проверка кнопок
    if (Button_flag == 0) {
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
            IF = 500000;
            lcd.setCursor(13,0);
            lcd.print("LSB");
          } 
          if (!lsb_usb_flag) {
            IF = 503000;
            lcd.setCursor(13,0);
            lcd.print("USB");
          }
        if ( tx_flag ) {
            si5351.set_freq( Ftx + (IF*100), SI5351_CLK2);
            si5351.set_freq( IF*100 , SI5351_CLK0);
        } else {
            si5351.set_freq( Ftx + (IF*100), SI5351_CLK0);
            si5351.set_freq( IF*100 , SI5351_CLK2);
        }
      }//End Button_LSB_USB
        
      if (digitalRead(Button_RIT) == 0){
        Button_flag = true;                 
        rit_flag = !rit_flag;
        menu_count = 0;
        rit_flag && !setup_flag ? tone(tone_pin,Ftone) : noTone(tone_pin);
      }//End Button_RIT
    }

    if (digitalRead(Button_TX) == 1 && digitalRead(Button_STEP) == 1 && digitalRead(Button_RIT) == 1 && digitalRead(Button_LSB_USB) == 1 && Button_flag == true){
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
                } else {
                    enc_flag = true;
                    F_setup();
                }
        }
    }
  //конец проверок кнопок

  if (enc_flag && enc_block == 0) {
    Ftx += (STEP*100)*enc_move;

    if (Ftx < Fmin){ Ftx = Fmin; }
    if (Ftx > Fmax){ Ftx = Fmax; }

    if (tx_flag == true) {
      si5351.set_freq(Ftx + (IF*100), SI5351_CLK2);
      si5351.set_freq(IF*100, SI5351_CLK0);
    } else {
      si5351.set_freq(Ftx + (IF*100), SI5351_CLK0);
      si5351.set_freq(IF*100, SI5351_CLK2);
    }

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

void Check_enc() {
  Enc_state = PIND&B00001100;
  enc_move = 0;
  
  if( Enc_state != Enc_last ){
        if(Enc_last == 12){
              if(Enc_state == 4) enc_move=-1*ENC_SPIN;
              if(Enc_state == 8) enc_move=1*ENC_SPIN;
        }
    Enc_last = Enc_state;
    enc_flag = true;
  }//End Проверка состояния encoder
}//End Check Enc

void F_tx() {
  if(enc_block) return;

  tx_flag = !tx_flag;
  if (tx_flag == true) {
      si5351.set_freq(Ftx + (IF*100), SI5351_CLK2);
      si5351.set_freq(IF*100, SI5351_CLK0);
      digitalWrite(TX_out, HIGH);

      lcd.setCursor(0,1);
      lcd.print("TX");
  } else {
      digitalWrite(TX_out, LOW);
      si5351.set_freq(Ftx + (IF*100), SI5351_CLK0);
      si5351.set_freq(IF*100, SI5351_CLK2);
      lcd.setCursor(0,1);
      lcd.print("                ");

      step_flag = 1;
      rewrite_flag = 1; 
  }
}//End F tx

void F_setup() {
    if (setup_flag == false) {
        enc_block = true;
        setup_flag = true;

        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Setup");
        delay(400);
        lcd.clear();
    }

    if (enc_flag) {
        if(rit_flag) {
          setup_count = setup_count + enc_move;
          if (setup_count > 12) { setup_count = 12; };
          if (setup_count == 0) { setup_count = 1; };
        }
        else {
	        switch (setup_count) {
            case 1: IF+=STEP*enc_move; if(IF > 4200000000) IF = 500000; if(IF > 40000000) IF = 40000000; break;
            case 2: XTAL+=enc_move; if(XTAL > 40) XTAL=40; break;
            case 3: Fcorr+=STEP*enc_move; if(Fcorr > 10000000) Fcorr=10000000; if(Fcorr < -10000000) Fcorr=-10000000;  break;
            case 4: Ftx+=(STEP*100)*enc_move; if(Ftx > Fmax) Ftx=Fmax; if(Ftx < Fmin) Ftx=Fmin; break;
            case 5: Ftone += STEP*enc_move/10; break;
            case 6: if(enc_move == 1){ ftone_flag = true; tone(tone_pin, Ftone); } if(enc_move == -1){ ftone_flag = false; noTone(tone_pin); }; break;
            case 7: if(enc_move == 1) { F_eeprom_w(); lcd.setCursor(0,1); lcd.print("    Complite!!! "); delay (1000); }; break; // EEPROM write
            case 8: if(enc_move == 1) softReset(); break; // soft reboot
            case 9: SI5351_DRIVE_CLK0+=enc_move*2; if(SI5351_DRIVE_CLK0 < 2) SI5351_DRIVE_CLK0 = 2; if(SI5351_DRIVE_CLK0 > 8) SI5351_DRIVE_CLK0 = 8;   break;
            case 10: SI5351_DRIVE_CLK1+=enc_move*2; if(SI5351_DRIVE_CLK1 < 2) SI5351_DRIVE_CLK1 = 2; if(SI5351_DRIVE_CLK1 > 8) SI5351_DRIVE_CLK1 = 8;   break;
            case 11: SI5351_DRIVE_CLK2+=enc_move*2; if(SI5351_DRIVE_CLK2 < 2) SI5351_DRIVE_CLK2 = 2; if(SI5351_DRIVE_CLK2 > 8) SI5351_DRIVE_CLK2 = 8;   break;
            case 12: if(enc_move == 1) ENC_SPIN = 1; if(enc_move == -1) ENC_SPIN = -1; break;
          }
        }
        
        lcd.clear();
        lcd.setCursor(0,0);
	      
        switch (setup_count) {
          case 1:  lcd.print("IF");lcd.setCursor(0,1); lcd.print(IF); break;
          case 2:  lcd.print("XTAL");lcd.setCursor(0,1); lcd.print(XTAL);lcd.print("MHz "); break;
          case 3:  lcd.print("Fcorr");lcd.setCursor(0,1); lcd.print(Fcorr); break;
          case 4:  lcd.print("Ftx");lcd.setCursor(0,1); lcd.print(Ftx/100); break;
          case 5:  lcd.print("Ftone");lcd.setCursor(0,1); lcd.print(Ftone); break;
          case 6:  lcd.print("Ftone On/Off");lcd.setCursor(0,1);if(ftone_flag) lcd.print("On"); else lcd.print("Off"); break;
          case 7:  lcd.print("EEPROM Save"); lcd.setCursor(0, 1); lcd.print("No/Yes?"); break;
          case 8:  lcd.print("Reboot No/Yes?"); break;
          case 9:  lcd.print("DRIVE_CLK0"); lcd.setCursor(0, 1); lcd.print(SI5351_DRIVE_CLK0); break;
          case 10: lcd.print("DRIVE_CLK1"); lcd.setCursor(0, 1); lcd.print(SI5351_DRIVE_CLK1); break;
          case 11: lcd.print("DRIVE_CLK2"); lcd.setCursor(0, 1); lcd.print(SI5351_DRIVE_CLK2); break;
          case 12: lcd.print("ENC_SPIN"); lcd.setCursor(0, 1); lcd.print(ENC_SPIN); break;
        }
  
        enc_flag = false;
    }
}// End F setup
