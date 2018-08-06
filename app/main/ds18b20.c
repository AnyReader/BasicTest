/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
     along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h"
#include "ds18b20.h"

int DS_GPIO;
//int init=0;
/// Sends one bit to bus
void ds18b20_send(char bit){
  gpio_set_direction(DS_GPIO, GPIO_MODE_OUTPUT);
  gpio_set_level(DS_GPIO,0);
  ets_delay_us(5);
  if(bit==1)gpio_set_level(DS_GPIO,1);
  ets_delay_us(80);
  gpio_set_level(DS_GPIO,1);
}

// Reads one bit from bus
unsigned char ds18b20_read(void){
  unsigned char PRESENCE=0;
  gpio_set_direction(DS_GPIO, GPIO_MODE_OUTPUT);
  gpio_set_level(DS_GPIO,0);
  ets_delay_us(2);
  gpio_set_level(DS_GPIO,1);
  ets_delay_us(15);
  gpio_set_direction(DS_GPIO, GPIO_MODE_INPUT);
  if(gpio_get_level(DS_GPIO)==1) PRESENCE=1; else PRESENCE=0;
  return(PRESENCE);
}

/*************************************************************************************************************************
*������:unsigned char DS18B20_Read2Bit(void)
*����  :��ȡ2bit����
*����  :��
*����  :data
*����  :chinglyn@163.com
*************************************************************************************************************************/

unsigned char ds18b20_read2Bit(void)
{
	unsigned char i,data = 0;
    for(i = 0;i < 2;i ++)
    {
        data <<= 1;
        if(ds18b20_read())
		{
			data = data|1;
		}
    }
    return data;
}




// Sends one byte to bus
void ds18b20_send_byte(char data){
  unsigned char i;
  unsigned char x;
  for(i=0;i<8;i++){
    x = data>>i;
    x &= 0x01;
    ds18b20_send(x);
  }
  ets_delay_us(100);
}
// Reads one byte from bus
unsigned char ds18b20_read_byte(void){
  unsigned char i;
  unsigned char data = 0;
  for (i=0;i<8;i++)
  {
    if(ds18b20_read()) data|=0x01<<i;
    ets_delay_us(15);
  }
  return(data);
}




// Sends reset pulse
//return 1 success
//return 0 error
unsigned char ds18b20_RST_PULSE(void)
{
	unsigned int i=0;
  unsigned char PRESENCE;
  gpio_set_direction(DS_GPIO, GPIO_MODE_OUTPUT);
  gpio_set_level(DS_GPIO,0);//pull down DQ
  ets_delay_us(500);
  gpio_set_level(DS_GPIO,1);
  gpio_set_direction(DS_GPIO, GPIO_MODE_INPUT);

#if 1
  while(gpio_get_level(DS_GPIO))
  {
	ets_delay_us(10);
	i++;
	if(i>50)
	{
		return 0;
	}
  }
  ets_delay_us(500);
  return 1;
#endif
#if 0
  ets_delay_us(30);
  if(gpio_get_level(DS_GPIO)==0) PRESENCE=1; else PRESENCE=0;//check ds18b20
  ets_delay_us(470);
  if(gpio_get_level(DS_GPIO)==1) PRESENCE=1; else PRESENCE=0;//check ds18b20
  return PRESENCE;
#endif

}




/*************************************************************************************************************************
*������:u8 DS18B20_SearchROM(u8 (*pID)[8],u8 Num)
*����  :��ѯDS18B20��ROM
*����  :(1)�� pIN:������DS18B20��ID�洢�Ļ�����ָ��
				(2)�� Num:DS18B20�ĸ�������MAXNUM���ж���
*����  :������������DS18B20�ĸ���
*����  :chinglyn@163.com
*************************************************************************************************************************/

unsigned char ds18b20_searchROM(unsigned char (*pID)[8],unsigned char Num)
{
	unsigned char k,l=0,ConflictBit,m,n,i;
	unsigned char BUFFER[MAXNUM]={0};  //****��ʼ��ջ��ֵΪ0�������Ϳ��Ա�֤ѭ�����ж�****������ѭ����� BUFFER[0]=0;
	unsigned char ss[64];
	unsigned char s=0;
	unsigned char num = 0;
	do
	{
	    unsigned char check;
	    check=ds18b20_RST_PULSE();	//��λDS18B20����
	    if(check==1)
	    {
		    ds18b20_send_byte(SEARCH_ROM);	//����ROM
		    for(m=0;m<8;m++)
		    {
		//      unsigned char s=0;
		        for(n=0;n<8;n++)
		        {
		            k=ds18b20_read2Bit();		// ����λ����
		            k=k&0x03;
		            s= s>>1;
		            if(k==0x01)							//0000 0001 �������������Ϊ0
		            {
		            	ds18b20_send(0);//д0��ʹ������Ϊ0��������Ӧ
		                ss[(m*8+n)]=0;
		            }
		            else if(k==0x02)				//0000 0010 �������������Ϊ1
		            {
		                s=s|0x80;
		                ds18b20_send(1);//д1��ʹ������Ϊ1��������Ӧ
		                ss[(m*8+n)]=1;
		            }
		            else if(k==0x00)//�����ȡ��������Ϊ00�����г�ͻ������г�ͻλ�ж�
		            {
		               ConflictBit=m*8+n+1;
		               if(ConflictBit>BUFFER[l])//�����ͻλ����ջ������д0
		               {
		                   ds18b20_send(0);
		                   ss[(m*8+n)]=0;
		                   BUFFER[++l]=ConflictBit;
		                }
		                else if(ConflictBit<BUFFER[l])//�����ͻλС��ջ������д��ǰ������
		                {
		                      s=s|((ss[(m*8+n)]&0x01)<<7);
		                      ds18b20_send(ss[(m*8+n)]);
		                }
		                else if(ConflictBit==BUFFER[l])//�����ͻλ����ջ������д1
		                {
		                    s=s|0x80;
		                    ds18b20_send(1);
		                    ss[(m*8+n)]=1;
		                    l=l-1;
		                }
		            }
		            else//�������������Ϊ0x03(0000 0011),��˵���������ϲ������κ��豸
		            {
		                return num; //������ɣ������������ĸ���
		            }
		            ets_delay_us(10);
		          }
		          pID[num][m]=s;
		          s=0;
		      }
		      num=num+1;
	    }
	}while(BUFFER[l]!=0&&(num<MAXNUM));
	return num;     //�����������ĸ���
}


//
unsigned char ds18b20_getROM(unsigned char *ROM)
{
    unsigned char check;

    check=ds18b20_RST_PULSE();
	if(check==1)
	{
		ds18b20_send_byte(0x33);
		for(int i=0;i<8;i++)
		{
			*ROM=ds18b20_read_byte();
			ROM++;
		}
		return 1;
	}
	else return 0;
	//return 1;
}


/**
//�ȴ�DS18B20�Ļ�Ӧ
//����1:δ��⵽DS18B20�Ĵ��� return 1: not found ds18B20
//����0:����     return 0: found ds18B20
unsigned char ds18b20_CHECK(void)//add by Charlin
{
	unsigned char retry=0;
	gpio_set_direction(DS_GPIO, GPIO_MODE_INPUT);//SET DS PORT INPUT
    while (gpio_get_level(DS_GPIO) && retry<200)
    {
    	retry++;
    	ets_delay_us(1);
    };

    if(retry>=200)return 1;
	else retry=0;

    while (!gpio_get_level(DS_GPIO)&&retry<240)
    {
    	retry++;
    	ets_delay_us(1);
    };
    if(retry>=240)return 1;
    return 0;
}
**/

// Returns temperature from sensor
//��ȡָ��ID��DS18B20�¶�
//���ȣ�
//����ֵ���¶�ֵ ��-55~125��
float ds18b20_aim_get_temp(unsigned char pID[8])
{
	unsigned char check;
	char tempL=0, tempH=0;
    check=ds18b20_RST_PULSE();
	if(check==1)
	{
		ds18b20_send_byte(0xCC);//ignore ROM
        ds18b20_send_byte(0x44);//
        ets_delay_us(750);//vTaskDelay(750 / portTICK_RATE_MS);
        check=ds18b20_RST_PULSE();
    	if(check==1)
    	{
    		ds18b20_send_byte(0x55);////�������к�ƥ������
    		for(int i= 0;i < 8;i ++)	//����8byte�����к�
    		{
    			ds18b20_send_byte(pID[i]);
    		}
    		ets_delay_us(10);
    		ds18b20_send_byte(0xBE);//
    		tempL=ds18b20_read_byte();//LSB
    		tempH=ds18b20_read_byte();//MSB
    		check=ds18b20_RST_PULSE();
    		if(tempH==0xFF&&tempL==0xFF) return 65535;
            float temp=0;
            temp=(float)((tempH<<8)+tempL)/16; //0.625=1/16  //65535*0.0625=
            return temp;
    	}
    	else return 65535;
	}
	else return 65535;
}


// Returns temperature from sensor
//��ds18b20�õ��¶�ֵ
//���ȣ�
//����ֵ���¶�ֵ ��-55~125��
float ds18b20_get_temp(void)
{
    unsigned char check;
    char tempL=0, tempH=0;

    check=ds18b20_RST_PULSE();
	if(check==1)
	{
		ds18b20_send_byte(0xCC);//ignore ROM
        ds18b20_send_byte(0x44);//
        ets_delay_us(750);//vTaskDelay(750 / portTICK_RATE_MS);
        check=ds18b20_RST_PULSE();
    	if(check==1)
    	{
    		ds18b20_send_byte(0xCC);//����ROM��������
    		ds18b20_send_byte(0xBE);//
    		tempL=ds18b20_read_byte();//LSB
    		tempH=ds18b20_read_byte();//MSB
    		check=ds18b20_RST_PULSE();
    		if(tempH==0xFF&&tempL==0xFF) return 65535;
            float temp=0;
            temp=(float)((tempH<<8)+tempL)/16; //0.625=1/16  //65535*0.0625=
            return temp;
    	}
    	else{return 65535;}
     }
     else{return 65535;}
}

//1:success
//0:fail
unsigned char ds18b20_init(int GPIO)
{
  DS_GPIO = GPIO;
  gpio_pad_select_gpio(DS_GPIO);
  return (ds18b20_RST_PULSE());
}
