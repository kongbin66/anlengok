#include "config.h"
//设置ip5306持续升压
int8_t BatteryLevel;
static bool setPowerBoostKeepOn(bool en)
{
  Wire.beginTransmission(IP5306_ADDR);
  Wire.write(IP5306_REG_SYS_CTL0);
  if (en)
  {
    Wire.write(0b110111);
    // 0x37 = 0b110111 TCALL example

    /*
 [1]      Boost EN (default 1)            [EXM note: if 0 ESP32 will not boot from battery]
 [1]      Charger EN (1)                  [EXM note: did  not observe difference]
 [1]      Reserved (1)                    [EXM note: did  not observe difference]
 [1]      Insert load auto power EN (1)   [EXM note: did  not observe difference]
 [1]      BOOST output normally open ( 1) [EXM note: if 0 will shutdown on ESP32 sleep after 32s]
 [1]      Key off EN: (0)                 [EXM note: could not detect difference]
  */
  }
  else
  {
    // Wire.write(0x35); // x35 => Autoshutdown - Will kill power of ESP32 while sleeping!
    //HEX 35 = 0b110101
  }
  return Wire.endTransmission() == 0;
}

/* //读取电池端实时电压
滤波100次，输出测量电压值 */
float getBatteryFromADC()
{
  bat_mv = 0;
  uint32_t oversample = 0;
  for (size_t i = 0; i < 100; i++)
  {
    oversample += (uint32_t)analogRead(BATTERY_ADC_PIN );
  }
  bat_mv = (int)oversample / 100;
  bat_mv = ((float)bat_mv / 4096) * 3.35 * 2 ;

  Serial.println(String("Battery from ADC: ") + bat_mv + String("V"));
  Serial.println(F("------------POWER-------------"));
  return bat_mv;
}

/* 设定电源芯片保持升压的同时，读取电量的电量等级级电池电压 */
void PowerManagment() //保持升压芯片持续工作
{
  // Wire.begin(SDA, SCL);
  delay(100);
  bool isOk = setPowerBoostKeepOn(true); // Disables auto-standby, to keep 5V high during ESP32 sleep after 32Sec

  // Set console baud rate
  Serial.println(F("------------POWER-------------"));
  Serial.println(String("IP5306 setPowerBoostKeepOn: ") + (isOk ? "OK" : "FAIL"));
  Serial.println(String("IP5306 Battery level:") + getBatteryLevel() + String("%"));

  getBatteryFromADC();
}

//检测电池电量等级
int8_t getBatteryLevel()
{
  Wire.beginTransmission(IP5306_ADDR);
  Wire.write(0x78);
  if (Wire.endTransmission(false) == 0 && Wire.requestFrom(0x75, 1))
  {
    switch (Wire.read() & 0xF0)
    {
     
    case 0xE0 ... 0xff:    
      return 25;
    case 0xC0 ... 0xdf:
      return 50;
    case 0x80 ... 0xbf:
      return 75;
    case 0x00 ... 0x7f:
      return 100;
    default:
      return 0;
    }
  }
  return -1;
}
//更新屏幕电量图片
/**********************************************************
      输出结果：
      -1：未读取到电量(异常)
      -2：其他不正确结果(异常)
      0:   1/4
      25： 2/4
      75： 3/4
      100：4/4
***********************************************************/
extern int8_t fun_Refresh_lcon(int x) //刷新更改图标
{
  int8_t i = -2;
 
  switch (x)
  {
  case -1:
    return -1; //未读取到电量(异常)
  case 0 ... 24:
    p1 = F16x16_b0, i = 0;
    break;
  case 25 ... 74:
    p1 = F16x16_b20, i = 25;
    break;
  case 75 ... 99:
    p1 = F16x16_b60, i = 75;
    break;
  case 100:
    p1 = F16x16_b100, i = 100;
    break;
  default:
    i = -2; //其他不正确结果(异常)
    break;
  }
  
  return i;
}

//电量检测与电量低报警检测
/* 1.采集芯片电量，刷新屏显示，电量低于系统设定温度时，更新报警标志POWER_warning_flag */

void power_alarm_test(uint8_t x)
{
  //
  int i=0,j=0;
  uint32_t k=0;
  float temp = 0;

  SerialMon.print("power_test...");
  temp =
  k=0;
  for(j=0;j<x;j++)
  {
      i = getBatteryLevel();
      while(i==(-1)||i==0)
      {
         i = getBatteryLevel();
         delay(100);

      }
      Serial.printf("add i:%d\n",i);
      k += i;
      
  }
  i=k/j;
   Serial.printf("K:%d\n",k);
   Serial.printf("j:%d\n",j);
   Serial.printf("power_test[ok!]:%d\n",i);
  //刷新屏幕显示电量
  fun_Refresh_lcon(i);
   
  if (i <= 25)
  {
    temp = getBatteryFromADC();   //读取电压
    if (temp < Power_min_voltage) //电压低了
    {
      POWER_warning_flag = 1; //上传报警后进入关机状态。
      SerialMon.println("power_voltage_low--xxx");
      //.........
    }
    else
      POWER_warning_flag = 0;
  }
}




/* //电池电量ADC采集
float battery_ADC()
{
  uint32_t i = 0;
   float j = 0.0;
  uint8_t k = 10; //滤波次数

  for (; k > 0; k--)
    i += analogRead(BATTERY_ADC_PIN);
  i = i / k;
  j = (i * 3.3 / 4096 + 0.23) * 2;
  SerialMon.print(j);
  SerialMon.printf("V\r\n");
  return j;
} */

/*  float power_getBatteryLevel() 
{
  uint8_t i = 0XFF, j = 0;
  float k=0.0;

  do
  {
    i = fun_getBatteryLevel(); //获取电源管理芯片电池电量
    j++;
  } while (i == -1 || j > POWER_READ_NUM ); //读取失败连续读直到读取次数溢出。


  // i=0;j=51;
  if (j > POWER_READ_NUM ) //读取不到电量（异常）
  {
    j = 0;
    SerialMon.println("ERR:Failed to read power!!");
  }
  else if (i == 0) //确定电源芯片反馈电压低
  {
    k = battery_ADC(); //AD检测和确定
  }
  return k;
} */

/* void Power_test(float a) //确定电量最小值
{
  SerialMon.println("++++++++++++++++++++++++++++++++++++++++++++++++++++");
  SerialMon.println(a);
  if (a < Power_min_voltage) //电压低了
  {
    POWER_warning_flag = 1;
  }
  else
  {
    power_getBatteryLevel(); //重新读取电量显示
    POWER_warning_flag = 0;
  }
  SerialMon.printf("POWER_warning_flag=%d",POWER_warning_flag);
} */