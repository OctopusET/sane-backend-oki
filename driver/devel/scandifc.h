/*
 * scandifc.h
 * scan daemon client API.
 * Copyright (c) 2012 Oki Data Corporation. All rights reserved.
 */

#ifndef SCANDIFC_H_
#define SCANDIFC_H_
#include <sys/socket.h>
#include <sys/un.h>

#include "libstatus.h"
#include "scandifdef.h"


#define DEFAULT_OUTPUT_DIR "."
#define DEFAULT_OUTPUT_FILENAME "out"


struct scan_param_t
{
	scan_req_t scan_prm;	
	char output_path[1024];	
	char* filename;	
	char* ext;	

	scan_param_t();
	int set_dir(const char*dir);
	int set_filename(const char* filename);
	int set_path(const char* path);
	int set_path(const char* dir, const char* filename);
};


struct scan_callback_t
{
	struct notify_t
	{
		enum {SCAN_BEGIN, SCAN_FINISHED, SCAN_PROGRESS} type;
		union {
			struct {
				int source;	
				int page;	
				odc::LibStatus status;	
				long total;	
				long transferred;	
				double prog;	
				const char* path;	
			} scan_prog;
		} data;
	};
	virtual void notify(notify_t* m) = 0;
	virtual ~scan_callback_t() {}
};



struct scandif
{
protected:
	int sock;
	struct sockaddr_un addr;
	bool canceled;
	struct timeval recv_timeout;

public:
	scandif();
	~scandif();

	odc::LibStatus open(int to_msec=0);
	void close();
	bool is_open();
	bool set_timeout(int msec);

	odc::LibStatus receive_message(scand_msg_t* msg, unsigned char* data=NULL);
	odc::LibStatus send_message(scand_msg_t* msg);

	odc::LibStatus register_client(char* reguser);
	odc::LibStatus scan(scan_param_t* param, scan_callback_t* callback=NULL);
	void cancel_scan();
};


#endif /* SCANDIFC_H_ */
