#include <DueTimer.h>
// Encoder 신호를 입력 받기 위한 포트 번호 설정
int pA = 26; // phase-A of rotary encoder
int pB = 28; // phase-B of rotary encoder
int pZ = 30; // phase-Z of rotary encoder

volatile signed long cnt = 0;	// 엔코더 신호 카운터 변수 (한 바퀴 돌면 0으로 초기화 됨)
volatile signed long cnt2 = 0;	// 엔코더 신호 카운터 변수 (0으로 초기화 되지 않고 계속 증가 또는 감소함)
volatile signed char dir = 1;	// 모터 회전 방향 변수 (1 또는 -1)
float RPM = 0;					// RPM 변수
signed long cnt2_old = 0;		// RPM 계산을 위한 이전 스텝의 카운터 변수
float dT = 50.0;				// step time (단위: ms)
float deg = 0;					// 엔코더 각도를 나타내는 변수
float desired_RPM = 2000;		// 제어 목표 RPM
float error_RPM = 0;			// RPM 오차
float error_RPM_old = 0;		// 이전 스텝에서의 RPM 오차
float d_error = 0;				// 에러의 변화율(에러의 시간에 대한 미분값)
float error_sum = 0;			// 에러 합산 (I 제어에 사용)
float desired_deg = 600;		// 제어 목표 각도
float error_deg = 0;			// 각도 오차
float error_deg_old = 0;		// 이전 스텝에서의 각도 오차

//void moterControlTimer();		// 함수 원형 선언 (아두이노에서도 이렇게 해줘야하는지??)

// 아두이노 기본 설정
void setup() {
    Serial.begin(9600);			// 시리얼 통신 속도 정의
    attachInterrupt(pA, encoderCount, FALLING);	// 엔코더 A상 신호의 Falling Edge를 인터럽트로 설정하고 encoderCount() 함수 호출
    pinMode(pB, INPUT);							// 엔코더 B상 신호를 입력 받기 위해 pB 포트를 '입력'으로 설정
    attachInterrupt(pZ, encoderReset, FALLING);	// 엔코더 Z상 신호의 Falling Edge를 인터럽트로 설정하고 encoderReset() 함수 호출
    analogWriteResolution(12);					// DAC 포트의 Resolution을 12비트로 설정 (0 ~ 4095 단계로 조절 가능)
    Timer3.attachInterrupt(moterControlTimer);	// Timer3가 작동할 때마다 제어기 함수 호출
    Timer3.start(50000); 						// 매 50000us (=50ms)마다 Timer3가 작동됨
}

void loop() { 
    // You don't need to code on this function.
}

void moterControlTimer(){

	// 속도 제어
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
    Serial.print(desired_RPM); Serial.print(','); Serial.println(RPM);	// 시리얼로 데이터 전송 (그래프로도 확인 가능)


/*	// 위치(각도) 제어
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
    Serial.print(desired_deg); Serial.print(','); Serial.println(deg);	// 시리얼로 데이터 전송 (그래프로도 확인 가능)
*/
    
    // RPM과 각도 계산
    RPM =  ((cnt2-cnt2_old)/1024.0)*60.0*(1000/dT);
    cnt2_old = cnt2;
    deg = cnt2/1024.0*360.0;
  }




void encoderCount() { // A상 신호의 폴링 에지에서 호출됨
    //B상의 값에 따라 회전 방향을 결정한다.
    dir = (digitalRead(pB)==HIGH)? 1:-1; 
    cnt += dir;
    cnt2 += dir;    
}
void encoderReset() { // Z상 신호의 폴링 에지에서 호출됨
    cnt = 0;
}

int outputValue(float desired_voltage){
/* 함수 설명:
이 함수는 제어 입력을 모터 드라이버 입력으로 변환시켜주는 코드이다.
아두이노의 DAC 포트에서는 이론상 0~3.3V까지의 전압을 만들 수 있다. (실제로는 약 0.57~2.79V가 나옴)
따라서 모터 드라이버의 기준 전압을 1.5V로 설정하고 모터 드라이버 입력 신호가 1.5V보다 크거나 작음에 따라 모터가 정방향 또는 역방향 회전을 하도록 미리 설정하였다.
예를 들면, 모터 드라이버 입력에 1.5V가 주어졌을 때 모터로 0A를 보낸다. (모터 정지)
모터 드라이버 입력에 2.5V가 주어졌을 때 모터로 1A를 보낸다. (정방향 회전)
모터 드라이버 입력에 0.5V가 주어졌을 때 모터로 -1A를 보낸다. (역방향 회전)
본 함수는 제어 입력(desired_voltage)이 0인 경우 (모터가 정지할 수 있도록) DAC 포트에서 1.5V가 나오도록 하는 DAC 값(outputValue)을 생성해준다.
*/

  float outputValue;
/*
  if (millis()<1000){			// 초기 1초 동안 모터가 정지 마찰력을 이겨내도록 input offset
    if (desired_voltage > 0){
      desired_voltage += 0.3;
    }else if (desired_voltage < 0){
      desired_voltage -= 0.15;
    }
  }
*/
	// input voltage 오버플로우 방지를 위한 input saturation
  if (desired_voltage > 1.29){			// DAC 최대값: 1.5 + 1.29 = 2.79V
    desired_voltage = 1.29;
  }
  else if (desired_voltage < -0.93){	// DAC 최소값: 1.5 - 0.93 = 0.57V
    desired_voltage = -0.93;
  }

  // 원하는 출력 전압을 만들기 위한 DAC 값 계산 (수정 금지)
  outputValue = (desired_voltage+1.5-0.57)/(0.0005445);
  
  return int(outputValue); 
}

