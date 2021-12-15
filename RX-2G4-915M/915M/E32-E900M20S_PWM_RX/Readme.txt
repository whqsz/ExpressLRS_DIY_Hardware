基于ExpressLRS 2.0官方源码库引脚定义整理电路图，射频模块选用亿佰特定制型号E32-900M20S
此型号模块内部同样采用SX1276方案，跟常用方案唯一区别就是把天线控制信号调整为由外部单片机端来控制，
所以要在原固件引脚定义下增加两行TXEN跟RXEN定义来驱动射频天线发收状态。
此版本硬件未加入电池电压检测回传，后续要等固件功能加入再做硬件定义。
排针PWM引脚复用功能，同样可以兼容DIY_900_RX CRSF协议固件

PWM1	-- BOOT0下载对GND短接
PWM2	-- TX串口下载 CRSF通讯
PWM3	-- RX串口下载 CRSF通讯
PWM4	--
PWM5	--
PWM6	--

-------------------------------------------------------------------------------------------------------------------------
ExpressLRS 2.0固件添加收发控制定义：
工程文件路径为（ExpressLRS-2.0.0-RC1\src\include\target\DIY_900_RX_PWMP.h）
文本编辑DIY_900_RX_PWMP.h文件

PWM输出去掉IO10口：
#if defined(DEBUG_LOG)
#define GPIO_PIN_PWM_OUTPUTS    {0, 5, 9}
#else
#define GPIO_PIN_PWM_OUTPUTS    {0, 1, 3, 5, 9}	//去掉后面的“,10”ESP8285端口紧张
#endif

文本结尾追加一行定义：
#define GPIO_PIN_RX_ENABLE      10
