

#include "bh1750.h"
#include <stdbool.h>
#include <stdio.h>
#include "../../components/camera/twi.h"
#include "rom/ets_sys.h"
#include "driver/gpio.h"

#define 	_delay_ms(ms)  ets_delay_us(1000*ms)

#define _scl_high(gpioNum)	 	gpio_set_level(gpioNum,1)
#define _sda_high(gpioNum) 		gpio_set_level(gpioNum,1)


/*************************************************************
  Function   :Cmd_Write_BH1750
  Description:写BH1750 命令
  Input      : cmd ---命令
  return     : 0 success  not 0 failure
*************************************************************/
uint8_t Cmd_Write_BH1750(uint8_t cmd)
{
//	I2C_Start();                  //起始信号
//    I2C_WriteByte(BH1750_Addr+0); //发送设备地址+写信号

//	I2C_WaiteForAck();
//    I2C_WriteByte(cmd);           //内部寄存器地址
//	I2C_WaiteForAck();
//	I2C_Stop();                   //发送停止信号
//

    uint8_t ret=0;

//    __disable_irq();
    if(twi_writeTo(BH1750_Addr, &cmd, 1, true) != 0) {
        ret=0xFF;
    }
//    __enable_irq();
    if (ret != 0) {
       // printf("BH1750_Write [%02x] failed\n", cmd);
    }
    _delay_ms(5);
    return ret;

}
/*************************************************************
  Function   : Start_BH1750
  Description: 启动BH1750
  Input      : none
  return     : none
*************************************************************/
void Start_BH1750(void)
{
	Cmd_Write_BH1750(BH1750_CON);  //一次H分辨率模式，至少120ms
	_delay_ms(180);
}


/*************************************************************
  Function   :Init_BH1750
  Description:初始化BH1750
  Input      : none
  return     : none
*************************************************************/
uint8_t Init_BH1750(int pin_sda, int pin_scl)
{
	uint8_t ret=0;
	twi_init(pin_sda, pin_scl);//I2Cx_Init();
	//_scl_high(pin_scl);
	//_sda_high(pin_sda);

	if(Cmd_Write_BH1750(BH1750_ON))	   //power on
	{
		ret=0xff;
		return ret;
	}
	if(Cmd_Write_BH1750(BH1750_RSET))	 //clear
	{
		ret=0xff;
		return ret;
	}
	return ret;

}

/*************************************************************
  Function   : Read_BH1750
  Description: BH1750读光照值
  Input      :  none
  return     : ---读回的数据  uint16_t
*************************************************************/
uint16_t Read_BH1750(void)
{
	uint8_t data[2];
	uint16_t temp=0;

 //   I2C_Start();                         //起始信号
//    I2C_WriteByte(BH1750_Addr+1);        //发送设备地址+读信号
//	I2C_WaiteForAck();
//	temp[0]=I2C_ReadByte(I2C_ACK);       //ACK read data msb first
//	temp[1]=I2C_ReadByte(I2C_NACK);      //NACK
//	I2C_Stop();                          //停止信号
//	data = temp[0];
//	data = (data<<8)+temp[1];			//合成数据，即光照数据
//    delay_ms(5);

	if(Cmd_Write_BH1750(BH1750_ON))	   //power on
	{
		return NULL;//fail
	}
	_delay_ms(100);
	if(Cmd_Write_BH1750(BH1750_CON))  //一次H分辨率模式，至少120ms
	{
		return NULL;
	}
	_delay_ms(200);


//    __disable_irq();
    int rc = twi_readFrom(BH1750_Addr, &data, 2, true);

//    __enable_irq();
    if (rc == 0) {
        temp=(data[0]<<8)+data[1];
    }
    else
    {
    	printf("BH1750_Read [%02x] failed rc=%d\n", data[0], rc);
    	return NULL;
    }
    return (temp);


}
/*************************************************************
  Function   : Convert_BH1750
	Description: 转化读取的光照值，unit:lx 精度0.1 lx
  Input      : light_value ---  bh1750的register光照度值
	return     : 转化后的光照度 unit :lx
*************************************************************/
float Convert_BH1750(void)
{

	uint16_t light_value=Read_BH1750();
	float result_lx=0;

	result_lx=(float)light_value/1.2;

	return result_lx;
}
