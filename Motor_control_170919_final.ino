#include <DueTimer.h>
int pA = 26; // phase-A of rotary encoder
int pB = 28; // phase-B of rotary encoder
int pZ = 30; // phase-Z of rotary encoder

volatile signed long cnt = 0;
volatile signed long cnt2 = 0;
volatile signed char dir = 1;
volatile signed long T = 0;
float RPM = 0;
signed long cnt2_old = 0;
float dT = 50.0;
float deg = 0;
int value = 0;
float desired_RPM = 2000;
float error_RPM = 0;
float error_RPM_old = 0;
float d_error = 0;
float error_sum = 0;
float desired_deg = 600;
float error_deg = 0;
float error_deg_old = 0;
char key = '0';
int k = 2047;
void moterControlTimer();



void setup() {
    Serial.begin(9600);
    attachInterrupt(pA, encoderCount, FALLING);
    pinMode(pB, INPUT);
    attachInterrupt(pZ, encoderReset, FALLING);
    analogWriteResolution(12);
    Timer3.attachInterrupt(moterControlTimer);
    Timer3.start(50000); // Calls every 50ms
}

void loop() {
    
    
}

void moterControlTimer(){

    //float P_gain = 0.0018;
    //float I_gain = 0.0002;
    //float D_gain = 0.0001;
    float P_gain = 0.0035;
    float I_gain = 0.0012;
    float D_gain = 0.0000;
    float control_input = 0;
    error_RPM = desired_RPM - RPM;    
    d_error = (error_RPM - error_RPM_old) / (dT/1000);
    error_sum += error_RPM*(dT/1000);
    control_input = error_RPM*P_gain + error_sum*I_gain + d_error*D_gain;
    error_RPM_old = error_RPM;
    analogWrite(DAC0, outputValue(control_input));
    Serial.print(desired_RPM); Serial.print(','); Serial.println(RPM);


/*
    float P_gain = 0.0003;
    float I_gain = 0.00003;
    float D_gain = 0.00003;
    float control_input = 0;
    error_deg = desired_deg - deg;
    error_sum += error_deg*(dT/1000);
    d_error = (error_deg - error_deg_old) / (dT/1000);
    control_input = error_deg*P_gain + error_sum*I_gain + d_error*D_gain;
    error_deg_old = error_deg;
    analogWrite(DAC0, outputValue(control_input));
    Serial.print(desired_deg); Serial.print(','); Serial.println(deg);  
*/
    
    RPM =  ((cnt2-cnt2_old)/1024.0)*60.0*(1000/dT);
    cnt2_old = cnt2;
    deg = cnt2/1024.0*360.0;
  }




void encoderCount() { // A상 신호의 폴링 에지에서 호출됨
    //B상의 값에 따라 방향을 결정한다.
    dir = (digitalRead(pB)==HIGH)? 1:-1; 
    cnt += dir;
    cnt2 += dir;

    //T = micros()*0.000001 - T;
    //RPM = 60/(1024*T);
    
}
void encoderReset() { // Z상 신호의 폴링 에지에서 호출됨
    cnt = 0;
}

int outputValue(float desired_voltage){
  float outputValue;

  if (millis()<1000){
    if (desired_voltage > 0){
      desired_voltage += 0.3;
    }else if (desired_voltage < 0){
      desired_voltage -= 0.15;
    }
  }
  if (desired_voltage > 1.29){
    desired_voltage = 1.29;
  }
  else if (desired_voltage < -0.93){
    desired_voltage = -0.93;
  }
  outputValue = (desired_voltage+1.5-0.57)/(0.0005445);

  
  return int(outputValue); 
}

