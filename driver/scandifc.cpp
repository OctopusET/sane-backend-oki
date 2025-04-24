/*
 * scandifc.cpp
 *
 * Copyright (c) 2012 Oki Data Corporation. All rights reserved.
 */

/*
 * 注意：
 * スキャンdaemonインタフェースを用いるクライアント実装のための共通コードです。
 * スキャンdaemon側では使用しません。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>

#include "scandifc.h"
#include "odc_scandif.h"

#include "odc_define.h"
#include "odc_boundary.h"
#include "odc_misc.h"
#include "odc_pnm.h"
#include "odc_tracer.h"
using namespace odc;
#define TRCID "scandifc"
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



scan_param_t::scan_param_t() {
	memset(&this->scan_prm, 0, sizeof(this->scan_prm));
	memset(&this->output_path, 0, sizeof(this->output_path));
	this->filename = this->output_path;
	this->ext = this->filename;
}

int scan_param_t::set_dir(const char*dir)
{
	int l = strlen(this->filename);
	if (l > 0) {
		// 既にファイル名が設定されている。
		if (&this->filename[l+1] == this->ext) {
			//拡張子有り->一旦連結
			this->filename[l] = '.';
			l = strlen(this->filename);
		}

		char temp[l+1];
		memcpy(temp, this->filename, l+1);

		this->set_path(dir, temp);
	} else {
		int rc;
		size_t rem = sizeof(this->output_path);
		rc = snprintf(this->output_path, rem, "%s", dir);
		if (rem < rc+1) {
			return -1;
		}

		this->filename = &this->output_path[rc];
		this->ext = this->filename;
	}
	return 0;
}

int scan_param_t::set_filename(const char* filename)
{
	TRC("filename=%s", filename);
	int rc;
	size_t off = strlen(this->output_path);
	off++;
	size_t rem = sizeof(this->output_path);
	rem -= off;

	rc = snprintf(&this->output_path[off], rem, "%s", filename);
	if (rem < rc+1) {
		return -1;
	}
	this->filename = &this->output_path[off];

	int l = rc;
	int i;
	this->ext = &this->filename[l];
	for (i=l-1 ; i>=0 ; i--) {
		if (this->filename[i] == '.') {
			this->filename[i] = '\0';
			this->ext = &this->filename[i+1];
			break;
		}
	}

	return 0;
}

int scan_param_t::set_path(const char* path)
{
	int rc;
	size_t rem = sizeof(this->output_path);
	rc = snprintf(this->output_path, rem, "%s", path);
	if (rem < rc+1) {
		return -1;
	}

	int l = rc;
	int i;

	this->filename = this->output_path;
	for (i=l-1 ; i>=0 ; i--) {
		if (this->output_path[i] == '/') {
			this->output_path[i] = '\0';
			this->filename = &this->output_path[i+1];
			break;
		}
	}

	l = strlen(this->filename);
	this->ext = &this->filename[l];
	for (i=l-1 ; i>=0 ; i--) {
		if (this->filename[i] == '.') {
			this->filename[i] = '\0';
			this->ext = &this->filename[i+1];
			break;
		}
	}

	return 0;

}
int scan_param_t::set_path(const char* dir, const char* filename)
{
	int rc;
	size_t off = 0;
	size_t rem = sizeof(this->output_path);
	rc = snprintf(this->output_path, rem, "%s", dir);
	if (rem < rc+1) {
		return -1;
	}
	rem -= (rc + 1);
	off = rc + 1;
	rc = snprintf(&this->output_path[off], rem, "%s", filename);
	if (rem < rc+1) {
		return -1;
	}
	this->filename = &this->output_path[off];


	int l = rc;
	int i;
	for (i=l-1 ; i>=0 ; i--) {
		if (this->filename[i] == '.') {
			this->filename[i] = '\0';
			this->ext = &this->filename[i+1];
			return 0;
		}
	}
	// 拡張子がない
	this->ext = &this->filename[l];

	return 0;
}




scandif::scandif()
{
	this->sock = -1;
	this->canceled = false;
}

scandif::~scandif()
{
	this->close();
}

LibStatus scandif::open(int to_msec)
{
	int rc;

	this->sock = socket(PF_UNIX, SOCK_STREAM, 0);
	if (this->sock < 0) {
		ERR(errno, "socket error.");
		return ERROR;
	}

	memset(&this->addr, 0, sizeof(this->addr));
	this->addr.sun_family = AF_UNIX;
	strcpy(this->addr.sun_path, SCAND_UDSOCKNAME);

	rc = connect(this->sock, reinterpret_cast<struct sockaddr*>(&this->addr), sizeof(this->addr.sun_family) + strlen(this->addr.sun_path));
	if (rc < 0) {
		if (errno == ECONNREFUSED) {
			ERR(errno, "connect error. (path=%s)", this->addr.sun_path);
			return CONNECTION_REFUSED;
		} else {
			ERR(errno, "connect error. (path=%s)", this->addr.sun_path);
			return IOERROR;
		}
    }

	this->set_timeout(to_msec);

	return SUCCESS;
}

void scandif::close()
{
	if (this->sock >= 0) {
		::shutdown(this->sock, SHUT_RDWR);
		::close(this->sock);
	}
	this->sock = -1;
}

bool scandif::is_open()
{
	if (this->sock >= 0) {
		char c;
		int ret = ::recv(this->sock, &c, 1, MSG_PEEK|MSG_DONTWAIT);
		TRC4("ret=%d", ret);
		if (ret > 0) {
			return true;
		} else if (ret == 0) {
			DBG("detect disconnect.");
		} else {
			if (EAGAIN == errno) {
				// no data
				TRC4("no data.");
				return true;
			}
		}
		scandif::close();
	}
	TRC4("not opened.");
	return false;
}


bool scandif::set_timeout(int msec)
{
	if (msec >= 0) {
		this->recv_timeout.tv_sec = (int)(msec / 1000);
		this->recv_timeout.tv_usec = (int)((msec % 1000) * 1000);

		TRC("set timeout ---> sec=%d, usec=%d", this->recv_timeout.tv_sec, this->recv_timeout.tv_usec);

		if (setsockopt(this->sock, SOL_SOCKET, SO_RCVTIMEO, &this->recv_timeout, sizeof(this->recv_timeout)) == -1) {
			ERR(errno, "setsockopt error.");
			return ERROR;
		}
		return SUCCESS;
	} else {
		return INVVAL;
	}
}


LibStatus scandif::register_client(char* reguser)
{
	scand_msg_t msg;
	LibStatus lsts;

	uid_t uid = geteuid();
	struct passwd *pwd = getpwuid(uid);
	if (pwd) {
		TRC("username=%s", pwd->pw_name);
	} else {
		ERR(errno, "getpwuid(%d) error.", uid);
		return ERROR;
	}

	msg.h.pre = 0xff;
	msg.h.type = MsgType(REQUEST, REGPUSHAP);
	msg.h.data_length = sizeof(register_req_t);
	strcpy(msg.d.register_req.username, pwd->pw_name);


	// 要求送信
	lsts = scandif_send(this->sock, &msg);
	if (lsts != SUCCESS) {
		return lsts;
	}


	// 応答受信
	lsts = scandif_recv(this->sock, &msg);


	lsts = (LibStatus)(msg.d.register_resp.result);
	TRC("result=%s(%d)", strlibstatus(lsts), lsts);

	if (lsts == BUSY) {
		TRC("username=%s", msg.d.register_resp.username);
		strncpy(reguser, msg.d.register_resp.username, sizeof(msg.d.register_resp.username));
	} else {
		reguser[0] = '\0';
	}

	return lsts;
}


LibStatus scandif::receive_message(scand_msg_t* msg, unsigned char* data/*=NULL*/)
{
	return scandif_recv(this->sock, msg, data);
}


LibStatus scandif::send_message(scand_msg_t* msg)
{
	return scandif_send(this->sock, msg);
}


LibStatus scandif::scan(scan_param_t* param, scan_callback_t* callback)
{
	int rc;
	LibStatus lsts;
	const char* dir = NULL;
	const char* filename = NULL;
	const char* ext = NULL;
	char path[4096];
	int pagecnt = 0;
	unsigned char buf[4096];
	size_t total_read;
	bool use_stdout = false;

	scand_msg_t msg;
	unsigned char scdata[param->scan_prm.max_trans_size];
	scand_data_t* dp = reinterpret_cast<scand_data_t*>(scdata);
	bool has_more_docs = false;
	uint8_t used_src = 0;
	size_t total_image_size;
	XY<int> image_size;


	PNMWriter* wp;
	PPMWriter ppmw;
	PGMWriter pgmw;
	PBMWriter pbmw;


	/*
	 * キャンセルフラグクリア
	 */
	this->canceled = false;



	/*
	 * 出力先指定判定
	 */
	if (strlen(param->output_path) > 0) {
		dir = param->output_path;
	}
	if (strlen(param->filename) > 0) {
		filename = param->filename;
	}
	if (strlen(param->ext) > 0) {
		ext = param->ext;
	}
	if (dir == NULL && filename == NULL) {
		use_stdout = true;
	} else {
		if (dir == NULL) {
			dir = DEFAULT_OUTPUT_DIR;
		}
		if (filename == NULL) {
			filename = DEFAULT_OUTPUT_FILENAME;
		}
	}
	/**/

	switch (param->scan_prm.mode) {
	case 0:
		wp = &ppmw;
		if (ext == NULL) ext = "ppm";
		break;
	case 1:
		wp = &pgmw;
		if (ext == NULL) ext = "pgm";
		break;
	case 2:
		wp = &pbmw;
		if (ext == NULL) ext = "pbm";
		break;
	default:
		return ERROR;
	}


	/*
	 * 経過通知情報
	 */
	scan_callback_t::notify_t m;

	while (true) {
		if (this->canceled) {
			INFO("Canceled.");
			lsts = CANCELED;
			break;
		}

		pagecnt ++;

		if (pagecnt == 1) {
			// SCANリクエスト送信
			msg.h.pre = 0xff;
			msg.h.type = MsgType(REQUEST, SCAN);
			msg.h.data_length = sizeof(param->scan_prm);
			memcpy(&msg.d.scan_req, &param->scan_prm, sizeof(param->scan_prm));

			lsts = scandif_send(this->sock, &msg);
			if (lsts != SUCCESS) {
				return lsts;
			}


			// 応答待ち
			while(true) {
				lsts = scandif_recv(this->sock, &msg);
				if (lsts == SUCCESS) {
					break;
				} else if (lsts == NODATA) {
					//continue;
				} else {
					return lsts;
				}
			}

			enum MsgTypeValue mtype = getMsgTypeValue(msg.h.type);
			if (mtype == SCAN) {
				TRC("p=%d", msg.d.scan_resp.p);
				TRC("src=%d", msg.d.scan_resp.src);
				TRC("size_bytes=%d", msg.d.scan_resp.size_bytes);
				TRC("width=%d", msg.d.scan_resp.width);
				TRC("height=%d", msg.d.scan_resp.height);

				total_image_size = msg.d.scan_resp.size_bytes;
				image_size.set(msg.d.scan_resp.width, msg.d.scan_resp.height);

				used_src = msg.d.scan_resp.src;
			} else if (mtype == ERRORSTATUS) {
				TRC("ERRORSTATUS.ind error_status=%s(%d)", strlibstatus((LibStatus)msg.d.error_ind.error_status), msg.d.error_ind.error_status);
				return (LibStatus)msg.d.error_ind.error_status;
			} else {
				// Illegal response
				WRN(0, "illegal message received. (type=%02x)", msg.h.type);
				return IOERROR;
			}
		} else {
			// continue
			DBG("continue");

			// CONTINUEリクエスト送信
			msg.h.pre = 0xff;
			msg.h.type = MsgType(REQUEST, SCANCONTINUE);
			msg.h.data_length = 0;

			lsts = scandif_send(this->sock, &msg);
			if (lsts != SUCCESS) {
				return lsts;
			}

			while (true) {
				// 応答待ち
				lsts = scandif_recv(this->sock, &msg);
				if (lsts != SUCCESS) {
					return lsts;
				}

				enum MsgTypeValue mtype = getMsgTypeValue(msg.h.type);
				if (mtype == SCANCONTINUE) {
					// OK
					break;
				} else if (mtype == SCANDATA) {
					// ?
					if (msg.d.scandata_ind.last == 1 && msg.d.scandata_ind.data_size == 0) {
						// 無視
					} else {
						WRN(0, "SCANDATA.ind received. (last=%d, data_size=%d)", msg.d.scandata_ind.last, msg.d.scandata_ind.data_size);
					}
					continue;
				} else if (mtype == ERRORSTATUS) {
					// Error
					TRC("ERRORSTATUS.ind error_status=%s(%d)", strlibstatus((LibStatus)msg.d.error_ind.error_status), msg.d.error_ind.error_status);
					return (LibStatus)msg.d.error_ind.error_status;
				} else {
					// Illegal response
					WRN(0, "illegal message received. (type=%02x)", msg.h.type);
					return IOERROR;
				}
			}
		}


		if (use_stdout) {
			rc = wp->open(stdout, image_size.x, image_size.y);
			if (rc < 0) {
				lsts = FIOERROR;
				break;
			}
		} else {
			if (used_src == 2/*ADF*/) {
				sprintf(path, "%s/%s[%d].%s", dir, filename, pagecnt, ext);
				rc = wp->open(path, image_size.x, image_size.y);
				if (rc < 0) {
					lsts = FIOERROR;
					break;
				}
			} else {
				sprintf(path, "%s/%s.%s", dir, filename, ext);
				rc = wp->open(path, image_size.x, image_size.y);
				if (rc < 0) {
					lsts = FIOERROR;
					break;
				}
			}
		}




		if (callback) {
			m.type = scan_callback_t::notify_t::SCAN_BEGIN;
			m.data.scan_prog.source = used_src;
			m.data.scan_prog.page = pagecnt;
			m.data.scan_prog.status = BUSY;
			m.data.scan_prog.total = total_image_size;
			m.data.scan_prog.transferred = 0;
			m.data.scan_prog.prog = 0;
			m.data.scan_prog.path = path;
			callback->notify(&m);

			m.type = scan_callback_t::notify_t::SCAN_PROGRESS;
		}

		for (lsts = SUCCESS, total_read=0 ; lsts == SUCCESS ; ) {
			if (this->canceled) {
				lsts = CANCELED;
				INFO("Canceled.");
				break;
			}


			lsts = scandif_recv(this->sock, &msg, scdata);
			if (lsts != SUCCESS) {
				return lsts;
			}

			enum MsgTypeValue mt = getMsgTypeValue(msg.h.type);
			if (mt == ERRORSTATUS) {
				TRC("ERRORSTATUS.ind error_status=%s(%d)", strlibstatus((LibStatus)dp->error_ind.error_status), dp->error_ind.error_status);
				return (LibStatus)dp->error_ind.error_status;
			} else if (mt != SCANDATA) {
				ERR(0, "illegal message received. (type=%02x)", msg.h.type);
				return IOERROR;
			}


			TRC4("data-size=%d", dp->scandata_ind.data_size);
			TRC4("last=%d", dp->scandata_ind.last);


			rc = wp->write(dp->scandata_ind.scan_data, dp->scandata_ind.data_size);
			if (rc < 0) {
				lsts = FIOERROR;
			}
			total_read += dp->scandata_ind.data_size;
//			if (used_src == 2/*ADF*/) {
//				fprintf(stderr, "#%d ", pagecnt);
//			}
//			fprintf(stderr, "scanning... (%5.1f%%)   \r", total_read/((double)total_image_size)*100);
			if (callback) {
				m.data.scan_prog.transferred = total_read;
				m.data.scan_prog.prog = total_read/((double)total_image_size)*100;
				callback->notify(&m);
			}

			if (dp->scandata_ind.last) {
				INFO("EOP");
				lsts = END_OF_PAGE;

				has_more_docs = (dp->scandata_ind.has_more_docs);
			//	if (used_src == 2/*ADF*/) {
//					if (has_more_docs) {
//						fprintf(stderr, "\n\nhas more documents.\n\n");
//					}
			//	}
			}
		}

		wp->close();

		TRC4("lsts=%s [%d]", strlibstatus(lsts), lsts);
		if (callback) {
			m.data.scan_prog.status = lsts;
			m.data.scan_prog.transferred = total_read;
			m.data.scan_prog.prog = total_read/((double)total_image_size)*100;
			callback->notify(&m);
		}
		if (lsts == SUCCESS) {
		} else if (lsts == END_OF_PAGE) {
//			if (used_src == 2/*ADF*/) {
//				fprintf(stderr, "#%d ", pagecnt);
//			}
//			fprintf(stderr, "scanning finished.         \n");
			lsts = SUCCESS;
		} else if (lsts == CANCELED) {
//			if (used_src == 2/*ADF*/) {
//				fprintf(stderr, "#%d ", pagecnt);
//			}
//			fprintf(stderr, "scanning canceled.         \n");
			break;
		} else  {
//			if (used_src == 2/*ADF*/) {
//				fprintf(stderr, "#%d ", pagecnt);
//			}
//			fprintf(stderr, "scanning error. %s         \n");
			break;
		}

		if (used_src == 2/*ADF*/) {
			if (has_more_docs == false) {
				lsts = SUCCESS;
				break;
			}
		} else {
			break;
		}
	}

	if (callback) {
		m.type = scan_callback_t::notify_t::SCAN_FINISHED;
		callback->notify(&m);
	}

	TRC("lsts=%d, this=%p", lsts, this);
	return lsts;
}


void scandif::cancel_scan()
{
	TRC("this=%p", this);
	this->canceled = true;
}






