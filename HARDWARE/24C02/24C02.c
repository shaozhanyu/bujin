#include "24c02.h"
#include "delay.h"


void IIC_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  //ʹ��GPIOBʱ�� ģ��IICͨѶ���������Ŷ���Ϊ�����ʽ

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;          //��ͨ���ģʽ
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;     //100MHz
  GPIO_Init(GPIOB,&GPIO_InitStructure);                 //��ʼ��IO
	IIC_SCL=1;        //����B8����Ϊ�ߵ�ƽ
	IIC_SDAOUT=1;   
}


void IIC_Start(void)          //��ʼ�źţ�SCLΪ�ߵ�ƽʱ��SDA�ɸߵ�ƽ��͵�ƽ���䣬��ʼ��������
{
	SDA_OUT();     //����SDA��Ϊ���    PB9��Ϊ���ݴ������ţ���������������ģʽ
	IIC_SDAOUT=1;	  	  
	IIC_SCL=1;
	delay_us(10);
 	IIC_SDAOUT=0;
	delay_us(10);
	IIC_SCL=0;     //׼�����ͻ�������� 
}	  

void IIC_Stop(void)    //�����źţ�SCLΪ�ߵ�ƽʱ��SDA�ɵ͵�ƽ��ߵ�ƽ���䣬�����������ݡ�
{
	SDA_OUT();    //����SDA��Ϊ���
	IIC_SCL=0;
	IIC_SDAOUT=0; 
 	delay_us(10);
	IIC_SCL=1;
  delay_us(10);				
	IIC_SDAOUT=1; //����I2C���߽����ź�				   	
}

u8 MCU_Wait_Ack(void)       //MCU�ȴ����豸Ӧ���źŵ���    �������MCU��Ϊ���Ͷˣ�IIC��Ϊ���ն�
	                         //�ȴ�IIC����һ���͵�ƽ���ź�
{
	u8 WaitTime=0;
	SDA_IN();      //����SDA��Ϊ����  
	IIC_SDAOUT=1;
	delay_us(10);	   
	IIC_SCL=1;
	delay_us(10);	 
	while(IIC_SDAIN)    //�ж�����SDAΪ1
	{
		WaitTime++;
		if(WaitTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL=0; 
	return 0;  
}

void MCU_Send_Ack(void)     //MCU����ACKӦ�𣬸�֪24Cxx��Ӧ���źţ�
{
	IIC_SCL=0;   //�źŵ�ƽ�õͣ������ƽ�õ�ʱ�����ܸ��������ߵĵ�ƽ״̬
	SDA_OUT();    //�����߹涨STM32������Ϊ��������SDA�����״̬
	IIC_SDAOUT=0;  //���SDA���ϵ�״̬Ϊ0  
	delay_us(10);
	IIC_SCL=1;     //ʱ���źŵ�ƽ�ø�
	delay_us(10);
	IIC_SCL=0;       
}

void MCU_NOAck(void)    //MCU������ACKӦ��
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDAOUT=1;
	delay_us(10);
	IIC_SCL=1;
	delay_us(10);
	IIC_SCL=0;
}	 

void IIC_Send_Byte(u8 txd)
{                        
	u8 t;   
	SDA_OUT(); 	    
	IIC_SCL=0;//����ʱ�ӿ�ʼ���ݴ���
	for(t=0;t<8;t++)
	{              
			//IIC_SDA=(txd&0x80)>>7;
	if((txd&0x80)>>7)
		IIC_SDAOUT=1;
	else
		IIC_SDAOUT=0;
	txd<<=1; 	  
	delay_us(10);   //��TEA5767��������ʱ���Ǳ����
	IIC_SCL=1;
	delay_us(10); 
	IIC_SCL=0;	
	delay_us(10);
	}	 
} 	


u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA����Ϊ����
  for(i=0;i<8;i++ )
	{
    IIC_SCL=0; 
    delay_us(10);
		IIC_SCL=1;
    receive<<=1;
    if(IIC_SDAIN)receive++;   
		delay_us(10); 
  }					 
	if (!ack)
			MCU_NOAck();//����nACK
	else
			MCU_Send_Ack(); //����ACK   
	return receive;
}

/*******************************IO��ģ��IIC*************************************
*******************************************************************************/


/*******************************************************************************
*************************����ΪEEPROM24C02��д����******************************
*******************************************************************************/
void AT24CXX_Init(void)
{
	IIC_Init();
}



//��ʼ��24c02��IIC�ӿ�
u8 AT24CXX_ReadOneByte(u16 ReadAddr)
{				  
	u8 temp=0;		  	    																 
    IIC_Start();  
	if(EE_TYPE>AT24C16)
	{
		IIC_Send_Byte(0XA0);	   //����д����
		MCU_Wait_Ack();
		IIC_Send_Byte(ReadAddr>>8);//���͸ߵ�ַ
		//IIC_Wait_Ack();		 
	}else IIC_Send_Byte(0XA0+((ReadAddr/256)<<1));   //����������ַ0XA0,д���� 	 

	MCU_Wait_Ack(); 
  IIC_Send_Byte(ReadAddr%256);   //���͵͵�ַ
	MCU_Wait_Ack();	    
	IIC_Start();  	 	   
	IIC_Send_Byte(0XA1);           //�������ģʽ			   
	MCU_Wait_Ack();	 
    temp=IIC_Read_Byte(0);		   
    IIC_Stop();//����һ��ֹͣ����	    
	return temp;
}
//��AT24CXXָ����ַд��һ������
//WriteAddr  :д�����ݵ�Ŀ�ĵ�ַ    
//DataToWrite:Ҫд�������
void AT24CXX_WriteOneByte(u16 WriteAddr,u8 DataToWrite)
{				   	  	    																 
    IIC_Start();  
	if(EE_TYPE>AT24C16)
	{
		IIC_Send_Byte(0XA0);	    //����д����
		MCU_Wait_Ack();
		IIC_Send_Byte(WriteAddr>>8);//���͸ߵ�ַ
 	}else
	{
		IIC_Send_Byte(0XA0+((WriteAddr/256)<<1));   //����������ַ0XA0,д���� 
	}	 
	MCU_Wait_Ack();	   
    IIC_Send_Byte(WriteAddr%256);   //���͵͵�ַ
	MCU_Wait_Ack(); 	 										  		   
	IIC_Send_Byte(DataToWrite);     //�����ֽ�							   
	MCU_Wait_Ack();  		    	   
    IIC_Stop();//����һ��ֹͣ���� 
	delay_ms(7);	 
}
//��AT24CXX�����ָ����ַ��ʼд�볤��ΪLen������
//�ú�������д��16bit����32bit������.
//WriteAddr  :��ʼд��ĵ�ַ  
//DataToWrite:���������׵�ַ
//Len        :Ҫд�����ݵĳ���2,4
void AT24CXX_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len)
{  	
	u8 t;
	for(t=0;t<Len;t++)
	{
		AT24CXX_WriteOneByte(WriteAddr+t,(DataToWrite>>(8*t))&0xff);
	}												    
}

//��AT24CXX�����ָ����ַ��ʼ��������ΪLen������
//�ú������ڶ���16bit����32bit������.
//ReadAddr   :��ʼ�����ĵ�ַ 
//����ֵ     :����
//Len        :Ҫ�������ݵĳ���2,4
u32 AT24CXX_ReadLenByte(u16 ReadAddr,u8 Len)
{  	
	u8 t;
	u32 temp=0;
	for(t=0;t<Len;t++)
	{
		temp<<=8;
		temp+=AT24CXX_ReadOneByte(ReadAddr+Len-t-1); 	 				   
	}
	return temp;												    
}
//���AT24CXX�Ƿ�����
//��������24XX�����һ����ַ(255)���洢��־��.
//���������24Cϵ��,�����ַҪ�޸�
//����1:���ʧ��
//����0:���ɹ�
u8 AT24CXX_Check(void)
{
	u8 temp;
	temp=AT24CXX_ReadOneByte(255);//����ÿ�ο�����дAT24CXX			   
	if(temp==0X55)return 0;		   
	else//�ų���һ�γ�ʼ�������
	{
		AT24CXX_WriteOneByte(255,0X55);
	    temp=AT24CXX_ReadOneByte(255);	  
		if(temp==0X55)return 0;
	}
	return 1;											  
}

//��AT24CXX�����ָ����ַ��ʼ����ָ������������
//ReadAddr :��ʼ�����ĵ�ַ ��24c02Ϊ0~255
//pBuffer  :���������׵�ַ
//NumToRead:Ҫ�������ݵĸ���
void AT24CXX_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead)
{
	while(NumToRead)
	{
		*pBuffer++=AT24CXX_ReadOneByte(ReadAddr++);	
		NumToRead--;
	}
}  
//��AT24CXX�����ָ����ַ��ʼд��ָ������������
//WriteAddr :��ʼд��ĵ�ַ ��24c02Ϊ0~255
//pBuffer   :���������׵�ַ
//NumToWrite:Ҫд�����ݵĸ���
void AT24CXX_Write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite)
{
	while(NumToWrite--)
	{
		AT24CXX_WriteOneByte(WriteAddr,*pBuffer);
		WriteAddr++;
		pBuffer++;
	}
}

/****************************************************************************
* ��    ��: u32 Buf_4Byte(u8 *pBuffer,u32 Date_4Byte,u8 Byte_num,u8 mode)
* ��    �ܣ���λ�����ֽڻ�ת
* ��ڲ�����mode��1:��λ��ת�ֳ��ֽ�   0:�ֽںϲ���һ����λ��
            Byte_num����Ҫת�����ֽ���
            *pBuffer���ֽڽ���������ֽ���������
            Date_4Byte����λ����
* ���ز�����modeΪ0ʱ�����ض�λ��
* ˵    ����Byte_num���Ϊ4���ֽڣ��ú����ں���Ĵ�����У��ʱ��ȡУ��ֵ���õ�
****************************************************************************/
u32 Buf_4Byte(u8 *pBuffer,u32 Date_4Byte,u8 Byte_num,u8 mode)
{
   u8 i; u32 middata=0;
	if(mode)    //��λ��ת�ֳ��ֽ�
	 {
	   for(i=0;i<Byte_num;i++)
	     {
	       *pBuffer++ =(Date_4Byte>>(8*i))&0xff;
	     }
			return 0; 
	 } 
	 else       //�ֽںϲ���һ����λ��
	 {
	    Date_4Byte=0;
		  pBuffer+=(Byte_num-1);
		  for(i=0;i<Byte_num;i++)
	      { 		
		      middata<<=8;
		      middata+= *pBuffer--;			   
	      }
			return middata;	
	 }
}

//void AT24CXXX_IPNET( )
//{
//		AT24CXX_Read(0,(u8*)IPbuffer,5);
//		if(IPbuffer[4]==crc_high_first((u8 *)IPbuffer,4))
//			{
//			sprintf((char *)strIP,"IP=%d.%d.%d.%d\r\n",IPbuffer[0],IPbuffer[1],IPbuffer[2],IPbuffer[3]);
//			uart1SendChars((u8*)strIP,25);
//			IP1=IPbuffer[0];
//			IP2=IPbuffer[1];
//			IP3=IPbuffer[2];
//			IP4=IPbuffer[3];
//			}		
//		else
//			{
//			AT24CXX_Read(40,(u8*)IPbuffer,5);
//			if(IPbuffer[4]==crc_high_first((u8 *)IPbuffer,4))
//				{
//			  sprintf((char *)strIP,"IP=%d.%d.%d.%d\r\n",IPbuffer[0],IPbuffer[1],IPbuffer[2],IPbuffer[3]);
//			  uart1SendChars((u8*)strIP,25);	
//			  IP1=IPbuffer[0];
//			  IP2=IPbuffer[1];
//			  IP3=IPbuffer[2];
//			  IP4=IPbuffer[3];
//				}
//			else
//				{
//			  IP1=192;
//			  IP2=168;
//			  IP3=1;
//			  IP4=240;
//			  }		
//			} 
//		AT24CXX_Read(8,(u8*)Netbuffer,5);
//		if(Netbuffer[4]==crc_high_first((u8 *)Netbuffer,4))
//			{
//			sprintf((char *)strNET,"NET=%d.%d.%d.%d\r\n",Netbuffer[0],Netbuffer[1],Netbuffer[2],Netbuffer[3]);
//			uart1SendChars((u8*)strNET,25);
//			NET1=Netbuffer[0];
//			NET2=Netbuffer[1];
//			NET3=Netbuffer[2];
//			NET4=Netbuffer[3];
//			}		
//		else
//			{
//			AT24CXX_Read(48,(u8*)Netbuffer,5);
//		  if(Netbuffer[4]==crc_high_first((u8 *)Netbuffer,4))
//			  {
//			  sprintf((char *)strNET,"NET=%d.%d.%d.%d\r\n",Netbuffer[0],Netbuffer[1],Netbuffer[2],Netbuffer[3]);
//			  uart1SendChars((u8*)strNET,25);	
//			  NET1=Netbuffer[0];
//			  NET2=Netbuffer[1];
//			  NET3=Netbuffer[2];
//			  NET4=Netbuffer[3];
//			  }
//		 else
//			  {
//			  NET1=255;
//			  NET2=255;
//			  NET3=255;
//			  NET4=0;
//			  }
//			}
//		AT24CXX_Read(16,(u8*)WGbuffer,5);
//		if(WGbuffer[4]==crc_high_first((u8 *)WGbuffer,4))
//			{
//			sprintf((char *)strWG,"WG=%d.%d.%d.%d\r\n",WGbuffer[0],WGbuffer[1],WGbuffer[2],WGbuffer[3]);
//			uart1SendChars((u8*)strWG,25);
//			WG1=WGbuffer[0];
//			WG2=WGbuffer[1];
//			WG3=WGbuffer[2];
//			WG4=WGbuffer[3];
//			}		
//		else
//			{
//			AT24CXX_Read(56,(u8*)WGbuffer,5);
//			if(WGbuffer[4]==crc_high_first((u8 *)WGbuffer,4))
//			  {
//			  sprintf((char *)strWG,"WG=%d.%d.%d.%d\r\n",WGbuffer[0],WGbuffer[1],WGbuffer[2],WGbuffer[3]);
//			  uart1SendChars((u8*)strWG,25);	
//			  WG1=WGbuffer[0];
//			  WG2=WGbuffer[1];
//			  WG3=WGbuffer[2];
//			  WG4=WGbuffer[3];
//			  }
//			else
//			 {
//			  WG1=192;
//			  WG2=168;
//			  WG3=1;
//			  WG4=1;
//			 }		
//			}
//		AT24CXX_Read(24,(u8*)PORTbuffer,3);    			
//		if(PORTbuffer[2]==crc_high_first((u8 *)PORTbuffer,2))
//		{
//     PORT1=(PORTbuffer[1]<<8)+PORTbuffer[0];
//		 sprintf((char *)strPORT,"PORT=%d\r\n",PORT1); 
//     uart1SendChars((u8 *)strPORT,25);			
//		}
//		else
//		{
//		 	AT24CXX_Read(64,(u8*)PORTbuffer,3); 
//      if(PORTbuffer[2]==crc_high_first((u8 *)PORTbuffer,2))
//      {
//			 PORT1=(PORTbuffer[1]<<8)+PORTbuffer[0];
//		   sprintf((char *)strPORT,"PORT=%d\r\n",PORT1); 
//       uart1SendChars((u8 *)strPORT,25);				
//			}
//      else
//      {
//			 PORT1=2040;
//			}						
//		}
//    AT24CXX_Read(32,(u8*)RS485buffer,3);
//    if(RS485buffer[1]==crc_high_first((u8 *)RS485buffer,1))
//		{
//		  RS485_ID=RS485buffer[0];
//		  sprintf((char *)strRS485ID,"ID=%d\r\n",RS485_ID); 
//      uart1SendChars((u8 *)strRS485ID,25);	      			
//		}			
//		else
//		{
//		 AT24CXX_Read(72,(u8*)RS485buffer,3);
//    if(RS485buffer[1]==crc_high_first((u8 *)RS485buffer,1))
//		{
//		  RS485_ID=RS485buffer[0];
//		  sprintf((char *)strRS485ID,"ID=%d\r\n",RS485_ID); 
//      uart1SendChars((u8 *)strRS485ID,25);	      			
//		}		
//    else	
//    {			
//		  RS485_ID=0x01;
//		}
//		}
//}


















