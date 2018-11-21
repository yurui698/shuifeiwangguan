/* ��վID��PA.00,PC.02,PB.10,PB.11PB.08,PB.09(1--6) �������ŵ���PD.02 PB.03~PB.06(1---5)������ͨ����J0---PC.03,J1---PB.12,J3---PB.13,J2---PB.14
������⣺I0---PC0(IN10)��I1---PB0(IN8),I2---PC1(IN11)��I3---PB1(IN9);�����ز�����2:TXD2---PA3 ,RXD2---PA2,RTS2---PA1;����3:TXD3---PC10,RXD3---PC11,RTS3---PA15,Remap
SWD�ӿ�:SWCLK---PA14,SWDIO---PA13,NRST---NRST;TTL����1��TXD1---PA9��RXD1---PA10������ָʾ--PC5;����ָʾ--PC12*/
#include "HandleTask.h"
#include "systemclock.h"
#include "string.h"
#include "DataFlash.h"
#include "Stm32_Configuration.h"
#include "si4463.h"

/*Modbus��վID*/
#define SLAVE_ID                 0x01
#define ID                       0x88
#define DEBUG_ID                 0x99

/*Modbus������*/
#define READ_COIL_STATUS         0x01   /*����Ȧ�Ĵ���  :��֧�ֹ㲥���涨Ҫ������ʼ��Ȧ����Ȧ��                  */
#define READ_INPUT_STATUS        0x02	/*��״̬�Ĵ���	:��֧�ֹ㲥���涨Ҫ����������ʼ��ַ���Լ������źŵ�����  */
#define READ_HOLDING_REGISTER    0x03	/*�����ּĴ���	:��֧�ֹ㲥���涨Ҫ���ļĴ�����ʼ��ַ���Ĵ���������      */
#define READ_INPUT_REGISTER      0x04 	/*������Ĵ���	:��֧�ֹ㲥���涨Ҫ���ļĴ�����ʼ��ַ���Ĵ���������      */
#define WRITE_SINGLE_COIL        0x05 	/*д����Ȧ�Ĵ���:֧�ֹ㲥���涨��������Ȧ��ON/OFF״̬��0xFF00ֵ������Ȧ��
														 ��ON״̬��0x0000ֵ������Ȧ����OFF״̬������ֵ����Ȧ��Ч */
#define WRITE_SINGLE_REGISTER    0x06 	/*д�����ּĴ���:֧�ֹ㲥�������Ԥ��ֵ�ڲ�ѯ������                      */
#define WRITE_MULTIPLE_COIL      0x0F	/*д����Ȧ�Ĵ���:֧�ֹ㲥                                                */
#define WRITE_MULTIPLE_REGISTER  0x10	/*д�ౣ�ּĴ���:֧�ֹ㲥                                                */

#define WirelessLedToggle  		 GPIO_WriteBit(GPIOC,GPIO_Pin_12, (BitAction)(1-GPIO_ReadOutputDataBit(GPIOC,GPIO_Pin_12)))//PC6-PC12�޸�
#define YX_LED_TOGGLE  		 GPIO_WriteBit(GPIOC,GPIO_Pin_5, (BitAction)(1-GPIO_ReadOutputDataBit(GPIOC,GPIO_Pin_5)))
#define YX_LED_ON               GPIO_ResetBits(GPIOC, GPIO_Pin_5)
/*RS485�շ��л�,PB12����*/
#define TXENABLE2		         GPIO_SetBits(GPIOA, GPIO_Pin_1)
#define RXENABLE2		         GPIO_ResetBits(GPIOA, GPIO_Pin_1)
#define TXENABLE3		         GPIO_SetBits(GPIOA, GPIO_Pin_15)
#define RXENABLE3		         GPIO_ResetBits(GPIOA, GPIO_Pin_15)

#define CONTROLLER        		0xEE

#define MEASURE_PERIOD          300

#define ROLLER1_UP   	do{GPIO_ResetBits(GPIOC, GPIO_Pin_3); GPIO_SetBits(GPIOB, GPIO_Pin_12);   }while(0)
#define ROLLER1_DOWN 	do{GPIO_SetBits(GPIOC, GPIO_Pin_3);   GPIO_ResetBits(GPIOB, GPIO_Pin_12); }while(0)
#define ROLLER1_STOP 	do{GPIO_SetBits(GPIOC, GPIO_Pin_3);   GPIO_SetBits(GPIOB, GPIO_Pin_12);   }while(0)
#define ROLLER2_UP   	do{GPIO_ResetBits(GPIOB, GPIO_Pin_14);GPIO_SetBits(GPIOB, GPIO_Pin_13);   }while(0)
#define ROLLER2_DOWN 	do{GPIO_SetBits(GPIOB, GPIO_Pin_14);  GPIO_ResetBits(GPIOB, GPIO_Pin_13); }while(0)
#define ROLLER2_STOP 	do{GPIO_SetBits(GPIOB, GPIO_Pin_14);GPIO_SetBits(GPIOB, GPIO_Pin_13);}while(0)
#define CTRL_OPEN0    GPIO_ResetBits(GPIOC, GPIO_Pin_3)
#define CTRL_OPEN1    GPIO_ResetBits(GPIOB, GPIO_Pin_12)
#define CTRL_OPEN2    GPIO_ResetBits(GPIOB, GPIO_Pin_14)
#define CTRL_OPEN3    GPIO_ResetBits(GPIOB, GPIO_Pin_13)
#define CTRL_CLOSE0   GPIO_SetBits(GPIOC, GPIO_Pin_3)
#define CTRL_CLOSE1   GPIO_SetBits(GPIOB, GPIO_Pin_12)
#define CTRL_CLOSE2   GPIO_SetBits(GPIOB, GPIO_Pin_14)
#define CTRL_CLOSE3   GPIO_SetBits(GPIOB, GPIO_Pin_13)

#define true 1
#define false 0
#define bool u8

/*CRC��λ�ֽڱ�*/
const unsigned char crc_lo[256]= {
    0x00,0xC0,0xC1,0x01,0xC3,0x03,0x02,0xC2,0xC6,0x06,0x07,0xC7,0x05,0xC5,0xC4,
    0x04,0xCC,0x0C,0x0D,0xCD,0x0F,0xCF,0xCE,0x0E,0x0A,0xCA,0xCB,0x0B,0xC9,0x09,
    0x08,0xC8,0xD8,0x18,0x19,0xD9,0x1B,0xDB,0xDA,0x1A,0x1E,0xDE,0xDF,0x1F,0xDD,
    0x1D,0x1C,0xDC,0x14,0xD4,0xD5,0x15,0xD7,0x17,0x16,0xD6,0xD2,0x12,0x13,0xD3,
    0x11,0xD1,0xD0,0x10,0xF0,0x30,0x31,0xF1,0x33,0xF3,0xF2,0x32,0x36,0xF6,0xF7,
    0x37,0xF5,0x35,0x34,0xF4,0x3C,0xFC,0xFD,0x3D,0xFF,0x3F,0x3E,0xFE,0xFA,0x3A,
    0x3B,0xFB,0x39,0xF9,0xF8,0x38,0x28,0xE8,0xE9,0x29,0xEB,0x2B,0x2A,0xEA,0xEE,
    0x2E,0x2F,0xEF,0x2D,0xED,0xEC,0x2C,0xE4,0x24,0x25,0xE5,0x27,0xE7,0xE6,0x26,
    0x22,0xE2,0xE3,0x23,0xE1,0x21,0x20,0xE0,0xA0,0x60,0x61,0xA1,0x63,0xA3,0xA2,
    0x62,0x66,0xA6,0xA7,0x67,0xA5,0x65,0x64,0xA4,0x6C,0xAC,0xAD,0x6D,0xAF,0x6F,
    0x6E,0xAE,0xAA,0x6A,0x6B,0xAB,0x69,0xA9,0xA8,0x68,0x78,0xB8,0xB9,0x79,0xBB,
    0x7B,0x7A,0xBA,0xBE,0x7E,0x7F,0xBF,0x7D,0xBD,0xBC,0x7C,0xB4,0x74,0x75,0xB5,
    0x77,0xB7,0xB6,0x76,0x72,0xB2,0xB3,0x73,0xB1,0x71,0x70,0xB0,0x50,0x90,0x91,
    0x51,0x93,0x53,0x52,0x92,0x96,0x56,0x57,0x97,0x55,0x95,0x94,0x54,0x9C,0x5C,
    0x5D,0x9D,0x5F,0x9F,0x9E,0x5E,0x5A,0x9A,0x9B,0x5B,0x99,0x59,0x58,0x98,0x88,
    0x48,0x49,0x89,0x4B,0x8B,0x8A,0x4A,0x4E,0x8E,0x8F,0x4F,0x8D,0x4D,0x4C,0x8C,
    0x44,0x84,0x85,0x45,0x87,0x47,0x46,0x86,0x82,0x42,0x43,0x83,0x41,0x81,0x80,
    0x40
};


/*CRC��λ�ֽڱ�*/
const unsigned char crc_hi[256]= {
    0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,
    0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,
    0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,
    0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,
    0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,
    0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,
    0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,
    0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
    0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,
    0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,
    0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,
    0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
    0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,
    0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x01,0xC0,
    0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,
    0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
    0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,
    0x40
};
extern u8   RxFlag3;	//����2���ձ�־λ��0���ڽ��գ�1�������
extern u8   RxFlag2;	//����2���ձ�־λ��0���ڽ��գ�1�������
extern u8   RxFlag1;//����1���ձ�־λ��0���ڽ��գ�1�������

extern u8   TxFlag1;//����1���ͱ�־λ��0���ڷ��ͣ�1�������
extern u8   TxFlag2;	//����2���ͱ�־λ��0���ڷ��ͣ�1�������
extern u8   TxFlag3;	//����2���ͱ�־λ��0���ڷ��ͣ�1�������
extern u8   RecDataBuffer3[];
extern u8   RecLen3;
extern u8   RecDataBuffer2[];
extern u8   RecLen2;
extern u8   RecDataBuffer1[];
extern u8   RecLen1;

extern u8   USART1SendTCB[];
extern u8   USART1BufferCNT;
extern u8   USART2SendTCB[];
extern u8   USART2BufferCNT;
extern u8   USART3SendTCB[];
extern u8   USART3BufferCNT;

extern __IO uint16_t ADC_ConvertedValue[];

extern u8 	SI4463_RxBUFF[];
extern u8 	SI4463_RxLenth;
extern u8 	Int_BUFF[];

static u8  ReportData2[128];
static u8 ReceiveData2[128];
static u8 RxReport2_len;

static u8  ReportData3[128];
static u8 ReceiveData3[128];
static u8 RxReport3_len;
__attribute__((section("NO_INIT"),zero_init)) static u16 ControlValue[4];

__attribute__((section("NO_INIT"),zero_init)) static u8  collector_data_buff[16];
static u8  ctrlslave_param_set[24]= {0xFF,0xFF,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0xFF,0xFF,
                                     0xFF,0xFF,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0xFF,0xFF
                                    };//����������Ҫ���趨�������������ʼ����ʱ�䡢��������ʱ��
__attribute__((section("NO_INIT"),zero_init)) static u8  collector_fertigation_data[16];//ʩ�ʣ���Induction Heater.sct�ж��壬Ŀǰ������2kbyte
//����Ϊ���趨�Ĳ������ɼ���ʽ�������������˲�������ͨ��5����6ͨ����ΪUSART3���涨
static u8  cjkzslave_param_set[36]= {0x00,0x00,0x01,0x00,0x00,0x00,
                                     0x00,0x00,0x02,0x00,0x00,0x00,
                                     0x00,0x00,0x01,0x00,0x32,0x00,
                                     0x00,0x00,0x01,0x00,0x32,0x00,
                                     0x00,0x00,0x01,0x00,0x00,0x00,
                                     0xFE,0x00,0x04,0x00,0x02,0x00
                                    };//��վ��ַ��������ʼ��ַ����������;0x0D���⣨RS485����ˮ��+�¶�+EC��
//�ɼ���ʽ���͡����ֽڣ��������������͡����ֽڣ����˲�������0~65536�����͡����ֽڣ���6*3*2=36
/*�ɼ���ʽ�ֱ��Ӧ0~5ͨ��˵��
00:                   ������;
01                    ���ն�����
02                    ������ʪ������
03                    4~20mA���루ȡ��50�����裬0~1Vdc���룩
04                    ������̼����
05                    ����ѹ������
06                    ����������
07                    Ƶ������
08                    0~1Vdc���루ȡ��50�����裩
09
10
28	ͨ��0�����գ���28(0x1C)  ����ʱ�� k=60��b=0 ��ʾ��λ��Сʱ��;
																		cjkzslave_param_set[0]~[5]={0x1C,0x00,0x0A,0x00,0x64,0x00},�������޼���10lux,��������100lux

    ͨ��5������3�� RS485����6ͨ����
             254��ͨ��5�� RS485����ˮ�֡��¶ȡ�EC��ˮ�����ɽ�8����ˮ��+�¶����ɽ�4����ˮ��+�¶�+EC�ɽ�2������
					    ������������վ��ַ��FE��FD��FC��FB��������������ˮ�֣�k=10,b=0;�����¶ȣ�k=10,b=-40;����EC�� k=10,b=0
					   �ɼ���ʽ=254,��������=����������,�˲�����=1(ˮ��)��=2��ˮ��+�¶ȣ���=3��ˮ��+�¶�+EC��


32   ͨ��4   cjkzslave_param_set[24]=32��0x20�� ֪ͨ������Ϊ���Ʋɼ���ʽ��ˮ�ʻ�ʩ�������Ʒ���RS485�ɼ�ʹ�ã���ͨ��5��ɼ�����ͬ��
*/
static u8  ReadDataCNT2        =  0;
static u8  ReadDataCNT1       =  0;
static u8  slave_ctrl_ID      =  0;
static u8  SI4463_TxBUFF[128];
static u16 regLen;
static u8  SI4463_Channel     =  0;

static u8  roller1_state=0;//0��ʾͣ��1��ʾ���У�2��ʾ����
static u8  roller2_state=0;//0��ʾͣ��1��ʾ���У�2��ʾ����
static u16 MotorCurrent0              =0;
static u16 MotorCurrent1              =0;
static u16 MotorMAX[4]= {0,0,0,0};
static u16 MotorCurrent2              =0;
static u16 MotorCurrent3              =0;

static u16 regLen2;

static void RxReport2(u8 len,u8 *pBuf);

static void SI4463RxReport(u8 len,u8 *pData);

static void sensor_data(void);
static u16  GetCRC16(u8 *Msg, u16 Len);
static void WriteDataToBuffer(u8 port,u8 *ptr,u8 start,u8 len);
static void Clear_Buf(u8 buf[], u8 length, u8 data);
//static void	Get_ID(void);
//static void	Get_Channel(void);
static void startadc(void);
static void IO_ctrl_cmd(void);
//�����趨�¶���
static u8 single_delay_time[4]= {0,0,0,0}; //���������ʱ����ʱ��
static u8 single_delay_flg[4]= {0,0,0,0}; //�������ֹͣ���б�־��single_delay_flg[i]!=0����������
static u8 bidirection_delay_time[4]= {0,0,0,0}; //����-ͣ-���򡢷���-ͣ-���� ��ʱ����ʱ��
static u8 bidirection_delay_flg[4]= {0,0,0,0}; //˫�����ֹͣ���б�־��bidirection_delay_flg[i]!=0����������
static u8 bidirection_run_time[4]= {0,0,0,0}; //��ת-��ת�л���ʱ����ʱ��
static u8 bidirection_run_flg[4]= {0,0,0,0}; //��ת-��ת�л����б�־��bidirection_run_flg[i]=0 ͣ״̬��=1�������У�=2 ��������
static u16 single_old_ControlValue[4] = {0x0000,0x0000,0x0000,0x0000};
static u16 bidirection_old_ControlValue[4] = {0x0008,0x0008,0x0008,0x0008};
static u16 max_limit_current[4] = {0xFFFF,0xFFFF,0xFFFF,0xFFFF};//max_limit_current[i]>0xFFF0 ��������max_limit_current[i]/65535*����
static u16 start_run_time[4] = {0x0000,0x0000,0x0000,0x0000};//start_run_time[i]=0x0000 ������������λ���룻65534/3600=18.2Сʱ
static u16 continue_run_time[4] = {0xFFFF,0xFFFF,0xFFFF,0xFFFF};//continue_run_time[i]=0xFFFF,�������У������ƣ���λ���룻65534/3600=18.2Сʱ
static u32 start_run_nowtime[4] = {0x0000,0x0000,0x0000,0x0000};//start_run_now[i]=0x0000 ������������λ���룻655280/3600=18.133Сʱ;��ʱ�������
static u32 continue_run_nowtime[4] = {0xFFFF0,0xFFFF0,0xFFFF0,0xFFFF0};//continue_run_time[i]>0xFF00,�������У������ƣ���λ���룻65534/3600=18.2Сʱ;��ʱ�������
static void slave_init_readflash(void);//��վ��ʼ����flash
static u8  bidirection_location_flg[4]= {0,0,0,0}; //�������״̬��λ�ã�0��8��ʾͣ��1��ʾ��תͣ��λ�ã�2��ʾ��תͣ��λ��
static u8  control_type[4]= {0,0,0,0}; //0��ʾ������ƣ�1��ʾ�������ƣ����յ��Ŀ��������趨�ġ�
static u8  batch_ctrl_finish[4]= {0,0,0,0}; //0��ʾ��������������ɣ�1��ʾ����ִ��������������
u8 close_433MHZ=1;//��ʹ��433MHZͨ�Ż�433MHZͨ�ų����⣬��close_433MHZ=0
//����Ϊ�Զ�ʶ��վ��ַ��433MHZ�ŵ�����
static u8  slaveID_radioID[2]= {0xFF,0xFF}; //վ��ַ��433MHZ�ŵ��趨

/*ADC�ɼ���������*/
u16 Get_Adclvbo(u8 TD_Xnumber,u16 TD_xiaxian,u16 TD_shangxian);	//�ɼ�100�����ݣ�����ȡ40~60ƽ��ֵ
u16 First_Getaverage(u8 td_xnumber,u8 maxlvbo_xnumber,u16 temp_adc);//��ʼ��ƽ��ֵ
u16 TD_Getaverage(u8 td_xnumber,u8 tdlvbo_xnumber,u16 temp_xadc,u8 tdcycle_xi);//ADCͨ����ƽ��ֵ
u16 SelectTD(u8 TD_number);//ѡ��ͨ�����ɼ�����ͨ����ֵ
/*��ͨ��ƽ��ֵ����*/
const u8 MaxTD_number= 4;		  //4��ADC�ɼ�ͨ��
const u8 Maxlvbo_number= 100;	  //ƽ�������������
static u8 TDlvbo_number[MaxTD_number]= {50,50,50,50};	//ͨ��0��50��������ƽ��ֵ��8��/��
//ͨ��1��50��������ƽ��ֵ��8��/��
static  u16 Adc_average[MaxTD_number][Maxlvbo_number]; //ADC��ƽ��ֵ���飬�����ʷ�ɼ�����
static u8 tdcycle_i[MaxTD_number]= {0,0,0,0}; //ѭ���������飬0~TDlvbo_number[i]-1��iΪͨ���ţ�i=0��1��....
static u8 First_adc_average[MaxTD_number]= {0,0,0,0};
/*�״��˲�����First_adc_average=0ʱ������First_Getaverage(u8 TD_number,u8 Maxlvbo_number,u16 temp_adc)
�������  TD_Getaverage(u8 TD_number,u8 TDlvbo_number[TD_number],u16 temp_adc,u8 tdcycle_i[TD_number])*/
//����3����
static void usart3_send_cmd(void);
static void RxReport3(u8 len,u8 *pBuf);
static u8 ReadData(u8 Slave_ID,u8 function_code,u16 addr,u16 num,u8 *temp);
static u8 USART3_send_sequence;
static u8 switch_cmd_TR485_addr=0xFE;
static u8 switch_cmd_RS485_CNT=0;
static u8 data_RS485[8][6];//����ˮ�֡��¶ȡ�EC�����ֻ������ˮ�֣��Ϳ�����8����������ֻʹ��data_RS485[n][0-1];��
static u8 coll_ctrl_zz3_type;
static u8  TD_param_num=0;
//static u8 test[50];
//�¼����м�ض���
static u8  evt_sensor_data;
static u8  evt_check_eachevt;
static u8  evt_wx_cmd=0;

void Period_Events_Handle(u32 Events)
{
    if(Events&SYS_INIT_EVT)
    {
        RXENABLE2;
//		ROLLER1_STOP;
//   	ROLLER2_STOP;

        //����ͨ�ŵ���
        YX_LED_ON;//��Ҫ��ԭ��ͼ��

        slave_init_readflash();
        startadc();
        if(RCC_GetFlagStatus(RCC_FLAG_PORRST)!=RESET)
        {
            ROLLER1_STOP;
            ROLLER2_STOP;
            memset(ControlValue,0x00,sizeof(ControlValue));
            memset(collector_data_buff,0x00,sizeof(collector_data_buff));
            memset(collector_fertigation_data,0x00,sizeof(collector_fertigation_data));
        }
        RCC_ClearFlag();
        if(close_433MHZ!=0)
        {
            SI4463_Init();
            Start_timerEx(WX_CMD_EVT,2000);
        }
        /*����3--RS485���룺Ĭ��9600-8-1-no;�ɼ���Ϊ��վ��cjkzslave_param_set[30]ΪҪ��ѯ����վ��ַ��cjkzslave_param_set[32]
        ΪҪ��ѯ����վ������ʼ��ַ��cjkzslave_param_set[34]ΪҪ��ѯ�ı���������*/
        if(cjkzslave_param_set[30]!=0)//��������3
        {
            USART3_Configuration();
            Start_timerEx(TX3_SEND_EVT,3000);
        }
        Start_timerEx(SENSOR_DATA_EVT,2000);
        Start_timerEx(IO_CTRL_CMD_EVT,1000);
        Start_timerEx(CHECK_EACHEVT_EVT,10000);
    }

    YX_LED_TOGGLE;
    if(Events&IO_CTRL_CMD_EVT)
    {
        IO_ctrl_cmd();
        Start_timerEx( IO_CTRL_CMD_EVT,100 );
    }

    if(Events&SENSOR_DATA_EVT)
    {
        evt_sensor_data=0;
        sensor_data();
        Start_timerEx( SENSOR_DATA_EVT, MEASURE_PERIOD );
        if(evt_check_eachevt<=250) {
            evt_check_eachevt++;
        }
        else {
            __set_FAULTMASK(1);
            NVIC_SystemReset();
            while(1);
        }
    }
    if(Events&RX2_DELAY_EVT)
    {

        RxFlag2 = 1;
        RxReport2(RxReport2_len,ReceiveData2);
    }

    if(Events&RX2_TIMEOUT_EVT)
    {
        RecDataBuffer2[0] = RecLen2-1;
        RecLen2 = 1;
        RxReport2_len=RecDataBuffer2[0];
        memcpy(ReceiveData2,RecDataBuffer2+1,RecDataBuffer2[0]);
        Start_timerEx(RX2_DELAY_EVT,3);
    }

    if(Events&WX_CMD_EVT)//����SI4463���ڽ���״̬���ȴ������ж�
    {
        evt_wx_cmd=0;
        Clear_Buf(SI4463_RxBUFF,SI4463_RxLenth,0);
        SI4463_SET_PROPERTY_1( PKT_FIELD_1_LENGTH_12_8, 0x00 );
        SI4463_SET_PROPERTY_1( PKT_FIELD_1_LENGTH_7_0, 0x01 );
        SI4463_START_RX( SI4463_Channel, 0, PACKET_LENGTH, 8, 3, 3 );
    }

    if(Events&WX_RECEIVE_EVT)
    {
        WirelessLedToggle;
        SI4463_RxLenth = SI4463_READ_PACKET(SI4463_RxBUFF);
        SI4463RxReport(SI4463_RxLenth,SI4463_RxBUFF);
        Start_timerEx(WX_CMD_EVT,50);
    }
    if(Events&TX3_SEND_EVT)
    {
        usart3_send_cmd();
        Start_timerEx(TX3_SEND_EVT,300);
    }
    if(Events&RX3_DELAY_EVT)
    {
        RxFlag3 = 1;
        RxReport3(RxReport3_len,ReceiveData3);
    }
    if(Events&RX3_TIMEOUT_EVT)
    {
        RecDataBuffer3[0] = RecLen3-1;
        RecLen3 = 1;
        RxReport3_len=RecDataBuffer3[0];
        memcpy(ReceiveData3,RecDataBuffer3+1,RecDataBuffer3[0]);
        Start_timerEx(RX3_DELAY_EVT,3);
    }
    if(Events&CHECK_EACHEVT_EVT)
    {
        evt_check_eachevt=0;
        if(evt_sensor_data<=50) {
            evt_sensor_data++;
        }
        else {
            __set_FAULTMASK(1);
            NVIC_SystemReset();
            while(1);
        }
        if(evt_wx_cmd<=180) {
            evt_wx_cmd++;
        }
        else
        {
            close_433MHZ=1;
            evt_wx_cmd=0;
            slave_init_readflash();
            SI4463_Init();
            Start_timerEx(WX_CMD_EVT,2000);
        }
        Start_timerEx(CHECK_EACHEVT_EVT,10000);
    }
}

void Scan_Events_Handle(void)
{
    IWDG_ReloadCounter();					//�������Ź�ι��
}



static void WriteDataToBuffer(u8 port,u8 *ptr,u8 start,u8 len)
{
    //USART_SendData�����У�u32 i=0x004FFFFF;//��ֹȫ˫��Ӳ��ͨ��ʱ��Ӳ��CTS����������ɵȴ�״̬��������watchdog(3S)��ͣ������;2S���ң��Կ��Ź���������Ϊ׼
    u8 i;
    if(port==2)
    {
        for(i=0; i<len; i++)
            USART2SendTCB[i] = ptr[start+i];
        USART2BufferCNT=len;
        TXENABLE2;
        TxFlag2=1;
        USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
    }

    else if(port==1)
    {
        for(i=0; i<len; i++)
            USART1SendTCB[i] = ptr[start+i];
        USART1BufferCNT=len;
        TxFlag1=0;
        USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
    }

    else
        return;
}

int a=0;

static u16 GetCRC16(u8 *Msg, u16 Len)
{
    u8 CRCHigh = 0xFF;//��CRC�ֽڳ�ʼ��
    u8 CRCLow  = 0xFF;//��CRC�ֽڳ�ʼ��
    u16 index;		  //CRCѭ���е�����
    u8 i=0;

    for(i=0; i<Len; i++)
    {
        index   = CRCHigh^*Msg++;
        CRCHigh = CRCLow^crc_hi[index];
        CRCLow  = crc_lo[index];
    }

    return(CRCLow<<8|CRCHigh);
}

/*���������յ�������*/
static void RxReport2(u8 len,u8 *pBuf)
{
    if(pBuf[0]==slave_ctrl_ID)
    {
        if(GetCRC16(pBuf,len)==0)
        {
            u16 address,CRCReport2;

//			RS485LedToggle;										//����ͨ�ŵƷ�ת
            if(pBuf[1]==0x03)//�ɼ��ĵ���������������
            {
                address = (pBuf[2]<<8) | pBuf[3];
                regLen2  = (pBuf[4]<<8) | pBuf[5];
                ReportData2[0] = slave_ctrl_ID;	                //վ��
                ReportData2[1] = 0x03;	                        //������
                ReportData2[2] = regLen2*2;	                    //�ֽ���
                memcpy(ReportData2+3,collector_data_buff+address*2,ReportData2[2]);
                CRCReport2=GetCRC16(ReportData2,3+regLen2*2);
                ReportData2[3+regLen2*2] = CRCReport2&0x00FF;      //CRC��λ
                ReportData2[4+regLen2*2] = (CRCReport2&0xFF00)>>8; //CRC��λ
                WriteDataToBuffer(2,ReportData2,0,5+regLen2*2);
                return;
            }
            if(pBuf[1]==0x06)
            {
                if(pBuf[2]==0x00&&pBuf[3]<=0x03) //pBuf[6]==0x08 �����տ�������
                {
                    ControlValue[pBuf[3]]=(pBuf[4]|(((u16)pBuf[5])<<8));		//ȡ������ָ��
                }
                memcpy(ReportData2,pBuf,8);//��������=�·����8���ֽ���ȫ��ͬ
                WriteDataToBuffer(2,ReportData2,0,8);
                return;
            }
            if(pBuf[1]==0x10)
            {
                memcpy(ReportData2,pBuf,6);//��������ǰ6���ֽ���ȫ��ͬ��ֻ���CRCУ����
                CRCReport2=GetCRC16(ReportData2,6);
                ReportData2[6] = CRCReport2&0x00FF;      //CRC��λ
                ReportData2[7] = (CRCReport2&0xFF00)>>8; //CRC��λ
                WriteDataToBuffer(2,ReportData2,0,8);
                if(pBuf[3]>=0x04&&pBuf[3]<0x10)//����������Ҫ���趨�������������ʼ����ʱ�䡢��������ʱ��,12��������24���ֽ�
                {
                    memcpy(ctrlslave_param_set,pBuf+7,pBuf[6]);//����������Ҫ���趨�������������ʼ����ʱ�䡢��������ʱ��
                    Flash_Write(0x0801C400, ctrlslave_param_set,24) ; //RBT6��FLASHΪ128k��ramΪ20k���˴��������趨����������flash����16k
                    slave_init_readflash();
                    return;
                }

                if(pBuf[3]>=0x10&&pBuf[3]<0x22)//�����������ɼ��趨����������flash����16k
                {
                    memcpy(cjkzslave_param_set,pBuf+7,pBuf[6]); //�ɼ�����������Ĳɼ������趨���Բɼ����������Ϊ1�飬ÿ��18��������ÿ���ɼ�ͨ��3������:�ɼ���ʽ�������������˲�������6ͨ���ɼ�����
                    Flash_Write(0x0801C000, cjkzslave_param_set,36) ; //RBT6��FLASHΪ128k��ramΪ20k���˴�������������flash����16k;�ɼ���ʽ�������������˲�����
                    slave_init_readflash();
                    return;
                }
            }
        }
    }
    if(pBuf[0]==0xF0&&pBuf[1]==0x10&&pBuf[3]==34)
    {
        if(GetCRC16(pBuf,len)==0)
        {
            u16 CRCReport2;
            if(pBuf[7]!=65)//���ŵ�=0xFF��ʾ433MHZͨ���ŵ�Ҫ�ı䣬pBuf[7]==65���е���վ����Ҫ�ı䣬����Ҫ�ظ���
            {
                memcpy(ReportData2,pBuf,6);//��������ǰ6���ֽ���ȫ��ͬ��ֻ���CRCУ����
                CRCReport2=GetCRC16(ReportData2,6);
                ReportData2[6] = CRCReport2&0x00FF;      //CRC��λ
                ReportData2[7] = (CRCReport2&0xFF00)>>8; //CRC��λ
                WriteDataToBuffer(2,ReportData2,0,8);
            }
            if(slaveID_radioID[0]==0xFF||slaveID_radioID[0]==0x00||(slaveID_radioID[0]==pBuf[7]&&pBuf[7]<=64&&pBuf[9]!=255))
            {
                slaveID_radioID[0]=pBuf[8];//�趨��վ��ַ���޸���վ��ַ
            }

            if(pBuf[9]==255&&(pBuf[7]==65||slaveID_radioID[0]==pBuf[7]))
            {
                slaveID_radioID[1]=pBuf[10];//slaveID_radioID[0]���޸ģ�ֻ���ŵ���������վ��ַ
            }//pBuf[7]��վ�ϵ�ַ��pBuf[8]��վ�µ�ַ��pBuf[9]��վ���ŵ���pBuf[10]��վ���ŵ�
            Flash_Write(0x0801C800, slaveID_radioID,2) ; //RBT6��FLASHΪ128k��ramΪ20k���˴�������������flash���16k�ĵ�3��bank����3k��;
            slave_init_readflash();
            if(close_433MHZ!=0&&pBuf[9]==255&&(pBuf[7]==65||slaveID_radioID[0]==pBuf[7]))//pBuf[7]==65Ϊ�����趨�ŵ�
            {
                SI4463_Init();
                Start_timerEx(WX_CMD_EVT,2000);
            }
        }
        return;
    }
    if(cjkzslave_param_set[24]==32&&pBuf[0]==slave_ctrl_ID-32)//ˮ��41~64������ͨ��������Ϊ9~32�ɼ���
    {
        if(GetCRC16(pBuf,len)==0)
        {
            u16 address,CRCReport2;
            if(pBuf[1]==0x03)//��ȡ������������
            {
                //        test++;
                address = (pBuf[2]<<8) | pBuf[3];
                regLen2  = (pBuf[4]<<8) | pBuf[5];
                ReportData2[0] = slave_ctrl_ID-32;	                //վ��
                ReportData2[1] = 0x03;	                        //������
                ReportData2[2] = regLen2*2;	                    //�ֽ���
                memcpy(ReportData2+3,collector_fertigation_data+address*2,ReportData2[2]);
                CRCReport2=GetCRC16(ReportData2,3+regLen2*2);
                ReportData2[3+regLen2*2] = CRCReport2&0x00FF;      //CRC��λ
                ReportData2[4+regLen2*2] = (CRCReport2&0xFF00)>>8; //CRC��λ
                WriteDataToBuffer(2,ReportData2,0,5+regLen2*2);
            }
        }
        return;
    }
}


//�յ�RBT6��������
static void SI4463RxReport(u8 len,u8 *pData)
{
//	memcpy(test,pData,30);
    if(pData[0]==slave_ctrl_ID)
    {
        if(GetCRC16(pData,len)==0)
        {
            u16 address,SI4463_CRC;

            if(pData[1]==0x03)
            {
                address = (pData[2]<<8) | pData[3];
                regLen  = (pData[4]<<8) | pData[5];
                SI4463_TxBUFF[0] = pData[0];	                //վ��
                SI4463_TxBUFF[1] = 0x03;	                        //������
                SI4463_TxBUFF[2] = regLen*2;	                    //�ֽ���
                memcpy(SI4463_TxBUFF+3,collector_data_buff+address*2,SI4463_TxBUFF[2]);
                SI4463_CRC=GetCRC16(SI4463_TxBUFF,3+regLen*2);
                SI4463_TxBUFF[3+regLen*2] = SI4463_CRC&0x00FF;      //CRC��λ
                SI4463_TxBUFF[4+regLen*2] = (SI4463_CRC&0xFF00)>>8; //CRC��λ
                SI4463_SEND_PACKET(SI4463_TxBUFF,5+regLen*2,SI4463_Channel,0);
                return;
            }
            if(pData[1]==0x06)
            {
                if(pData[2]==0x00&&pData[3]<=0x03)
                {
                    ControlValue[pData[3]]=(pData[4]|(((u16)pData[5])<<8));		//ȡ������ָ��
                }
                memcpy(SI4463_TxBUFF,pData,8);//��������=�·����8���ֽ���ȫ��ͬ
                SI4463_SEND_PACKET(SI4463_TxBUFF,8,SI4463_Channel,0);
                return;
            }
            if(pData[1]==0x10)
            {
                memcpy(SI4463_TxBUFF,pData,6);//��������ǰ6���ֽ���ȫ��ͬ��ֻ���CRCУ����
                SI4463_CRC=GetCRC16(SI4463_TxBUFF,6);
                SI4463_TxBUFF[6] = SI4463_CRC&0x00FF;      //CRC��λ
                SI4463_TxBUFF[7] = (SI4463_CRC&0xFF00)>>8; //CRC��λ
                SI4463_SEND_PACKET(SI4463_TxBUFF,8,SI4463_Channel,0);
                if((pData[3]>=0x04)&(pData[3]<0x10)) //�������趨��1��������Ϊ1��,ÿ��12��������ÿ������ͨ��3��������
                {
                    memcpy( ctrlslave_param_set,pData+7,pData[6]);
                    Flash_Write(0x0801C400, ctrlslave_param_set,24) ; //RBT6��FLASHΪ128k��ramΪ20k���˴�������������flash����16k
                    slave_init_readflash();
                    return;
                }
                if((pData[3]>=0x10)&(pData[3]<0x22))  //�ɼ�����������Ĳɼ������趨���Բɼ����������Ϊ1�飬ÿ��18��������ÿ���ɼ�ͨ��3��������6ͨ���ɼ����������ߡ����߹���
                {
                    memcpy( cjkzslave_param_set,pData+7,pData[6]);
                    Flash_Write(0x0801C000, cjkzslave_param_set,36) ; //RBT6��FLASHΪ128k��ramΪ20k���˴�������������flash����16k
                    slave_init_readflash();
                    return;
                }
            }
        }
    }
    if(pData[0]==0xF0&&pData[1]==0x10&&pData[3]==34)
    {
        if(GetCRC16(pData,len)==0)
        {
            u16 CRCReport2;
            if(pData[7]!=65)//���ŵ�=0xFF��ʾ433MHZͨ���ŵ�Ҫ�ı䣬pData[7]==65���е���վ����Ҫ�ı䣬����Ҫ�ظ���
            {
                memcpy(ReportData2,pData,6);//��������ǰ6���ֽ���ȫ��ͬ��ֻ���CRCУ����
                CRCReport2=GetCRC16(ReportData2,6);
                ReportData2[6] = CRCReport2&0x00FF;      //CRC��λ
                ReportData2[7] = (CRCReport2&0xFF00)>>8; //CRC��λ
                WriteDataToBuffer(2,ReportData2,0,8);
            }
            if(slaveID_radioID[0]==0xFF||slaveID_radioID[0]==0x00||(slaveID_radioID[0]==pData[7]&&pData[7]<=64&&pData[9]!=255))
            {
                slaveID_radioID[0]=pData[8];//�趨��վ��ַ���޸���վ��ַ
            }
            if(pData[9]==255&&(pData[7]==65||slaveID_radioID[0]==pData[7]))
            {
                slaveID_radioID[1]=pData[10];//slaveID_radioID[0]���޸ģ�ֻ���ŵ���������վ��ַ
            }//pData[7]��վ�ϵ�ַ��pData[8]��վ�µ�ַ��pData[9]��վ���ŵ���pData[10]��վ���ŵ�
            Flash_Write(0x0801C800, slaveID_radioID,2) ; //RBT6��FLASHΪ128k��ramΪ20k���˴�������������flash���16k�ĵ�3��bank����3k��;
            slave_init_readflash();
            if(close_433MHZ!=0&&pData[9]==255&&(pData[7]==65||slaveID_radioID[0]==pData[7]))//pData[7]==65Ϊ�����趨�ŵ�
            {
                SI4463_Init();
                Start_timerEx(WX_CMD_EVT,2000);
            }
        }
        return;
    }
    if(cjkzslave_param_set[24]==32&&pData[0]==slave_ctrl_ID-32)//ˮ��41~64������ͨ��������Ϊ9~32�ɼ���
    {
        if(GetCRC16(pData,len)==0)
        {
            u16 address,SI4463_CRC;

            if(pData[1]==0x03&&pData[2]==0x00&&pData[3]==0x00&&pData[4]==0x00)
            {
                address = (pData[2]<<8) | pData[3];
                regLen  = (pData[4]<<8) | pData[5];
                SI4463_TxBUFF[0] = slave_ctrl_ID-32;	                //վ��
                SI4463_TxBUFF[1] = 0x03;	                        //������
                SI4463_TxBUFF[2] = regLen*2;	                    //�ֽ���
                memcpy(SI4463_TxBUFF+3,collector_fertigation_data+address*2,SI4463_TxBUFF[2]);
                SI4463_CRC=GetCRC16(SI4463_TxBUFF,3+regLen*2);
                SI4463_TxBUFF[3+regLen*2] = SI4463_CRC&0x00FF;      //CRC��λ
                SI4463_TxBUFF[4+regLen*2] = (SI4463_CRC&0xFF00)>>8; //CRC��λ
                SI4463_SEND_PACKET(SI4463_TxBUFF,5+regLen*2,SI4463_Channel,0);
            }
        }
    }
}


static void Clear_Buf(u8 buf[], u8 length, u8 data)
{
    u8 i = 0;
    for(i = 0; i < length; i++)
        buf[i] = data;

}

static void startadc(void)
{
    DMA_Cmd(DMA1_Channel1,ENABLE);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);  //����ת����ʼ��ADCͨ��DMA��ʽ���ϵĸ���RAM����
}
//ADC_ConvertedValue[0]--PC0--IN10--I0--����ͨ��0,[1]--PB0--IN8--I2--����ͨ��1,[2]--PC1--IN11--I1--����ͨ��2,[3]--PB1--IN9--I3--����ͨ��3
static void sensor_data(void)
{
    u8   TD_number;
    u16  temp_adc;
    switch(ReadDataCNT1)
    {
    case 0:
        if(control_type[0]==0&&control_type[1]==0)//0Ϊ�������
        {
            TD_number=0;
            temp_adc=Get_Adclvbo(TD_number,262,1305); //(4~20ma)*50��;0.2V��Ӧ262��1.0V��Ӧ1305
            if(First_adc_average[TD_number]==0)
            {
                temp_adc=First_Getaverage( TD_number, Maxlvbo_number, temp_adc);
                First_adc_average[TD_number]=1;
            }
            temp_adc=TD_Getaverage( TD_number,TDlvbo_number[TD_number], temp_adc,tdcycle_i[TD_number]);
            First_adc_average[TD_number]=1;
            tdcycle_i[TD_number]++;
            if(tdcycle_i[TD_number]>=TDlvbo_number[TD_number])
            {
                tdcycle_i[TD_number]=0;
            }
            MotorCurrent0 = temp_adc;

            TD_number=1;
            temp_adc=Get_Adclvbo(TD_number,262,1305);
            if(First_adc_average[TD_number]==0)
            {
                temp_adc=First_Getaverage( TD_number, Maxlvbo_number, temp_adc);
                First_adc_average[TD_number]=1;
            }
            temp_adc=TD_Getaverage( TD_number,TDlvbo_number[TD_number], temp_adc,tdcycle_i[TD_number]);
            First_adc_average[TD_number]=1;
            tdcycle_i[TD_number]++;
            if(tdcycle_i[TD_number]>=TDlvbo_number[TD_number])
            {
                tdcycle_i[TD_number]=0;
            }
            MotorCurrent1 = temp_adc;
            if(MotorCurrent0> MotorMAX[0])
            {
                MotorMAX[0]=MotorCurrent0;
            }
            if(MotorCurrent1> MotorMAX[1])
            {
                MotorMAX[1]=MotorCurrent1;
            }
            if((max_limit_current[0]<0xFF00)&&(max_limit_current[0]<MotorMAX[0]))ControlValue[0]=0;
            if((max_limit_current[1]<0xFF00)&&(max_limit_current[0]<MotorMAX[1]))ControlValue[1]=0;
            collector_data_buff[0]   =  MotorCurrent0&0x00ff;         	//0#�����ǰ������λ
            collector_data_buff[1]   =  (MotorCurrent0&0xff00)>>8;		//0#�����ǰ������λ
            collector_data_buff[2]   =  MotorMAX[0]&0x00ff;				     //0#�����������λ
            collector_data_buff[3]   =  (MotorMAX[0]&0xff00)>>8;			//0#�����������λ
            collector_data_buff[4]   =  MotorCurrent1&0x00ff;         	//1#�����ǰ������λ
            collector_data_buff[5]   =  (MotorCurrent1&0xff00)>>8;		//1#�����ǰ������λ
            collector_data_buff[6]   =  MotorMAX[1]&0x00ff;				     //1#�����������λ
            collector_data_buff[7]   =  (MotorMAX[1]&0xff00)>>8;			//1#�����������λ
        }
        else //0Ϊ����������
        {
            TD_number=0;
            temp_adc=Get_Adclvbo(TD_number,0,1436); //1436=1305*1.1;0.2V��Ӧ262(0x0106)��1.0V��Ӧ1305(0x0519)
            if(First_adc_average[TD_number]==0)
            {
                temp_adc=First_Getaverage( TD_number, Maxlvbo_number, temp_adc);
                First_adc_average[TD_number]=1;
            }
            temp_adc=TD_Getaverage( TD_number,TDlvbo_number[TD_number], temp_adc,tdcycle_i[TD_number]);
            First_adc_average[TD_number]=1;
            tdcycle_i[TD_number]++;
            if(tdcycle_i[TD_number]>=TDlvbo_number[TD_number])
            {
                tdcycle_i[TD_number]=0;
            }
            MotorCurrent0 = temp_adc;
            if(MotorCurrent0<100) {
                MotorCurrent0=0;   //testʱȡ��
            }
            if(MotorCurrent0> MotorMAX[0]) {
                MotorMAX[0]=MotorCurrent0;
            }
            MotorCurrent1=MotorCurrent0;
            if(MotorCurrent1> MotorMAX[1]) {
                MotorMAX[1]=MotorCurrent1;
            }
            if(roller1_state==0)
            {
                collector_data_buff[0]   =  0;                              //0#�����ǰ���������λ
                collector_data_buff[1]   =  0;		                         //0#�����ǰ���������λ
                collector_data_buff[2]   =  0;				                    //0#���������������λ
                collector_data_buff[3]   =  0;			                     //0#���������������λ
                collector_data_buff[4]   =  0;         	                   //0#�����ǰ���������λ
                collector_data_buff[5]   =  0;		                        //0#�����ǰ���������λ
                collector_data_buff[6]   =  0;				                   //0#������������λ
                collector_data_buff[7]   =  0;			                    //0#�������������λ
            }
            if(roller1_state==1)
            {
                if((max_limit_current[0]<0xFF00)&&(max_limit_current[0]<MotorMAX[0]))
                {
                    ControlValue[0]=8;
                    ControlValue[1]=8;
                }
                collector_data_buff[0]   =  MotorCurrent0&0x00ff;         //0#�����ǰ���������λ
                collector_data_buff[1]   =  (MotorCurrent0&0xff00)>>8;		//0#�����ǰ���������λ
                collector_data_buff[2]   =  MotorMAX[0]&0x00ff;				   //0#���������������λ
                collector_data_buff[3]   =  (MotorMAX[0]&0xff00)>>8;			//0#���������������λ
                collector_data_buff[4]   =  0;         	                   //0#�����ǰ���������λ
                collector_data_buff[5]   =  0;		                        //0#�����ǰ���������λ
                collector_data_buff[6]   =  0;				                   //0#������������λ
                collector_data_buff[7]   =  0;			                    //0#�������������λ
            }
            if(roller1_state==2)
            {
                if((max_limit_current[1]<0xFF00)&&(max_limit_current[1]<MotorMAX[1]))
                {
                    ControlValue[0]=8;
                    ControlValue[1]=8;
                }
                collector_data_buff[0]   =  0;                             //0#�����ǰ���������λ
                collector_data_buff[1]   =  0;		                        //0#�����ǰ���������λ
                collector_data_buff[2]   =  0;				                   //0#���������������λ
                collector_data_buff[3]   =  0;			                    //0#���������������λ
                collector_data_buff[4]   =  MotorCurrent1&0x00ff;         	//0#�����ǰ���������λ
                collector_data_buff[5]   =  (MotorCurrent1&0xff00)>>8;		//0#�����ǰ���������λ
                collector_data_buff[6]   =  MotorMAX[1]&0x00ff;				     //0#�������������λ
                collector_data_buff[7]   =  (MotorMAX[1]&0xff00)>>8;			//0#�������������λ
            }
        }
        break;
//ADC_ConvertedValue[0]--PC0--IN10--I0--����ͨ��0,[1]--PB0--IN8--I2--����ͨ��1,[2]--PC1--IN11--I1--����ͨ��2,[3]--PB1--IN9--I3--����ͨ��3
    case 1:
        if(control_type[2]==0&&control_type[3]==0)//0Ϊ�������
        {
            TD_number=2;
            temp_adc=Get_Adclvbo(TD_number,262,1305); //(4~20ma)*50��;0.2V��Ӧ262��1.0V��Ӧ1305
            if(First_adc_average[TD_number]==0)
            {
                temp_adc=First_Getaverage( TD_number, Maxlvbo_number, temp_adc);
                First_adc_average[TD_number]=1;
            }
            temp_adc=TD_Getaverage( TD_number,TDlvbo_number[TD_number], temp_adc,tdcycle_i[TD_number]);
            First_adc_average[TD_number]=1;
            tdcycle_i[TD_number]++;
            if(tdcycle_i[TD_number]>=TDlvbo_number[TD_number])
            {
                tdcycle_i[TD_number]=0;
            }
            MotorCurrent2 = temp_adc;

            TD_number=3;
            temp_adc=Get_Adclvbo(TD_number,262,1305);
            if(First_adc_average[TD_number]==0)
            {
                temp_adc=First_Getaverage( TD_number, Maxlvbo_number, temp_adc);
                First_adc_average[TD_number]=1;
            }
            temp_adc=TD_Getaverage( TD_number,TDlvbo_number[TD_number], temp_adc,tdcycle_i[TD_number]);
            First_adc_average[TD_number]=1;
            tdcycle_i[TD_number]++;
            if(tdcycle_i[TD_number]>=TDlvbo_number[TD_number])
            {
                tdcycle_i[TD_number]=0;
            }
            MotorCurrent3 = temp_adc;//(4~20ma)*50��;0.2V��Ӧ262��1.0V��Ӧ1305

            if(MotorCurrent2> MotorMAX[2])
            {
                MotorMAX[2]=MotorCurrent2;
            }
            if(MotorCurrent3> MotorMAX[3])
            {
                MotorMAX[3]=MotorCurrent3;
            }
            if((max_limit_current[2]<0xFF00)&&(max_limit_current[2]<MotorMAX[2]))ControlValue[2]=0;
            if((max_limit_current[3]<0xFF00)&&(max_limit_current[3]<MotorMAX[3]))ControlValue[3]=0;
            collector_data_buff[8]   =  MotorCurrent2&0x00ff;         	//2#�����ǰ������λ
            collector_data_buff[9]   =  (MotorCurrent2&0xff00)>>8;		 //2#�����ǰ������λ
            collector_data_buff[10]   =  MotorMAX[2]&0x00ff;				    //2#�����������λ
            collector_data_buff[11]   =  (MotorMAX[2]&0xff00)>>8;			 //2#�����������λ
            collector_data_buff[12]   =  MotorCurrent3&0x00ff;         	//3#�����ǰ������λ
            collector_data_buff[13]   =  (MotorCurrent3&0xff00)>>8;		 //3#�����ǰ������λ
            collector_data_buff[14]   =  MotorMAX[3]&0x00ff;				    //3#�����������λ
            collector_data_buff[15]   =  (MotorMAX[3]&0xff00)>>8;			 //3#�����������λ
        }
        else //0Ϊ����������
        {
            TD_number=2;
            temp_adc=Get_Adclvbo(TD_number,0,1436); //1436=1305*1.1;0.2V��Ӧ262(0x0106)��1.0V��Ӧ1305(0x0519)
            if(First_adc_average[TD_number]==0)
            {
                temp_adc=First_Getaverage( TD_number, Maxlvbo_number, temp_adc);
                First_adc_average[TD_number]=1;
            }
            temp_adc=TD_Getaverage( TD_number,TDlvbo_number[TD_number], temp_adc,tdcycle_i[TD_number]);
            First_adc_average[TD_number]=1;
            tdcycle_i[TD_number]++;
            if(tdcycle_i[TD_number]>=TDlvbo_number[TD_number])
            {
                tdcycle_i[TD_number]=0;
            }
            MotorCurrent2 = temp_adc;//1436=1305*1.1;0.2V��Ӧ262(0x0106)��1.0V��Ӧ1305(0x0519)
            if(MotorCurrent2<100) {
                MotorCurrent2=0;   //testʱȡ��
            }
            if(MotorCurrent2> MotorMAX[2]) {
                MotorMAX[2]=MotorCurrent2;
            }
            MotorCurrent3=MotorCurrent2;
            if(MotorCurrent3> MotorMAX[3]) {
                MotorMAX[3]=MotorCurrent3;
            }
            if(roller2_state==0)
            {
                collector_data_buff[8]    =  0;                             //2#�����ǰ���������λ
                collector_data_buff[9]    =  0;		                         //2#�����ǰ���������λ
                collector_data_buff[10]   =  0;				                    //2#���������������λ
                collector_data_buff[11]   =  0;			                     //2#���������������λ
                collector_data_buff[12]   =  0;         	                 //2#�����ǰ���������λ
                collector_data_buff[13]   =  0;		                        //2#�����ǰ���������λ
                collector_data_buff[14]   =  0;				                   //2#������������λ
                collector_data_buff[15]   =  0;			                    //2#�������������λ
            }
            if(roller2_state==1)
            {
                if((max_limit_current[2]<0xFF00)&&(max_limit_current[2]<MotorMAX[2]))
                {
                    ControlValue[2]=8;
                    ControlValue[3]=8;
                }
                collector_data_buff[8]   =  MotorCurrent2&0x00ff;           //2#�����ǰ���������λ
                collector_data_buff[9]   =  (MotorCurrent2&0xff00)>>8;		  //2#�����ǰ���������λ
                collector_data_buff[10]   =  MotorMAX[2]&0x00ff;				     //2#���������������λ
                collector_data_buff[11]   =  (MotorMAX[2]&0xff00)>>8;			//2#���������������λ
                collector_data_buff[12]   =  0;         	                 //2#�����ǰ���������λ
                collector_data_buff[13]   =  0;		                        //2#�����ǰ���������λ
                collector_data_buff[14]   =  0;				                   //2#������������λ
                collector_data_buff[15]   =  0;			                    //2#�������������λ
            }
            if(roller2_state==2)
            {
                if((max_limit_current[3]<0xFF00)&&(max_limit_current[3]<MotorMAX[3]))
                {
                    ControlValue[2]=8;
                    ControlValue[3]=8;
                }
                collector_data_buff[8]   =   0;                             //2#�����ǰ���������λ
                collector_data_buff[9]   =   0;		                         //2#�����ǰ���������λ
                collector_data_buff[10]   =  0;				                    //2#���������������λ
                collector_data_buff[11]   =  0;			                     //2#���������������λ
                collector_data_buff[12]   =  MotorCurrent3&0x00ff;          	//2#�����ǰ���������λ
                collector_data_buff[13]   =  (MotorCurrent3&0xff00)>>8;		   //2#�����ǰ���������λ
                collector_data_buff[14]   =  MotorMAX[3]&0x00ff;				      //2#�������������λ
                collector_data_buff[15]   =  (MotorMAX[3]&0xff00)>>8;			   //2#�������������λ
            }
        }
        break;
    case 2:
        TD_param_num=0;
        if(cjkzslave_param_set[30]==254)//����RS485������
        {
            u8 i;
            u16 temp_value;
            for(i=0; i<cjkzslave_param_set[32]; i++)
            {
                if(cjkzslave_param_set[34]>0&&TD_param_num<15)
                {
                    collector_fertigation_data[TD_param_num++]=data_RS485[i][1];//����ˮ�ֵ�
                    collector_fertigation_data[TD_param_num++]=data_RS485[i][0];//����ˮ�ָ�
                }
                if(cjkzslave_param_set[34]>1&&cjkzslave_param_set[34]<=3&&TD_param_num<15)
                {
                    temp_value=400+(data_RS485[i][2]<<8|data_RS485[i][3]);//�¶����̣�-40~90�棻ת�����޷�������0~130���¶�=��temp_value-400)/10=temp_value/10-40
                    collector_fertigation_data[TD_param_num++]=temp_value&0x00ff;
                    collector_fertigation_data[TD_param_num++]=(temp_value&0xff00)>>8;
                }
                if(cjkzslave_param_set[34]>2&&cjkzslave_param_set[34]<=3&&TD_param_num<15)
                {
                    collector_fertigation_data[TD_param_num++]=data_RS485[i][5];//����EC��
                    collector_fertigation_data[TD_param_num++]=data_RS485[i][4];//����EC��
                }
            }
            break;
        }
        break;
    }
    if(ReadDataCNT1>=2) {
        ReadDataCNT1=0;
    }
    else {
        ReadDataCNT1++;
    }
}

static void slave_init_readflash(void)//��վ��ʼ����flash
{
    u8 init_flash_flg[2]= {0,0},i;

    Flash_Read(0x0801C000,  init_flash_flg, 2);//��վ�趨��վ�ɼ�������Ҫд��flash0x0801 C000����112k��ʼд�룬���16kΪ�����趨,ÿ��д�����1Kbyte
    if(init_flash_flg[0]!=0xFF||init_flash_flg[1]!=0xFF) //����Ѿ������������ã���������վ�ɼ������趨������ʹ�ó����ֵ
    {
        Flash_Read(0x0801C000,  cjkzslave_param_set, 36);
    }

    Flash_Read(0x0801C400,  init_flash_flg, 2);//��վ�趨��վ���Ʋ�����Ҫд��flash0x0801 C400����112k��ʼд�룬���16kΪ�����趨,ÿ��д�����1Kbyte
    if(init_flash_flg[0]!=0xFF||init_flash_flg[1]!=0xFF) //����Ѿ������������ã���������վ���Ʋ����趨������ʹ�ó����ֵ
    {
        Flash_Read(0x0801C400,  ctrlslave_param_set, 24);
    }
    for(i=0; i<24; i=i+6)
    {
        max_limit_current[i/6]=ctrlslave_param_set[i]|(ctrlslave_param_set[i+1]<<8);//max_limit_current[i]>0xFFF0 ��������max_limit_current[i]/65535*����
        start_run_time[i/6]=ctrlslave_param_set[i+2]|(ctrlslave_param_set[i+3]<<8);//start_run_time[i]=0x0000 ������������λ���룻65534/3600=18.2Сʱ
        start_run_nowtime[i/6]=start_run_time[i/6]*10;
        continue_run_time[i/6]=ctrlslave_param_set[i+4]|(ctrlslave_param_set[i+5]<<8);//continue_run_time[i]=0xFFFF,�������У������ƣ���λ���룻65534/3600=18.2Сʱ
        continue_run_nowtime[i/6]= continue_run_time[i/6]*10;
    }
    Flash_Read(0x0801C800,  slaveID_radioID, 2);//��ȡվ��ַ��433MHZ�ŵ�
    slave_ctrl_ID=slaveID_radioID[0];
    if(slaveID_radioID[1]==0xFF) {
        SI4463_Channel=0;
    }
    else {
        SI4463_Channel=slaveID_radioID[1];
    }
}


static void IO_ctrl_cmd(void)
{
    u8 i;
    for(i=0; i<=3; i++)
    {
        switch (ControlValue[i])//ControlValue[0]Ϊ0��ͨ������ֵ���Դ�����;����������Ҫ���趨�������������ʼ����ʱ�䡢��������ʱ��
        {
        case 0x0000://������ֱ�ӿ��������
            control_type[i]=0;//�������
            if(single_delay_flg[i]==0)
            {
                if(i==0) {
                    CTRL_CLOSE0;
                }
                if(i==1) {
                    CTRL_CLOSE1;
                }
                if(i==2) {
                    CTRL_CLOSE2;
                }
                if(i==3) {
                    CTRL_CLOSE3;
                }
                if(single_old_ControlValue[i]!=0)
                {
                    single_delay_time[i]=30;//100msѭ��һ�Σ�30��ѭ������3S
                    single_old_ControlValue[i]=0;
                }
                single_delay_flg[i]=1;
            }
            else if(single_delay_time[i]>=1) {
                single_delay_time[i]--;   //��ʱ�䣨3�뼰���ϣ����ֹͣ���У����������������
            }
            break;
        case 0x0001://������ֱ�ӿ��������
            control_type[i]=0;//�������
            if(single_delay_time[i]==0)
            {
                if(start_run_nowtime[i]>=1) {
                    start_run_nowtime[i]--;
                }
                if(start_run_nowtime[i]==0)//��������ʱ������������������ʱ��������ز���ʱ
                {
                    if(i==0) {
                        CTRL_OPEN0;
                    }
                    if(i==1) {
                        CTRL_OPEN1;
                    }
                    if(i==2) {
                        CTRL_OPEN2;
                    }
                    if(i==3) {
                        CTRL_OPEN3;
                    }
                    if(continue_run_nowtime[i]>=1&&continue_run_nowtime[i]<=0xFF00*10) {
                        continue_run_nowtime[i]--;
                    }
                    if(continue_run_nowtime[i]==0)
                    {
                        ControlValue[i]=0x0000;
                        continue_run_nowtime[i]=continue_run_time[i]*10;
                        start_run_nowtime[i]=start_run_time[i]*10;
                    }
                }
                if(single_old_ControlValue[i]!=1) {
                    single_old_ControlValue[i]=1;
                    MotorMAX[i]=0;
                    First_adc_average[i]=0;
                }
                single_delay_flg[i]=0;
            }
            if(single_delay_time[i]>=1) {
                single_delay_time[i]--;
            }
            break;

        case 0x0002://����ת�����������ת
            control_type[i]=1;//�����������,0��ʾ������ƣ�1��ʾ�������ƣ����յ��������趨������������¼�ʹ�ã������������ַ�ʽ�ϱ�������С
            if(i==0&&bidirection_location_flg[i]!=2)//bidirection_location_flg[i]=2��ʾ��ת�Ѿ���ȫ��λ����������ת�ˡ�
            {
                if(batch_ctrl_finish[0]==0)
                {
                    if(start_run_nowtime[i+1]==0)//��תʹ��ͨ��1�Ĳ�����Ϊ��ת���Ʋ���;batch_ctrl_finish[0]=0��1#��ת��ɣ�;batch_ctrl_finish[1]=0��1#��ת���
                    {
                        ROLLER1_DOWN;
                        batch_ctrl_finish[1]=1;
                        bidirection_location_flg[i]=0;
                        if(roller1_state!=2) {
                            MotorMAX[1]=0;
                            First_adc_average[1]=0;
                        }
                        roller1_state=2;//0��ʾͣ��1��ʾ���У�2��ʾ����
                        if(continue_run_nowtime[i+1]>=1&&continue_run_nowtime[i+1]<=0xFF00*10) {
                            continue_run_nowtime[i+1]--;
                        }
                        if(continue_run_nowtime[i+1]==0)
                        {
                            batch_ctrl_finish[1]=0;
                            bidirection_location_flg[i]=2;
                            continue_run_nowtime[i+1]=continue_run_time[i+1]*10;
                            start_run_nowtime[i+1]=start_run_time[i+1]*10;
                            ROLLER1_STOP;
                            roller1_state=0;//0��ʾͣ��1��ʾ���У�2��ʾ����
                        }
                    }
                    else if(start_run_nowtime[i+1]>=1) {
                        start_run_nowtime[i+1]--;
                    }
                }
                else
                {
                    batch_ctrl_finish[0]=0;
                    bidirection_location_flg[i]=1;
                    continue_run_nowtime[i]=continue_run_time[i]*10;
                    start_run_nowtime[i]=start_run_time[i]*10;
                    ROLLER1_STOP;
                    roller1_state=0;//0��ʾͣ��1��ʾ���У�2��ʾ����
                }
            }

            if(i==2&&bidirection_location_flg[i]!=2)//bidirection_location_flg[i]=2��ʾ��ת�Ѿ���ȫ��λ����������ת�ˡ�batch_ctrl_finish[2]=0��2#��ת��ɣ�;batch_ctrl_finish[3]=0��2#��ת���
            {
                if(batch_ctrl_finish[2]==0)
                {
                    if(start_run_nowtime[i+1]==0)//��תʹ��ͨ��3�Ĳ�����Ϊ��ת���Ʋ���
                    {
                        ROLLER2_DOWN;
                        batch_ctrl_finish[3]=1;
                        bidirection_location_flg[i]=0;
                        if(roller2_state!=2) {
                            MotorMAX[3]=0;
                            First_adc_average[3]=0;
                        }
                        roller2_state=2;//0��ʾͣ��1��ʾ���У�2��ʾ����
                        if(continue_run_nowtime[i+1]>=1&&continue_run_nowtime[i+1]<=0xFF00*10) {
                            continue_run_nowtime[i+1]--;
                        }
                        if(continue_run_nowtime[i+1]==0)
                        {
                            batch_ctrl_finish[3]=0;
                            bidirection_location_flg[i]=2;
                            continue_run_nowtime[i+1]=continue_run_time[i+1]*10;
                            start_run_nowtime[i+1]=start_run_time[i+1]*10;
                            ROLLER2_STOP;
                            roller2_state=0;//0��ʾͣ��1��ʾ���У�2��ʾ����,�ڵ���������ж����л����еļ�����������
                        }
                    }
                    else if(start_run_nowtime[i+1]>=1) {
                        start_run_nowtime[i+1]--;
                    }
                }
                else
                {
                    batch_ctrl_finish[2]=0;
                    bidirection_location_flg[i]=1;
                    continue_run_nowtime[i]=continue_run_time[i]*10;
                    start_run_nowtime[i]=start_run_time[i]*10;
                    ROLLER2_STOP;
                    roller2_state=0;//0��ʾͣ��1��ʾ���У�2��ʾ����
                }
            }

            i++;//ÿ����ͨ������1������ת���
            break;

        case 0x0003://����ת��������������ת
            control_type[i]=1;//�����������
            if(i==0&&bidirection_location_flg[i]!=1)//bidirection_location_flg[i]=1��ʾ��ת�Ѿ���ȫ��λ����������ת�ˡ�
            {
                if(batch_ctrl_finish[1]==0)
                {
                    if(start_run_nowtime[i]==0)//��תʹ��ͨ��0�Ĳ�����Ϊ��ת���Ʋ���
                    {
                        ROLLER1_UP;
                        batch_ctrl_finish[0]=1;
                        bidirection_location_flg[i]=0;
                        if(roller1_state!=1) {
                            MotorMAX[0]=0;
                            First_adc_average[0]=0;
                        }
                        roller1_state=1;//0��ʾͣ��1��ʾ���У�2��ʾ����
                        if(continue_run_nowtime[i]>=1&&continue_run_nowtime[i]<=0xFF00*10) {
                            continue_run_nowtime[i]--;
                        }
                        if(continue_run_nowtime[i]==0)
                        {
                            batch_ctrl_finish[0]=0;
                            bidirection_location_flg[i]=1;
                            continue_run_nowtime[i]=continue_run_time[i]*10;
                            start_run_nowtime[i]=start_run_time[i]*10;
                            ROLLER1_STOP;
                            roller1_state=0;//0��ʾͣ��1��ʾ���У�2��ʾ����
                        }
                    }
                    else if(start_run_nowtime[i]>=1) {
                        start_run_nowtime[i]--;
                    }
                }
                else
                {
                    batch_ctrl_finish[1]=0;
                    bidirection_location_flg[i]=2;
                    continue_run_nowtime[i+1]=continue_run_time[i+1]*10;
                    start_run_nowtime[i+1]=start_run_time[i+1]*10;
                    ROLLER1_STOP;
                    roller1_state=0;//0��ʾͣ��1��ʾ���У�2��ʾ����
                }
            }

            if(i==2&&bidirection_location_flg[i]!=1)//bidirection_location_flg[i]=1��ʾ��ת�Ѿ���ȫ��λ����������ת�ˡ�
            {
                if(batch_ctrl_finish[3]==0)
                {
                    if(start_run_nowtime[i]==0)//��תʹ��ͨ��2�Ĳ�����Ϊ��ת���Ʋ���
                    {
                        ROLLER2_UP;
                        batch_ctrl_finish[2]=1;
                        bidirection_location_flg[i]=0;
                        if(roller2_state!=1) {
                            MotorMAX[2]=0;
                            First_adc_average[2]=0;
                        }
                        roller2_state=1;//0��ʾͣ��1��ʾ���У�2��ʾ����
                        if(continue_run_nowtime[i]>=1&&continue_run_nowtime[i]<=0xFF00*10) {
                            continue_run_nowtime[i]--;
                        }
                        if(continue_run_nowtime[i]==0)
                        {
                            batch_ctrl_finish[2]=0;
                            bidirection_location_flg[i]=1;
                            continue_run_nowtime[i]=continue_run_time[i]*10;
                            start_run_nowtime[i]=start_run_time[i]*10;
                            ROLLER2_STOP;
                            roller2_state=0;//0��ʾͣ��1��ʾ���У�2��ʾ����
                        }
                    }
                    else if(start_run_nowtime[i]>=1) {
                        start_run_nowtime[i]--;
                    }
                }
                else
                {
                    batch_ctrl_finish[3]=0;
                    bidirection_location_flg[i]=2;
                    continue_run_nowtime[i+1]=continue_run_time[i+1]*10;
                    start_run_nowtime[i+1]=start_run_time[i+1]*10;
                    ROLLER2_STOP;
                    roller2_state=0;//0��ʾͣ��1��ʾ���У�2��ʾ����,�ڵ���������ж����л����еļ�����������
                }
            }

            i++;
            break;
        case 0x0004://�������������������
            control_type[i]=0;//�������
            if(i==0) {
                CTRL_CLOSE0;
            }
            if(i==1) {
                CTRL_CLOSE1;
            }
            if(i==2) {
                CTRL_CLOSE2;
            }
            if(i==3) {
                CTRL_CLOSE3;
            }
            break;
        case 0x0005://�������������������
            control_type[i]=0;//�������
            if(start_run_nowtime[i]>=1) {
                start_run_nowtime[i]--;
                MotorMAX[i]=0;
                First_adc_average[i]=0;
            }
            if(start_run_nowtime[i]==0)//��������ʱ������������������ʱ��������ز���ʱ
            {
                if(i==0) {
                    CTRL_OPEN0;
                }
                if(i==1) {
                    CTRL_OPEN1;
                }
                if(i==2) {
                    CTRL_OPEN2;
                }
                if(i==3) {
                    CTRL_OPEN3;
                }
                if(continue_run_nowtime[i]>=1&&continue_run_nowtime[i]<=0xFF00*10) {
                    continue_run_nowtime[i]--;
                }
                if(continue_run_nowtime[i]==0)
                {
                    ControlValue[i]=0x0004;
                    continue_run_nowtime[i]=continue_run_time[i]*10;
                    start_run_nowtime[i]=start_run_time[i]*10;
                }
            }
            break;
        case 0x0006://��ʱδ����
            break;
        case 0x0007://��ʱδ����
            break;
        case 0x0008://����תֱ�ӿ������ͣ
            control_type[i]=1;//�����������
            if(bidirection_delay_flg[i]==0)
            {
                if(i==0) {
                    ROLLER1_STOP;    //0��ʾͣ��1��ʾ���У�2��ʾ����
                    roller1_state=0;
                }
                if(i==2) {
                    ROLLER2_STOP;    //0��ʾͣ��1��ʾ���У�2��ʾ����
                    roller2_state=0;
                }

                if(ControlValue[i]!=bidirection_old_ControlValue[i])
                {
                    bidirection_delay_time[i]=30;
                    bidirection_old_ControlValue[i]=ControlValue[i];
                }
                bidirection_delay_flg[i]=1;
                bidirection_run_flg[i]=0;
                bidirection_run_time[i]=0;
            }
            else if(bidirection_delay_time[i]>=1) {
                bidirection_delay_time[i]--;   //��ʱ�䣨3�뼰���ϣ����ֹͣ���У����������������
            }
            i++;
            break;
        case 0x0009://����תֱ�ӿ��������ת
            control_type[i]=1;//�����������
            if(bidirection_old_ControlValue[i]==11)//�ӷ�תֱ��ת����ת����Ҫ��ʱ5S
            {
                if(i==0) {
                    ROLLER1_STOP;    //0��ʾͣ��1��ʾ���У�2��ʾ����
                    roller1_state=0;
                }
                if(i==2) {
                    ROLLER2_STOP;    //0��ʾͣ��1��ʾ���У�2��ʾ����
                    roller2_state=0;
                }
                bidirection_delay_time[i]=50;
                bidirection_old_ControlValue[i]=9;
            }
            if(bidirection_delay_time[i]==0&&(bidirection_run_time[i]==0||bidirection_run_flg[i]==2))
            {
                if(i==0&&bidirection_location_flg[i]!=1)//bidirection_location_flg[i]��ʾλ���źţ�ֻ����i=0��2������i=1��3���������״̬��λ�ã�0��8��ʾͣ��1��ʾ��תͣ��λ�ã�2��ʾ��תͣ��λ��
                {
                    ROLLER1_UP;
                    if(bidirection_old_ControlValue[i]!=9) {
                        bidirection_old_ControlValue[i]=9;
                    }
                    if(roller1_state!=1) {
                        MotorMAX[0]=0;
                        First_adc_average[0]=0;
                    }
                    roller1_state=1;//0��ʾͣ��1��ʾ���У�2��ʾ����

                    if(continue_run_nowtime[i]>=1&&continue_run_nowtime[i]<=0xFF00*10) {
                        continue_run_nowtime[i]--;
                    }
                    if(continue_run_nowtime[i]==0)
                    {
                        ROLLER1_STOP;
                        roller1_state=0;//0��ʾͣ��1��ʾ���У�2��ʾ����
                        bidirection_location_flg[i]=1;
                        continue_run_nowtime[i]=continue_run_time[i]*10;
                    }
                }

                if(i==2&&bidirection_location_flg[i]!=1)
                {
                    ROLLER2_UP;
                    if(bidirection_old_ControlValue[i]!=9) {
                        bidirection_old_ControlValue[i]=9;
                    }
                    if(roller2_state!=1) {
                        MotorMAX[2]=0;
                        First_adc_average[2]=0;
                    }
                    roller2_state=1;//0��ʾͣ��1��ʾ���У�2��ʾ����

                    if(continue_run_nowtime[i]>=1&&continue_run_nowtime[i]<=0xFF00*10) {
                        continue_run_nowtime[i]--;
                    }
                    if(continue_run_nowtime[i]==0)
                    {
                        ROLLER2_STOP;
                        roller2_state=0;//0��ʾͣ��1��ʾ���У�2��ʾ����
                        bidirection_location_flg[i]=1;
                        continue_run_nowtime[i]=continue_run_time[i]*10;
                    }
                }

                bidirection_delay_flg[i]=0;
                bidirection_run_flg[i]=2;
                bidirection_run_time[i]=30;
            }
            if(bidirection_delay_time[i]>=1) {
                bidirection_delay_time[i]--;
            }
            if(bidirection_run_flg[i]!=2&&bidirection_run_time[i]>=1) {
                bidirection_run_time[i]--;
            }

            i++;
            break;
        case 0x000B://����תֱ�ӿ��������ת
            control_type[i]=1;//�����������
            if(bidirection_old_ControlValue[i]==9)//����תֱ��ת��ת����Ҫ��ʱ5S
            {
                if(i==0) {
                    ROLLER1_STOP;    //0��ʾͣ��1��ʾ���У�2��ʾ����
                    roller1_state=0;
                }
                if(i==2) {
                    ROLLER2_STOP;    //0��ʾͣ��1��ʾ���У�2��ʾ����
                    roller2_state=0;
                }
                bidirection_delay_time[i]=50;
                bidirection_old_ControlValue[i]=11;
            }
            if(bidirection_delay_time[i]==0&&(bidirection_run_time[i]==0||bidirection_run_flg[i]==1))
            {

                if(i==0&&bidirection_location_flg[i]!=2)
                {
                    ROLLER1_DOWN;
                    if(bidirection_old_ControlValue[i]!=11) {
                        bidirection_old_ControlValue[i]=11;
                    }
                    if(roller1_state!=2) {
                        MotorMAX[1]=0;
                        First_adc_average[1]=0;
                    }
                    roller1_state=2;//0��ʾͣ��1��ʾ���У�2��ʾ����

                    if(continue_run_nowtime[i+1]>=1&&continue_run_nowtime[i+1]<=0xFF00*10) {
                        continue_run_nowtime[i+1]--;
                    }
                    if(continue_run_nowtime[i+1]==0)
                    {
                        ROLLER1_STOP;
                        roller1_state=0;//0��ʾͣ��1��ʾ���У�2��ʾ����
                        bidirection_location_flg[i]=2;
                        continue_run_nowtime[i+1]=continue_run_time[i+1]*10;
                    }
                }

                if(i==2&&bidirection_location_flg[i]!=2)//bidirection_location_flg[i]��ʾλ���źţ�ֻ����i=0��2������i=0��2
                {
                    ROLLER2_DOWN;
                    if(bidirection_old_ControlValue[i]!=11) {
                        bidirection_old_ControlValue[i]=11;
                    }
                    if(roller2_state!=2) {
                        MotorMAX[3]=0;
                        First_adc_average[3]=0;
                    }
                    roller2_state=2;//0��ʾͣ��1��ʾ���У�2��ʾ����

                    if(continue_run_nowtime[i+1]>=1&&continue_run_nowtime[i+1]<=0xFF00*10) {
                        continue_run_nowtime[i+1]--;   //��ת����ͨ��1��ͨ��3���Ʋ���
                    }
                    if(continue_run_nowtime[i+1]==0)
                    {
                        ROLLER2_STOP;
                        roller2_state=0;//0��ʾͣ��1��ʾ���У�2��ʾ����
                        bidirection_location_flg[i]=2;
                        continue_run_nowtime[i+1]=continue_run_time[i+1]*10;
                    }
                }

                bidirection_delay_flg[i]=0;
                bidirection_run_flg[i]=1;
                bidirection_run_time[i]=30;
            }
            if(bidirection_delay_time[i]>=1) {
                bidirection_delay_time[i]--;
            }
            if(bidirection_run_flg[i]!=1&&bidirection_run_time[i]>=1) {
                bidirection_run_time[i]--;
            }

            i++;
            break;
        default:
            break;

        }
    }
}
/*��ƽ������*/
u16 Get_Adclvbo(u8 TD_Xnumber,u16 TD_xiaxian,u16 TD_shangxian)
{
    u8 i,j;
    u16 AdcLvbo[100],Temp_adc,t;
    u32 Temp_adcto=0;
    for(i=0; i<100; i++)
    {
        Temp_adc=ADC_ConvertedValue[TD_Xnumber];
        if(Temp_adc<TD_xiaxian) {
            Temp_adc=TD_xiaxian;   //����
        }
        if(Temp_adc>TD_shangxian) {
            Temp_adc=TD_shangxian;   //����
        }
        AdcLvbo[i]= Temp_adc;
    }

    for(i=0; i<100; i++) //100��������
    {
        for(j=i+1; j<100; j++)
        {
            if(AdcLvbo[i]>=AdcLvbo[j])
            {
                t=AdcLvbo[i];
                AdcLvbo[i]=AdcLvbo[j];
                AdcLvbo[j]=t;
            }
        }
    }
    for(i=40; i<60; i++)
    {
        Temp_adcto=Temp_adcto+AdcLvbo[i];
    }
    Temp_adc=Temp_adcto/20;
    return Temp_adc;
}

u16 First_Getaverage(u8 td_xnumber,u8 maxlvbo_xnumber,u16 temp_adc)
{   u8 i;
    for(i=0; i<maxlvbo_xnumber; i++)
    {
        Adc_average[td_xnumber][i] =temp_adc;

    }
    return temp_adc;
}

u16 TD_Getaverage(u8 td_xnumber,u8 tdlvbo_xnumber,u16 temp_xadc,u8 tdcycle_xi)
{   u8 i;
    u32 average_adcto=0;
    Adc_average[td_xnumber][tdcycle_xi] =temp_xadc;
    for(i=0; i<tdlvbo_xnumber; i++)
    {
        average_adcto=average_adcto+Adc_average[td_xnumber][i];  //���
    }
    temp_xadc=average_adcto/tdlvbo_xnumber;  //��ƽ��ֵ
    return temp_xadc;
}

static void usart3_send_cmd(void)//�ɼ����򴫸��������ݲ�ѯ����
{
    switch(USART3_send_sequence)
    {
    case 0x00:
        if(cjkzslave_param_set[30]>=251&&cjkzslave_param_set[30]<=254)//������������ѯ
        {
            u8 bytelen3;
            bytelen3=ReadData(switch_cmd_TR485_addr,READ_HOLDING_REGISTER,0x0000,cjkzslave_param_set[34],ReportData3);
            WriteDataToBuffer(3,(u8 *)ReportData3,0,bytelen3);
            switch_cmd_RS485_CNT++;
            if(switch_cmd_RS485_CNT<cjkzslave_param_set[32])//RS485����ˮ��+�¶�+EC(����������վ��ַ��0xFE��ʼ���ݼ���ַ)
            {
                switch_cmd_TR485_addr--;
            }
            else
            {
                switch_cmd_TR485_addr=0xFE;
                switch_cmd_RS485_CNT=0;
            }
            coll_ctrl_zz3_type=1;
        }
        break;
    }
    USART3_send_sequence++;
    if(USART3_send_sequence>=0x01) {
        USART3_send_sequence=0;
    }
}

static void RxReport3(u8 len,u8 *pBuf)//USART3�յ�����������
{
    if(GetCRC16(pBuf,len)==0)
    {
        if(pBuf[2]<=16&&coll_ctrl_zz3_type==1)
        {
            u8 temp_I;
            temp_I=0xFE-pBuf[0];//վ��ַת��Ϊ�����±�;վ��ַ��0xFE;0xFD;...;���8��
            if(temp_I<=7)
            {
                memcpy(data_RS485[temp_I],pBuf+3,pBuf[2]);
            }
            coll_ctrl_zz3_type=0;
            return;
        }
    }
    memcpy(collector_fertigation_data,data_RS485,16);
}
static u8 ReadData(u8 Slave_ID,u8 function_code,u16 addr,u16 num,u8 *temp)
{
    u16 rd_CRC_Val;
    temp[0] = Slave_ID;
    temp[1] = function_code;
    temp[2] = (addr&0xFF00)>>8;
    temp[3] = addr&0x00FF;
    temp[4] = (num&0xFF00)>>8;
    temp[5] = num&0x00FF;
    rd_CRC_Val = GetCRC16(temp,6);
    temp[6] = rd_CRC_Val&0x00FF;
    temp[7] = (rd_CRC_Val&0xFF00)>>8;
    return 8;
}







