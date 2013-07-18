#include<STC_NEW_8051.h>
#include<string.h>
#include<intrins.h>
#include<UART2.h>
#include<UART1.h>
#include<Delay_ms.h>
#include<Init_Timer0.h>
#include<unicode.h>
//#include "bmp_pixel.h"
//#include "nokia_5110.h"
//#include <stdio.h>

#define Buf1_Max 200 //500 					//串口1缓存长度
#define Buf2_Max 200					//串口2缓存长度

sbit Yellow_LED = P0^4;					//LED1黄色指示灯
sbit Green_LED  = P0^5;					//LED2绿色指示灯
sbit Red_LED    = P0^6;					//LED3红色指示灯
sbit IGT 		= P2^5;  				//启动TC35I信号
sbit GSM_ON		= P3^4;

unsigned char Buf_First_Serial[Buf1_Max];
unsigned char runxun_ser1 = 0,Flag_Buf1_Rec = 0;
unsigned char Buf_Second_Serial[Buf2_Max];
unsigned int  Start_weizi,End_weizi;
unsigned int  First_Int = 0,Second_Int = 0;
unsigned int  First_Set=0, Second_Set=0;
unsigned char Timer0_Start = 0;
unsigned int  Times = 0,shijian = 0,Count_time = 0;
unsigned char Flag_Bug = 0;
unsigned char CSCA_Num[14]={'0','0','0','0','0','0','0','0','0','0','0','0','0','F'};
#define MOBILE_NUM "AT+CMGS=\"+8613923784483\""
char code str1[]="0891";
char code str2[]="11000D9168";
char code str3[]="000800";

/*********************************发送换行回车函数*******************************/

void Send_LR(void)
{
//	Send_Hex(0x0D);	
//	Send_Hex(0x0A);	
	Second_Serial_Port_Send(0x0D);
	Second_Serial_Port_Send(0x0A);
}

/*****************************清除串口1缓存数据函数******************************/

void CLR_Buf1(void)
{
	unsigned int k;
    for(k=0;k<Buf1_Max;k++)      //将缓存内容清零
    {
		Buf_First_Serial[k] = 0x30;
	}
    First_Int = 0;              //接收字符串的起始存储位置
    First_Set = 0;//read buf1 index
}

void CLR_Buf2(void)
{
	unsigned int k;
    for(k=0;k<Buf2_Max;k++)      //将缓存内容清零
    {
		Buf_Second_Serial[k] = 0x30;
	}
    First_Int = 0;              //接收字符串的起始存储位置
}
/**********************判断缓存中是否含有指定的字符串函数************************/

unsigned char Hand(unsigned char *a)
{ 
    if(strstr(Buf_First_Serial,a)!=NULL)
	    return 1;
	else
		return 0;
}

/*******************************定位字串"+32"符位置******************************/
void Find_dw_TEXT(void)
{
	unsigned int k;
	for(k=0;k<Buf1_Max;k++)
	{
		if(Buf_First_Serial[k]=='+'&&Buf_First_Serial[k+1]=='3'&&Buf_First_Serial[k+2]=='2')
		{
			Start_weizi = k+6;	
			break;				
		}
	}
	for(k=0;k<Buf1_Max;k++)
	{
		if(Buf_First_Serial[k]=='O'&&Buf_First_Serial[k+1]=='K')
		{
			End_weizi = k-5;	
			break;				
		}
	}
}

/*****************************定位字串"089168"符位置*****************************/

unsigned char Find_dw_PDU(void)
{
	unsigned int k;
	for(k=0;k<Buf1_Max;k++)
	{
		if(Buf_First_Serial[k]=='0'&&Buf_First_Serial[k+1]=='8'&&Buf_First_Serial[k+2]=='9')
		if(Buf_First_Serial[k+3]=='1'&&Buf_First_Serial[k+4]=='6'&&Buf_First_Serial[k+5]=='8')	
		{
			Start_weizi = k+58;	
			break;				
		}
	}
	for(k=0;k<Buf1_Max;k++)
	{
		if(Buf_First_Serial[k]=='O'&&Buf_First_Serial[k+1]=='K')
		{
			End_weizi = k-5;	
			break;				
		}
	}
	if(Buf_First_Serial[Start_weizi-17]=='8')			//表示为中文信息
		return 0;
	else												//否则为英文信息	
		return 1;
}


//*******************************************************************

 void LED_Flash()
 {
  unsigned char i;
  for(i=0;i<10;i++)
   {
	Yellow_LED = 0;					//LED1黄色指示灯
	Green_LED  = 0;					//LED2绿色指示灯
	Red_LED    = 0;
	Delay_ms(500);
	Yellow_LED = 1;					//LED1黄色指示灯
	Green_LED  = 1;					//LED2绿色指示灯
	Red_LED    = 1;
	Delay_ms(500);
	}
 }
/*--------- add zigbee application begin ------*/
// use UART 1 communicate with zigbee 2530
typedef unsigned char BYTE;
typedef unsigned short  UINT;//use 16bit,

//z-stack define
#define SOF   0xFE
#define Z_APP_MSG_S_SEND    0x2900 //sync cmd send
#define Z_APP_MSG_A_SEND    0x6900 //async cmd send
#define Z_APP_MSG_S_ACK     0x6980 //sync cmd 's respond

//zzu define
#define SEP   0x02
//ZZU cmd id
#define ZZU_READ        0x0001
#define ZZU_READ_ACK    0x8001
#define ZZU_WRITE       0x0002
#define ZZU_WRITE_ACK   0x8002
#define ZZU_REPORT      0x0003
//zzu commond device param list
#define ZZU_CMD_SW      0x0001
#define ZZU_CMD_HW      0x0002
#define ZZU_CMD_DOC     0x0003
#define ZZU_CMD_TIME    0x0004
#define ZZU_CMD_TYPE    0x0005
#define ZZU_CMD_NET_ID  0x0011
#define ZZU_CMD_NET_ADDR  0x0012
#define ZZU_CMD_CHANNEL   0x0013
#define ZZU_CMD_MAC     0x0014
#define ZZU_CMD_NODE    0x0015 // child or neighor
//zzu device 
#define ZZU_COORDINATOR 0x00
#define ZZU_COOR_ADDR   0x0000
#define ZZU_TEMPER      0x01//temperature
#define ZZU_LIGHT       0x02
#define ZZU_SWITCH      0x03
#define ZZU_BODY        0x04
#define ZZU_CO2         0x05
#define ZZU_DISTANCE    0x06
#define ZZU_HUMIDITY    0x07//humidity
#define ZZU_RFID        0x08
#define ZZU_FINGER      0x09
//#define ZZU_       0x0A
#define ZZU_ACCELER     0x0B//acceleration

#define MSG_LEN_OFFSET  1
#define Z_CMD_H_OFFSET  2
#define Z_CMD_L_OFFSET  3
#define ADDR_H_OFFSET   4
#define ADDR_L_OFFSET   5
#define ZZU_CMD_H_OFFSET   6
#define ZZU_CMD_L_OFFSET   7

struct NODE{ 
    BYTE type;  //0005 --00:no use,first node is coordinate
    BYTE addrh;
    BYTE addrl;
    BYTE child_num;//0015
    /*
    BYTE sw[2];//0001
    BYTE hw[2]; //0002
    BYTE doc[2]; //0003
    BYTE time[6]; //0004
    BYTE net_id[2];//0011
    BYTE net_addr[2];//0012
    BYTE channel[4];//0013
    BYTE mac[8]; //0014 
    */
};
#define MAX_NODE 4
struct NODE device[MAX_NODE];// node[0]is coordinator
//for simple 0:coor;1:temp;2:hum;3:body
BYTE temp_count = 0;
BYTE body_count = 0;
BYTE hum_count = 0;
#define ALARM_LEVEL 5


#define WAIT_TIME   20000//10000 for work
//#define ZZU_BUF_MAX     50//Buf1_Max/4 //no check too long

#define COOR_DETAIL_NUM 6 //14
code BYTE coor_detail[COOR_DETAIL_NUM]={
    /*addr*/
    0x00,0x00,
    /*cmd read*/
    0x00,0x01,
    /*read param*/
    /*0x00,0x01, 0x00,0x02, 0x00,0x05, 0x00,0x14,*/ 0x00,0x15
};

//#define ZZU_TEMPER      0x01//temperature
#define TEMPER_VALUE    0x0101
#define TEMPER_MODE     0x0102
#define TEMP_ALARM_VALUE 0x10
/*#define TEMPER_DETAIL_NUM 6
code BYTE temper_detail[TEMPER_DETAIL_NUM]={
    0x00,0x01,//read cmd
    0x01,0x01,//temperature value 
    0x01,0x02//work mode: 0 quiet
};
#define TEMPER_QUIET_NUM 5
code BYTE temper_stop[TEMPER_QUIET_NUM]={
    0x00,0x02, //write com
    0x01,0x02,0x00//work mode: 0 quiet
};*/

//#define ZZU_BODY        0x04
#define BODY_MODE   0x0401
#define BODY_VALUE  0x0402
#define BODY_ALARM_VALUE 1
/*#define BODY_DETAIL_NUM 6
code BYTE body_detail[BODY_DETAIL_NUM]={
    0x00,0x01,
    0x04,0x01,//0:stop; 1:work report 
    0x04,0x02//0: nobody; 1: have some one
};
#define BODY_QUIET_NUM 5
code BYTE body_stop[BODY_QUIET_NUM]={
    0x00,0x02,
    0x04,0x01,0x00//work mode: 0 quiet
};*/

//#define ZZU_HUMIDITY    0x07//humidity
#define HUM_MODE   0x0701
#define HUM_VALUE  0x0702
#define HUM_ALARM_VALUE 100
/*#define HUM_DETAIL_NUM 6
code BYTE hum_detail[HUM_DETAIL_NUM]={
    0x00,0x01,
    0x07,0x01,//0:stop; 1:work report 
    0x07,0x02//value
};
#define HUM_QUIET_NUM 5
code BYTE hum_stop[HUM_QUIET_NUM]={
    0x00,0x02,
    0x07,0x01,0x00//work mode: 0 quiet
};*/

void Debug(BYTE value)
{
    Send_Hex(0xff);
    Send_Hex(value);
    Send_Hex(0xff);
}
/*
void Send_Short_Msg()
{
    Second_Serial_Send_ASCII(MOBILE_NUM);
	//Second_Serial_Send_ASCII("AT+CMGS=\"+8613078047847\"");
	Send_LR();
	Delay_ms(1000);
	Second_Serial_Send_ASCII("HELLO ,qin!");
	Delay_ms(300);
	Second_Serial_Port_Send(0x1A);
	Send_LR();
}
*/
void Temp_Alarm(BYTE value)
{
    BYTE buf[4]={0,0,0,0};
  unsigned char i=0;
  for(i=0;i<5;i++)
   {
	Yellow_LED = 0;					//LED1黄色指示灯
	Delay_ms(500);
	Yellow_LED = 1;					//LED1黄色指示灯
	Delay_ms(500);
	}
    
  	//Debug(value);
    if(++temp_count > ALARM_LEVEL)
    {
        temp_count = 0;
		
        Second_Serial_Send_ASCII(MOBILE_NUM);
    	//Second_Serial_Send_ASCII("AT+CMGS=\"+8613078047847\"");
    	Send_LR();
    	Delay_ms(1000);
    	Second_Serial_Send_ASCII("Alarm! temperature now :");
//      Debug(value);
        buf[0] = value / 100 + 0x30;
        buf[1] = (value%100) / 10 + 0x30;
        buf[2] = value % 10+ 0x30;
        buf[3] = '\0';
        Second_Serial_Send_ASCII(buf);
    	Delay_ms(300);
    	Second_Serial_Port_Send(0x1A);
    	Send_LR();
		 
       for(i=0;i<10;i++)
       {
    	Yellow_LED = 0;					//LED1黄色指示灯
    	Delay_ms(500);
    	Yellow_LED = 1;					//LED1黄色指示灯
    	Delay_ms(500);
    	}
    }
}

void Body_Alarm()
{
  unsigned char i=0;
  for(i=0;i<3;i++)
   {
	Red_LED = 0;					//LED1黄色指示灯
	Delay_ms(500);
	Red_LED = 1;					//LED1黄色指示灯
	Delay_ms(500);
	}
    
    if(++body_count > ALARM_LEVEL)
    {
        body_count = 0;
        
        Second_Serial_Send_ASCII(MOBILE_NUM);
    	//Second_Serial_Send_ASCII("AT+CMGS=\"+8613078047847\"");
    	Send_LR();
    	Delay_ms(1000);
    	Second_Serial_Send_ASCII("Body Sensor Alarm");
    	Delay_ms(300);
    	Second_Serial_Port_Send(0x1A);
    	Send_LR();

       for(i=0;i<5;i++)
       {
    	Red_LED = 0;					
    	Delay_ms(500);
    	Red_LED = 1;					
    	Delay_ms(500);
    	}
    }
}
void Hum_Alarm(BYTE value)
{
	BYTE buf[4]={0,0,0,0};
  unsigned char i=0;
  for(i=0;i<3;i++)
   {
	Green_LED = 0;					//LED1黄色指示灯
	Delay_ms(500);
	Green_LED = 1;					//LED1黄色指示灯
	Delay_ms(500);
	}
    
    if(++hum_count > ALARM_LEVEL)
    {
        hum_count = 0;
        
        Second_Serial_Send_ASCII(MOBILE_NUM);
    	//Second_Serial_Send_ASCII("AT+CMGS=\"+8613078047847\"");
    	Send_LR();
    	Delay_ms(1000);
    	Second_Serial_Send_ASCII("Alarm! now humidity:");
        buf[0] = value / 100 + 0x30;
        buf[1] = (value%100) / 10 + 0x30;
        buf[2] = value % 10+ 0x30;
        buf[3] = '\0';
        Second_Serial_Send_ASCII(buf);
    	Delay_ms(300);
    	Second_Serial_Port_Send(0x1A);
    	Send_LR();

       for(i=0;i<5;i++)
       {
    	Green_LED = 0;					
    	Delay_ms(500);
    	Green_LED = 1;					
    	Delay_ms(500);
    	}
    }
}

/*
BYTE Equal_2byte(const BYTE* arr, UINT value)
{
    return ((arr[0] == ((value>>8) & 0xff))&&(arr[1]== (value& 0xff)));
}

void Set_2byte(BYTE* arr, UINT value)
{
    arr[0] = (value >> 8) & 0xff;
    arr[1] = value & 0xff;
}
*/
//data & datalen much correct datalen < 252
//data[0,1]:target addr ; data[2,n]:cmd1 & param
void ZZU_Send(const BYTE* dat,BYTE datalen)
{
    BYTE buf[255];
	BYTE i=0,j=0,len = 0,fcs =0;
    //z-stack //sof + data'len + cmd +data
    buf[0] = SOF;
    buf[1] = datalen  + 1 ;//msg len = datalen + CMD + SEP 
    buf[2] = 0x29;//Z_APP_MSG_S_SEND CMD high byte
    buf[3] = 0x00;//Z_APP_MSG_S_SEND CMD low byte
    //data field--zzu protocal
    //sep + target addr +cmdn
    buf[4] = SEP;
     
   for(;i<datalen;i++)
    {
        buf[5 + i] = dat[i];
    }

	j = 1;
	len = buf[1]+1+2;//LEN +CMD 
	for(i=0;i < len;i++)
    {
       	fcs ^= buf[j];
		j++;
    }
	buf[5 + datalen] = fcs;
     
    len = buf[1] + 1 + 1  + 2 + 1;//add buf SOF +LEN+CMD FCS 
    
    for(j=0; j <  len ; j++)
    {
        Send_Hex(buf[j]);
    }
}
/*
//check cmd respond ok,no value respond
BYTE Check_ZZU_Respond()
{
    //todo use First_Int to detect
    return Equal_2byte((BYTE*)&Buf_First_Serial[2],Z_APP_MSG_A_SEND);
//  return (Buf_First_Serial[2] == 0x69 && Buf_First_Serial[3] == 0x00 && Buf_First_Serial[4] == 0x00);
}
*/
void Stop_Temperature(BYTE addrh, BYTE addrl)
{
    BYTE temper_stop[7]={0,0,//sensor zigbee addr
                        0x00,0x02, //write cmd
                        0x01,0x02,0x00};//work mode: 0 quiet
    temper_stop[0] = addrh;
    temper_stop[1] = addrl;

    ZZU_Send(temper_stop,7);
    //ignore result
}

void Stop_Body(BYTE addrh, BYTE addrl)
{
    BYTE body_stop[7]={0,0,//sensor zigbee addr
                        0x00,0x02, //write cmd
                        0x04,0x01,0x00};//work mode: 0 quiet
    body_stop[0] = addrh;
    body_stop[1] = addrl;

    ZZU_Send(body_stop,7);
}

void Stop_Humidity(BYTE addrh, BYTE addrl)
{
    BYTE hum_stop[7]={0,0,//sensor zigbee addr
                        0x00,0x02, //write com
                        0x07,0x01,0x00};//work mode: 0 quiet
    hum_stop[0] = addrh;
    hum_stop[1] = addrl;

    ZZU_Send(hum_stop,7);
}
void Query_Temperature(BYTE addrh, BYTE addrl)
{
//  BYTE idx = 12;
    BYTE check_status[8]={0,0,//sensor zigbee addr
                            0x00,0x01, //read cmd
                            0x01,0x01,//temperature value 
                            0x01,0x02};//work mode: 0 quiet
    check_status[0] = addrh;
    check_status[1] = addrl;

    ZZU_Send(check_status,8);
    /*
    Delay_ms(WAIT_TIME);
    if(Check_ZZU_Respond())//6byte
    {
        for(idx = 12;idx < ZZU_BUF_MAX; idx++)//
        {
            if(Equal_2byte(&Buf_First_Serial[idx],TEMPER_VALUE)){
                if(Buf_First_Serial[idx + 2] >= TEMP_ALARM_VALUE)
                {
                    //alarm send mse
                    Yellow_LED = 0;//debug light led
                }
                //Buf_First_Serial[idx + 5] work mode...
            }
        }
    }
    */
}

void Query_Body(BYTE addrh, BYTE addrl)
{
//  BYTE idx = 12;
    BYTE check_status[8]={0,0,//sensor zigbee addr
                            0x00,0x01, //read cmd
                            0x04,0x01,//work mode: 0 quiet
                            0x04,0x02};//
    check_status[0] = addrh;
    check_status[1] = addrl;

    ZZU_Send(check_status,8);
    /*
    Delay_ms(WAIT_TIME);
    if(Check_ZZU_Respond())//6byte
    {
        Red_LED = 0;
        for(idx = 12;idx < ZZU_BUF_MAX; idx++)//
        {
            if(Equal_2byte(&Buf_First_Serial[idx],BODY_VALUE)){
                if(Buf_First_Serial[idx + 2] >= BODY_ALARM_VALUE)
                {
                    //alarm send mse
                    Green_LED = 0;//debug light led
                }
                //Buf_First_Serial[idx -1] work mode...
            }
        }
    }
    */
}

void Query_Humidity(BYTE addrh, BYTE addrl)
{
//  BYTE idx = 12;
    BYTE check_status[8]={0,0,//sensor zigbee addr
                            0x00,0x01, //read cmd
                            0x07,0x01,//work mode: 0 quiet
                            0x07,0x02};//VALUE
    check_status[0] = addrh;
    check_status[1] = addrl;

    ZZU_Send(check_status,8);
    /*Delay_ms(WAIT_TIME);
    if(Check_ZZU_Respond())//6byte
    {
        for(idx = 12;idx < ZZU_BUF_MAX; idx++)//
        {
            if(Equal_2byte(&Buf_First_Serial[idx],HUM_VALUE)){
                if(Buf_First_Serial[idx + 2] >= HUM_ALARM_VALUE)
                {
                    //alarm send mse
                    Red_LED = 0;//debug light led
                }
                //Buf_First_Serial[idx -1] work mode...
            }
        }
    } 
    */ 
}

//read device interrupt report msg
//only check first alarm item,then clear
void Process_Report_Msg()
{
    BYTE idx = 0,addrh=0,addrl=0,type =0 ,value=0;

    // get device type & value ,check & send alarm msg
    addrh = Buf_First_Serial[(First_Set + ADDR_H_OFFSET) % Buf1_Max];
    addrl = Buf_First_Serial[(First_Set + ADDR_L_OFFSET) % Buf1_Max];
    type = Buf_First_Serial[(First_Set + ZZU_CMD_L_OFFSET + 1) % Buf1_Max];
//  value = Buf_First_Serial[(First_Set + ZZU_CMD_L_OFFSET + 3) % Buf1_Max];

    //clear alarm senser, it may no work again till reboot
    switch (type)
    {
        case ZZU_TEMPER:
            Stop_Temperature(addrh,addrl);
            value = Buf_First_Serial[(First_Set + ZZU_CMD_L_OFFSET + 3) % Buf1_Max];
            if(value >= TEMP_ALARM_VALUE)
            {
                Temp_Alarm(value);
            }
            break;
        case ZZU_BODY:
            Stop_Body(addrh,addrl);
            value = Buf_First_Serial[(First_Set + ZZU_CMD_L_OFFSET + 5) % Buf1_Max];
//          Debug(value);
            if(value >= BODY_ALARM_VALUE)
            {
                Body_Alarm();
            }
            break;
        case ZZU_HUMIDITY:
            Stop_Humidity(addrh,addrl);
            value = Buf_First_Serial[(First_Set + ZZU_CMD_L_OFFSET + 5) % Buf1_Max];
            if(value >= HUM_ALARM_VALUE)
            {
                Hum_Alarm(value);
            }
            break;
        default:
//          LED_Flash();//Yellow_LED = 1;
            break;
    }
}

void Check_New_Child(void)
{
    BYTE idx = 5,i=0,num =0;
//  num = Buf_First_Serial[(First_Set + ZZU_CMD_L_OFFSET +4) % Buf1_Max];
    for(i=ZZU_CMD_L_OFFSET; i < Buf_First_Serial[(First_Set + MSG_LEN_OFFSET)% Buf1_Max];i++)
    {
        if((Buf_First_Serial[(First_Set + i)% Buf1_Max] == 0x00)
           && (Buf_First_Serial[(First_Set + i +1)% Buf1_Max] == 0x15))
        {
            idx = (First_Set + i +2)% Buf1_Max;
            num = Buf_First_Serial[idx];
            break;
        }
    }

    if(num >= MAX_NODE)
    {
        num = MAX_NODE -1;
    }
//  Debug(num);

    if(num != device[0].child_num)
    {
        device[0].child_num = num;
//      idx = (First_Set + ZZU_CMD_L_OFFSET +4) % Buf1_Max ;
        for(i=1; i <= num; i++)//reget
        {
            device[i].addrh = Buf_First_Serial[(++idx) % Buf1_Max];
            device[i].addrl = Buf_First_Serial[(++idx) % Buf1_Max];
//          Debug(device[i].addrh);
//          Debug(device[i].addrl);
            device[i].type = 0;
        }
        for(;i < MAX_NODE; i++) //i == num
        {
            device[i].addrh = 0;
            device[i].addrl = 0;
            device[i].type = 0;
        }
    }
/*
    CLR_Buf1();
    ZZU_Send(coor_detail,COOR_DETAIL_NUM);
    Delay_ms(WAIT_TIME);
   if(Check_ZZU_Respond())
    {
        for(idx = 12;idx < ZZU_BUF_MAX; idx++)//
            {
                if(Equal_2byte(&Buf_First_Serial[idx],ZZU_CMD_NODE))
                {
                    num = Buf_First_Serial[idx + 2];
                    if(num >= MAX_NODE)
                    {
                        num = MAX_NODE -1;
                    }
                    if(num != device[0].child_num)
                    {
                        device[0].child_num = num;
                        idx +=3;
                        for(i=1; i <= num; i++)//reget
                        {
                            device[i].addrh = Buf_First_Serial[idx++];
                            device[i].addrl = Buf_First_Serial[idx++];
                            device[i].type = 0;
                        }
                        for(;i < MAX_NODE; i++) //i == num
                        {
                            device[i].addrh = 0;
                            device[i].addrl = 0;
                            device[i].type = 0;
                        }
                        break;
                    }
                }
            }
    }
   */
}
void Parse_Device_Status()
{
    BYTE type = 0,value =0;
    type = Buf_First_Serial[(First_Set + ZZU_CMD_L_OFFSET + 2) % Buf1_Max];
//  Debug(type);
    //  value = Buf_First_Serial[(First_Set + ZZU_CMD_L_OFFSET + 3) % Buf1_Max];
    switch(type)
    {
        case ZZU_TEMPER:
            value = Buf_First_Serial[(First_Set + ZZU_CMD_L_OFFSET + 4) % Buf1_Max];
			//Debug(value);
            if(value >= TEMP_ALARM_VALUE)
            {
                Temp_Alarm(value);
            }
            break;
        case ZZU_BODY:
            value = Buf_First_Serial[(First_Set + ZZU_CMD_L_OFFSET + 6) % Buf1_Max];
            if(value >= BODY_ALARM_VALUE)
            {
                Body_Alarm();
            }
            break;
    case ZZU_HUMIDITY:
            value = Buf_First_Serial[(First_Set + ZZU_CMD_L_OFFSET + 6) % Buf1_Max];
            if(value >= HUM_ALARM_VALUE)
            {
                Hum_Alarm(value);
            }
            break;
        default:
//          LED_Flash();//Yellow_LED = 1;
            break;
    }
}
/*
void Query_Device_Status(void)
{
    BYTE i = 1,type = 0;
    for(; i < MAX_NODE; i++)
    {
        type = device[i].type;
        switch(type)
        {
        case ZZU_TEMPER:
            Query_Temperature(device[i].addrh,device[i].addrl);
            break;
        case ZZU_BODY:
            Query_Body(device[i].addrh,device[i].addrl);
            break;
        case ZZU_HUMIDITY:
            Query_Humidity(device[i].addrh,device[i].addrl);
            break;
        default:
            break;
        }
    }
}
*/
void Parse_Device_Type(void)
{
    BYTE i =1;
    for(i = 1; i < MAX_NODE; i++)
    {
        if((Buf_First_Serial[(First_Set + ADDR_H_OFFSET) % Buf1_Max] == device[i].addrh)
           && (Buf_First_Serial[(First_Set + ADDR_L_OFFSET) % Buf1_Max] == device[i].addrl))
        {
            device[i].type = Buf_First_Serial[(First_Set + ZZU_CMD_L_OFFSET +4) % Buf1_Max];
//          Debug(device[i].type);
            break;
        }
    }
}
/*
void Query_Device_Type(void)
{
    BYTE i = 1;
    BYTE check_type[6]={0,0,//sensor zigbee addr
                            0x00,0x01, //read cmd
                            0x00,0x05};//device type
                            //todo get device's child 0x00,0x15

    for(i = 1; i < MAX_NODE; i++)
    {
        if((device[i].addrh != 0x00)
           && (device[i].addrl != 0x00)
           && (device[i].type == 0x00))
        {
//          Debug(i);
            check_type[0] = device[i].addrh;
            check_type[1] = device[i].addrl;
            ZZU_Send(check_type,6);
            Delay_ms(WAIT_TIME);

        }
    }
}
*/

void Query_Device(BYTE idx)
{
    BYTE check_type[6]={0,0,//sensor zigbee addr
                        0x00,0x01, //read cmd
                        0x00,0x05};//device type

    if(idx == 0)
    {//coordinate
        ZZU_Send(coor_detail,COOR_DETAIL_NUM);
    }
    else{
        if((device[idx].addrh != 0x00)
           && (device[idx].addrl != 0x00)
           && (device[idx].type == 0x00))
        {
            check_type[0] = device[idx].addrh;
            check_type[1] = device[idx].addrl;
            ZZU_Send(check_type,6);
//          Delay_ms(WAIT_TIME);
        }
        else //if(device[idx].type != 0x00)
        {
            switch(device[idx].type)
            {
            case ZZU_TEMPER:
                Query_Temperature(device[idx].addrh,device[idx].addrl);
                break;
            case ZZU_BODY:
                Query_Body(device[idx].addrh,device[idx].addrl);
                break;
            case ZZU_HUMIDITY:
                Query_Humidity(device[idx].addrh,device[idx].addrl);
                break;
            default:
                break;
            }
        }
    }
}

void Empty_Device(void){
    BYTE i = 0;
    for(i=0;i < MAX_NODE; i++) //i == num
    {
        device[i].addrh = 0;
        device[i].addrl = 0;
        device[i].type = 0;
        device[i].child_num = 0;
    }
}

void Zigbee_Network(void)
{    
    BYTE cnt = 0;
    Empty_Device();
    CLR_Buf1();
    while(1)
    {
        while(First_Set != First_Int)//process all msg
        {
            if((Buf_First_Serial[First_Set] != SOF)
               && (Buf_First_Serial[(First_Set + Z_CMD_H_OFFSET) % Buf1_Max] != 0x69)
               && (Buf_First_Serial[(First_Set + Z_CMD_L_OFFSET) % Buf1_Max] != 0x80))
            {// only check Z_APP_MSG_S_ACK info(cmd ack & report msg)
                First_Set = (First_Set + 1) % Buf1_Max ; //get next one byte
                continue;
            }
            if((Buf_First_Serial[(First_Set + ZZU_CMD_H_OFFSET) % Buf1_Max] == 0x80)
               && (Buf_First_Serial[(First_Set + ZZU_CMD_L_OFFSET) % Buf1_Max] == 0x01))
            {//ZZU_READ_ACK
                if((Buf_First_Serial[(First_Set + ADDR_H_OFFSET) % Buf1_Max] == 0x00)
                   && (Buf_First_Serial[(First_Set + ADDR_L_OFFSET) % Buf1_Max] == 0x00))
                {//ZZU_COOR_ADDR :parsing new child
                    Check_New_Child();
                }
                else{// terminal node: poll status & device type
                    if((Buf_First_Serial[(First_Set + ZZU_CMD_H_OFFSET +3) % Buf1_Max] == 0x00)
                       && (Buf_First_Serial[(First_Set + ZZU_CMD_L_OFFSET +3) % Buf1_Max] == 0x05)
                       && (Buf_First_Serial[(First_Set + MSG_LEN_OFFSET) % Buf1_Max] == 0x08))
                    {//type 
                        Parse_Device_Type();
                    }
                    else{//status
                        Parse_Device_Status();
                    }
                }
            }
            else if((Buf_First_Serial[(First_Set + ZZU_CMD_H_OFFSET) % Buf1_Max] == 0x00)
                    && (Buf_First_Serial[(First_Set + ZZU_CMD_L_OFFSET) % Buf1_Max] == 0x03))
            {//ZZU_REPORT
                Process_Report_Msg();
            }
            //ignore ZZU_WRITE_ACK

            First_Set = (First_Set + 1) % Buf1_Max; 
            if(First_Set == First_Int) break;
        }

        
        if(cnt < MAX_NODE)
            {//check child info & report
                Query_Device(cnt);
                cnt++;
            }
        else 
            {
                cnt = 0;
            }
        

        Delay_ms(WAIT_TIME);
    }
}
/*--------- add zigbee application end ------*/


/****************************************主函数**********************************/

void main(void)
{	
//	unsigned char i;	
	Delay_ms(100);
	IGT = 0;
	Delay_ms(5000);
	IGT = 1;
	Timer0_init();								//定时器0初始化
    Serial_Init();          					//第一个串口初始化
	Second_Serial_Port_Initial();				//第二个串口初始化
	LED_Flash();
	GSM_ON=1;
	Delay_ms(100);
	GSM_ON=0;
	Delay_ms(5000);
	Delay_ms(5000);

	
	Delay_ms(5000);
	    Send_ASCII("uart1");
	//	Second_Serial_Send_ASCII("uart2");
		//
	//qin test
  	//First_Set=0, Second_Set=0;
	CLR_Buf1();
	CLR_Buf2();
	Second_Serial_Send_ASCII("AT+CMGF=1");
	Send_LR();
	Delay_ms(1000);
	if(Hand("OK"))
      LED_Flash();

    //Zigbee_Network();
     /* 
	  Second_Serial_Send_ASCII("AT+CMGS=\"+8613794462701\"");
	//Second_Serial_Send_ASCII("AT+CMGS=\"+8613078047847\"");
	Send_LR();
	Delay_ms(1000);
	Second_Serial_Send_ASCII("HELLO ,pan2!");
	Delay_ms(300);
	Second_Serial_Port_Send(0x1A);
	Send_LR();
	Delay_ms(3000);
	*/
    Zigbee_Network();
/*

	while(1)
	{
	//**********************************
	while(First_Set!=First_Int)
	{
		Second_Serial_Port_Send(Buf_First_Serial[First_Set]);
		if(First_Set==Buf1_Max)
		{	
			First_Set=0;	
		}
		else   First_Set++;
	}
	//***********************************
	while(Second_Set!=Second_Int)
	{
		Send_Hex(Buf_Second_Serial[Second_Set]);
		if(Second_Set==Buf2_Max)
		{	
			Second_Set=0;	
		}
		else   Second_Set++;
	}

//	Second_Serial_Send_ASCII("AT+CMGF=1");
//	Send_LR();
//	Delay_ms(5000);
//	Send_ASCII("uart1");
	}
*/
}

/*******************************定时器0中断处理函数******************************/

void Timer0(void) interrupt 1
{
	TR0 = 0;									//停止定时器0
	Count_time++;
	if(Count_time >= 60000)					
		{					
			Count_time = 0;
		}
	if(Timer0_Start == 1)
		Times++;
	if(Times > (20*shijian))
	{
		Timer0_Start = 0;
		Times = 0;
	}
	TH0 = 0x4C;									//设置溢出一次为50ms
	TL0 = 0x00;
	TR0 = 1;									//开启定时器0
}

/*********************************串口1中断处理函数******************************/

void Serial_Int(void) interrupt 4 
{	
	ES = 0;	 									//关串口中断，防止中断嵌套
	if(TI)	 									//如果是发送中断，则不做任何处理
	{
		TI = 0;  								//清除发送中断标志位
	}
	if(RI)			 							//如果是接送中断，则进行处理
	{	
		RI = 0; 								//清除接收中断标志位
		Buf_First_Serial[First_Int] = SBUF;  	//将接收到的字符串存到缓存中
		First_Int++;                			//缓存指针向后移动
		if(First_Int > Buf1_Max)       			//如果缓存满,将缓存指针指向缓存的首地址
		{
			First_Int = 0;
		}
	}
	ES = 1;	 									//开启串口中断
}

/****************************第二个串口接收数据函数******************************/

void Second_Serial_Int(void) interrupt 8
{
	unsigned char k = 0;
	IE2 = 0x00; 							//关串口2中断,ES2=0
	k = S2CON;
	k = k & 0x01;
	if(k==1)
	{	
		S2CON = S2CON & 0xFE; 				//1111,1110 清除串口2接收中断标志位
		//QIN 
		Buf_Second_Serial[Second_Int] = S2BUF;  	//将接收到的字符串存到缓存中
		Second_Int++;                			//缓存指针向后移动
		if(Second_Int > Buf2_Max)       			//如果缓存满,将缓存指针指向缓存的首地址
		{
			Second_Int = 0;
		}
	/*	if(Flag_Bug == 0)
		{	
			Flag_Bug = 1;
			Second_Int = 0;
			Times = 0;
			shijian = 1;					//接收时间限制时间1秒
			Timer0_Start = 1;	
		}
		if(Timer0_Start == 1)
		{
			Buf_Second_Serial[Second_Int] = S2BUF;
			Second_Int++;
			if(Second_Int > 153)			//最多接收DX+手机号(11位)+汉字(最多支持70个汉字)
				Second_Int = 0;
		}  */
	}
	else
	{
		 S2CON = S2CON & 0xFD; 				//1111,1101 清除串口2发送中断标志位
	}
	IE2 = 0x01; 							//允许串口2中断,ES2=1
}

/***************************************END**************************************/
