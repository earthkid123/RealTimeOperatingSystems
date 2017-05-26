// AP_Lab6.c
// Runs on either MSP432 or TM4C123
// see GPIO.c file for hardware connections 

// Daniel Valvano and Jonathan Valvano
// September 18, 2016
// CC2650 booster or CC2650 LaunchPad, CC2650 needs to be running SimpleNP 2.2 (POWERSAVE)

#include <stdint.h>
#include "../inc/UART0.h"
#include "../inc/UART1.h"
#include "../inc/AP.h"
#include "AP_Lab6.h"
#include <string.h> 
//**debug macros**APDEBUG defined in AP.h********
#ifdef APDEBUG
#define OutString(STRING) UART0_OutString(STRING)
#define OutUHex(NUM) UART0_OutUHex(NUM)
#define OutUHex2(NUM) UART0_OutUHex2(NUM)
#define OutChar(N) UART0_OutChar(N)
#else
#define OutString(STRING)
#define OutUHex(NUM)
#define OutUHex2(NUM)
#define OutChar(N)
#endif

//****links into AP.c**************
extern const uint32_t RECVSIZE;
extern uint8_t RecvBuf[];
typedef struct characteristics{
  uint16_t theHandle;          // each object has an ID
  uint16_t size;               // number of bytes in user data (1,2,4,8)
  uint8_t *pt;                 // pointer to user data, stored little endian
  void (*callBackRead)(void);  // action if SNP Characteristic Read Indication
  void (*callBackWrite)(void); // action if SNP Characteristic Write Indication
}characteristic_t;
extern const uint32_t MAXCHARACTERISTICS;
extern uint32_t CharacteristicCount;
extern characteristic_t CharacteristicList[];
typedef struct NotifyCharacteristics{
  uint16_t uuid;               // user defined 
  uint16_t theHandle;          // each object has an ID (used to notify)
  uint16_t CCCDhandle;         // generated/assigned by SNP
  uint16_t CCCDvalue;          // sent by phone to this object
  uint16_t size;               // number of bytes in user data (1,2,4,8)
  uint8_t *pt;                 // pointer to user data array, stored little endian
  void (*callBackCCCD)(void);  // action if SNP CCCD Updated Indication
}NotifyCharacteristic_t;
extern const uint32_t NOTIFYMAXCHARACTERISTICS;
extern uint32_t NotifyCharacteristicCount;
extern NotifyCharacteristic_t NotifyCharacteristicList[];
//**************Lab 6 routines*******************
// **********SetFCS**************
// helper function, add check byte to message
// assumes every byte in the message has been set except the FCS
// used the length field, assumes less than 256 bytes
// FCS = 8-bit EOR(all bytes except SOF and the FCS itself)
// Inputs: pointer to message
//         stores the FCS into message itself
// Outputs: none
void SetFCS(uint8_t *msg){
//****You implement this function as part of Lab 6*****
  uint8_t FCS;
	uint8_t length;
	uint8_t data;
	
  FCS=0;
  msg++;//Skip SOF
  data=*msg; length=data;
	FCS=FCS^data; msg++;   // LSB length
  data=*msg; FCS=FCS^data; 
	msg++;   // MSB length=0 (assume length field <256
  data=*msg; FCS=FCS^data;
	msg++;   // CMD0
  data=*msg; FCS=FCS^data; 
	msg++;   // CMD1
  for(int i=0;i<length;i++)
	{
    data=*msg; FCS=FCS^data; msg++; // payload
  }
  *msg=FCS;                         // append FCS	 at the end
  
  
}
//*************BuildGetStatusMsg**************
// Create a Get Status message, used in Lab 6
// Inputs pointer to empty buffer of at least 6 bytes
// Output none
// build the necessary NPI message that will Get Status
void BuildGetStatusMsg(uint8_t *msg){
// hint: see NPI_GetStatus in AP.c
//****You implement this function as part of Lab 6*****
  uint8_t *ptr;
  ptr=msg;
	
	*msg=SOF;
	msg++;   // assign SOF then increment msg pointer
  *msg=0x00;
	msg++;  // LSB length=0, no payload
  *msg=0x00;
	msg++;  // MSB length	
  *msg=0x55;
	msg++;  // CMD0 from API table, page 11 of 41
  *msg=0x06;
	msg++;  // CMD1 from API table, page 11 of 41
  SetFCS(ptr);	// msg is incremented. use pt w/c is unincremented to point back to compute fcs
  
}
//*************Lab6_GetStatus**************
// Get status of connection, used in Lab 6
// Input:  none
// Output: status 0xAABBCCDD
// AA is GAPRole Status
// BB is Advertising Status
// CC is ATT Status
// DD is ATT method in progress
uint32_t Lab6_GetStatus(void){volatile int r; uint8_t sendMsg[8];
  OutString("\n\rGet Status");
  BuildGetStatusMsg(sendMsg);
  r = AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  return (RecvBuf[4]<<24)+(RecvBuf[5]<<16)+(RecvBuf[6]<<8)+(RecvBuf[7]);
}

//*************BuildGetVersionMsg**************
// Create a Get Version message, used in Lab 6
// Inputs pointer to empty buffer of at least 6 bytes
// Output none
// build the necessary NPI message that will Get Status
void BuildGetVersionMsg(uint8_t *msg){
// hint: see NPI_GetVersion in AP.c
//****You implement this function as part of Lab 6*****
	//{SOF,0x00,0x00,0x35,0x03,0x36};
  uint8_t *ptr;
  ptr=msg;
	
	*msg=SOF;
	msg++;   // assign SOF then increment msg pointer
  *msg=0x00;
	msg++;  // LSB length=0, no payload
  *msg=0x00;
	msg++;  // MSB length	
  *msg=0x35;
	msg++;  // CMD0 from API table, page 11 of 41
  *msg=0x03;
	msg++;  // CMD1 from API table, page 11 of 41
  SetFCS(ptr);	
  
}
//*************Lab6_GetVersion**************
// Get version of the SNP application running on the CC2650, used in Lab 6
// Input:  none
// Output: version
uint32_t Lab6_GetVersion(void){volatile int r;uint8_t sendMsg[8];
  OutString("\n\rGet Version");
  BuildGetVersionMsg(sendMsg);
  r = AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE); 
  return (RecvBuf[5]<<8)+(RecvBuf[6]);
}

//*************BuildAddServiceMsg**************
// Create an Add service message, used in Lab 6
// Inputs uuid is 0xFFF0, 0xFFF1, ...
//        pointer to empty buffer of at least 9 bytes
// Output none
// build the necessary NPI message that will add a service
void BuildAddServiceMsg(uint16_t uuid, uint8_t *msg){
//****You implement this function as part of Lab 6*****
  uint8_t *ptr;
  ptr=msg;
	
	*msg=SOF;
	msg++;   // assign SOF then increment msg pointer
  *msg=0x03;
	msg++;  // LSB length=3 (Primary service message and 2 uuid blocks)
  *msg=0x00;
	msg++;  // MSB length	
  *msg=0x35;
	msg++;  // CMD0 from API table, page 11 of 41
  *msg=0x81;
	msg++;  // CMD1 from API table, page 11 of 41
	*msg=0x01;//Primary Serive
	msg++;
	*msg=0xFF&uuid;
	msg++;
	*msg=uuid>>8;
	msg++;
  SetFCS(ptr);	// msg is incremented. use ptr to point to beginning of msg

}
//*************Lab6_AddService**************
// Add a service, used in Lab 6
// Inputs uuid is 0xFFF0, 0xFFF1, ...
// Output APOK if successful,
//        APFAIL if SNP failure
int Lab6_AddService(uint16_t uuid){ int r; uint8_t sendMsg[12];
  OutString("\n\rAdd service");
  BuildAddServiceMsg(uuid,sendMsg);
  r = AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);  
  return r;
}
//*************AP_BuildRegisterServiceMsg**************
// Create a register service message, used in Lab 6
// Inputs pointer to empty buffer of at least 6 bytes
// Output none
// build the necessary NPI message that will register a service
void BuildRegisterServiceMsg(uint8_t *msg){
//****You implement this function as part of Lab 6*****
	uint8_t *ptr;
  ptr=msg;
	
	*msg=SOF;
	msg++;   // assign SOF then increment msg pointer
  *msg=0x00;
	msg++;  // LSB length=0, no payload
  *msg=0x00;
	msg++;  // MSB length	
  *msg=0x35;
	msg++;  // CMD0 from API table, page 11 of 41
  *msg=0x84;
	msg++;  // CMD1 from API table, page 11 of 41
  SetFCS(ptr);	
  
}
//*************Lab6_RegisterService**************
// Register a service, used in Lab 6
// Inputs none
// Output APOK if successful,
//        APFAIL if SNP failure
int Lab6_RegisterService(void){ int r; uint8_t sendMsg[8];
  OutString("\n\rRegister service");
  BuildRegisterServiceMsg(sendMsg);
  r = AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  return r;
}

//*************BuildAddCharValueMsg**************
// Create a Add Characteristic Value Declaration message, used in Lab 6
// Inputs uuid is 0xFFF0, 0xFFF1, ...
//        permission is GATT Permission, 0=none,1=read,2=write, 3=Read+write 
//        properties is GATT Properties, 2=read,8=write,0x0A=read+write, 0x10=notify
//        pointer to empty buffer of at least 14 bytes
// Output none
// build the necessary NPI message that will add a characteristic value
void BuildAddCharValueMsg(uint16_t uuid,  
  uint8_t permission, uint8_t properties, uint8_t *msg){
  const uint8_t NPI_AddCharValue[] = 
		{   
      SOF,0x08,0x00,  // length = 8
      0x35,0x82,      // SNP Add Characteristic Value Declaration
      0x03,           // 0=none,1=read,2=write, 3=Read+write, GATT Permission (byte 5)
      0x0A,           // 2=read,8=write,0x0A=read+write,0x10=notify, GATT Properties (byte 6)
      0x00,           // not used (byte 7)
      0x00,           // RFU  (byte 8)
      0x00,0x02,      // Maximum length of the attribute value=512  (byte 9-10)
      0xF1,0xFF,      // UUID  (byte 11-12)
      0xBA};          // FCS (calculated by AP_SendMessageResponse)
    uint8_t cnt = sizeof(NPI_AddCharValue)/sizeof(uint8_t);
    memcpy(msg, NPI_AddCharValue, cnt);
    msg[5] = permission;
    msg[6] = properties;
    msg[11] = 0xFF&uuid; 
		msg[12] = uuid>>8;
    SetFCS(msg);   
    
}

//*************BuildAddCharDescriptorMsg**************
// Create a Add Characteristic Descriptor Declaration message, used in Lab 6
// Inputs name is a null-terminated string, maximum length of name is 20 bytes
//        pointer to empty buffer of at least 14 bytes
// Output none
// build the necessary NPI message that will add a Descriptor Declaration
void BuildAddCharDescriptorMsg(char name[], uint8_t *msg){
// set length and maxlength to the string length
// set the permissions on the string to read
// for a hint see NPI_AddCharDescriptor in AP.c
// for a hint see second half of AP_AddCharacteristic
//****You implement this function as part of Lab 6*****
  uint8_t NPI_AddCharDescriptor[] = 
	{   
  SOF,0x17,0x00,  // length determined at run time 6+string length
  0x35,0x83,      // SNP Add Characteristic Descriptor Declaration
  0x80,           // User Description String
  0x01,           // GATT Read Permissions
  0x11,0x00,      // Maximum Possible length of the user description string
  0x11,0x00,      // Initial length of the user description string
  'C','h','a','r','a','c','t','e','r','i','s','t','i','c',' ','0',0}; // Initial user description string
	uint8_t SizeofName = strlen(name)+1;
	uint8_t cnt = sizeof(NPI_AddCharDescriptor)/sizeof(uint8_t);
  memcpy(msg, NPI_AddCharDescriptor, 11);
	*(msg+1) = 6+SizeofName;
	*(msg+7) = SizeofName;
	*(msg+9) = SizeofName;
	uint8_t i;

	for(i=0;i<SizeofName;i++)
	*(msg+11+i) = name[i];
	*(msg+11+SizeofName) = 0;
  SetFCS(msg);
  
}

//*************Lab6_AddCharacteristic**************
// Add a read, write, or read/write characteristic, used in Lab 6
//        for notify properties, call AP_AddNotifyCharacteristic 
// Inputs uuid is 0xFFF0, 0xFFF1, ...
//        thesize is the number of bytes in the user data 1,2,4, or 8 
//        pt is a pointer to the user data, stored little endian
//        permission is GATT Permission, 0=none,1=read,2=write, 3=Read+write 
//        properties is GATT Properties, 2=read,8=write,0x0A=read+write
//        name is a null-terminated string, maximum length of name is 20 bytes
//        (*ReadFunc) called before it responses with data from internal structure
//        (*WriteFunc) called after it accepts data into internal structure
// Output APOK if successful,
//        APFAIL if name is empty, more than 8 characteristics, or if SNP failure
int Lab6_AddCharacteristic(uint16_t uuid, uint16_t thesize, void *pt, uint8_t permission,
  uint8_t properties, char name[], void(*ReadFunc)(void), void(*WriteFunc)(void)){
  int r; uint16_t handle; 
  uint8_t sendMsg[32];  
  if(thesize>8) return APFAIL;
  if(name[0]==0) return APFAIL;       // empty name
  if(CharacteristicCount>=MAXCHARACTERISTICS) return APFAIL; // error
  BuildAddCharValueMsg(uuid,permission,properties,sendMsg);
  OutString("\n\rAdd CharValue");
  r=AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  if(r == APFAIL) return APFAIL;
  handle = (RecvBuf[7]<<8)+RecvBuf[6]; // handle for this characteristic
  OutString("\n\rAdd CharDescriptor");
  BuildAddCharDescriptorMsg(name,sendMsg);
  r=AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  if(r == APFAIL) return APFAIL;
  CharacteristicList[CharacteristicCount].theHandle = handle;
  CharacteristicList[CharacteristicCount].size = thesize;
  CharacteristicList[CharacteristicCount].pt = (uint8_t *) pt;
  CharacteristicList[CharacteristicCount].callBackRead = ReadFunc;
  CharacteristicList[CharacteristicCount].callBackWrite = WriteFunc;
  CharacteristicCount++;
  return APOK; // OK
} 
  

//*************BuildAddNotifyCharDescriptorMsg**************
// Create a Add Notify Characteristic Descriptor Declaration message, used in Lab 6
// Inputs name is a null-terminated string, maximum length of name is 20 bytes
//        pointer to empty buffer of at least 14 bytes
// Output none
// build the necessary NPI message that will add a Descriptor Declaration
void BuildAddNotifyCharDescriptorMsg(char name[], uint8_t *msg){
// set length and maxlength to the string length
// set the permissions on the string to read
// set User Description String
// set CCCD parameters read+write
// for a hint see NPI_AddCharDescriptor4 in VerySimpleApplicationProcessor.c
// for a hint see second half of AP_AddNotifyCharacteristic
//****You implement this function as part of Lab 6*****
	uint8_t NPI_AddCharDescriptor4[] = 
	{   
  SOF,12,0x00,    // length = 12
  0x35,0x83,      // SNP Add Characteristic Descriptor Declaration
  0x84,           // User Description String+CCCD
  0x03,           // CCCD parameters read+write
  0x01,           // GATT Read Permissions
  0x06,0x00,      // Maximum Possible length of the user description string
  0x06,0x00,      // Initial length of the user description string
  'C','o','u','n','t',0}; // Initial user description string
  //0x0E};  
  uint8_t SizeofName = strlen(name)+1;
	//uint8_t cnt = sizeof(NPI_AddCharDescriptor)/sizeof(uint8_t);
  memcpy(msg, NPI_AddCharDescriptor4, 12);
	*(msg+1) = 7+SizeofName;
	*(msg+8) = SizeofName;
	*(msg+10) = SizeofName;
	uint8_t i;

	for(i=0;i<SizeofName;i++)
	*(msg+12+i) = name[i];
	*(msg+12+SizeofName) = 0;
  SetFCS(msg);
}
  
//*************Lab6_AddNotifyCharacteristic**************
// Add a notify characteristic, used in Lab 6
//        for read, write, or read/write characteristic, call AP_AddCharacteristic 
// Inputs uuid is 0xFFF0, 0xFFF1, ...
//        thesize is the number of bytes in the user data 1,2,4, or 8 
//        pt is a pointer to the user data, stored little endian
//        name is a null-terminated string, maximum length of name is 20 bytes
//        (*CCCDfunc) called after it accepts , changing CCCDvalue
// Output APOK if successful,
//        APFAIL if name is empty, more than 4 notify characteristics, or if SNP failure
int Lab6_AddNotifyCharacteristic(uint16_t uuid, uint16_t thesize, void *pt,   
  char name[], void(*CCCDfunc)(void)){
  int r; uint16_t handle; 
  uint8_t sendMsg[32];  
  if(thesize>8) return APFAIL;
  if(NotifyCharacteristicCount>=NOTIFYMAXCHARACTERISTICS) return APFAIL; // error
  BuildAddCharValueMsg(uuid,0,0x10,sendMsg);
  OutString("\n\rAdd Notify CharValue");
  r=AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  if(r == APFAIL) return APFAIL;
  handle = (RecvBuf[7]<<8)+RecvBuf[6]; // handle for this characteristic
  OutString("\n\rAdd CharDescriptor");
  BuildAddNotifyCharDescriptorMsg(name,sendMsg);
  r=AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  if(r == APFAIL) return APFAIL;
  NotifyCharacteristicList[NotifyCharacteristicCount].uuid = uuid;
  NotifyCharacteristicList[NotifyCharacteristicCount].theHandle = handle;
  NotifyCharacteristicList[NotifyCharacteristicCount].CCCDhandle = (RecvBuf[8]<<8)+RecvBuf[7]; // handle for this CCCD
  NotifyCharacteristicList[NotifyCharacteristicCount].CCCDvalue = 0; // notify initially off
  NotifyCharacteristicList[NotifyCharacteristicCount].size = thesize;
  NotifyCharacteristicList[NotifyCharacteristicCount].pt = (uint8_t *) pt;
  NotifyCharacteristicList[NotifyCharacteristicCount].callBackCCCD = CCCDfunc;
  NotifyCharacteristicCount++;
  return APOK; // OK
}

//*************BuildSetDeviceNameMsg**************
// Create a Set GATT Parameter message, used in Lab 6
// Inputs name is a null-terminated string, maximum length of name is 24 bytes
//        pointer to empty buffer of at least 36 bytes
// Output none
// build the necessary NPI message to set Device name
void BuildSetDeviceNameMsg(char name[], uint8_t *msg){
	uint8_t iSize= 0;
	//find length of name to get message
	//add name to message
	for (iSize= 0; name[iSize]!= 0; iSize++)
		*(msg+ 8+ iSize)= name[iSize];

	*msg= SOF;
	*(msg+ 1)= iSize+ 3;
	*(msg+ 2)= 0x00,
	*(msg+ 3)= 0x35;
	*(msg+ 4)= 0x8C;
	*(msg+ 5)= 0x01;
	*(msg+ 6)= 0x00;
	*(msg+ 7)= 0x00;
	SetFCS(msg);
  
  
}
//*************BuildSetAdvertisementData1Msg**************
// Create a Set Advertisement Data message, used in Lab 6
// Inputs pointer to empty buffer of at least 16 bytes
// Output none
// build the necessary NPI message for Non-connectable Advertisement Data
void BuildSetAdvertisementData1Msg(uint8_t *msg){

 uint8_t NPI_SetAdvertisement1[] =
	{   
  SOF,11,0x00,    // length = 11
  0x55,0x43,      // SNP Set Advertisement Data
  0x01,           // Not connected Advertisement Data
  0x02,0x01,0x06, // GAP_ADTYPE_FLAGS,DISCOVERABLE | no BREDR
  0x06,0xFF,      // length, manufacturer specific
  0x0D ,0x00,     // Texas Instruments Company ID
  0x03,           // TI_ST_DEVICE_ID
  0x00,           // TI_ST_KEY_DATA_ID
  0x00};           // Key state
	uint8_t cnt = sizeof(NPI_SetAdvertisement1)/sizeof(uint8_t);
  memcpy(msg, NPI_SetAdvertisement1, cnt);
  SetFCS(msg);	
}

//*************BuildSetAdvertisementDataMsg**************
// Create a Set Advertisement Data message, used in Lab 6
// Inputs name is a null-terminated string, maximum length of name is 24 bytes
//        pointer to empty buffer of at least 36 bytes
// Output none
// build the necessary NPI message for Scan Response Data
void BuildSetAdvertisementDataMsg(char name[], uint8_t *msg){
uint8_t SizeofName = strlen(name);
uint8_t NPI_SetAdvertisementData[] = 
	{   
  SOF,31,0x00,    // length = 32
  0x55,0x43,      // SNP Set Advertisement Data
  0x00,           // Scan Response Data
  20,0x09,        // length, type=LOCAL_NAME_COMPLETE
  'S','h','a','p','e',' ','t','h','e',' ','W','o','r','l','d',' ','0','0','1',
// connection interval range
  0x05,           // length of this data
  0x12,           // GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE
  0x50,0x00,      // DEFAULT_DESIRED_MIN_CONN_INTERVAL
  0x20,0x03,      // DEFAULT_DESIRED_MAX_CONN_INTERVAL
// Tx power level
  0x02,           // length of this data
  0x0A,           // GAP_ADTYPE_POWER_LEVEL
  0x00};           // 0dBm

	uint8_t cnt = sizeof(NPI_SetAdvertisementData)/sizeof(uint8_t);
  memcpy(msg, NPI_SetAdvertisementData, 8);
	*(msg+1)= SizeofName + 11;
	*(msg+6)=SizeofName;
	uint8_t i;
	for(i=0;i<SizeofName;i++)
		*(msg+8+i) = name[i];
	*(msg+8+SizeofName) = 0x05; //SizeofName after loop 
	*(msg+9+SizeofName) = 0x12;
	*(msg+10+SizeofName) = 0x50;
	*(msg+11+SizeofName) = 0x00;
	*(msg+12+SizeofName) = 0x20;
	*(msg+13+SizeofName) = 0x03;
	*(msg+14+SizeofName) = 0x02;
	*(msg+15+SizeofName) = 0x0A;
	*(msg+16+SizeofName) = 0x00;
  SetFCS(msg);	
  
}
//*************BuildStartAdvertisementMsg**************
// Create a Start Advertisement Data message, used in Lab 6
// Inputs advertising interval
//        pointer to empty buffer of at least 20 bytes
// Output none
// build the necessary NPI message to start advertisement
void BuildStartAdvertisementMsg(uint16_t interval, uint8_t *msg){
//****You implement this function as part of Lab 6*****
  uint8_t NPI_StartAdvertisement[] = 
	{   
  SOF,14,0x00,    // length = 14
  0x55,0x42,      // SNP Start Advertisement
  0x00,           // Connectable Undirected Advertisements
  0x00,0x00,      // Advertise infinitely.
  0x64,0x00,      // Advertising Interval (100 * 0.625 ms=62.5ms)
  0x00,           // Filter Policy RFU
  0x00,           // Initiator Address Type RFU
  0x00,0x01,0x00,0x00,0x00,0xC5, // RFU
  0x02};           // Advertising will restart with connectable advertising when a connection is terminated
  
	
	uint8_t cnt = sizeof(NPI_StartAdvertisement)/sizeof(uint8_t);
  memcpy(msg, NPI_StartAdvertisement, cnt);
	*(msg+8) = interval;
	SetFCS(msg);
}
	
//*************Lab6_StartAdvertisement**************
// Start advertisement, used in Lab 6
// Input:  none
// Output: APOK if successful,
//         APFAIL if notification not configured, or if SNP failure
int Lab6_StartAdvertisement(void){volatile int r; uint8_t sendMsg[32];
  OutString("\n\rSet Device name");
  BuildSetDeviceNameMsg("Shape the World",sendMsg);
  r =AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  OutString("\n\rSetAdvertisement1");
  BuildSetAdvertisementData1Msg(sendMsg);
  r =AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  OutString("\n\rSetAdvertisement Data");
  BuildSetAdvertisementDataMsg("Shape the World",sendMsg);
  r =AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  OutString("\n\rStartAdvertisement");
  BuildStartAdvertisementMsg(100,sendMsg);
  r =AP_SendMessageResponse(sendMsg,RecvBuf,RECVSIZE);
  return r;
}