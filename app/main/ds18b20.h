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
#ifndef DS18B20_H_  
#define DS18B20_H_


//定义总线上的DS18B20的最大个数MAXNUM
#define	MAXNUM		1

//DS18B20指令
typedef enum
{
	SEARCH_ROM			=	0xf0,	//搜索ROM指令
	READ_ROM			=	0x33,	//读取ROM指令
	MATH_ROM			=	0x55,	//匹配ROM指令
	SKIP_ROM			=	0xcc,	//忽略ROM指令
	ALARM_SEARCH		=	0xec,	//报警搜索指令
	CONVERT_T			=	0x44,	//温度转换指令
	WRITE_SCRATCHPAD	=	0x4e,	//写暂存器指令
	READ_SCRATCHPAD		=	0xbe,	//读取暂存器指令
	COPY_SCRATCHPAD		=	0x48,	//拷贝暂存器指令
	RECALL_E2			=	0xb8,	//召回EEPROM指令
	READ_POWER_SUPPLY	=	0xb4,	//读取电源模式指令
} DS18B20_CMD;

//DS18B20 ROM编码
typedef struct
{
	unsigned char  DS18B20_CODE;   //DS18B20单总线编码:0x19
	unsigned char  SN_1;           //序列号第1字节
	unsigned char  SN_2;           //序列号第2字节
	unsigned char  SN_3;           //序列号第3字节
	unsigned char  SN_4;           //序列号第4字节
	unsigned char  SN_5;           //序列号第5字节
	unsigned char  SN_6;           //序列号第6字节
	unsigned char  crc8;           //CRC8校验
} DS18B20_ROM_CODE;

unsigned char ds18b20_searchROM(unsigned char (*pID)[8],unsigned char Num);
float ds18b20_aim_get_temp(unsigned char pID[8]);
unsigned char ds18b20_getROM(unsigned char *ROM);

void ds18b20_send(char bit);
unsigned char ds18b20_read(void);
void ds18b20_send_byte(char data);
unsigned char ds18b20_read_byte(void);
unsigned char ds18b20_RST_PULSE(void);
float ds18b20_get_temp(void);
unsigned char ds18b20_init(int GPIO);

#endif
