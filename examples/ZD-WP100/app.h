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


typedef struct
{
	u8 status;	                /*传感器状态 0代表正常，-1代表故障，1代表水浸报警*/
} Sensor_Info;


/*设备事件*/
enum ProtocolEvent
{
  NONE = 0x00,
  DEVALARM = 0x01,  //设备主动告警上报
  DEVINIT = 0x02,   //设备初始化上报
  DEVPOLL = 0x04,   //定期巡检主动上报
  CHDALARM = 0x06,  //子设备告警上报
  DEVQUERY = 0x31,  //设备查询
  DEVSET  = 0x32,   //设备设置
  DEVBEAT = 0xff,   //心跳数据
};

/*协议*/
enum ZDST_PROTOCOL{
	MSG_HEAD = 0x26,     
	PROTOCOL_ID = 0x44,
	MSG_END = 0x21,
	TERM_TYPE = 0x0102,  //水浸传感器
	MSG_DEMAND = 0X80, 
	MSG_RESPOND = 0X00, 
};

/*协议数据封装结构体*/
#pragma   pack(1)
struct msg{
	uint8_t   Head;                      //协议头
	uint8_t	 Protocol_id;            	 //协议版本
	uint32_t	 Data_length;            //协议长度
	uint16_t  Schksum;                   //校验单元
	uint16_t   Term_type;                //终端类型
	uint8_t  Term_addr[9];               //设备地址：1个厂商标识+8字节的mac地址
	uint16_t  CommunicationPacket_mark;  //通信标识
	uint8_t   Communication_mark;		 //交互标识
	uint8_t   Command_unit;				 //命令单元
	uint8_t   Dataload[80];				 //数据单元
};
#pragma   pack()

uint8_t ChkSum8(uint8_t *sdata , uint16_t len);
u16 protocolPackingFunc(uint8_t type);
void sensorEventHandler(void);





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

