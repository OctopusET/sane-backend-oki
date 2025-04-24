/*
    This file is part of the Scanner backend for SANE.
    Copyright (c) 2012 Oki Data Corporation. All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#define BUILD 0

#include "../include/config.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#include "../include/_stdint.h"

#include "../include/sane/sane.h"
extern "C" {
#include "../include/sane/sanei_debug.h"
#include "../include/sane/sanei.h"
#include "../include/sane/saneopts.h"
#include "../include/sane/sanei_config.h"
#include "../include/sane/sanei_backend.h"
}
extern int DBG_LEVEL;



#include "odc.h"
#include "odc_device.h"

#undef DBG 
#include "../driver/devel/tracer.h"
using namespace odc;
#define TRCID STRINGIFY(BACKEND_NAME)
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
#define DUMP(DATA, SIZE) T_DUMP(TRCID, DATA, SIZE)


static SANE_Bool inited = SANE_FALSE;


extern "C" {

int sttrclevel(SANE_Status st)
{
	switch(st) {
	case SANE_STATUS_GOOD:
		return TL_MESSAGE;
	case SANE_STATUS_UNSUPPORTED:	
		return TL_MESSAGE;
	case SANE_STATUS_CANCELLED:	
		return TL_MESSAGE;
	case SANE_STATUS_DEVICE_BUSY:	
		return TL_MESSAGE;
	case SANE_STATUS_INVAL:		
		return TL_WARNING;
	case SANE_STATUS_EOF:		
		return TL_MESSAGE;
	case SANE_STATUS_JAMMED:		
		return TL_MESSAGE;
	case SANE_STATUS_NO_DOCS:	
		return TL_MESSAGE;
	case SANE_STATUS_COVER_OPEN:	
		return TL_MESSAGE;
	case SANE_STATUS_IO_ERROR:	
		return TL_ERROR;
	case SANE_STATUS_NO_MEM:		
		return TL_ERROR;
	case SANE_STATUS_ACCESS_DENIED:	
		return TL_WARNING;
	default:
		return TL_FATAL;
	}
}

SANE_Status
sane_init (
		SANE_Int * __sane_unused__ version_code,
		SANE_Auth_Callback __sane_unused__ authorize
	)
{
	DBG_INIT ();
	INFO("sane_init:");
	T_INIT(DBG_LEVEL,NULL);

	if (version_code) {
		*version_code = SANE_VERSION_CODE (SANE_CURRENT_MAJOR, SANE_CURRENT_MINOR, BUILD);
	}

	if (inited) {
		WRN(0, "sane_init: warning: already inited");
	}

	SANE_Status sts;
	sts = device::SaneDevice::initDeviceList();
	if (sts != SANE_STATUS_GOOD) {
		ERR(0, "sane_init: initialize error.");
		return sts;
	}

	inited = SANE_TRUE;

	INFO("sane_init: OK");
	return SANE_STATUS_GOOD;
}

void
sane_exit (void)
{
	INFO("sane_exit:");

	device::SaneDevice::cleanDeviceList();

	inited = SANE_FALSE;

	INFO("sane_exit: OK");
	return;
}


SANE_Status
sane_get_devices (
		const SANE_Device *** device_list,
		SANE_Bool UNUSED(local_only)
	)
{
	INFO("sane_get_devices:");

	if (!inited) {
		ERR(0, "sane_get_devices: not inited, call sane_init() first");
		return SANE_STATUS_INVAL;
	}

	*device_list = reinterpret_cast<const SANE_Device **>(device::SaneDevice::getDevices());

	INFO("sane_get_devices: OK");
	return SANE_STATUS_GOOD;
}

SANE_Status
sane_open (SANE_String_Const devicename, SANE_Handle * handle)
{
	INFO("sane_open: devicename=%s", devicename);

	SANE_Status sts = SANE_STATUS_GOOD;
	do {
		*handle = NULL;

		device::SaneDevice* devp = NULL;
		if (strlen(devicename) == 0) {
			const device::SaneDevice** p = device::SaneDevice::getDevices();
			if (p != NULL) {
				devp = const_cast<device::SaneDevice*>(p[0]);
			}
			if (devp == NULL) {
				ERR(0, "sane_open: no devices found.");
				sts = SANE_STATUS_INVAL;
				break;
			}
		} else {
			devp = device::SaneDevice::findDevice(devicename);
			if (devp == NULL) {
				ERR(0, "sane_open: device `%s' not found", devicename);
				sts = SANE_STATUS_INVAL;
				break;
			}
		}

		sts = devp->open();
		if (sts != SANE_STATUS_GOOD) {
			ERR(0, "sane_open: device open error. device `%s' -- %s", devicename, sane_strstatus(sts));
			break;
		}

		*handle = devp;
	} while (false);

	return sts;
}

void
sane_close (SANE_Handle handle)
{
	INFO("sane_close: handle=%p", handle);

	device::SaneDevice *devp = device::SaneDevice::checkPointer(handle);
	if (devp == NULL) {
		ERR(0, "sane_close: illegal handle. handle=%p", handle);
		return;
	}

	devp->close();

	INFO("sane_close: OK");
	return;
}

const SANE_Option_Descriptor *
sane_get_option_descriptor (SANE_Handle handle, SANE_Int option)
{
#ifndef ENABLE_DBGTOOL
	INFO("sane_get_option_descriptor: handle=%p, option=%d", handle, option);
#endif

	if (!inited) {
		ERR(0, "sane_get_option_descriptor: not inited, call sane_init() first");
		return NULL;
	}

	device::SaneDevice *devp = device::SaneDevice::checkPointer(handle);
	if (devp == NULL) {
		ERR(0, "sane_get_option_descriptor: illegal handle. handle=%p", handle);
		return NULL;
	}

	return devp->get_option_descriptor(option);
}


SANE_Status
sane_control_option (SANE_Handle handle, SANE_Int option, SANE_Action action,
		     void *value, SANE_Int * info)
{
#ifndef ENABLE_DBGTOOL
	if (info != NULL) {
		INFO("sane_control_option: handle=%p, opt=%d, act=%d, val=%p(%d), info=%p(%x)",
				(void *) handle, option, action, (void *)value, *(int*)value, (void *)info, *info);
	} else {
		INFO("sane_control_option: handle=%p, opt=%d, act=%d, val=%p(%d), info=%p",
				(void *) handle, option, action, (void *)value, *(int*)value, (void *)info);
	}
#endif

	if (!inited) {
		ERR(0, "sane_control_option: not inited, call sane_init() first");
		return SANE_STATUS_INVAL;
	}


	device::SaneDevice *devp = device::SaneDevice::checkPointer(handle);
	if (devp == NULL) {
		ERR(0, "sane_control_option: illegal handle. handle=%p", handle);
		return SANE_STATUS_INVAL;
	}


	SANE_Status sts = devp->control_option(option, action, value, info);

#ifndef ENABLE_DBGTOOL
	if (info != NULL) {
		char b[256];
		device::DBG_InfoToString(*info, b);
		INFO("sane_control_option: finished(sts=%s), info=%x:%s", sane_strstatus(sts), *info, b);
	} else {
		INFO("sane_control_option: finished(sts=%s)", sane_strstatus(sts));
	}
#endif

	return sts;
}


SANE_Status
sane_get_parameters (SANE_Handle handle, SANE_Parameters * params)
{
	INFO("sane_get_parameters: handle=%p, params=%p", handle, params);

	if (!inited) {
		ERR(0, "sane_get_parameters: not inited, call sane_init() first");
		return SANE_STATUS_INVAL;
	}


	device::SaneDevice *devp = device::SaneDevice::checkPointer(handle);
	if (devp == NULL) {
		ERR(0, "sane_get_parameters: illegal handle. handle=%p", handle);
		return SANE_STATUS_INVAL;
	}

	SANE_Status status = devp->get_parameters(params);
	if (status != SANE_STATUS_GOOD) {
		TRACE((ODC_Trace_Level)sttrclevel(status), 0, "sane_get_parameters: error");
		return status;
	}

	if(T_ENA_INFO_L()) {
		SANE_Parameters *p = params;
		SANE_String_Const text_format;
		switch (p->format) {
		case SANE_FRAME_GRAY:
			text_format = "gray";
			break;
		case SANE_FRAME_RGB:
			text_format = "rgb";
			break;
		case SANE_FRAME_RED:
			text_format = "red";
			break;
		case SANE_FRAME_GREEN:
			text_format = "green";
			break;
		case SANE_FRAME_BLUE:
			text_format = "blue";
			break;
		default:
			text_format = "unknown";
			break;
		}
		INFOl("params->format=%d", p->format);
		INFOl("params->last_frame=%d", p->last_frame);
		INFOl("params->bytes_per_line=%d", p->bytes_per_line);
		INFOl("params->pixels_per_line=%d", p->pixels_per_line);
		INFOl("params->lines=%d", p->lines);
		INFOl("params->depth=%d", p->depth);
	}

	return SANE_STATUS_GOOD;
}


SANE_Status
sane_start (SANE_Handle handle)
{
	INFO("sane_start: handle=%p", handle);

	if (!inited) {
		ERR(0, "sane_start: not inited, call sane_init() first");
		return SANE_STATUS_INVAL;
	}


	device::SaneDevice *devp = device::SaneDevice::checkPointer(handle);
	if (devp == NULL) {
		ERR(0, "sane_start: illegal handle. handle=%p", handle);
		return SANE_STATUS_INVAL;
	}

	SANE_Status status = devp->start();
	if (status != SANE_STATUS_GOOD) {
		TRACE((ODC_Trace_Level)sttrclevel(status), 0, "sane_start: error -- %s", sane_strstatus(status));
		return status;
	}

	return SANE_STATUS_GOOD;
}


SANE_Status
sane_read (SANE_Handle handle, SANE_Byte * data,
	   SANE_Int max_length, SANE_Int * length)
{
	INFO("sane_read: handle=%p, data=%p, max_length = %d, length=%p", handle, data, max_length, (void *) length);

	if (!inited) {
		ERR(0, "sane_read: not inited, call sane_init() first");
		return SANE_STATUS_INVAL;
	}


	device::SaneDevice *devp = device::SaneDevice::checkPointer(handle);
	if (devp == NULL) {
		ERR(0, "sane_read: illegal handle. handle=%p", handle);
		return SANE_STATUS_INVAL;
	}

	SANE_Status status = devp->read(data, max_length, length);
	if (status != SANE_STATUS_GOOD) {
		TRACE((ODC_Trace_Level)sttrclevel(status), 0, "sane_read: error -- %s", sane_strstatus(status));
	}

	return status;
}


void
sane_cancel (SANE_Handle handle)
{
	INFO("sane_cancel: handle=%p", handle);

	if (!inited) {
		ERR(0, "sane_cancel: not inited, call sane_init() first");
		return;
	}


	device::SaneDevice *devp = device::SaneDevice::checkPointer(handle);
	if (devp == NULL) {
		ERR(0, "sane_cancel: illegal handle. handle=%p", handle);
		return;
	}

	devp->cancel();

	INFOl("sane_cancel: OK");
	return;
}


SANE_Status
sane_set_io_mode (SANE_Handle handle, SANE_Bool non_blocking)
{
	INFO("sane_set_io_mode: handle=%p", handle);

	if (!inited) {
		ERR(0, "sane_set_io_mode: not inited, call sane_init() first");
		return SANE_STATUS_INVAL;
	}


	device::SaneDevice *devp = device::SaneDevice::checkPointer(handle);
	if (devp == NULL) {
		ERR(0, "sane_set_io_mode: illegal handle. handle=%p", handle);
		return SANE_STATUS_INVAL;
	}

	SANE_Status status = devp->set_io_mode(non_blocking);
	if (status != SANE_STATUS_GOOD) {
		TRACE((ODC_Trace_Level)sttrclevel(status), 0, "sane_set_io_mode: error. %s", sane_strstatus(status));
		return status;
	}

	return SANE_STATUS_GOOD;
}


SANE_Status
sane_get_select_fd (SANE_Handle handle, SANE_Int * fd)
{
	INFO("sane_get_select_fd: handle=%p", handle);

	if (!inited) {
		ERR(0, "sane_get_select_fd: not inited, call sane_init() first");
		return SANE_STATUS_INVAL;
	}


	device::SaneDevice *devp = device::SaneDevice::checkPointer(handle);
	if (devp == NULL) {
		ERR(0, "sane_get_select_fd: illegal handle. handle=%p", handle);
		return SANE_STATUS_INVAL;
	}

	SANE_Status status = devp->get_select_fd(fd);
	if (status != SANE_STATUS_GOOD) {
		TRACE((ODC_Trace_Level)sttrclevel(status), 0, "sane_get_select_fd: %s.", sane_strstatus(status));
		return status;
	}

	return SANE_STATUS_GOOD;
}

}
