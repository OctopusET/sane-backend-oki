/*
 * odc_scandif.h
 *
 * Copyright (c) 2012 Oki Data Corporation. All rights reserved.
 */

#ifndef ODC_SCANDIF_H_
#define ODC_SCANDIF_H_
#include "devel/libstatus.h"

#include "devel/scandifdef.h"


/** 受信リトライ回数 */
#define SCANDIF_RECV_RETRY (4)

odc::LibStatus scandif_send(int sock, scand_msg_t* msg);
odc::LibStatus scandif_send(int sock, msg_header_t* h, unsigned char* data);
odc::LibStatus scandif_recv(int sock, scand_msg_t* msg, unsigned char* data=NULL);


#endif /* ODC_SCANDIF_H_ */
