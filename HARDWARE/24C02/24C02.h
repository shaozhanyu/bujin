#ifndef __24C02_H
#define __24C02_H
#include "sys.h"  

////////////////////////////////////////////////////////////////////////////////// 

#define AT24C01		127
#define AT24C02		255
#define AT24C04		511
#define AT24C08		1023
#define AT24C16		2047
#define AT24C32		4095
#define AT24C64	    8191
#define AT24C128	16383
#define AT24C256	32767  
//Mini STM32������ʹ�õ���24c08�����Զ���EE_TYPEΪAT24C08
#define EE_TYPE AT24C256



//IIC_SDA��IO��������
#define SDA_IN()  {GPIOB->CRH&=0XFFFF0FFF;GPIOB->CRH|=(u32)8<<12;}
#define SDA_OUT() {GPIOB->CRH&=0XFFFF0FFF;GPIOB->CRH|=(u32)3<<12;}
//IO��������	 
#define IIC_SCL      PBout(10) //SCL
#define IIC_SDAOUT   PBout(11) //���SDA	 
#define IIC_SDAIN    PBin(11)  //����SDA 

//IIC��غ���
void IIC_Init(void);          //��ʼ��IIC��IO��				 
void IIC_Start(void);				  //����IIC��ʼ�ź�
void IIC_Stop(void);	  			//����IICֹͣ�ź�
u8 MCU_Wait_Ack(void); 				//IIC�ȴ�ACK�ź�
void MCU_Send_Ack(void);					  //IIC����ACK�ź�
void MCU_NOAck(void);				  //IIC������ACK�ź�
void IIC_Send_Byte(u8 txd);
u8 IIC_Read_OneByte(u8 ack);
	
//EEPROM24c02��غ���
//u8 AT24C02_ReadByte(u8 ReadAddr);							     //ָ����ַ��ȡһ���ֽ�
//void AT24C02_WriteByte(u8 WriteAddr,u8 DataToWrite);		//ָ����ַд��һ���ֽ�

u32 Buf_4Byte(u8 *pBuffer,u32 Date_4Byte,u8 Byte_num,u8 mode);

//void AT24C02_Write(u8 WriteAddr,u8 *pBuffer,u8 WriteNum);	//��ָ����ַ��ʼд��ָ�����ȵ�����
//void AT24C02_Read(u8 ReadAddr,u8 *pBuffer,u8 ReadNum);   	//��ָ����ַ��ʼ����ָ�����ȵ�����

//u8 AT24C02_Test(void);  //�������
//void AT24C02_Init(void); //��ʼ��IIC

u8 AT24CXX_ReadOneByte(u16 ReadAddr);							//ָ����ַ��ȡһ���ֽ�
void AT24CXX_WriteOneByte(u16 WriteAddr,u8 DataToWrite);		//ָ����ַд��һ���ֽ�
void AT24CXX_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len);//ָ����ַ��ʼд��ָ�����ȵ�����
u32 AT24CXX_ReadLenByte(u16 ReadAddr,u8 Len);					//ָ����ַ��ʼ��ȡָ����������
void AT24CXX_Write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite);	//��ָ����ַ��ʼд��ָ�����ȵ�����
void AT24CXX_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead);   	//��ָ����ַ��ʼ����ָ�����ȵ�����

u8 AT24CXX_Check(void);  //�������
void AT24CXX_Init(void); //��ʼ��IIC

void AT24CXXX_IPNET(void);

#endif

