/*
 * odc_scandif.cpp
 *
 * Copyright (c) 2012 Oki Data Corporation. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>

#include "odc_scandif.h"

#include "odc_define.h"
#include "odc_boundary.h"
#include "odc_misc.h"
#include "odc_tracer.h"
using namespace odc;
#define TRCID "scandif"
#define TRACE(LEVEL, ERRNO, ...) T_TRACE((LEVEL), TRCID, ERRNO, __VA_ARGS__)
#define FATAL(ERRNO, ...) T_FATAL(TRCID, ERRNO, __VA_ARGS__)
#define ERR(ERRNO, ...) T_ERR(TRCID, ERRNO, __VA_ARGS__)
#define WRN(ERRNO, ...) T_WARN(TRCID, ERRNO, __VA_ARGS__)
#define INFOh(...) T_INFO_H(TRCID, __VA_ARGS__)
#define INFO(...) T_INFO(TRCID, __VA_ARGS__)
#define INFOl(...) T_INFO_L(TRCID, __VA_ARGS__)
#define MESSAGE(...) T_MESSAGE(TRCID, __VA_ARGS__)
#define DBG(...) T_DBG(TRCID, __VA_ARGS__)
#define TRC(...) T_TRC1(TRCID, __VA_ARGS__)
#define TRC2(...) T_TRC2(TRCID, __VA_ARGS__)
#define TRC3(...) T_TRC3(TRCID, __VA_ARGS__)
#define TRC4(...) T_TRC4(TRCID, __VA_ARGS__)
#define TMP(...) T_TMP(TRCID, 0, __VA_ARGS__)
#define DUMP(DATA, SIZE) T_DUMP(TRCID, DATA, SIZE)




void typetostr(msg_type_t t, char* s)
{
	const char* ts = "";
	if (isRequest(t)) {
		ts = "req";
	} else if(isResponse(t)) {
		ts = "resp";
	} else if(isIndicate(t)) {
		ts = "ind";
	} else if(isConfirm(t)) {
		ts = "conf";
	}
	switch (getMsgTypeValue(t)) {
	case GETDEVLIST:		sprintf(s, "GETDEVLIST.%s", ts);	break;
	case REGPUSHAP:		sprintf(s, "REGPUSHAP.%s", ts);	break;
	case PUSHSCAN:		sprintf(s, "PUSHSCAN.%s", ts);	break;
	case PUSHSCANCCL:	sprintf(s, "PUSHSCANCCL.%s", ts);	break;
	case SCAN:			sprintf(s, "SCAN.%s", ts);	break;
	case SCANDATA:		sprintf(s, "SCANDATA.%s", ts);	break;
	case SCANCONTINUE:	sprintf(s, "SCANCONTINUE.%s", ts);	break;
	case ERRORSTATUS:	sprintf(s, "ERRORSTATUS.%s", ts);	break;
	default:				sprintf(s, "[%d]", getMsgTypeValue(t));	break;
	}
}



LibStatus scandif_send(int sock, scand_msg_t* msg)
{
	int rc;

	DUMP(msg, MSGSIZE(*msg));

	rc = send(sock, msg, MSGSIZE(*msg), MSG_NOSIGNAL);
	if (rc < 0) {
		if (errno == EPIPE) {
			WRN(0, "disconnected.");
			return IOERROR;
		}
		ERR(errno, "send error.");
		return IOERROR;
	}

	return SUCCESS;
}


odc::LibStatus scandif_send(int sock, msg_header_t* h, unsigned char* data)
{
	int rc;

	DUMP(h, sizeof(msg_header_t));
	DUMP(data, h->data_length);

	rc = send(sock, h, sizeof(msg_header_t), MSG_NOSIGNAL);
	if (rc < 0) {
		if (errno == EPIPE) {
			WRN(0, "disconnected.");
			return IOERROR;
		}
		ERR(errno, "send error.");
		return IOERROR;
	}

	rc = send(sock, data, h->data_length, MSG_NOSIGNAL);
	if (rc < 0) {
		if (errno == EPIPE) {
			WRN(0, "disconnected.");
			return IOERROR;
		}
		ERR(errno, "send error.");
		return IOERROR;
	}

	return SUCCESS;
}


LibStatus scandif_recv(int sock, scand_msg_t* msg, unsigned char* data)
{
	int rc;
	int cnt;

	// 応答待ち
	for (cnt=0 ; cnt<SCANDIF_RECV_RETRY ; cnt++) {
		rc = recv(sock, &msg->h, sizeof(msg->h), 0);
		if (rc < 0) {
			if (errno == EAGAIN) {
				TRACE(TL_TRACE4, errno, "could not receive.");
				continue;
			} else if (errno == EINTR) {
				TRACE(TL_TRACE4, errno, "interrupted.");
				continue;
			} else {
				ERR(errno, "recv error.");
				return IOERROR;
			}
		} else if (rc == 0) {
			TRACE(TL_INFO, errno, "disconnected.");
			return DISCONNECTED;
		}
		break;
	}
	if (cnt == SCANDIF_RECV_RETRY) {
		DBG("recv retry-out.");
		return NODATA;
	}
	TRC("recv data len=%d", msg->h.data_length);

	if (msg->h.data_length > 0) {
		char* p;
		if (data) {
			p = reinterpret_cast<char*>(data);
		} else {
			p = reinterpret_cast<char*>(&msg->d);
		}
		size_t off = 0;
		size_t remain = msg->h.data_length;

		for (cnt=0 ; cnt<SCANDIF_RECV_RETRY ; cnt++) {
			rc = recv(sock, &p[off], remain, 0);
			if (rc < 0) {
				if (errno == EAGAIN) {
					TRACE(TL_INFO, errno, "could not receive.");
					continue;
				} else if (errno == EINTR) {
					TRACE(TL_INFO, errno, "interrupted.");
					return IOERROR;
				} else {
					ERR(errno, "recv error.");
					return IOERROR;
				}
			} else if (rc == 0) {
				WRN(errno, "disconnected.");
				return DISCONNECTED;
			}

			remain -= rc;
			off += rc;
			if (remain == 0) {
				DUMP(p, msg->h.data_length);
				return SUCCESS;
			}
		}

		DBG("retry-out.");
		return IOERROR;
	}

	return SUCCESS;
}



