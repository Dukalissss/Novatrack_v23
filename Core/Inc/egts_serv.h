#define MAX_PARSE_TIMEOUT		(uint16_t)1000
#define Parse_Error()				{egts_parse.rx_cnt=0;egts_parse.sdr_cnt=0;egts_parse.parse_timeout=0;egts_parse.sfr_cnt=0;egts_parse.rd_cnt=0;return;}
#define RTE_FLAG						0x20
#define timshift 						0x12CFF786

#define MNE_FLAG						0x80
#define	BSE_FLAG						0x40
#define	NIDE_FLAG						0x20
#define	SSRA_FLAG						0x10
#define	LNGCE_FLAG					0x08
#define	IMSIE_FLAG					0x04
#define	IMEIE_FLAG					0x02
#define	HDIDE_FLAG					0x01


#define SSOD_FLAG						0x80
#define RSOD_FLAG						0x40
#define GRP_FLAG						0x20
#define RPP_MEDIUM_FLAG			0x10
#define TMFE_FLAG						0x04
#define EVFE_FLAG						0x02
#define OBFE_FLAG						0x01


#define SFRD_MAX_LENGTH			220
#define RD_MAX_LENGTH				210
#define SRD_MAX_LENGTH			200

#define ALTH_FLAG						0x80
#define LOHS_FLAG						0x40
#define LAHS_FLAG						0x20
#define MV_FLAG							0x10
#define BB_FLAG							0x08
#define CS_FLAG							0x04
#define FIX_FLAG						0x02
#define VLD_FLAG						0x01

#define DIRH_FLAG						0x80
#define ALTS_FLAG						0x40


#define LLSEF_FLAG					0x40
#define LLSVU1_FLAG					0x10
#define LLSVU2_FLAG					0x20
#define RDF_FLAG						0x08

#define EGTS_SR_POS_DATA_TYPE 						16
#define EGTS_SR_EXT_POS_DATA_TYPE					17
#define EGTS_SR_AD_SENSORS_DATA_TYPE			18
#define EGTS_SR_STATE_DATA_TYPE						20
#define EGTS_SR_ABS_AN_SENS_DATA_TYPE			24
#define EGTS_SR_LIQUID_LEVEL_SENSOR_TYPE 	27

#define NSFE_FLAG						0x10
#define SFE_FLAG						0x08
#define PFE_FLAG						0x04
#define HFE_FLAG						0x02
#define VFE_FLAG						0x01

#define NMS_FLAG						0x04
#define IBU_FLAG						0x02
#define BBU_FLAG						0x01


#define DIOE1_FLAG					0x01
#define ASFE1_FLAG					0x01
#define ASFE2_FLAG					0x01

enum{
	EGTS_SR_RECORD_RESPONSE_TYPE=0,
	EGTS_SR_TERM_IDENTITY_TYPE,
	EGTS_SR_MODULE_DATA
};

enum{
	EGTS_AUTH_SERVICE=1,
	EGTS_TELEDATA_SERVICE
};

//enum{
//	EGTS_SR_POS_DATA_TYPE=16,
//	EGTS_SR_EXT_POS_DATA_TYPE,//17
//	EGTS_SR_AD_SENSORS_DATA_TYPE,//18
//	EGTS_SR_COUNTERS_DATA_TYPE,//19
//	EGTS_SR_STATE_DATA_TYPE,//20
//	EGTS_SR_LOOPIN_DATA_TYPE,//21
//	EGTS_SR_ABS_DIG_SENS_DATA_TYPE,//22
//	EGTS_SR_ABS_AN_SENS_DATA_TYPE,//23
//	EGTS_SR_ABS_CNTR_DATA_TYPE,//24
//	EGTS_SR_ABS_LOOPIN_DATA_TYPE,//25
//	EGTS_SR_LIQUID_LEVEL_SENSOR_TYPE,//26
//	EGTS_SR_PASSENGERS_COUNTERS_TYPE
//};
typedef __packed struct {
	uint8_t		PRV;
	uint8_t		SKID;
	uint8_t		FLAGS;
	uint8_t		HL;
	uint8_t		HE;
	uint16_t	FDL;
	uint16_t	PID;
	uint8_t		PT;
//	uint16_t	PRA;
//	uint16_t	RCA;
//	uint8_t		TTL;
	uint8_t		HCS;
	uint8_t		SFRD[SFRD_MAX_LENGTH];
	uint16_t	SFRCS;
} PACKET_STRUCT;
extern PACKET_STRUCT	EGTS_PACKET;

typedef __packed struct {
	uint16_t		RL;
	uint16_t		RN;
	uint8_t			RFL;
	uint32_t		OID;
//	uint32_t		EVID;
//	uint32_t		TM;
	uint8_t			SST;
	uint8_t			RST;
	uint8_t			RD[RD_MAX_LENGTH];
} RECORD_STRUCT;
extern RECORD_STRUCT	RECORD;


typedef __packed struct {
	uint8_t		SRT;
	uint16_t	SRL;
	uint8_t		SRD[SRD_MAX_LENGTH];
} SUBRECORD_STRUCT;
extern SUBRECORD_STRUCT SUBRECORD;

typedef __packed struct {
	uint32_t		TID;
	uint8_t			FLAGS;
//	uint16_t		HDID;
	char				IMEI[15];
//	char				IMSI[16];
//	char				LNGC[3];
//	uint8_t			NID[3];
	uint16_t		BS;
//	char				MSISDN[15];
} EGTS_SR_TERM_IDENTITY_STRUCT;
extern EGTS_SR_TERM_IDENTITY_STRUCT EGTS_SR_TERM_IDENTITY;

typedef __packed struct {
	uint32_t		NTM;
	uint32_t		LAT;
	uint32_t		LONG;
	uint8_t			FLG;
	uint16_t		SPD;
	uint8_t			DIR;
	uint8_t			ODM[3];
	uint8_t			DIN;
	uint8_t			SRC;
//	uint8_t			ALT[3];
	uint16_t		SRCD;
	
} EGTS_SR_POS_DATA_STRUCT;
extern EGTS_SR_POS_DATA_STRUCT EGTS_SR_POS_DATA;


typedef __packed struct {
	uint8_t			FL;
	uint16_t		MADDR;
	uint32_t		LLSD;
} EGTS_SR_LIQUID_LEVEL_SENSOR_STRUCT;
extern EGTS_SR_LIQUID_LEVEL_SENSOR_STRUCT EGTS_SR_LIQUID_LEVEL_SENSOR;


typedef __packed struct {
	uint8_t			FL;
//	uint16_t		VDOP;
	uint16_t		HDOP;
//	uint16_t		PDOP;
	uint8_t			SAT;
//	uint16_t		NS;
} EGTS_SR_EXT_POS_DATA_STRUCT;
extern EGTS_SR_EXT_POS_DATA_STRUCT EGTS_SR_EXT_POS_DATA;


typedef __packed struct {
	uint8_t			ASN;
	uint8_t			ASV[3];
} EGTS_SR_ABS_AN_SENS_DATA_STRUCT;
extern EGTS_SR_ABS_AN_SENS_DATA_STRUCT EGTS_SR_ABS_AN_SENS_DATA;

typedef __packed struct {
	uint8_t			ST;
	uint8_t			MPSV;
	uint8_t			BBV;
	uint8_t			IBV;
	uint8_t			FL;
} EGTS_SR_STATE_DATA_STRUCT;
extern EGTS_SR_STATE_DATA_STRUCT EGTS_SR_STATE_DATA;


typedef __packed struct {
	uint8_t			DIO_FL;
	uint8_t			DOUT;
	uint8_t			AS_FL;
	uint8_t			ADIO1;
//	uint8_t			ADIO2;
//	uint8_t			ADIO3;
//	uint8_t			ADIO4;
//	uint8_t			ADIO5;
//	uint8_t			ADIO6;
//	uint8_t			ADIO7;
//	uint8_t			ADIO8;
	uint8_t			ANS1[3];
	uint8_t			ANS2[3];
//	uint8_t			ANS3[3];
//	uint8_t			ANS4[3];
//	uint8_t			ANS5[3];
//	uint8_t			ANS6[3];
//	uint8_t			ANS7[3];
//	uint8_t			ANS8[3];
} EGTS_SR_AD_SENSORS_DATA_STRUCT;
extern EGTS_SR_AD_SENSORS_DATA_STRUCT EGTS_SR_AD_SENSORS_DATA;







typedef struct {
	uint8_t 	rx_cnt;
	uint8_t		sfr_cnt;
	uint8_t		sdr_cnt;
	uint8_t		rd_cnt;
	uint16_t	parse_timeout;
	uint16_t	read_index;
	uint16_t	parse_index;
	uint8_t		parsing_ongoing;
	uint8_t		half_word_cnt;
	uint8_t		check_cnt;	
	uint8_t		login_successful;
	uint8_t		check_buf[30];


	
	//HEADER
	uint8_t 	PRV;
	uint8_t 	SKID;
	uint8_t		FL;
	uint8_t 	HL;
	uint8_t 	HE;
	uint16_t 	FDL;
	uint16_t 	PID;
	uint8_t 	PT;
	uint16_t 	PRA;
	uint16_t 	RCA;
	uint8_t 	TTL;
	uint8_t 	HCS;
	//SFRD
	uint16_t	RPID;
	uint8_t		PR;
	//SDR
	uint16_t	RL;
	uint16_t	RN;
	uint8_t		RFL;
	uint32_t	OID;
	uint32_t	EVID; 
	uint32_t	TM;
	uint8_t		SST;
	uint8_t		RST;
	//RD
	uint8_t		SRT;
	uint16_t	SRL;
	
	
	uint16_t	SFRCS_received;
	uint16_t	SFRCS;
} EGTS_PARSE_STRUCT;
extern EGTS_PARSE_STRUCT egts_parse;

//структура всего пакета
enum{
	PRV=0,
	SKID,
	FL,
	HL,
	HE,
	FDL,
	PID,
	PT,
	PRA,
	RCA,
	TTL,
	HCS,
	SFRD,
	SFRCS
};
//структура записи SFRD типа EGTS_PT_RESPONSE
enum{
	RPID=0,
	PR,
	SDR1
};
//структура записи уровня поддержки услуг SDR
enum{
	RL=0,
	RN,
	RFL,
	OID,
	EVID, 
	TM,
	SST,
	RST,
	RD
};
//структура подзаписи уровня поддержки услуг RD
enum{
	SRT=0,
	SRL,
	SRD
};

enum{
	EGTS_PT_RESPONSE=0,
	EGTS_PT_APPDATA,
	EGTS_PT_SIGNED_APPDATA
};
enum{
	EGTS_PC_OK=0,
	EGTS_PC_IN_PROGRESS
};	




void EGTS_Serv(void);
uint8_t	Check_Receive_Packet(uint8_t *r_buf);
uint8_t	crc8_byte(uint8_t b, uint8_t crc_old);
uint8_t crc8(unsigned char *pcBlock, uint16_t len);
void Parse_EGTS_Data(uint8_t rx_b);
unsigned short egts_Crc16(unsigned char* pcBlock, unsigned short len);
unsigned short egts_Crc16_byte(unsigned char c_byte, unsigned short old_crc);
void Data_Response_OK(void);
uint16_t EGTS_Init_Login(void);
void EGTS_SR_TERM_IDENTITY_Subrecord_Prepare(void);
void Record_Prepare(uint8_t	rec_service);
uint32_t Calculate_Current_TM(void);
void Packet_Prepare(void);
uint16_t Format_Half_Word(uint16_t	hlfw);
void EGTS_Init_Data_Packet(void);
void EGTS_SR_POS_DATA_Subrecord_Prepare(void);
void EGTS_SR_LIQUID_LEVEL_SENSOR_Subrecord_Prepare(uint8_t dut_number);
void EGTS_SR_EXT_POS_DATA_Subrecord_Prepare(void);
uint32_t Calculate_Blackbox_TM(void);
void EGTS_SR_STATE_DATA_Subrecord_Prepare(void);
void EGTS_SR_AD_SENSORS_DATA_Subrecord_Prepare(void);



