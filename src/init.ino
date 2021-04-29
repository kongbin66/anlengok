/*----------------------------------------------------------
                 軟、硬件初始化操作相关
*---------------------------------------------------------*/
#include "config.h"
//硬件初始化
void hardware_init()
{
  
  pinMode     (13, OUTPUT); //KEY引脚
  digitalWrite(13,LOW); 
 // digitalWrite(13,HIGH); 
  pinMode     (MODEM_POWER_ON, OUTPUT); //电源引脚
  digitalWrite(MODEM_POWER_ON,LOW);
  pinMode     (33, OUTPUT); //LED引脚
  //digitalWrite(33,LOW); 
  digitalWrite(33,HIGH);

  //保持升压芯片持续工作
  PowerManagment();
  //电量检测及欠压报警检测
  power_alarm_test();
  adcAttachPin(BATTERY_ADC_PIN); //将引脚连接到ADC
  //adcStart(BATTERY_ADC_PIN);     //在连接的引脚总线上开始ADC转换

  bool i;
  //初始化DS1302引脚
  ds_rtc.init();
  i=ds_rtc.isHalted();//检查运行DS1302
  if(i) ds_rtc.halt(0);//启动1302

  Wire.begin();
  SerialMon.begin(115200); //初始化调试串口
  SerialMon.printf("/**************************************************************/\n");
  //唤醒方式
  Serial.print("esp_sleep_get_wakeup_cause");
  Serial.println(esp_sleep_get_wakeup_cause());
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX); //初始化AT串口
  sht20.begin();//溫濕度傳感器初始化
  EEPROM.begin(4096);//E2PROM初始化
  SPIFFS.begin();
  display.init();
  display.flipScreenVertically();
  key_init();
  
  
}
/*----------------------------------------------------------
                 软件初始化操作相关
*---------------------------------------------------------*/
void software_init()//軟件初始化
{
  tempAndHumi_Ready = false;
  Serial.printf("workingState:%d\r\n", workingState);
  Serial.printf("oledState:%d\r\n", oledState);
  loopStartTime = millis();
  screen_loopEnabled = true;
  //下面是固定参数，需要修改时再保存到EEPROM中
  show_tip_screen_last = 3;
  show_BLE_screen_last = 3;
  show_rec_stop_screen_last = 3;

  
  screen_On_Start = sys_sec;
  screen_On_now = sys_sec;
  
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED) //如果是系统复位唤醒的, 则停止工作, 亮屏
  {
    workingState = NOT_WORKING;
    oledState = OLED_ON;
    list_first_flag=1;
    lose_first_flag=1;
    firstBootFlag=1; //第一次启动标志位
  } 
  keyState = NOKEYDOWN;
  screenState = MAIN_TEMP_SCREEN;
  bleState = BLE_OFF;
  lockState = UNLOCKED;
  //qualifiedState = QUALITIFY_RIGHT;


  
}