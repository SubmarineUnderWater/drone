#include <Wire.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  Wire.begin();
  Wire.setClock(400000);

  Wire.beginTransmission(0x68);
  Wire.write(0x6b);
  Wire.write(0x0);
  Wire.endTransmission(true);
}

void loop() {
  // put your main code here, to run repeatedly:
  Wire.beginTransmission(0x68);
  Wire.write(0x3b);
  Wire.endTransmission(true);
  Wire.requestFrom((uint16_t) 0x68, (uint8_t) 14, true);

  int16_t AcXH = Wire.read();
  int16_t AcXL = Wire.read();
  int16_t AcYH = Wire.read();
  int16_t AcYL = Wire.read();
  int16_t AcZH = Wire.read();
  int16_t AcZL = Wire.read();
  int16_t TmpH = Wire.read();
  int16_t TmpL = Wire.read();
  int16_t GyXH = Wire.read();
  int16_t GyXL = Wire.read();
  int16_t GyYH = Wire.read();
  int16_t GyYL = Wire.read();
  int16_t GyZH = Wire.read();
  int16_t GyZL = Wire.read();

  int16_t AcX=AcXH<<8|AcXL;
  int16_t AcY=AcYH<<8|AcYL;
  int16_t AcZ=AcZH<<8|AcZL;
  int16_t GyX=GyXH<<8|GyXL;
  int16_t GyY=GyYH<<8|GyYL;
  int16_t GyZ=GyZH<<8|GyZL;

  static int32_t AcXSum=0, AcYSum=0, AcZSum=0;
  static int32_t GyXSum=0, GyYSum=0, GyZSum=0;

  static double AcXOff=0.0, AcYOff=0.0, AcZOff=0.0;
  static double GyXOff=0.0, GyYOff=0.0, GyZOff=0.0;

  static int cnt_sample=1000;
  
  if(cnt_sample>0){
    AcXSum+=AcX; AcYSum+=AcY;AcZSum+=AcZ;
    GyXSum+=GyX; GyYSum+=GyY;GyZSum+=GyZ;
    cnt_sample--;

    if(cnt_sample==0){
      AcXOff=AcXSum/1000.0;
      AcYOff=AcYSum/1000.0;
      AcZOff=AcZSum/1000.0;
      GyXOff=GyXSum/1000.0;
      GyYOff=GyYSum/1000.0;
      GyZOff=GyZSum/1000.0;
    }
    delay(1);
    return;
  }

  double AcXD=AcX-AcXOff;
  double AcYD=AcY-AcYOff;
  double AcZD=AcZ-AcZOff+16384;

  double GyXD=GyX-GyXOff;
  double GyYD=GyY-GyYOff;
  double GyZD=GyZ-GyZOff;

  static unsigned long t_prev=0;
  unsigned long t_now = micros();
  double dt = (t_now-t_prev)/1000000.0;
  t_prev=t_now;

  const float GYROXYZ_TO_DEGREES_PER_SEC=131;
  double GyXR=GyXD/GYROXYZ_TO_DEGREES_PER_SEC;
  double GyYR=GyYD/GYROXYZ_TO_DEGREES_PER_SEC;
  double GyZR=GyZD/GYROXYZ_TO_DEGREES_PER_SEC;

  const float RADIANS_TO_DEGREES=180/3.14159;
  double AcYZD=sqrt(pow(AcYD, 2)+pow(AcZD, 2));
  double AcXZD=sqrt(pow(AcXD, 2)+pow(AcZD, 2));
  double acAngleY=atan(-AcXD/AcYZD)*RADIANS_TO_DEGREES;
  double acAngleX=atan(-AcYD/AcXZD)*RADIANS_TO_DEGREES;
  double acAngleZ=0;
  
  static double gyAngleX=0.0, gyAngleY=0.0, gyAngleZ=0.0;
  gyAngleX+=GyXR*dt;
  gyAngleY+=GyYR*dt;
  gyAngleZ+=GyZR*dt;

  const double ALPHA = 0.96;
  static double cmAngleX=0.0, cmAngleY=0.0, cmAngleZ=0.0;
  cmAngleX=ALPHA*(cmAngleX+GyXR*dt)+(1.0-ALPHA)*acAngleX;
  cmAngleY=ALPHA*(cmAngleY+GyYR*dt)+(1.0-ALPHA)*acAngleY;
  cmAngleZ=gyAngleZ;

  static double tAngleX=0.0, tAngleY=0.0, tAngleZ=0.0;
  double eAngleX=tAngleX-cmAngleX;
  double eAngleY=tAngleY-cmAngleY;
  double eAngleZ=tAngleZ-cmAngleZ;
  
  double Kp=1.0;
  double BalX=Kp*eAngleX;
  double BalY=Kp*eAngleY;
  double BalZ=Kp*eAngleZ;

  static int cnt_loop;
  cnt_loop++;
  if(cnt_loop%200!=0) return;
  
//  Serial.printf(" AcX = %8.1f", AcXD);
//  Serial.printf(" | AcY = %8.1f", AcYD);
//  Serial.printf(" | AcZ = %8.1f", AcZD);
//  Serial.printf(" | GyX = %8.1f", GyXD);
//  Serial.printf(" | GyY = %8.1f", GyYD);
//  Serial.printf(" | GyZ = %8.1f", GyZD);
  Serial.printf("dt = %8.6f", dt);
  Serial.printf(" | cmAngleX = %6.1f", cmAngleX);
  Serial.printf(" | cmAngleY = %6.1f", cmAngleY);
  Serial.printf(" | cmAngleZ = %6.1f", cmAngleZ);
  Serial.printf(" | BalX = %6.1f", BalX);
  Serial.printf(" | BalY = %6.1f", BalY);
  Serial.printf(" | BalZ = %6.1f", BalZ);
  Serial.println();
}
