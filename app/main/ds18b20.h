#ifndef DS18B20_H_  
#define DS18B20_H_


//���������ϵ�DS18B20��������MAXNUM
#define	MAXNUM		1

//DS18B20ָ��
typedef enum
{
	SEARCH_ROM			=	0xf0,	//����ROMָ��
	READ_ROM			=	0x33,	//��ȡROMָ��
	MATH_ROM			=	0x55,	//ƥ��ROMָ��
	SKIP_ROM			=	0xcc,	//����ROMָ��
	ALARM_SEARCH		=	0xec,	//��������ָ��
	CONVERT_T			=	0x44,	//�¶�ת��ָ��
	WRITE_SCRATCHPAD	=	0x4e,	//д�ݴ���ָ��
	READ_SCRATCHPAD		=	0xbe,	//��ȡ�ݴ���ָ��
	COPY_SCRATCHPAD		=	0x48,	//�����ݴ���ָ��
	RECALL_E2			=	0xb8,	//�ٻ�EEPROMָ��
	READ_POWER_SUPPLY	=	0xb4,	//��ȡ��Դģʽָ��
} DS18B20_CMD;

//DS18B20 ROM����
typedef struct
{
	unsigned char  DS18B20_CODE;   //DS18B20�����߱���:0x19
	unsigned char  SN_1;           //���кŵ�1�ֽ�
	unsigned char  SN_2;           //���кŵ�2�ֽ�
	unsigned char  SN_3;           //���кŵ�3�ֽ�
	unsigned char  SN_4;           //���кŵ�4�ֽ�
	unsigned char  SN_5;           //���кŵ�5�ֽ�
	unsigned char  SN_6;           //���кŵ�6�ֽ�
	unsigned char  crc8;           //CRC8У��
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
