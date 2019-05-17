/*
 * lpm_downward.h
 *
 *  Created on: 2019��1��10��
 *      Author: ASUS
 */

#ifndef __SMOKE_H_
#define __SMOKE_H_

typedef int32_t  s32;
typedef int16_t s16;
typedef int8_t  s8;

typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;

extern uint8_t ZDST_TX_BUF[128] ;
extern uint8_t ZDST_RX_BUF[128] ;

#define MSG_S ((struct msg*)  (ZDST_TX_BUF))

#define MSG_R ((struct msg*)  (ZDST_RX_BUF))

struct Loc_run_param{//45bi

	uint8_t 		 Chksum;
	uint8_t       Version[4];  //4bit
	uint8_t       Domain[32];//32bit
	uint16_t      Port;//2bit
	uint32_t	     sys_addr;//4bit


	uint8_t       Normal_send_time;//1bit
	uint8_t       EM_send_time;//1bit

};

extern struct Loc_run_param  Sys;

enum ProtocolComy
{
  NONE = 0x00,
  DEVALARM = 0x01,
  DEVINIT = 0x02,
  DEVPOLL = 0x04,
  CHDALARM = 0x06,
  DEVQUERY = 0x31,
  DEVSET  = 0x32,
  DEVBEAT = 0xff,
};

enum ZDST_PROTOCOL{
	MSG_HEAD = 0x26,
	PROTOCOL_ID = 0x44,
	MSG_END = 0x21,
	TERM_TYPE = 0x22,
	MSG_DEMAND = 0X80, //��������
	MSG_RESPOND = 0X00, //����Ӧ��
//	MSG_SET_TERM = 0x32, //����
//	MSG_REQ_TERM  = 0x31 //��ѯ
};

#pragma   pack(1)
struct msg{
	uint8_t   Head;
	uint8_t	 Protocol_id;
	uint32_t	 Data_length;
	uint16_t  Schksum;
	uint16_t   Term_type;
	uint8_t  Term_addr[9];
	uint16_t  CommunicationPacket_mark;
	uint8_t   Communication_mark;
	uint8_t   Command_unit;
	uint8_t   Dataload[80];//���ݵ�Ԫ = ��������� + �����ֵ
	//uint8_t    End;
};
#pragma   pack()

struct msgdata
{//

	uint16_t   Command_id;//minglingdanyuan
	uint8_t  Dataload[1];//19
};

struct bit_32{
	uint32_t Data[1];
};

typedef struct {

	uint8_t  hour;
	uint8_t  minute;
	uint8_t  second;

}Calendar;

typedef union
{
	uint8_t byte;	                /**< the whole byte */
	struct
	{
            uint8_t volt :  1;
            uint8_t erro :  1;
            uint8_t dirt :  1;
            uint8_t fire :  3;
            uint8_t chek :  1;
            uint8_t faut :  1;
	} status;
} Smk_Info;


uint8_t ChkSum8(uint8_t *sdata , uint16_t len);
u16 Zdst_DoPacket(uint8_t type);//ͨ��״̬  ���ݳ���
void Zdst_Deal_Accord(void);
uint8_t Zdst_ReadParam(uint8_t *rec,uint32_t addr);
void Zdst_WriteParam(uint8_t *rec,uint32_t addr);
void Zdst_GetSysConfig(void);
void Zdst_SetSysConfig(void);

void init_scl();

#define LITTLE_ENDIAN

#if defined(BIG_ENDIAN)
#define htons(A) (A)
#define htonl(A) (A)
#define ntohs(A) (A)
#define ntohl(A) (A)
#elif defined(LITTLE_ENDIAN)
#define htons(A) (((((u16)A) & 0xFF00) >> 8) | \
		((((u16)A) & 0x00FF) << 8))
#define htonl(A) (((((u32)A) & 0xFF000000) >> 24) | \
		((((u32)A) & 0x00FF0000) >> 8) | \
		((((u32)A) & 0x0000FF00) << 8) | \
		((((u32)A) & 0x000000FF) << 24))
#define ntohs htons
#define ntohl htonl
#else
#error "User Must define LITTLE_ENDIAN or BIG_ENDIAN!!!"
#endif



PROCESS_NAME(app_process);
PROCESS_NAME(report_process);








#endif /* EXAMPLES_PLATFORM_SPECIFIC_CC26X0_CC13X0_VERY_SLEEPY_DEMO_LPM_DOWNWARD_LPM_DOWNWARD_H_ */
