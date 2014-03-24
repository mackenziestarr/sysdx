/* DX7.c — an Arduino-based sysex controller for the Yamaha DX7
   Created by: Mackenzie Starr 10.13.13
*/

// Sysex Frame [status][id][sub_status][parameter_group][*parameter_number][*data_byte][EOX]

#include <MIDI.h>
#include <MuxShield.h>
#define OP 6 //#_of_operators


MuxShield muxShield;

//Detune/Fine/Coarse Switch
const int sw_3A = 3;
const int sw_3B = 5;
int sw_state_A = digitalRead(sw_3A);
int sw_state_B = digitalRead(sw_3B);


//EG Select
const int  buttonPin = 9;
int buttonPushCounter = 0;   
int buttonState = 0;       
int lastButtonState = 0; 


//Frequency Arrays
int IO2AnalogVals[OP];
int oldAnalogVals[OP];
int detune[OP];
int coarse[OP];
int fine[OP];

//Output Level Arrays
int IO2_output_val[OP];
int prev_output_val[OP];


//Fixed/Ratio Arrays
int IO3_fixed_ratio[OP];
int prev_fixed_ratio[OP];


//EG lvl Arrays
int IO2_eg_lvl[4];
int prev_eg_lvl[4];
int eg_sel;


//SYSEX FRAME for YAMAHA DX Series
byte data[5]={0x43, 0x10, 0x00, 0x00, 0x00};


//writes a number to a seven_segment display
void seven_seg_write(int digit){

  //seven segment display matrix
  byte seven_seg_digits[10][7] = { { 1,1,1,1,1,1,0 },  // = 0
                                 { 0,1,1,0,0,0,0 },      // = 1
                                 { 1,1,0,1,1,0,1 },      // = 2
                                 { 1,1,1,1,0,0,1 },      // = 3
                                 { 0,1,1,0,0,1,1 },      // = 4
                                 { 1,0,1,1,0,1,1 },      // = 5
                                 { 1,0,1,1,1,1,1 },      // = 6
                                 { 1,1,1,0,0,0,0 },      // = 7
                                 { 1,1,1,1,1,1,1 },      // = 8
                                 { 1,1,1,0,0,1,1 }};     // = 9
        for (int i=0; i < 7; i++){
          //write only ones digit
         muxShield.digitalWriteMS(1,i,seven_seg_digits[digit][i]);
        }
        delay(1000);
}


//initialize muxShield, Midi.h and INPUT/OUTPUT pins
void setup(){
  MIDI.begin(4);     //channel 0
  muxShield.setMode(2,ANALOG_IN);
  muxShield.setMode(3,ANALOG_IN);
  muxShield.setMode(1,DIGITAL_OUT); 
  pinMode(sw_3A, INPUT);
  pinMode(sw_3B, INPUT);
  pinMode(buttonPin, INPUT);
  digitalWrite(sw_3A, HIGH); 
  digitalWrite(sw_3B, HIGH);
  seven_seg_write(0);

}



//execute
void loop(){  


  //COARSE_FREQ OP: 1-6
  for (int i=0; i < OP; i++){
  //Analog read on first 6 inputs on IO2 AND map(~0-31)
  fine[i] = map(muxShield.analogReadMS(2,i), 0, 1023, 0, 99);
  detune[i] = map(muxShield.analogReadMS(2,i), 0, 1023, 0, 14);
  coarse[i] = map(muxShield.analogReadMS(2,i), 0, 1023, 0, 31);

  if (abs(fine[i] - oldAnalogVals[i]) >= 1){
      if(sw_state_A == LOW){
        //FINE
        data[3]= (byte)(124 - (i * 21));
        data[4]= (byte)fine[i];
        MIDI.sendSysEx(5, data);
        delay(80);
        }
      else if(sw_state_B == LOW){
        //DETUNE
        data[3]= (byte)(125 - (i * 21));
        data[4]= (byte)detune[i];
        MIDI.sendSysEx(5, data);
        delay(80);
        }
      else{
        //COARSE
        data[3]= (byte)(123 - (i * 21));
        data[4]= (byte)coarse[i];
        MIDI.sendSysEx(5, data);
        delay(80);
      }
    }
  oldAnalogVals[i] = fine[i];
  }


  //OUTPUT LVL OP: 1-6
  for (int i=0; i < OP; i++){
    IO2_output_val[i] = map(muxShield.analogReadMS(2, i+6), 0, 1023, 0, 99);
    if(abs(IO2_output_val[i] - prev_output_val[i]) >= 1){
      data[3]= (byte)(121 - (i * 21));
      data[4]= (byte)IO2_output_val[i];
      MIDI.sendSysEx(5, data);
      delay(80);
    }
  prev_output_val[i] = IO2_output_val[i];

  }

  //FIXED/RATIO OP:1-6
  for (int i=0; i < OP; i++){
    IO3_fixed_ratio[i] = map(muxShield.analogReadMS(3, i), 0, 1023, 0, 1);
    if(abs(IO3_fixed_ratio[i] - prev_fixed_ratio[i]) == 1){
      data[3]= (byte)(122 - (i * 21));
      data[4]= (byte)IO3_fixed_ratio[i];
      MIDI.sendSysEx(5, data);
      delay(80);
    }
  prev_fixed_ratio[i] = IO3_fixed_ratio[i];

  }

  //EG LVL
  for (int i=0; i < 4; i++){
    IO2_eg_lvl[i] = map(muxShield.analogReadMS(2, i+12), 0, 1023, 0, 99);
    if(abs(IO2_eg_lvl[i] - prev_eg_lvl[i]) >= 1){
      if (eg_sel == 6){
        data[3]= (byte)(4 + i);
        data[4]= (byte)IO2_eg_lvl[i];
      }
      else if (eg_sel == 5){
        data[3]= (byte)(25 + i);
        data[4]= (byte)IO2_eg_lvl[i];
      }
      else if (eg_sel == 4){
        data[3]= (byte)(46 + i);
        data[4]= (byte)IO2_eg_lvl[i];
      }
      else if (eg_sel == 3){
        data[3]= (byte)(67 + i);
        data[4]= (byte)IO2_eg_lvl[i];
      }
      else if (eg_sel == 2){
        data[3]= (byte)(88 + i);
        data[4]= (byte)IO2_eg_lvl[i];
      }
      else if (eg_sel == 1){
        data[3]= (byte)(88 + i);
        data[4]= (byte)IO2_eg_lvl[i];
      }
        MIDI.sendSysEx(5, data);
        delay(80);
      }
    prev_eg_lvl[i] = IO2_eg_lvl[i];
    }


  sw_state_A = digitalRead(sw_3A);
  sw_state_B = digitalRead(sw_3B);

  //EG OP SELECT
  buttonState = digitalRead(buttonPin);
  if (buttonState != lastButtonState) {
    // if the state has changed, increment the counter
    if (buttonState == HIGH) {
      // if the current state is HIGH then the button
      // wend from off to on:
      buttonPushCounter++;
      seven_seg_write(buttonPushCounter % 6 + 1);
      eg_sel = buttonPushCounter % 6 + 1;

    }
  lastButtonState = buttonState;
  }

}




















