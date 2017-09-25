# Bluno2 蓝牙组网编码传输库

完成星型网络下编码传输

![SVG1](https://raw.githubusercontent.com/DFRobot/binaryfiles/master/DFR0492/DFR0492svg1.png)

# DFROBOT_Bluno2 Library for Arduino
Provides an Arduino library for FireBeetle Board-328P with BLE4.1
## Table of Contents

* [Summary](#summary)
* [Methods](#methods)
* [Compatibility](#Compatibility)
* [History](#history)
* [Credits](#credits)
<snippet>
<content>

## Summary
The library is used to Send and receive GPRS or NB-IOT data (TCP,UDP)

## Methods

```C++

/*
 * @brief 开始传输
 *
 * @param s_通信用串口
 *
 * @return
 *     ture   Success
 *     false  Failed
 */
bool begin(Stream &s_);

/*
 * @brief 设置调试用串口，调用这个函数，调试串口会打印过程信息。不调用此函数，调试信息将被隐藏
 *
 * @param s_调试用串口
 *
 * @return
 *     ture   Success
 *     false  Failed
 */
bool setDbgSerial(Stream &s_){dbg = &s_; return true;}

/*
 * @brief 从串口读取制定长度数据
 *
 * @param 
 *     buf_ 期待读取数据存放首地址
 *     len  期待读取数据的长度
 *
 * @return 实际返回的数据长度
 */
size_t readN(void* buf_, size_t len);


/*
 * @brief 从事件队列中读取事件
 *
 * @return 事件编码
 */
uint8_t getEvent();

/*
 * @brief 向蓝牙网络中发送数据
 *
 * @param
 *     dst 目标蓝牙节点网络ID
 *     src   源蓝牙节点网络ID
 *     message 待发送消息首地址
 *     len     待发送消息长度
 * @return
 *     ture   Success
 *     false  Failed
 */
bool sendPacket(uint8_t dst, uint8_t src, const void *message, uint16_t len);

/*
 * @brief 蓝牙编码传输库的循环查询函数，用户接收并处理收到的蓝牙包
 * 
 * @note
 *     这个函数必须被循环调用
 */
void loop( void );

```

## Compatibility

MCU                | Work Well | Work Wrong | Untested  | Remarks
------------------ | :----------: | :----------: | :---------: | -----
FireBeetle-BLE4.1  |      √       |             |            | 


## History

- data 2017-9-25
- version V1.0

## Credits

- author [wuxiao  <zed.wu@dfrobot.com>]
