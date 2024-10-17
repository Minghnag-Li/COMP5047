#include <Arduino.h>

// 定义引脚
#define S0 13
#define S1 14
#define S2 0
#define S3 26
#define OUT_PIN 25

// 存储颜色频率的变量
volatile unsigned long redPulseCount = 0;
volatile unsigned long greenPulseCount = 0;
volatile unsigned long bluePulseCount = 0;

// 定义硬件定时器
hw_timer_t *timer = NULL;

void IRAM_ATTR onTimer() {
  // 计时周期结束，开始处理频率计算
  static int currentColor = 0; // 0: Red, 1: Green, 2: Blue

  if (currentColor == 0) {
    // 设置红色滤波器
    digitalWrite(S2, LOW);
    digitalWrite(S3, LOW);
    redPulseCount = pulseIn(OUT_PIN, LOW, 1000000);  // 读取脉冲
    currentColor = 1;  // 切换到绿色
  } else if (currentColor == 1) {
    // 设置绿色滤波器
    digitalWrite(S2, HIGH);
    digitalWrite(S3, HIGH);
    greenPulseCount = pulseIn(OUT_PIN, LOW, 1000000);  // 读取脉冲
    currentColor = 2;  // 切换到蓝色
  } else if (currentColor == 2) {
    // 设置蓝色滤波器
    digitalWrite(S2, LOW);
    digitalWrite(S3, HIGH);
    bluePulseCount = pulseIn(OUT_PIN, LOW, 1000000);  // 读取脉冲
    currentColor = 0;  // 切换回红色
  }
}

void setup() {
  // 设置串口通信
  Serial.begin(115200);

  // 定义引脚模式
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(OUT_PIN, INPUT);

  // 设置频率输出为20%
  digitalWrite(S0, HIGH);
  digitalWrite(S1, HIGH);

  // 初始化硬件定时器 (Timer 0)
  timer = timerBegin(0, 80, true);  // Timer 0, 分频80
  timerAttachInterrupt(timer, &onTimer, true);  // 绑定中断服务函数
  timerAlarmWrite(timer, 1000000, true);  // 1秒触发一次
  timerAlarmEnable(timer);  // 启用定时器警报
}

void loop() {
  // 主循环中通过串口打印读取到的颜色脉冲
  Serial.print("Red: ");
  if (redPulseCount > 0) {
    Serial.print(1000000 / redPulseCount);  // 将频率转换为相对值
  } else {
    Serial.print("N/A");
  }

  Serial.print("  Green: ");
  if (greenPulseCount > 0) {
    Serial.print(1000000 / greenPulseCount);
  } else {
    Serial.print("N/A");
  }

  Serial.print("  Blue: ");
  if (bluePulseCount > 0) {
    Serial.print(1000000 / bluePulseCount);
  } else {
    Serial.print("N/A");
  }

  Serial.println();

  // 延时用于串口显示
  delay(1000);  // 1秒打印一次
}
