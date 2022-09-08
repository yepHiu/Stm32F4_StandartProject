#include "i2c.h"

void InitI2C(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;// 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB,&GPIO_InitStructure);
    IIC_SCL=1;
    IIC_SDA=1;
}

void IIC_Start(void)
{
    SDA_OUT(); // sda 线输出
    IIC_SDA = 1;
    IIC_SCL = 1;
    delay_us(4);
    IIC_SDA = 0; // START:when CLK is high,DATA change form high to low
    delay_us(4);
    IIC_SCL = 0; // 钳住 I2C 总线，准备发送或接收数据
}

void IIC_Stop(void)
{
    SDA_OUT(); // sda 线输出
    IIC_SCL = 0;
    IIC_SDA = 0; // STOP:when CLK is high DATA change form low to high
    delay_us(4);
    IIC_SCL = 1;
    IIC_SDA = 1; // 发送 I2C 总线结束信号
    delay_us(4);
}

uint8_t IIC_Wait_Ack(void)
{
    u8 ucErrTime = 0;
    SDA_IN(); // SDA 设置为输入
    IIC_SDA = 1;
    delay_us(1);
    IIC_SCL = 1;
    delay_us(1);
    while (READ_SDA)
    {
        ucErrTime++;
        if (ucErrTime > 250)
        {
            IIC_Stop();
            return 1;
        }
    }
    IIC_SCL = 0; // 时钟输出 0
    return 0;
}

void IIC_Ack(void)
{
    IIC_SCL = 0;
    SDA_OUT();
    IIC_SDA = 0;
    delay_us(2);
    IIC_SCL = 1;
    delay_us(2);
    IIC_SCL = 0;
}

void IIC_NAck(void)
{
    IIC_SCL = 0;
    SDA_OUT();
    IIC_SDA = 1;
    delay_us(2);
    IIC_SCL = 1;
    delay_us(2);
    IIC_SCL = 0;
}

void IIC_Send_Byte(uint8_t txd)
{
    u8 t;
    SDA_OUT();
    IIC_SCL = 0; // 拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {
        IIC_SDA = (txd & 0x80) >> 7;
        txd <<= 1;
        delay_us(2); // 对 TE A5767 这三个延时都是必须的
        IIC_SCL = 1;
        delay_us(2);
        IIC_SCL = 0;
        delay_us(2);
    }
}

uint8_t IIC_Read_Byte(uint8_t ack)
{
    unsigned char i, receive = 0;
    SDA_IN(); // SDA 设置为输入
    for (i = 0; i < 8; i++)
    {
        IIC_SCL = 0;
        delay_us(2);
        IIC_SCL = 1;
        receive <<= 1;
        if (READ_SDA)
            receive++;
        delay_us(1);
    }
    if (!ack)
        IIC_NAck(); // 发送 nACK
    else
        IIC_Ack(); // 发送 ACK
    return receive;
}   
