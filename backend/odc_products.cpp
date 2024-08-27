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

#include "../include/config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

#include "../include/_stdint.h"

#include "../include/sane/sane.h"
extern "C" {
#include "../include/sane/sanei.h"
#include "../include/sane/saneopts.h"
#include "../include/sane/sanei_config.h"
}


#include "odc.h"

#include "odc_device.h"
#include "odc_products.h"


#include "../driver/devel/misc.h"
#include "../driver/devel/pnm.h"
#include "../driver/devel/tracer.h"
using namespace odc;
#define TRCID STRINGIFY(BACKEND_NAME)":products"
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

#define CANCEL_SIG_WAIT_MAX	(60)


namespace products {



DeviceBaseImpl::ScanResolution::ScanResolution(AbstractDevice* owner, const char* name, const char* title, const char* desc) :
		OptionFixed(name, title, desc, 0, SANE_UNIT_DPI)
{
	this->owner = owner;
	this->set_info(SANE_INFO_RELOAD_PARAMS);


	this->range.min = SANE_FIX(50.0);
	this->range.max = SANE_FIX(1200.0);
	this->range.quant = SANE_FIX(1.0);
	this->set_constraint(&this->range);
	this->set_value_w(this->range.min);
}

SANE_Status DeviceBaseImpl::ScanResolution::set_value(void *value, SANE_Int * info)
{
	double res = SANE_UNFIX(*(SANE_Fixed *) value);
	SANE_Word setw = *(SANE_Word*) value;
	SANE_Int rinfo = 0;
	const int minidx = 1;
	if (res == SANE_FIX(setw)) {
		return OptionFixed::set_value(value, info);
	} else {
		*(SANE_Fixed *) value = setw;
		SANE_Status rs = OptionFixed::set_value(value, info);
		if (info != NULL)
			*info |= rinfo;
		return rs;
	}
}

SANE_Status DeviceBaseImpl::ScanResolution::get_value(void *value)
{
	SANE_Status rs = OptionFixed::get_value(value);
	return rs;
}



DeviceBaseImpl::ScanSource::ScanSource(AbstractDevice* owner) :
		OptionString(FX750_NAME_SCAN_SOURCE, FX750_TITLE_SCAN_SOURCE, FX750_DESC_SCAN_SOURCE, source_list, source_list[0])
{
	this->owner = owner;
	this->set_info(SANE_INFO_INEXACT|SANE_INFO_RELOAD_OPTIONS);
}

SANE_Status DeviceBaseImpl::ScanSource::set_value(void *value, SANE_Int * info)
{
	TRC(">>> '%s'", (SANE_String)value);
	DeviceBaseImpl* p = dynamic_cast<DeviceBaseImpl*>(this->owner);
	SANE_Status sts = OptionString::set_value(value, info);
	if (strcmp(p->o_scan_source.val.s, FX750_VALUE_SCAN_SOURCE_AUTO) == 0 ||
			strcmp(p->o_scan_source.val.s, FX750_VALUE_SCAN_SOURCE_ADF) == 0) {
		TRC(">>> %u", p->options[opt_duplex_scan_onoff]->val.b);
		p->o_duplex_scan_onoff.set_soft_select();
		if (p->o_duplex_scan_onoff.val.b) {
			p->o_binding.set_soft_select();
		} else {
			p->o_binding.set_hard_select();
		}
	} else {
		p->o_duplex_scan_onoff.set_hard_select();
		p->o_binding.set_hard_select();
	}
	return sts;
}


DeviceBaseImpl::DuplexScanningOnOff::DuplexScanningOnOff(AbstractDevice* owner) :
		OptionBool(FX750_NAME_DUPSCAN, FX750_TITLE_DUPSCAN, FX750_DESC_DUPSCAN, SANE_FALSE)
{
	this->owner = owner;
	this->set_info(SANE_INFO_INEXACT|SANE_INFO_RELOAD_OPTIONS);
}

SANE_Status DeviceBaseImpl::DuplexScanningOnOff::set_value(void *value, SANE_Int * info)
{
	DeviceBaseImpl* p = dynamic_cast<DeviceBaseImpl*>(this->owner);
	SANE_Status sts = OptionBool::set_value(value, info);
	if (*(SANE_Bool*)value) {
		p->o_binding.set_soft_select();
	} else {
		p->o_binding.set_hard_select();
	}
	return sts;
}


DeviceBaseImpl::EdgeEraseOnOff::EdgeEraseOnOff(AbstractDevice* owner) :
		OptionBool(FX750_NAME_EDGEERACE, FX750_TITLE_EDGEERACE, FX750_DESC_EDGEERACE, SANE_FALSE)
{
	this->owner = owner;
	this->set_info(SANE_INFO_INEXACT|SANE_INFO_RELOAD_OPTIONS);
}

SANE_Status DeviceBaseImpl::EdgeEraseOnOff::set_value(void *value, SANE_Int * info)
{
	DeviceBaseImpl* p = dynamic_cast<DeviceBaseImpl*>(this->owner);
	SANE_Status sts = OptionBool::set_value(value, info);
	if (*(SANE_Bool*)value) {
		p->o_edge_erase_range.set_soft_select();
	} else {
		p->o_edge_erase_range.set_hard_select();
	}
	return sts;
}



DeviceBaseImpl::CalibrationButton::CalibrationButton(AbstractDevice* owner) :
				OptionButton(FX750_NAME_CALIBRATION, FX750_TITLE_CALIBRATION, FX750_DESC_CALIBRATION)
{
	this->owner = owner;
	this->desc.cap = SANE_CAP_SOFT_SELECT|SANE_CAP_SOFT_DETECT|SANE_CAP_ADVANCED;
}
SANE_Status DeviceBaseImpl::CalibrationButton::set_value(void *value, SANE_Int * info)
{
	return (dynamic_cast<DeviceBaseImpl*>(this->owner))->calibration();
}



SANE_String_Const DeviceBaseImpl::source_list[] = {
	FX750_VALUE_SCAN_SOURCE_AUTO,
	FX750_VALUE_SCAN_SOURCE_FLATBED,
	FX750_VALUE_SCAN_SOURCE_ADF,
	0
};

SANE_String_Const DeviceBaseImpl::mode_list[] = {
	FX750_VALUE_SCAN_MODE_COLOR,
	FX750_VALUE_SCAN_MODE_GRAY,
	FX750_VALUE_SCAN_MODE_BW,
	0
};


SANE_String_Const DeviceBaseImpl::sharpness_list[] = {
	FX750_VALUE_SHARPNESS_NONE,
	FX750_VALUE_SHARPNESS_SHARPEN,
	FX750_VALUE_SHARPNESS_SHARPEN_MORE,
	0
};

SANE_String_Const DeviceBaseImpl::binding_list[] = {
	FX750_VALUE_BINDING_RIGHTLEFT,
	FX750_VALUE_BINDING_TOP,
	0
};

SANE_Range DeviceBaseImpl::edgeerase_range = {
	SANE_FIX(5.0),
	SANE_FIX(50.0),
	SANE_FIX(1.0)
};

SANE_String_Const DeviceBaseImpl::bgeliminat_list[] = {
	FX750_VALUE_BGELIMINAT_LEVEL0,
	FX750_VALUE_BGELIMINAT_LEVEL1,
	FX750_VALUE_BGELIMINAT_LEVEL2,
	FX750_VALUE_BGELIMINAT_LEVEL3,
	FX750_VALUE_BGELIMINAT_LEVEL4,
	FX750_VALUE_BGELIMINAT_LEVEL5,
	FX750_VALUE_BGELIMINAT_LEVEL6,
	0
};

SANE_Range DeviceBaseImpl::margin_range = {
	SANE_FIX(0.0),
	SANE_FIX(25.0),
	SANE_FIX(1.0)
};




DeviceBaseImpl::DeviceBaseImpl(SANE_Device* sane_device) : AbstractDevice(sane_device)
		,o_numopts(num_options)
		,o_scan_source(this)
		,o_mode(FX750_NAME_SCAN_MODE, FX750_TITLE_SCAN_MODE, FX750_DESC_SCAN_MODE, mode_list, mode_list[0])
		,o_x_resolution(this, FX750_NAME_SCAN_RESOLUTION, FX750_TITLE_SCAN_RESOLUTION, FX750_DESC_SCAN_RESOLUTION)
		,o_preview_mode(FX750_NAME_PREVIEW_MODE, FX750_TITLE_PREVIEW_MODE, FX750_DESC_PREVIEW_MODE, SANE_FALSE)
		,o_dupscan_group(FX750_NAME_DUPSCAN_GROUP, FX750_TITLE_DUPSCAN_GROUP, "")
		,o_duplex_scan_onoff(this)
		,o_binding(FX750_NAME_BINDING_DIR, FX750_TITLE_BINDING_DIR, FX750_DESC_BINDING_DIR, binding_list, binding_list[0])
		,o_filter_group(FX750_NAME_FILTER_GROUP, FX750_TITLE_FILTER_GROUP, "")
		,o_sharpness(FX750_NAME_SHARPNESS, FX750_TITLE_SHARPNESS, FX750_DESC_SHARPNESS, sharpness_list, sharpness_list[0])
		,o_bg_eliminat_level(FX750_NAME_BGELIMINAT, FX750_TITLE_BGELIMINAT, FX750_DESC_BGELIMINAT, bgeliminat_list, bgeliminat_list[0])
		,o_edge_erase_onoff(this)
		,o_edge_erase_range(FX750_NAME_EDGEERACE_RANGE, FX750_TITLE_EDGEERACE_RANGE, FX750_DESC_EDGEERACE_RANGE, &edgeerase_range, edgeerase_range.min, SANE_UNIT_MM)
		,o_font_smoothing_onoff(FX750_NAME_FONTSMOOTHING, FX750_TITLE_FONTSMOOTHING, FX750_DESC_FONTSMOOTHING, SANE_FALSE)
		,o_moire_elimination_onoff(FX750_NAME_MOIREELIMINATION, FX750_TITLE_MOIREELIMINATION, FX750_DESC_MOIREELIMINATION, SANE_FALSE)
		,o_tlX(FX750_NAME_SCAN_TL_X, FX750_TITLE_SCAN_TL_X, FX750_DESC_SCAN_TL_X, 0, SANE_UNIT_MM)
		,o_tlY(FX750_NAME_SCAN_TL_Y, FX750_TITLE_SCAN_TL_Y, FX750_DESC_SCAN_TL_Y, 0, SANE_UNIT_MM)
		,o_brX(FX750_NAME_SCAN_BR_X, FX750_TITLE_SCAN_BR_X, FX750_DESC_SCAN_BR_X, 0, SANE_UNIT_MM)
		,o_brY(FX750_NAME_SCAN_BR_Y, FX750_TITLE_SCAN_BR_Y, FX750_DESC_SCAN_BR_Y, 0, SANE_UNIT_MM)
#if defined(ENABLE_MARGIN)
		,o_margin_group(FX750_NAME_MARGIN_GROUP, FX750_TITLE_MARGIN_GROUP, "")
		,o_margin_right(FX750_NAME_MARGIN_RIGHT, FX750_TITLE_MARGIN_RIGHT, FX750_DESC_MARGIN_RIGHT, &margin_range, margin_range.min, SANE_UNIT_MM)
		,o_margin_bottom(FX750_NAME_MARGIN_BOTTOM, FX750_TITLE_MARGIN_BOTTOM, FX750_DESC_MARGIN_BOTTOM, &margin_range, margin_range.min, SANE_UNIT_MM)
#endif /*defined(ENABLE_MARGIN)*/
{
	TRC("construction");

	this->use_adf = false;
	this->non_blocking = false;

	this->scanning = false;
	this->canceled = false;
	this->return_eof = false;

	this->number_of_options = num_options;
	this->options = this->o_list;
	this->options[opt_num_opts] = &this->o_numopts;
	this->options[opt_std_group] = &this->o_com_grp_standard;
	this->options[opt_scan_source] = &this->o_scan_source;
	this->options[opt_mode] = &this->o_mode;
	this->o_mode.set_info(SANE_INFO_INEXACT|SANE_INFO_RELOAD_OPTIONS|SANE_INFO_RELOAD_PARAMS);
	this->options[opt_resolution] = &this->o_x_resolution;
	this->options[opt_preview_mode] = &this->o_preview_mode;

	this->options[opt_dupscan_group] = &this->o_dupscan_group;
	this->options[opt_duplex_scan_onoff] = &this->o_duplex_scan_onoff;
	this->options[opt_binding] = &this->o_binding;
	this->o_duplex_scan_onoff.set_soft_select();
	this->o_binding.set_hard_select();

	this->options[opt_filter_group] = &this->o_filter_group;
	this->options[opt_sharpness] = &this->o_sharpness;
	this->options[opt_bg_eliminat_level] = &this->o_bg_eliminat_level;
	this->options[opt_edge_erase_onoff] = &this->o_edge_erase_onoff;
	this->options[opt_edge_erase_range] = &this->o_edge_erase_range;
	this->o_edge_erase_range.set_hard_select();
	this->options[opt_font_smoothing_onoff] = &this->o_font_smoothing_onoff;
	this->options[opt_moire_elimination_onoff] = &this->o_moire_elimination_onoff;


	this->options[opt_geometry_group] = &this->o_com_grp_geometry;
	this->options[opt_tl_x] = &this->o_tlX;
	this->options[opt_tl_y] = &this->o_tlY;
	this->options[opt_br_x] = &this->o_brX;
	this->options[opt_br_y] = &this->o_brY;
	this->o_tlX.set_info(SANE_INFO_RELOAD_PARAMS);
	this->o_tlY.set_info(SANE_INFO_RELOAD_PARAMS);
	this->o_brX.set_info(SANE_INFO_RELOAD_PARAMS);
	this->o_brY.set_info(SANE_INFO_RELOAD_PARAMS);

#if defined(ENABLE_MARGIN)
	this->options[opt_margin_group] = &this->o_margin_group;
	this->options[opt_margin_right] = &this->o_margin_right;
	this->options[opt_margin_bottom] = &this->o_margin_bottom;
#endif /*defined(ENABLE_MARGIN)*/

}

DeviceBaseImpl::~DeviceBaseImpl()
{
	TRC("~DeviceBaseImpl:");
	TRC("~DeviceBaseImpl: OK");
}


SANE_Status DeviceBaseImpl::configure()
{
	TRC("start configure...");
	AbstractDevice::configure();

	this->geometry_x_range.min = SANE_FIX(0.0);
	this->geometry_x_range.max = SANE_FIX(210.0);
	this->geometry_x_range.quant = SANE_FIX(1.0);
	this->geometry_y_range.min = SANE_FIX(0.0);
	this->geometry_y_range.max = SANE_FIX(297.0);
	this->geometry_y_range.quant = SANE_FIX(1.0);


	LibStatus lsts;
	scandif scif;
	scand_msg_t msg;

	lsts = scif.open(4000);
	if (lsts != SUCCESS) {
		return SANE_STATUS_INVAL;
	}


	msg.h.pre = 0xff;
	msg.h.type = MsgType(REQUEST, MODELINFO);
	msg.h.data_length = sizeof(modelinfo_req_t);
	strncpy(msg.d.modelinfo_req.model, this->sane_device->model, sizeof(msg.d.modelinfo_req.model));
	lsts = scif.send_message(&msg);
	if (lsts != SUCCESS) {
		return SANE_STATUS_INVAL;
	}

	lsts = scif.receive_message(&msg);
	if (lsts == SUCCESS) {
		if (msg.h.type == MsgType(RESPONSE, MODELINFO)) {

			this->geometry_x_range.min = SANE_FIX(0.0);
			this->geometry_x_range.max = SANE_FIX(msg.d.modelinfo_resp.max_paper_width);
			this->geometry_x_range.quant = SANE_FIX(1.0);
			this->geometry_y_range.min = SANE_FIX(0.0);
			this->geometry_y_range.max = SANE_FIX(msg.d.modelinfo_resp.max_paper_height);
			this->geometry_y_range.quant = SANE_FIX(1.0);

			this->o_x_resolution.range.min = SANE_FIX(msg.d.modelinfo_resp.scan_resolution_min);
			this->o_x_resolution.range.max = SANE_FIX(msg.d.modelinfo_resp.scan_resolution_max);
			this->o_x_resolution.range.quant = SANE_FIX(msg.d.modelinfo_resp.scan_resolution_quant);

		} else if (msg.h.type == MsgType(INDICATE, ERRORSTATUS)) {
			WRN(0, "ERRORSTATUS.ind error_status=%s(%d)", strlibstatus((LibStatus)msg.d.error_ind.error_status), msg.d.error_ind.error_status);
			SANE_Status ssts = this->libsts2sanests((LibStatus)msg.d.error_ind.error_status);
			return ssts;
		} else {
			WRN(0, "invalid message received. (type=%02x)", msg.h.type);
			return SANE_STATUS_INVAL;
		}
	}

	this->o_tlX.set_constraint(&this->geometry_x_range, this->geometry_x_range.min);
	this->o_tlY.set_constraint(&this->geometry_y_range, this->geometry_y_range.min);
	this->o_brX.set_constraint(&this->geometry_x_range, this->geometry_x_range.max);
	this->o_brY.set_constraint(&this->geometry_y_range, this->geometry_y_range.max);


	TRC("end configure.");
	return SANE_STATUS_GOOD;
}

SANE_Status DeviceBaseImpl::open_device()
{
	return AbstractDevice::open_device();
}

void DeviceBaseImpl::close_device()
{
	INFOl("close_device: closing...");
	this->scif.close();
	AbstractDevice::close_device();
	INFOl("close_device: OK");
}

SANE_Status DeviceBaseImpl::calibration()
{
	INFO("calibration: ");
	return SANE_STATUS_GOOD;
}



int DeviceBaseImpl::get_scan_source()
{
	if (strcmp(this->options[opt_scan_source]->val.s, FX750_VALUE_SCAN_SOURCE_AUTO) == 0) {
		return 0;
	} else if (strcmp(this->options[opt_scan_source]->val.s, FX750_VALUE_SCAN_SOURCE_FLATBED) == 0) {
		return 1;
	} else if (strcmp(this->options[opt_scan_source]->val.s, FX750_VALUE_SCAN_SOURCE_ADF) == 0) {
		return 2;
	} else {
		ERR(0, "unknown scan source. source='%s'", this->options[opt_scan_source]->val.s);
		return -1;
	}
}

int DeviceBaseImpl::get_scan_mode()
{
	if (strcmp(this->options[opt_mode]->val.s, FX750_VALUE_SCAN_MODE_COLOR) == 0) {
		return 0;
	} else if (strcmp(this->options[opt_mode]->val.s, FX750_VALUE_SCAN_MODE_GRAY) == 0) {
		return 1;
	} else if (strcmp(this->options[opt_mode]->val.s, FX750_VALUE_SCAN_MODE_HALFTONE) == 0) {
		ERR(0, "not support half-tone mode. mode='%s'", this->options[opt_mode]->val.s);
		return -1;
	} else if (strcmp(this->options[opt_mode]->val.s, FX750_VALUE_SCAN_MODE_BW) == 0) {
		return 2;
	} else {
		ERR(0, "unknown color mode. mode='%s'", this->options[opt_mode]->val.s);
		return -1;
	}
}

int DeviceBaseImpl::get_option_duplex_scan()
{
	if (this->o_duplex_scan_onoff.val.b) {
		if (strcmp(this->o_binding.val.s, FX750_VALUE_BINDING_TOP) == 0) {
			return 2;
		} else {
			return 1;
		}
	} else {
		return 0;
	}
}

int DeviceBaseImpl::get_option_sharpness()
{
	if (strcmp(this->o_sharpness.val.s, FX750_VALUE_SHARPNESS_SHARPEN) == 0) {
		return 1;
	} else if (strcmp(this->o_sharpness.val.s, FX750_VALUE_SHARPNESS_SHARPEN_MORE) == 0) {
		return 2;
	} else {
		return 0;
	}
}

int DeviceBaseImpl::get_option_bgelm()
{
	if (strcmp(this->o_bg_eliminat_level.val.s, FX750_VALUE_BGELIMINAT_LEVEL0) == 0) {
		return 0;
	} else if (strcmp(this->o_bg_eliminat_level.val.s, FX750_VALUE_BGELIMINAT_LEVEL1) == 0) {
		return 1;
	} else if (strcmp(this->o_bg_eliminat_level.val.s, FX750_VALUE_BGELIMINAT_LEVEL2) == 0) {
		return 2;
	} else if (strcmp(this->o_bg_eliminat_level.val.s, FX750_VALUE_BGELIMINAT_LEVEL3) == 0) {
		return 3;
	} else if (strcmp(this->o_bg_eliminat_level.val.s, FX750_VALUE_BGELIMINAT_LEVEL4) == 0) {
		return 4;
	} else if (strcmp(this->o_bg_eliminat_level.val.s, FX750_VALUE_BGELIMINAT_LEVEL5) == 0) {
		return 5;
	} else if (strcmp(this->o_bg_eliminat_level.val.s, FX750_VALUE_BGELIMINAT_LEVEL6) == 0) {
		return 6;
	}
	return -1;
}

int DeviceBaseImpl::get_option_edge_erase()
{
	if (this->o_edge_erase_onoff.val.b) {
		return static_cast<int>(SANE_UNFIX(this->o_edge_erase_range.val.w));
	} else {
		return 0;
	}
}

SANE_Status DeviceBaseImpl::build_scanreq(scand_msg_t* msg)
{

	msg->h.pre = 0xff;
	msg->h.type = MsgType(REQUEST, SCAN);
	msg->h.data_length = sizeof(scan_req_t);

	msg->d.scan_req.pushscanid = 0;

	device::SaneDevice* devp = static_cast<device::SaneDevice*>(this->sane_device);
	strncpy(msg->d.scan_req.devname, devp->get_real_name(), sizeof(msg->d.scan_req.devname));

	msg->d.scan_req.p = false;

	msg->d.scan_req.src = this->get_scan_source();
	if (msg->d.scan_req.src < 0) {
		return SANE_STATUS_INVAL;
	}

	msg->d.scan_req.mode = this->get_scan_mode();
	if (msg->d.scan_req.mode < 0) {
		return SANE_STATUS_INVAL;
	}

	msg->d.scan_req.dc = 0;

	msg->d.scan_req.dup = get_option_duplex_scan();

	msg->d.scan_req.shp = this->get_option_sharpness();

	msg->d.scan_req.bgelm = this->get_option_bgelm();

	msg->d.scan_req.edger = this->get_option_edge_erase();

	msg->d.scan_req.fs = (this->o_font_smoothing_onoff.val.b) ? 1 : 0;

	msg->d.scan_req.me = (this->o_moire_elimination_onoff.val.b) ? 1 : 0;


	msg->d.scan_req.resolution = SANE_UNFIX (this->options[opt_resolution]->val.w);

	double lt_x = SANE_UNFIX (this->options[opt_tl_x]->val.w);
	double lt_y = SANE_UNFIX (this->options[opt_tl_y]->val.w);
	double br_x = SANE_UNFIX (this->options[opt_br_x]->val.w);
	double br_y = SANE_UNFIX (this->options[opt_br_y]->val.w);
	if (lt_x > br_x)
		swap<double>(&lt_x, &br_x);
	if (lt_y > br_y)
		swap<double>(&lt_y, &br_y);
	msg->d.scan_req.lt_x = lt_x;
	msg->d.scan_req.lt_y = lt_y;
	msg->d.scan_req.width = (br_x - lt_x);
	msg->d.scan_req.height = (br_y - lt_y);
	msg->d.scan_req.paper_width = 0.0;
	msg->d.scan_req.paper_height = 0.0;

	msg->d.scan_req.max_trans_size = sizeof(this->buffer) - 4;

	return SANE_STATUS_GOOD;
}


SANE_Status DeviceBaseImpl::update_sane_parameters(SANE_Parameters *p, unsigned mode, size_t w, size_t h, size_t bytes)
{
	switch (mode) {
	case 0:
		p->format = SANE_FRAME_RGB;
		p->depth = 8;
		p->pixels_per_line = static_cast<SANE_Int>(w);
		p->bytes_per_line = p->pixels_per_line * 3;
		p->last_frame = SANE_TRUE;
		p->lines = static_cast<SANE_Word>(h);
		break;
	case 1:
		p->format = SANE_FRAME_GRAY;
		p->depth = 8;
		p->pixels_per_line = static_cast<SANE_Int>(w);
		p->bytes_per_line = p->pixels_per_line;
		p->last_frame = SANE_TRUE;
		p->lines = static_cast<SANE_Word>(h);
		break;
	case 2:
		p->format = SANE_FRAME_GRAY;
		p->depth = 1;
		p->pixels_per_line = static_cast<SANE_Int>(w);
		p->bytes_per_line = (p->pixels_per_line+7) / 8;
		p->last_frame = SANE_TRUE;
		p->lines = static_cast<SANE_Word>(h);
		break;
	default:
		WRN(0, "invalid mode. (mode=%d)", mode);
		return SANE_STATUS_INVAL;
	}
	return SANE_STATUS_GOOD;
}


SANE_Status DeviceBaseImpl::get_parameters (SANE_Parameters * params)
{
	scandif lscif;
	LibStatus lsts;
	scand_msg_t msg;
	scand_msg_t rmsg;


	if (this->scanning) {
		if (params) {
			*params = this->params;
		}
		return SANE_STATUS_GOOD;
	}

	int mode = this->get_scan_mode();
	if (mode < 0) {
		return SANE_STATUS_INVAL;
	}
	double resolution = SANE_UNFIX (this->options[opt_resolution]->val.w);

	double lt_x = SANE_UNFIX (this->options[opt_tl_x]->val.w);
	double lt_y = SANE_UNFIX (this->options[opt_tl_y]->val.w);
	double br_x = SANE_UNFIX (this->options[opt_br_x]->val.w);
	double br_y = SANE_UNFIX (this->options[opt_br_y]->val.w);
	if (lt_x > br_x)
		swap<double>(&lt_x, &br_x);
	if (lt_y > br_y)
		swap<double>(&lt_y, &br_y);
	int w = mm2pixel(resolution, (br_x - lt_x));
	int h = mm2pixel(resolution, (br_x - lt_x));

	this->update_sane_parameters(params, mode, w, h, 0);

	return SANE_STATUS_GOOD;
}


SANE_Status DeviceBaseImpl::start ()
{
	INFO("start:");
	LibStatus lsts;
	SANE_Status ssts;
	scand_msg_t msg;
	scand_msg_t rmsg;

	this->trns_data_size = 0;
	this->canceled = false;
	this->scanning = true;
	this->return_eof = false;
	this->buffer_offset = 0;
	this->buffer_received = 0;


	while (true) {
		if (!this->use_adf) {
			if (this->scif.is_open() == false) {
				this->use_adf = false;
				lsts = this->scif.open();
				if (lsts != SUCCESS) {
					return SANE_STATUS_INVAL;
				}
			}

			this->build_scanreq(&msg);
			lsts = this->scif.send_message(&msg);
			if (lsts != SUCCESS) {
				this->scif.close();
				return SANE_STATUS_INVAL;
			}

			lsts = this->scif.receive_message(&rmsg);
			if (lsts == SUCCESS) {
				if (rmsg.h.type == MsgType(RESPONSE, SCAN)) {
					this->use_adf = (rmsg.d.scan_resp.src == 2);
					this->scan_data_size = rmsg.d.scan_resp.size_bytes;
					this->update_sane_parameters(&this->params, msg.d.scan_req.mode, rmsg.d.scan_resp.width, rmsg.d.scan_resp.height, rmsg.d.scan_resp.size_bytes);
					INFO("Total: %d bytes, Size: %d x %d pixel", rmsg.d.scan_resp.size_bytes, rmsg.d.scan_resp.width, rmsg.d.scan_resp.height);
					break;
				} else if (rmsg.h.type == MsgType(INDICATE, ERRORSTATUS)) {
					WRN(0, "ERRORSTATUS.ind error_status=%s(%d)", strlibstatus((LibStatus)rmsg.d.error_ind.error_status), rmsg.d.error_ind.error_status);
					ssts = this->libsts2sanests((LibStatus)rmsg.d.error_ind.error_status);
					return ssts;
				} else {
					WRN(0, "invalid message received. (type=%02x)", rmsg.h.type);
					return SANE_STATUS_INVAL;
				}
			} else {
				return SANE_STATUS_INVAL;
			}
		} else {
			msg.h.pre = 0xff;
			msg.h.type = MsgType(REQUEST, SCANCONTINUE);
			msg.h.data_length = 0;

			lsts = this->scif.send_message(&msg);
			if (lsts != SUCCESS) {
				this->scif.close();
				return SANE_STATUS_INVAL;
			}

			lsts = this->scif.receive_message(&rmsg);
			if (lsts == SUCCESS) {
				if (rmsg.h.type == MsgType(RESPONSE, SCANCONTINUE)) {
					INFO("Total: %d bytes, Size: %d x %d pixel", rmsg.d.scan_resp.size_bytes, rmsg.d.scan_resp.width, rmsg.d.scan_resp.height);
					break;
				} else if (rmsg.h.type == MsgType(INDICATE, ERRORSTATUS)) {
					WRN(0, "ERRORSTATUS.ind error_status=%s(%d)", strlibstatus((LibStatus)rmsg.d.error_ind.error_status), rmsg.d.error_ind.error_status);
					if (rmsg.d.error_ind.error_status != REJECT) {
						ssts = this->libsts2sanests((LibStatus)rmsg.d.error_ind.error_status);
						return ssts;
					} else {
						this->use_adf = false;
						continue;
					}
				} else {
					WRN(0, "invalid message received. (type=%02x)", rmsg.h.type);
					return SANE_STATUS_INVAL;
				}
			} else {
				return SANE_STATUS_INVAL;
			}
		}
	}

	TRC("start: OK");
	return SANE_STATUS_GOOD;
}

SANE_Status DeviceBaseImpl::read (SANE_Byte * data, SANE_Int max_length, SANE_Int * length)
{
	INFO("read:");
	LibStatus lsts;
	SANE_Status ssts;
	size_t max = max_length;
	size_t recvsize;
	bool last;
	scand_msg_t msg;
	scand_data_t* dp = reinterpret_cast<scand_data_t*>(this->buffer);

	if (length) {
		*length = 0;
	}

	if (this->scif.is_open()) {
		ssize_t rc;
		int flags;
		bool finished = false;
		bool discon = false;

		flags = (this->non_blocking) ? MSG_DONTWAIT : 0;

		if (this->buffer_received > 0 && (this->buffer_received - this->buffer_offset) > 0) {
			TRC("has data. (buffer_received=%d, buffer_offset=%d)", this->buffer_received, this->buffer_offset);
		} else {

			if (this->trns_data_size > 0 && this->scan_data_size <= this->trns_data_size) {
				this->scanning = false;
				return SANE_STATUS_EOF;
			}


			while (true) {
				lsts = this->scif.receive_message(&msg, this->buffer);
				if (lsts == SUCCESS) {
					break;
				} else if (lsts == NODATA) {
				} else {
					return SANE_STATUS_INVAL;
				}
			}

			enum MsgTypeValue mt = getMsgTypeValue(msg.h.type);
			if (mt == ERRORSTATUS) {
				TRC("ERRORSTATUS.ind error_status=%s(%d)", strlibstatus((LibStatus)dp->error_ind.error_status), dp->error_ind.error_status);
				ssts = this->libsts2sanests((LibStatus)dp->error_ind.error_status);
				return ssts;
			} else if (mt != SCANDATA) {
				ERR(0, "illegal message received. (type=%02x)", msg.h.type);
				return SANE_STATUS_INVAL;
			}

			this->buffer_offset = 0;
			this->buffer_received = dp->scandata_ind.data_size;

			TRC("data-size=%d", dp->scandata_ind.data_size);
			TRC("last=%d", dp->scandata_ind.last);
		}


		if (this->buffer_received > 0) {
			int buf_remain = this->buffer_received - this->buffer_offset;
			if (buf_remain > 0) {
				size_t cpsz = max_length;
				if (cpsz > buf_remain) {
					cpsz = buf_remain;
				}

				TRC("copy data. (cpsz=%d, buffer_offset=%d)", cpsz, this->buffer_offset);
				memcpy(data, &dp->scandata_ind.scan_data[this->buffer_offset], cpsz);

				this->buffer_offset += cpsz;
				this->trns_data_size += cpsz;
				if (this->scan_data_size <= this->trns_data_size) {
					INFOl("transfer complete. (scan_data_size=%d, trns_data_size=%d)", this->scan_data_size, this->trns_data_size);
					this->scanning = false;
				}

				if (length) {
					*length = cpsz;
				}
			} else {
				this->buffer_received = 0;
				this->buffer_offset = 0;
			}

			INFOl("transfer... (scan_data_size=%d, trns_data_size=%d)", this->scan_data_size, this->trns_data_size);
			return SANE_STATUS_GOOD;
		} else {
			DBG("no data.");
			return SANE_STATUS_GOOD;
		}
	} else {
		if (this->canceled) {
			TRC("canceled.");
			return SANE_STATUS_CANCELLED;
		}
		return SANE_STATUS_INVAL;
	}
}







void DeviceBaseImpl::cancel ()
{
	INFO("cancel: canceled=%d, scanning=%d", this->canceled, this->scanning);
	if (this->scif.is_open()) {
		if (this->scanning && !this->canceled) {
			this->canceled = true;
			this->scanning = false;
		}
		this->scif.close();
	}
	this->use_adf = false;
	INFOl("cancel: OK");
}






SANE_Status DeviceBaseImpl::set_io_mode (SANE_Bool non_blocking)
{
	if (non_blocking) {
		TRC("set non-blocking mode.");
		this->non_blocking = true;
	} else {
		TRC("set blocking mode.");
		this->non_blocking = false;
	}
	return SANE_STATUS_GOOD;
}

SANE_Status DeviceBaseImpl::get_select_fd (SANE_Int * UNUSED(fd))
{
	return SANE_STATUS_UNSUPPORTED;
}


uint8_t DeviceBaseImpl::statusLevel(SANE_Status s)
{
	static SANE_Status sane_status_level[] = {
		SANE_STATUS_GOOD,			
		SANE_STATUS_CANCELLED,		
		SANE_STATUS_NO_DOCS,		
		SANE_STATUS_JAMMED,			
		SANE_STATUS_COVER_OPEN,		
		SANE_STATUS_DEVICE_BUSY,	
		SANE_STATUS_EOF,				
		SANE_STATUS_UNSUPPORTED,	
		SANE_STATUS_INVAL,			
		SANE_STATUS_IO_ERROR,		
		SANE_STATUS_ACCESS_DENIED,	
		SANE_STATUS_NO_MEM,			
	};

	int i;
	int n = sizeof(sane_status_level) / sizeof(SANE_Status);
	for (i=0 ; i<n ; i++) {
		if (s == sane_status_level[i]) {
			return (uint8_t)i;
		}
	}
	return 255;
}

SANE_Status DeviceBaseImpl::libsts2sanests(LibStatus s)
{
	static struct stscvtbl {
		LibStatus lsts;
		SANE_Status ssts;
	} _stscvtbl[] = {
		{ SUCCESS, SANE_STATUS_GOOD },
		{ CANCELED, SANE_STATUS_CANCELLED },
		{ END_OF_PAGE, SANE_STATUS_EOF },

		{ ERROR, SANE_STATUS_INVAL },
		{ INVALID, SANE_STATUS_INVAL },
		{ BUSY, SANE_STATUS_DEVICE_BUSY },
		{ IOERROR, SANE_STATUS_IO_ERROR },
		{ NOMEMORY, SANE_STATUS_NO_MEM },

		{ DEVICE_BUSY, SANE_STATUS_DEVICE_BUSY },
		{ SYSTEM_BUSY, SANE_STATUS_DEVICE_BUSY },
		{ ACCESS_REFUSED, SANE_STATUS_ACCESS_DENIED },
		{ ACCESS_DENIED, SANE_STATUS_ACCESS_DENIED },
		{ ADF_NO_PAPER,	 SANE_STATUS_NO_DOCS },
		{ ADF_PAPER_COVER_OPEN, SANE_STATUS_COVER_OPEN },
		{ ADF_PAPER_JAMMED, SANE_STATUS_JAMMED },
		{ ADF_PAPER_FEED_ERROR, SANE_STATUS_JAMMED },
		{ CCD_CARRIAGE_LOCK, SANE_STATUS_DEVICE_BUSY },
		{ HOME_POSITION_ERROR, SANE_STATUS_DEVICE_BUSY },
		{ MEMORY_FULL, SANE_STATUS_DEVICE_BUSY },
		{ INVALID_COMMAND_CODE, SANE_STATUS_INVAL },
		{ INVALID_VALUE, SANE_STATUS_INVAL },
		{ INVALID_PARAMETER, SANE_STATUS_INVAL },
		{ INVALID_DATA,	SANE_STATUS_INVAL },
		{ INVALID_COMMAND_LENGTH, SANE_STATUS_INVAL },
		{ INVALID_DATA_LENGTH, SANE_STATUS_INVAL },
		{ RAM_TEST_FAILED,	SANE_STATUS_DEVICE_BUSY },
		{ CALIBRATION_TEST_FAILED, SANE_STATUS_DEVICE_BUSY },
		{ DEVICE_INTERNAL_ERROR, SANE_STATUS_DEVICE_BUSY },
		{ SENSOR_ERROR, SANE_STATUS_DEVICE_BUSY },
		{ HOME_SENSOR_ERROR, SANE_STATUS_DEVICE_BUSY },
		{ LAMP_ERROR, SANE_STATUS_DEVICE_BUSY },
		{ FAN_ERROR, SANE_STATUS_DEVICE_BUSY },
		{ UNPLUGGED_ERROR,	SANE_STATUS_IO_ERROR },
		{ MIRROR_CARRIAGE_ERROR, SANE_STATUS_DEVICE_BUSY },
		{ COMMUNICATION_ERROR, SANE_STATUS_IO_ERROR },
		{ DETECT_CD, SANE_STATUS_DEVICE_BUSY },
	};
	int i;
	int n = sizeof(_stscvtbl) / sizeof(stscvtbl);
	for (i=0 ; i<n ; i++) {
		if (s == _stscvtbl[i].lsts) {
			INFO("libsts2sanests LibStatus=%d ---> SANE_Status=%s", s, sane_strstatus(_stscvtbl[i].ssts));
			return _stscvtbl[i].ssts;
		}
	}
	ERR(0, "unknown status. LibStatus=%d", s);
	return SANE_STATUS_INVAL;
}

};
