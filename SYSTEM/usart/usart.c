#include "sys.h"
#include "usart.h"
#include "gpio.h"
#include "AMIS30543.h"
#include "math.h"
#include "stdio.h"
#include "Motor.h"
//////////////////////////////////////////////////////////////////////////////////
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "includes.h" //ucos ʹ��
#endif
//////////////////////////////////////////////////////////////////////////////////
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//����1��ʼ��
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/8/18
//�汾��V1.5
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved

//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB
#if 1
#pragma import(__use_no_semihosting)
//��׼����Ҫ��֧�ֺ���
struct __FILE
{
	int handle;
};

FILE __stdout;
//����_sys_exit()�Ա���ʹ�ð�����ģʽ
int _sys_exit(int x)
{
	x = x;
	return x;
}
//�ض���fputc����
int fputc(int ch, FILE *f)
{
	while ((USART1->SR & 0X40) == 0)
		; //ѭ������,ֱ���������
	USART1->DR = (u8)ch;
	return ch;
}
#endif

#if EN_USART1_RX				 //���ʹ���˽���
								 // �������ԭ���ʱ������¼��ֵ
short crc_check;				 //����crc�Ľ��
short crc_result1;				 //�洢��crcֵ
extern short gul_fuyang_jiaodu;	 //�����Ƕ�ֵ
extern short gul_fangwei_jiaodu; //��λ�Ƕ�ֵ
u8 Res;
u8 last_Res;
u8 RS422_receive_str[30]; //���յ�������
u8 RS232_receive_str[30]; //�ظ�
u8 RS422_byte_count;
u8 last_RS422_byte_count;
u8 RS232_receive_flag;
u8 Yuntai_ID_flag;						 //��̨ID��ѯ״̬��־
u8 Yuntai_zhuangtai_flag;				 //��̨״̬��ѯ��־
u8 Yuntai_kongzhi_flag;					 //��̨���Ʊ�־
u8 Yuntai_fuwei_flag;					 //��̨��λ��־
u8 Yuntai_jiaodushezhi_flag;			 //��̨�Ƕ����ñ�־
u8 Yuntai_chushishezhi_flag;			 //��̨��ʼ���ñ�־��Ŀǰû����
u8 Yuntai_yingdafangshi_flag;			 //��̨Ӧ�����ñ�־
u8 Yuntai_tingzhi_flag;					 //��ֹ̨ͣ���ñ�־
u8 Yuntai_fangwei_shezhi_chenggong_flag; //��̨���óɹ���־,��ʾ��λ������������Ч������ִ��
u8 Yuntai_fuyang_shezhi_chenggong_flag;
double fangwei_TargetPos;		 //��λĿ��λ��
double fuyang_TargetPos;		 //����Ŀ��λ��
short fangwei_Targetjiaodu;		 //��λĿ��Ƕ�
double fangwei_Targethudu;		 //��λĿ�껡��
short fuyang_Targetjiaodu;		 //����Ŀ��Ƕ�
double fuyang_Targethudu;		 //����Ŀ�껡��
extern unsigned char guc_StepM1; //ϸ����
extern float gf_MotorStep;		 //�����������
double fangwei_TargetPos_temp;	 //��λĿ��Ƕ��м�ֵ
double fuyang_TargettPos_temp;	 //����Ŀ��Ƕ��м�ֵ
extern u8 AT24c256_storage[30];	 //Ҫ�洢������
extern char In_place_flag1;
extern char In_place_flag;
extern u8 fangwei_yundong_daowei_flag;
extern u8 fuyang_yundong_daowei_flag;
extern int gul_Targettimeout_temp;
extern u32 guc_timejishi;


void uart_init(u32 bound)
{
	//GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE); //ʹ��USART1��GPIOAʱ��

	//USART1_TX   GPIOA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //�����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);			//��ʼ��GPIOA.9

	//USART1_RX	  GPIOA.10��ʼ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;			  //PA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);				  //��ʼ��GPIOA.10

	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; //��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);							  //����ָ���Ĳ�����ʼ��VIC�Ĵ���

	//USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;										//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;								//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//�շ�ģʽ

	USART_Init(USART1, &USART_InitStructure);	   //��ʼ������1
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); //�������ڽ����ж�
	USART_Cmd(USART1, ENABLE);					   //ʹ�ܴ���1
}

void USART1_IRQHandler(void) //����1�жϷ������
{

#if SYSTEM_SUPPORT_OS //���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();
#endif
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET)
		{
			Res = USART_ReceiveData(USART1); //��ȡ���յ�������
		}
		if ((Res == 0xc0) && ((last_Res == 0) || (last_Res == 0xC0)))
		{
			RS422_byte_count = 0;
		}
		else if (Res != 0xc0)
		{
			RS422_receive_str[RS422_byte_count] = Res;
			RS422_byte_count++;
		}
		else if ((Res == 0xc0) && (RS422_byte_count != 0) && (RS422_byte_count != 1))
		{
			last_RS422_byte_count = RS422_byte_count;
			RS422_byte_count = 0;
			RS232_receive_flag = 1;
			//				  RS232_data_Anal();
		}
		last_Res = Res;
	}
#if SYSTEM_SUPPORT_OS //���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();
#endif
}

void RS485_Send_Data(u8 *buf, u8 len)
{
	uint32_t	t;
	for(t=0;t<len;t++)
	{
		while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=1);//�ȴ����ͽ���
		USART_SendData(USART1, buf[t]);//�򴮿�1��������
		while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=1);//�ȴ����ͽ���
	}
	
}

u16 cal_crc(unsigned char *ptr, unsigned char len)
{
	unsigned char i;
	u16 crc = 0;

	while (len-- != 0)
	{
		for (i = 0x80; i != 0; i >>= 1)
		{
			if ((crc & 0x8000) != 0)
			{
				crc <<= 1;
				crc ^= (crc_mul);
			}
			else
			{
				crc <<= 1;
			}
			if ((*ptr & i) != 0)
			{
				crc ^= (crc_mul);
			}
		}
		ptr++;
	}
	return (crc);
}

void RS232_data_Anal()
{
	u8 flag = 0;
	u8 i;
	u8 t = 0;

	if (flag == 0)
	{
		for (i = 0; i < last_RS422_byte_count; i++)
		{
			if ((RS422_receive_str[i] == 0xdb) && (RS422_receive_str[i + 1] == 0xdc))
			{
				i++;
				RS232_receive_str[t] = 0xc0;
				t++;
			}
			else if ((RS422_receive_str[i] == 0xdb) && (RS422_receive_str[i + 1] == 0xdd))
			{
				i++;
				RS232_receive_str[t] = 0xdb;
				t++;
			}
			else
			{
				RS232_receive_str[t] = RS422_receive_str[i];
				t++;
			}
		}
		crc_check = cal_crc(RS232_receive_str, (t - 2)); //RS232_receive_str��������ȫû��ת��֮ǰ������
		crc_result1 = (short)((RS232_receive_str[t - 2] << 8) | RS232_receive_str[t - 1]);

		if (crc_check == crc_result1)
		{
			flag++;
		}
		else
		{
			flag = 0;
		}
	}
	if (flag == 1)
	{
		flag=0;
		if (RS422_receive_str[0] == 0x01)
		{
			Yuntai_ID_flag = 1;
		}
		if (RS422_receive_str[0] == 0x02)
		{
			Yuntai_zhuangtai_flag = 1;
		}
		if (RS422_receive_str[0] == 0x03)
		{
			Yuntai_kongzhi_flag = 1;
			if (RS422_receive_str[3] == 0x01)
			{
				Yuntai_fuwei_flag = 1;
			}
			if (RS422_receive_str[3] == 0x00)
			{
				if((0==fangwei_xch_dir) &&(0==fuyang_xch_dir) )
					Yuntai_tingzhi_flag = 1;
			}
		}
		if ((RS422_receive_str[0] == 0x04) && (In_place_flag1 == 1) && (In_place_flag == 1)) //&& (gb_SHUN_NI == 2) && (gb_SHUN_NI1 == 2))
		{
			Yuntai_jiaodushezhi_flag = 1; //��־����̨���յ���λ�������ýǶȱ�־
			fuyang_Targetjiaodu = (short)(RS232_receive_str[6] << 8 | RS232_receive_str[7]);
			Yuntai_yingdafangshi_flag = RS232_receive_str[8];
			fangwei_Targetjiaodu = (short)(RS232_receive_str[4] << 8 | RS232_receive_str[5]);
			if ((fangwei_Targetjiaodu >= -1500) && (fangwei_Targetjiaodu <= 1500))
			{
				Yuntai_fangwei_shezhi_chenggong_flag = 1;
			}
			else
			{
				Yuntai_fangwei_shezhi_chenggong_flag = 0;
			}
			if ((fuyang_Targetjiaodu >= -300) && (fuyang_Targetjiaodu <= 500))
			{
				Yuntai_fuyang_shezhi_chenggong_flag = 1;
			}
			else
			{
				Yuntai_fuyang_shezhi_chenggong_flag = 0;
			}
			if ((Yuntai_fangwei_shezhi_chenggong_flag == 1) && (Yuntai_fuyang_shezhi_chenggong_flag == 1))//�ж������Ƕ����ö��Ϸ�
			{
				for (i = 0; i < t; i++)
				{
					AT24c256_storage[i] = RS232_receive_str[i];
				}
				
				fangwei_Targethudu = (double)((1500 - fangwei_Targetjiaodu + 4389.97574) * 3.14159265 / 18000); //���㻡��
				fangwei_TargetPos_temp = cos(fangwei_Targethudu);
				fangwei_TargetPos_temp = (double)(sqrt(57.2713 * 57.2713 + 120.6785 * 120.6785 - fangwei_TargetPos_temp * 2 * 57.2713 * 120.6785) - 87.625);
				if ((fangwei_TargetPos_temp >= 0) && (fangwei_TargetPos_temp <= 38))//Ҫ����³���λ��
				{
					fangwei_TargetPos = fangwei_TargetPos_temp;//�Ƹ�ʵ����Ҫ������Գ���ֵ
				}
				fuyang_Targethudu = 0;
				fuyang_Targethudu = (double)((fuyang_Targetjiaodu + 2162.1881+339.81465) * 3.14159265 / 18000);

				fuyang_TargettPos_temp = cos(fuyang_Targethudu);
				fuyang_TargettPos_temp = (double)(sqrt(553.5480 * 553.5480 + 409.1304 * 409.1304 - fuyang_TargettPos_temp * 2 * 553.5480 * 409.1304) - 229.625);
				if ((fuyang_TargettPos_temp >= 0) && (fuyang_TargettPos_temp <= 58))//�ж�����������Χ��
				{
					fuyang_TargetPos = fuyang_TargettPos_temp;
				}
				if(fuyang_TargetPos - gl_currentPos >= 0.002)//�ж��������˶�
				{
						if(gb_SHUN_NI ==0 )//�жϵ�ǰ״̬�Ƿ�ת�Ǿ�ֹ�У�������Ҫ��ת��˵����Ҫ���ٻ���
						{
							if( gl_currentPos > 5.0)//�жϷ������ֹͣ�����㹻��������Ϊ����ʱ��̶���һ��û�о����ײ��
							{
								fuyang_xch_dir =2;//���û����־2
								Yuntai_fuyang_shezhi_chenggong_flag=2;//�޸����ýǶȱ�־����ʱת�����ֹͣ���Ȳ�ִ��ʵ�ʽǶ��˶�
								Yuntai_jiaodushezhi_flag =3;
							}
							else
							{
								Yuntai_fuyang_shezhi_chenggong_flag=0;								
							}
						}
				}
				else  if(fuyang_TargetPos - gl_currentPos < -0.002)//�жϷ����˶�
				{
						if(gb_SHUN_NI ==1 )//�жϵ�ǰ״̬����ת�Ǿ�ֹ�У�������Ҫ��ת��˵����Ҫ���ٻ���
						{
							if( gl_currentPos < 33)//�ж��������ֹͣ�����㹻��������Ϊ����ʱ��̶���һ��û�о����ײ��
							{
								fuyang_xch_dir =2;//���û����־2
							
								Yuntai_fuyang_shezhi_chenggong_flag=2;//�޸����ýǶȱ�־����ʱת�����ֹͣ���Ȳ�ִ��ʵ�ʽǶ��˶�
								Yuntai_jiaodushezhi_flag =3;
							}
							else
							{
								Yuntai_fuyang_shezhi_chenggong_flag=0;								
							}
						}
				}	
				
				if(fangwei_TargetPos - gl_currentPos1 >= 0.002)//�ж��������˶�
				{
						if(gb_SHUN_NI1 ==0 )//�жϵ�ǰ״̬�Ƿ�ת�Ǿ�ֹ�У�������Ҫ��ת��˵����Ҫ���ٻ���
						{
							if( gl_currentPos1 < 25.0 )//�жϷ������ֹͣ�����㹻��������Ϊ����ʱ��̶���һ��û�о����ײ��
							{
								Yuntai_fangwei_shezhi_chenggong_flag=2;//�޸����ýǶȱ�־����ʱת�����ֹͣ���Ȳ�ִ��ʵ�ʽǶ��˶�
								Yuntai_jiaodushezhi_flag =4;
								fangwei_xch_dir =2;//���û����־2
							}
							else
							{
								Yuntai_fangwei_shezhi_chenggong_flag = 0;
							}
							
						}
				}
				else  if(fangwei_TargetPos - gl_currentPos1 < -0.002)//�жϷ����˶�
				{
						if(gb_SHUN_NI1 ==1 )//�жϵ�ǰ״̬����ת�Ǿ�ֹ�У�������Ҫ��ת��˵����Ҫ���ٻ���
						{
							if( gl_currentPos1 >5.0)//�ж��������ֹͣ�����㹻��������Ϊ����ʱ��̶���һ��û�о����ײ��
							{
								fangwei_xch_dir =2;//���û����־2
								Yuntai_fangwei_shezhi_chenggong_flag=2;//�޸����ýǶȱ�־����ʱת�����ֹͣ���Ȳ�ִ��ʵ�ʽǶ��˶�
								Yuntai_jiaodushezhi_flag =4;
							}
							else
							{
								Yuntai_fangwei_shezhi_chenggong_flag=0;
							}
						}
				}	
				
			}
			else if ((Yuntai_fangwei_shezhi_chenggong_flag == 0) || (Yuntai_fuyang_shezhi_chenggong_flag == 0))
			{
				fuyang_Targetjiaodu = gul_fuyang_jiaodu;
				fangwei_Targetjiaodu = gul_fangwei_jiaodu;
			}
		}
		if (RS422_receive_str[0] == 0x05)
		{
			Yuntai_chushishezhi_flag = 1;
		}
		
	}
	
}

void RS422_Send_Data(u8 *buf, u8 len)
{
	u8 t;
	for (t = 0; t < len; t++)
	{
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
			;
		USART_SendData(USART1, buf[t]);
	}
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
		;
	FORCEON = 0;
}

#endif
