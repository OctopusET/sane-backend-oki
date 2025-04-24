/*
 * scandifdef.h
 * scan daemon interface definitions.
 * Copyright (c) 2012 Oki Data Corporation. All rights reserved.
 */

#ifndef SCANDIF_H_
#define SCANDIF_H_
#include <stdint.h>


#define SCAND_UDSOCKNAME "/dev/"OEMNAME"scand"


enum MsgTypeKind
{
	REQUEST = 0x00,		
	RESPONSE = 0x80,	
	INDICATE = 0x40,	
	CONFIRM = 0xc0,		
};

enum MsgTypeValue
{
	GETDEVLIST = 1,		
	REGPUSHAP = 2,		
	PUSHSCAN = 3,		
	PUSHSCANCCL = 4,	
	SCAN = 5,			
	SCANDATA = 6,		
	SCANCONTINUE = 7,	
	KEEPALIVE = 8,		
	MODELINFO = 9,		
	ERRORSTATUS = 63,	
};


typedef unsigned char msg_type_t;
inline msg_type_t MsgType(enum MsgTypeKind k, enum MsgTypeValue v) {return (msg_type_t)(((k & 0x0C0) | (v & 0x3F)));}
inline enum MsgTypeValue getMsgTypeValue(msg_type_t t) {return (enum MsgTypeValue)(t & 0x3F);}
inline bool isRequest(msg_type_t t) {return ((t & 0x0c0) == REQUEST);}
inline bool isResponse(msg_type_t t) {return ((t & 0x0c0) == RESPONSE);}
inline bool isIndicate(msg_type_t t) {return ((t & 0x0c0) == INDICATE);}
inline bool isConfirm(msg_type_t t) {return ((t & 0x0c0) == CONFIRM);}
void typetostr(msg_type_t t, char* s);

struct msg_header_t
{
	uint8_t pre;		
	msg_type_t type;
	uint16_t data_length;
};


struct dev_data_t
{
	uint32_t adf : 2;
	uint32_t dc : 30;
	char vendor[32];
	char model[32];
	char name[32];
	char type[32];
};

struct devlist_resp_t
{
	uint8_t num;
	uint8_t dc[3];
	dev_data_t dev[0];
};

struct register_req_t
{
	char username[32];
};

struct register_resp_t
{
	uint32_t result;
	char username[32];
};

struct pushscan_ind_t
{
	uint32_t pushscanid;
	uint8_t action;
	uint8_t dc[3];
	char devname[32];
};

struct pushscan_conf_t
{
	uint32_t pushscanid;
	char devname[32];
	uint8_t result;
};

struct pushscanccl_ind_t
{
	uint32_t pushscanid;
	char devname[32];
};


struct scan_req_t
{
	uint32_t pushscanid;
	char devname[32];

	uint32_t p : 1;
	uint32_t src : 3;
	uint32_t mode : 4;

	uint32_t dc : 8;

	uint32_t dup : 2;
	uint32_t shp : 2;
	uint32_t bgelm : 3;
	uint32_t fs : 1;

	uint32_t me : 1;
	uint32_t edger : 7;

	float resolution;
	float lt_x;
	float lt_y;
	float width;
	float height;
	float paper_width;
	float paper_height;
	uint32_t max_trans_size;
};

struct scan_resp_t
{
	uint32_t p : 1;
	uint32_t src : 3;
	uint32_t dc : 28;

	uint32_t width;
	uint32_t height;
	uint32_t size_bytes;
};


struct scandata_ind_t
{
	uint16_t data_size;
	uint16_t last : 1;
	uint16_t has_more_docs : 1;
	uint16_t dc : 14;
	uint8_t scan_data[0];
};

struct modelinfo_req_t
{
	char model[32];
};

struct modelinfo_resp_t
{
	uint32_t network_support : 1;
	uint32_t dc : 31;
	char model[32];
	char vendor[32];
	char type[32];
	float max_paper_width;
	float max_paper_height;
	float scan_resolution_min;
	float scan_resolution_max;
	float scan_resolution_quant;
};


struct error_ind_t
{
	uint32_t error_status;
};


union scand_data_t {
	devlist_resp_t devlist_resp;
	register_req_t register_req;
	register_resp_t register_resp;
	pushscan_ind_t pushscan_ind;
	pushscan_conf_t pushscan_conf;
	pushscanccl_ind_t pushscanccl_ind;
	scan_req_t scan_req;
	scan_resp_t scan_resp;
	scandata_ind_t scandata_ind;
	modelinfo_req_t modelinfo_req;
	modelinfo_resp_t modelinfo_resp;
	error_ind_t error_ind;
};

struct scand_msg_t
{
	msg_header_t h;
	scand_data_t d;
};

#define MSGSIZE(MSG) (sizeof(msg_header_t) + (MSG).h.data_length)

#endif /* SCANDIF_H_ */
