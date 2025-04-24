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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include "../include/_stdint.h"

#include "../include/sane/sane.h"
extern "C" {
#include "../include/sane/saneopts.h"
}


#include "odc.h"
#include "odc_device.h"

#include "odc_products.h"

#include "../driver/devel/scandifc.h"
#include "../driver/devel/tracer.h"
using namespace odc;
#define TRCID STRINGIFY(BACKEND_NAME)":device"
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


namespace device {

void DBG_ShowProperties(const Properties* prop)
{
	char buf[4096];
	prop->_str(buf);
	DBG("properties: %s", buf);
}

void DBG_InfoToString(SANE_Int info, char* out)
{
	int rc;
	char* p = out;

	p[0] = '\0';

	if ((info & SANE_INFO_INEXACT)) {
		rc = sprintf(p, "SANE_INFO_INEXACT");
		p += rc;
	}
	if ((info & SANE_INFO_RELOAD_PARAMS)) {
		if (p != out) {
			rc = sprintf(p, ",");
			p += rc;
		}
		rc = sprintf(p, "SANE_INFO_RELOAD_PARAMS");
		p += rc;
	}
	if ((info & SANE_INFO_RELOAD_OPTIONS)) {
		if (p != out) {
			rc = sprintf(p, ",");
			p += rc;
		}
		rc = sprintf(p, "SANE_INFO_RELOAD_OPTIONS");
		p += rc;
	}
}


size_t max_string_size (SANE_String_Const strings[])
{
  size_t size, max_size = 0;
  SANE_Int i;

  for (i = 0; strings[i]; ++i)
    {
      size = strlen (strings[i]) + 1;
      if (size > max_size)
	max_size = size;
    }
  return max_size;
}

SANE_String_Const get_unit_str(SANE_Unit unit) {
	switch(unit) {
	case SANE_UNIT_NONE:
		return "(none)";
	case SANE_UNIT_PIXEL:
		return "pixel";
	case SANE_UNIT_BIT:
		return "bit";
	case SANE_UNIT_MM:
		return "mm";
	case SANE_UNIT_DPI:
		return "dpi";
	case SANE_UNIT_PERCENT:
		return "%%";
	case SANE_UNIT_MICROSECOND:
		return "usec";
	default:
		return "(unknown)";
	}
}


Option::Option()
{
	TRC("OCD_Option::ODC_Option: construct");
	memset(&this->desc, 0, sizeof(this->desc));
	this->val.w = 0;
	this->loaded = SANE_FALSE;
}

Option::~Option()
{
	TRC("OCD_Option::~ODC_Option: destruct");

}

void Option::clear_constraint()
{
	if (this->desc.type == SANE_TYPE_STRING) {
		if (this->val.s != NULL) {
			free(this->val.s);
		}
	}
	this->desc.constraint_type = SANE_CONSTRAINT_NONE;
	this->desc.constraint.range = 0;
}

void Option::set_constraint(SANE_String_Const *string_list, SANE_String_Const initval)
{
	if (this->desc.type == SANE_TYPE_STRING) {
		this->desc.constraint_type = SANE_CONSTRAINT_STRING_LIST;
		this->desc.constraint.string_list = string_list;

		this->desc.size = max_string_size (string_list);
		if (this->val.s != NULL) {
			free(this->val.s);
		}
		this->val.s = static_cast<SANE_String>(malloc (this->desc.size));
		if (!this->val.s)
			throw SANE_STATUS_NO_MEM;

		if (initval != NULL) {
			strncpy(this->val.s, initval, this->desc.size);
		}
	}
}

void Option::set_constraint(const SANE_Word *word_list)
{
	this->desc.constraint_type = SANE_CONSTRAINT_WORD_LIST;
	this->desc.constraint.word_list = word_list;
}

void Option::set_constraint(const SANE_Word *word_list, SANE_Word initval)
{
	this->desc.constraint_type = SANE_CONSTRAINT_WORD_LIST;
	this->desc.constraint.word_list = word_list;
	this->val.w = initval;
}


void Option::set_constraint(const SANE_Range *range)
{
	this->desc.constraint_type = SANE_CONSTRAINT_RANGE;
	this->desc.constraint.range = range;
}


void Option::set_constraint(const SANE_Range *range, SANE_Word initval)
{
	this->desc.constraint_type = SANE_CONSTRAINT_RANGE;
	this->desc.constraint.range = range;
	this->val.w = initval;
}




OptionBool::OptionBool(
		SANE_String_Const name,
		SANE_String_Const title,
		SANE_String_Const desc,
		SANE_Bool value)
{
	this->desc.name = name;
	this->desc.title = title;
	this->desc.desc = desc;
	this->desc.type = SANE_TYPE_BOOL;
	this->desc.unit = SANE_UNIT_NONE;
	this->desc.size = sizeof(SANE_Bool);
	this->desc.cap = SANE_CAP_SOFT_SELECT | SANE_CAP_SOFT_DETECT;

	this->desc.constraint_type = SANE_CONSTRAINT_NONE;
	this->desc.constraint.range = 0;

	this->val.w = value;

	this->infoval = 0;

	this->loaded = SANE_FALSE;
}

SANE_Status OptionBool::set_auto()
{
	this->val.w = SANE_TRUE;
	TRC("ODC_Option_Bool::set_auto: set option '%s' automatically to %s",
			this->desc.name,
			this->val.w == SANE_TRUE ? "true" : "false");
	return SANE_STATUS_GOOD;
}

SANE_Status OptionBool::set_value(void *value, SANE_Int * info)
{
	if (value == NULL) {
		ERR(0, "ODC_Option_Bool::set_value: value NULL.");
		return SANE_STATUS_INVAL;
	}
	if (this->val.w == *(SANE_Bool *) value) {
		TRC("ODC_Option_Bool::set_value: option '%s' not changed",
				this->desc.name);
	} else {
		this->val.w = *(SANE_Bool *) value;
		TRC("ODC_Option_Bool::set_value: set option '%s' changed to %s",
				this->desc.name,
				this->val.w == SANE_TRUE ? "true" : "false");
	}
	if (info != NULL) {
		*info = this->infoval;
	}
	return SANE_STATUS_GOOD;
}

SANE_Status OptionBool::get_value(void *value)
{
	if (value == NULL) {
		ERR(0, "ODC_Option_Bool::get_value: value NULL.");
		return SANE_STATUS_INVAL;
	}
	*(SANE_Bool *) value = this->val.w;
	TRC("ODC_Option_Bool::get_value: get option '%s', value=%s",
			this->desc.name,
			*(SANE_Bool *) value == SANE_TRUE ? "true" : "false");
	return SANE_STATUS_GOOD;
}




OptionInt::OptionInt(
		SANE_String_Const name,
		SANE_String_Const title,
		SANE_String_Const desc,
		SANE_Word value,
		SANE_Unit unit)
{
	this->desc.name = name;
	this->desc.title = title;
	this->desc.desc = desc;
	this->desc.type = SANE_TYPE_INT;
	this->desc.unit = unit;
	this->desc.size = sizeof(SANE_Word);
	this->desc.cap = SANE_CAP_SOFT_SELECT | SANE_CAP_SOFT_DETECT;

	this->desc.constraint_type = SANE_CONSTRAINT_NONE;
	this->desc.constraint.range = 0;

	this->val.w = value;

	this->infoval = 0;

	this->loaded = SANE_FALSE;
}

OptionInt::OptionInt(
		SANE_String_Const name,
		SANE_String_Const title,
		SANE_String_Const desc,
		const SANE_Word *word_list,
		SANE_Word value,
		SANE_Unit unit)
{
	this->desc.name = name;
	this->desc.title = title;
	this->desc.desc = desc;
	this->desc.type = SANE_TYPE_INT;
	this->desc.unit = unit;
	this->desc.size = sizeof(SANE_Word);
	this->desc.cap = SANE_CAP_SOFT_SELECT | SANE_CAP_SOFT_DETECT;

	this->desc.constraint_type = SANE_CONSTRAINT_WORD_LIST;
	this->desc.constraint.word_list = word_list;

	this->val.w = value;

	this->infoval = 0;

	this->loaded = SANE_FALSE;
}

OptionInt::OptionInt(
		SANE_String_Const name,
		SANE_String_Const title,
		SANE_String_Const desc,
		const SANE_Range *range,
		SANE_Word value,
		SANE_Unit unit)
{
	this->desc.name = name;
	this->desc.title = title;
	this->desc.desc = desc;
	this->desc.type = SANE_TYPE_INT;
	this->desc.unit = unit;
	this->desc.size = sizeof(SANE_Word);
	this->desc.cap = SANE_CAP_SOFT_SELECT | SANE_CAP_SOFT_DETECT;

	this->desc.constraint_type = SANE_CONSTRAINT_RANGE;
	this->desc.constraint.range = range;

	this->val.w = value;

	this->infoval = 0;

	this->loaded = SANE_FALSE;
}

SANE_Status OptionInt::set_auto()
{
	return SANE_STATUS_GOOD;
}

SANE_Status OptionInt::set_value(void *value, SANE_Int * info)
{
	if (value == NULL) {
		ERR(0, "ODC_Option_Int::set_value: value NULL.");
		return SANE_STATUS_INVAL;
	}
	if (this->val.w == *(SANE_Word *) value) {
		TRC("ODC_Option_Int::set_value: option '%s' not changed",
				this->desc.name);
	} else {
		this->val.w = *(SANE_Word *) value;
		TRC("ODC_Option_Int::set_auto: set option '%s' changed to %d",
				this->desc.name,
				this->val.w);
	}
	if (info != NULL) {
		*info = this->infoval;
	}
	return SANE_STATUS_GOOD;
}

SANE_Status OptionInt::get_value(void *value)
{
	if (value == NULL) {
		ERR(0, "ODC_Option_Int::get_value: value NULL.");
		return SANE_STATUS_INVAL;
	}
	*(SANE_Word *) value = this->val.w;
	TRC("ODC_Option_Int::get_value: get option '%s', value=%d",
			this->desc.name,
			*(SANE_Word *) value);
	return SANE_STATUS_GOOD;
}


OptionFixed::OptionFixed(
		SANE_String_Const name,
		SANE_String_Const title,
		SANE_String_Const desc,
		SANE_Word value,
		SANE_Unit unit)
{
	this->desc.name = name;
	this->desc.title = title;
	this->desc.desc = desc;
	this->desc.type = SANE_TYPE_FIXED;
	this->desc.unit = unit;
	this->desc.size = sizeof(SANE_Word);
	this->desc.cap = SANE_CAP_SOFT_SELECT | SANE_CAP_SOFT_DETECT;

	this->desc.constraint_type = SANE_CONSTRAINT_NONE;
	this->desc.constraint.range = 0;

	this->val.w =value;

	this->infoval = 0;

	this->loaded = SANE_FALSE;
}

OptionFixed::OptionFixed(
		SANE_String_Const name,
		SANE_String_Const title,
		SANE_String_Const desc,
		const SANE_Word *word_list,
		SANE_Word value,
		SANE_Unit unit)
{
	this->desc.name = name;
	this->desc.title = title;
	this->desc.desc = desc;
	this->desc.type = SANE_TYPE_FIXED;
	this->desc.unit = unit;
	this->desc.size = sizeof(SANE_Word);
	this->desc.cap = SANE_CAP_SOFT_SELECT | SANE_CAP_SOFT_DETECT;

	this->desc.constraint_type = SANE_CONSTRAINT_WORD_LIST;
	this->desc.constraint.word_list = word_list;

	this->val.w =value;

	this->infoval = 0;

	this->loaded = SANE_FALSE;
}

OptionFixed::OptionFixed(
		SANE_String_Const name,
		SANE_String_Const title,
		SANE_String_Const desc,
		const SANE_Range *range,
		SANE_Word value,
		SANE_Unit unit)
{
	this->desc.name = name;
	this->desc.title = title;
	this->desc.desc = desc;
	this->desc.type = SANE_TYPE_FIXED;
	this->desc.unit = unit;
	this->desc.size = sizeof(SANE_Word);
	this->desc.cap = SANE_CAP_SOFT_SELECT | SANE_CAP_SOFT_DETECT;

	this->desc.constraint_type = SANE_CONSTRAINT_RANGE;
	this->desc.constraint.range = range;

	this->val.w =value;

	this->infoval = 0;

	this->loaded = SANE_FALSE;
}

SANE_Status OptionFixed::set_auto()
{
	return SANE_STATUS_GOOD;
}

SANE_Status OptionFixed::set_value(void *value, SANE_Int * info)
{
	if (value == NULL) {
		ERR(0, "ODC_Option_Fixed::set_value: value NULL.");
		return SANE_STATUS_INVAL;
	}
	if (this->val.w == *(SANE_Fixed *) value) {
		TRC("ODC_Option_Fixed::set_value: option '%s' not changed. value=%.0f",
				this->desc.name, SANE_UNFIX(this->val.w));
	} else {
		this->val.w = *(SANE_Fixed *) value;
		TRC(
				"ODC_Option_Fixed::set_value: set option '%s' to %.0f %s",
				this->desc.name,
				SANE_UNFIX (*(SANE_Fixed *) value),
				get_unit_str(this->desc.unit));
	}
	if (info != NULL) {
		*info = this->infoval;
	}
	return SANE_STATUS_GOOD;
}

SANE_Status OptionFixed::get_value(void *value)
{
	if (value == NULL) {
		ERR(0, "ODC_Option_Fixed::get_value: value NULL.");
		return SANE_STATUS_INVAL;
	}
	*(SANE_Fixed *) value = this->val.w;
	TRC(
			"ODC_Option_Fixed::get_value: get option '%s', value=%.1f %s",
			this->desc.name,
			SANE_UNFIX (*(SANE_Fixed *) value),
			get_unit_str(this->desc.unit));
	return SANE_STATUS_GOOD;
}


OptionString::OptionString(
		SANE_String_Const name,
		SANE_String_Const title,
		SANE_String_Const desc,
		SANE_Int maxlength)
{
	this->desc.name = name;
	this->desc.title = title;
	this->desc.desc = desc;
	this->desc.type = SANE_TYPE_STRING;
	this->desc.unit = SANE_UNIT_NONE;
	this->desc.size = 0;
	this->desc.cap = SANE_CAP_SOFT_SELECT | SANE_CAP_SOFT_DETECT;

	this->desc.constraint_type = SANE_CONSTRAINT_NONE;
	this->desc.constraint.range = 0;

	this->val.s = NULL;
	if (maxlength > 0) {
		this->desc.size = maxlength;
		this->val.s = static_cast<SANE_String>(malloc (this->desc.size));
		if (!this->val.s)
			throw SANE_STATUS_NO_MEM;
	}

	this->infoval = 0;

	this->loaded = SANE_FALSE;
}
OptionString::OptionString(
		SANE_String_Const name,
		SANE_String_Const title,
		SANE_String_Const desc,
		SANE_String_Const *string_list,
		SANE_String_Const initval)
{
	this->desc.name = name;
	this->desc.title = title;
	this->desc.desc = desc;
	this->desc.type = SANE_TYPE_STRING;
	this->desc.unit = SANE_UNIT_NONE;
	this->desc.size = max_string_size (string_list);
	this->desc.cap = SANE_CAP_SOFT_SELECT | SANE_CAP_SOFT_DETECT;

	this->desc.constraint_type = SANE_CONSTRAINT_STRING_LIST;
	this->desc.constraint.string_list = string_list;

	this->val.s = static_cast<SANE_String>(malloc (this->desc.size));
	if (!this->val.s) {
		throw SANE_STATUS_NO_MEM;
	}

	if (initval != NULL) {
		strncpy(this->val.s, initval, this->desc.size);
	} else {
		strcpy(this->val.s, string_list[0]);
	}

	this->infoval = 0;

	this->loaded = SANE_FALSE;
}

OptionString::~OptionString()
{
	if (this->val.s != NULL) {
		free(this->val.s);
		this->val.s = NULL;
	}
}

SANE_Status OptionString::set_auto()
{
	return SANE_STATUS_GOOD;
}

SANE_Status OptionString::set_value(void *value, SANE_Int * info)
{
	if (value == NULL) {
		ERR(0, "ODC_Option_String::set_value: value NULL.");
		return SANE_STATUS_INVAL;
	}
	if (strcmp(this->val.s, static_cast<const char*>(value)) == 0) {
		TRC("ODC_Option_String::set_value: option '%s' not changed",
				this->desc.name);
	} else {
		strcpy(this->val.s, (SANE_String) value);
		TRC("ODC_Option_String::set_value: set option '%s' to `%s'",
				this->desc.name, (SANE_String) value);
	}
	if (info != NULL) {
		*info = this->infoval;
	}
	return SANE_STATUS_GOOD;
}

SANE_Status OptionString::get_value(void *value)
{
	if (value == NULL) {
		ERR(0, "ODC_Option_String::get_value: value NULL.");
		return SANE_STATUS_INVAL;
	}
	strcpy(static_cast<char*>(value), this->val.s);
	TRC("ODC_Option_String::get_value: get option '%s', value=`%s'",
			this->desc.name, (SANE_String) value);
	return SANE_STATUS_GOOD;
}


OptionButton::OptionButton(SANE_String_Const name, SANE_String_Const title, SANE_String_Const desc)
{
	this->desc.name = name;
	this->desc.title = title;
	this->desc.desc = desc;
	this->desc.type = SANE_TYPE_BUTTON;
	this->desc.unit = SANE_UNIT_NONE;
	this->desc.size = 0;
	this->desc.cap = SANE_CAP_SOFT_SELECT | SANE_CAP_SOFT_DETECT;

	this->desc.constraint_type = SANE_CONSTRAINT_NONE;
	this->desc.constraint.range = 0;

	this->val.w = 0;

	this->loaded = SANE_FALSE;
}

SANE_Status OptionButton::set_auto()
{
	return SANE_STATUS_GOOD;
}

SANE_Status OptionButton::set_value(void *UNUSED(value), SANE_Int * UNUSED(info))
{
	TRC("Yes! You pressed me!");
	TRC("ODC_Option_Button::set_value: set option '%s'",
			this->desc.name);
	return SANE_STATUS_GOOD;
}

SANE_Status OptionButton::get_value(void *UNUSED(value))
{
	return SANE_STATUS_GOOD;
}


OptionGroup::OptionGroup(SANE_String_Const name, SANE_String_Const title, SANE_String_Const desc)
{
	this->desc.name = name;
	this->desc.title = title;
	this->desc.desc = desc;
	this->desc.type = SANE_TYPE_GROUP;
	this->desc.unit = SANE_UNIT_NONE;
	this->desc.size = 0;
	this->desc.cap = 0;

	this->desc.constraint_type = SANE_CONSTRAINT_NONE;
	this->desc.constraint.range = 0;

	this->val.w = 0;

	this->loaded = SANE_FALSE;
}

SANE_Status OptionGroup::set_auto()
{
	ERR(0, "sane_control_option: option is a group");
	return SANE_STATUS_INVAL;
}

SANE_Status OptionGroup::set_value(void *UNUSED(value), SANE_Int * UNUSED(info))
{
	ERR(0, "sane_control_option: option is a group");
	return SANE_STATUS_INVAL;
}

SANE_Status OptionGroup::get_value(void *UNUSED(value))
{
	ERR(0, "sane_control_option: option is a group");
	return SANE_STATUS_INVAL;
}




OptNumOpts::OptNumOpts(SANE_Word numopts) :
		OptionInt(
				ODC_COM_NAME_NUM_OPTIONS,
				ODC_COM_TITLE_NUM_OPTIONS,
				ODC_COM_DESC_NUM_OPTIONS,
				numopts)
{
	this->desc.cap = SANE_CAP_SOFT_DETECT;
	this->loaded = SANE_TRUE;
}









AbstractDevice::AbstractDevice(SANE_Device* sane_device) :
			o_com_grp_standard(ODC_COM_NAME_STANDARD, ODC_COM_TITLE_STANDARD, ODC_COM_DESC_STANDARD),
			o_com_grp_geometry(ODC_COM_NAME_GEOMETRY, ODC_COM_TITLE_GEOMETRY, ODC_COM_DESC_GEOMETRY),
			o_com_grp_enhancement(ODC_COM_NAME_ENHANCEMENT, ODC_COM_TITLE_ENHANCEMENT, ODC_COM_DESC_ENHANCEMENT),
			o_com_grp_advanced(ODC_COM_NAME_ADVANCED, ODC_COM_TITLE_ADVANCED, ODC_COM_DESC_ADVANCED),
			o_com_grp_sensors(ODC_COM_NAME_SENSORS, ODC_COM_TITLE_SENSORS, ODC_COM_DESC_SENSORS)
{
	TRC ("construct");
	this->sane_device = sane_device;
	this->open = SANE_FALSE;
}

AbstractDevice::~AbstractDevice()
{
	TRC ("destruct");
}


SANE_Status AbstractDevice::alloc_options_mem(SANE_Int num)
{
	this->number_of_options = num;
	this->options = static_cast<Option**>(malloc(this->number_of_options * sizeof(Option*)));
	if (!this->options) {
		ERR (0, "alloc_options_mem NG(SANE_STATUS_NO_MEM)");
		return SANE_STATUS_NO_MEM;
	}
	return SANE_STATUS_GOOD;
}


SANE_Status AbstractDevice::configure()
{
	return SANE_STATUS_GOOD;
}


SANE_Status AbstractDevice::open_device()
{
	TRC ("AbstractDevice::open_device");
	this->open = SANE_TRUE;
	return SANE_STATUS_GOOD;
}

void AbstractDevice::close_device()
{
	TRC ("AbstractDevice::close_device");
	this->open = SANE_FALSE;
}


SANE_Status AbstractDevice::calibration()
{
	TRC ("AbstractDevice::calibration");
	return SANE_STATUS_GOOD;
}



const SANE_Option_Descriptor * AbstractDevice::get_option_descriptor (SANE_Int option) {
	if (option < 0 || option >= this->number_of_options) {
		TRC(0, "sane_get_option_descriptor: option < 0 || "
				"option > number_of_options(%d)", this->number_of_options);
		return 0;
	}
	this->options[option]->loaded = SANE_TRUE;
	return &this->options[option]->desc;
}


SANE_Status AbstractDevice::control_option (SANE_Int option, SANE_Action action, void *value, SANE_Int * info)
{
	if (option < 0 || option >= this->number_of_options) {
		TRC(0, "sane_get_option_descriptor: option < 0 || "
				"option > number_of_options(%d)", this->number_of_options);
		return SANE_STATUS_INVAL;
	}

	if (!this->options[option]->loaded) {
		ERR(0, "sane_control_option: option not loaded");
		return SANE_STATUS_INVAL;
	}

	if (!SANE_OPTION_IS_ACTIVE (this->options[option]->desc.cap)) {
		TRC(0, "sane_control_option: option is inactive");
		return SANE_STATUS_INVAL;
	}

	if (this->options[option] == NULL) {
		ERR(0, "not assigned.");
		return SANE_STATUS_INVAL;
	}


	SANE_Status sts;

	switch (action) {
	case SANE_ACTION_SET_AUTO:
		sts = this->options[option]->set_auto();
		break;

	case SANE_ACTION_SET_VALUE:
		sts = this->options[option]->set_value(value, info);
		break;

	case SANE_ACTION_GET_VALUE:
		sts = this->options[option]->get_value(value);
		break;

	default:
		ERR(0, "sane_control_option: trying unexpected action %d", action);
		sts = SANE_STATUS_INVAL;
		break;
	}

	return sts;
}

SANE_Status AbstractDevice::get_parameters (SANE_Parameters * params)
{
	SANE_Parameters *p = &this->params;

	p->format = SANE_FRAME_RGB;
	p->depth = 24;
	p->pixels_per_line = 300;
	p->bytes_per_line = p->pixels_per_line * 3;
	p->last_frame = SANE_FALSE;
	p->lines = 300;;

	if (params) {
		*params = *p;
	}
	return SANE_STATUS_GOOD;
}

SANE_Status AbstractDevice::start ()
{
	return SANE_STATUS_GOOD;
}

SANE_Status AbstractDevice::read (SANE_Byte * UNUSED(data), SANE_Int UNUSED(max_length), SANE_Int * UNUSED(length))
{
	return SANE_STATUS_GOOD;
}

void AbstractDevice::cancel ()
{
}

SANE_Status AbstractDevice::set_io_mode (SANE_Bool non_blocking)
{
	TRC("non_blocking=%s", (non_blocking==SANE_TRUE) ? "SANE_TRUE":"SANE_FALSE");
	return SANE_STATUS_UNSUPPORTED;
}

SANE_Status AbstractDevice::get_select_fd (SANE_Int * UNUSED(fd))
{
	return SANE_STATUS_UNSUPPORTED;
}






void AbstractDevice::print_options()
{
	SANE_Option_Descriptor *od;
	SANE_Word i;
	SANE_Char caps[1024];

	for (i = 0; i < this->number_of_options; i++) {
		od = &this->options[i]->desc;
		TRC("-----> number: %d", i);
		TRC("         name: `%s'", od->name);
		TRC("        title: `%s'", od->title);
		TRC("  description: `%s'", od->desc);
		TRC(
				"         type: %s",
				od->type == SANE_TYPE_BOOL ? "SANE_TYPE_BOOL" :
				od->type == SANE_TYPE_INT ? "SANE_TYPE_INT" :
				od->type == SANE_TYPE_FIXED ? "SANE_TYPE_FIXED" :
				od->type == SANE_TYPE_STRING ? "SANE_TYPE_STRING" :
				od->type == SANE_TYPE_BUTTON ? "SANE_TYPE_BUTTON" :
				od->type == SANE_TYPE_GROUP ? "SANE_TYPE_GROUP" : "unknown");
		TRC(
				"         unit: %s",
				od->unit == SANE_UNIT_NONE ? "SANE_UNIT_NONE" :
				od->unit == SANE_UNIT_PIXEL ? "SANE_UNIT_PIXEL" :
				od->unit == SANE_UNIT_BIT ? "SANE_UNIT_BIT" :
				od->unit == SANE_UNIT_MM ? "SANE_UNIT_MM" :
				od->unit == SANE_UNIT_DPI ? "SANE_UNIT_DPI" :
				od->unit == SANE_UNIT_PERCENT ? "SANE_UNIT_PERCENT" :
				od->unit == SANE_UNIT_MICROSECOND ?
						"SANE_UNIT_MICROSECOND" : "unknown");
		TRC("         size: %d", od->size);
		caps[0] = '\0';
		if (od->cap & SANE_CAP_SOFT_SELECT)
			strcat(caps, "SANE_CAP_SOFT_SELECT ");
		if (od->cap & SANE_CAP_HARD_SELECT)
			strcat(caps, "SANE_CAP_HARD_SELECT ");
		if (od->cap & SANE_CAP_SOFT_DETECT)
			strcat(caps, "SANE_CAP_SOFT_DETECT ");
		if (od->cap & SANE_CAP_EMULATED)
			strcat(caps, "SANE_CAP_EMULATED ");
		if (od->cap & SANE_CAP_AUTOMATIC)
			strcat(caps, "SANE_CAP_AUTOMATIC ");
		if (od->cap & SANE_CAP_INACTIVE)
			strcat(caps, "SANE_CAP_INACTIVE ");
		if (od->cap & SANE_CAP_ADVANCED)
			strcat(caps, "SANE_CAP_ADVANCED ");
		TRC(" capabilities: %s", caps);
		TRC(
				"constraint type: %s",
				od->constraint_type == SANE_CONSTRAINT_NONE ?
						"SANE_CONSTRAINT_NONE" :
				od->constraint_type == SANE_CONSTRAINT_RANGE ?
						"SANE_CONSTRAINT_RANGE" :
				od->constraint_type == SANE_CONSTRAINT_WORD_LIST ?
						"SANE_CONSTRAINT_WORD_LIST" :
				od->constraint_type == SANE_CONSTRAINT_STRING_LIST ?
						"SANE_CONSTRAINT_STRING_LIST" : "unknown");
	}
}





AbstractDevice* AbstractDevice::newInstance(SANE_Device* sane_device)
{
	return new products::DeviceBaseImpl(sane_device);
}



PointerList<SaneDevice> SaneDevice::devlist;


SANE_Status SaneDevice::initDeviceList()
{
	LibStatus lsts;
	SANE_Status sts;
	size_t i;
	scandif scif;
	scand_msg_t msg;
	devlist_resp_t* rp;
	unsigned char buf[8192];

	devlist.init(true);

	lsts = scif.open(4000);
	if (lsts != SUCCESS) {
		return SANE_STATUS_INVAL;
	}


	msg.h.pre = 0xff;
	msg.h.type = MsgType(REQUEST, GETDEVLIST);
	msg.h.data_length = 0;
	lsts = scif.send_message(&msg);
	if (lsts != SUCCESS) {
		return SANE_STATUS_INVAL;
	}

	lsts = scif.receive_message(&msg, buf);
	if (lsts == SUCCESS) {
		if (msg.h.type == MsgType(RESPONSE, GETDEVLIST)) {
			rp = reinterpret_cast<devlist_resp_t*>(buf);
			TRC("# of dev=%d", rp->num);

			for (i=0 ; i<rp->num ; i++) {
				TRC("DEVICE: %s %s %s %s",
						rp->dev[i].name,
						rp->dev[i].vendor,
						rp->dev[i].model,
						rp->dev[i].type);
				SaneDevice* p = new SaneDevice(
						rp->dev[i].name,
						rp->dev[i].vendor,
						rp->dev[i].model,
						rp->dev[i].type);
				devlist.add(p);
			}
		} else if (msg.h.type == MsgType(INDICATE, ERRORSTATUS)) {
			WRN(0, "error message received. (status=%02x)", msg.d.error_ind.error_status);
			return SANE_STATUS_INVAL;
		} else {
			WRN(0, "invalid message received. (type=%02x)", msg.h.type);
			return SANE_STATUS_INVAL;
		}
	}
	return SANE_STATUS_GOOD;
}

void SaneDevice::cleanDeviceList()
{
	devlist.clean();
}

const SaneDevice** SaneDevice::getDevices()
{
	return const_cast<const SaneDevice**>(devlist.toArrayPtr());
}


SaneDevice* SaneDevice::findDevice(const char* name)
{
	TRC("search for '%s'", name);
	size_t i;

	for (i=0 ; i<devlist.size() ; i++) {
		if (strcmp(devlist[i]->name, name) == 0) {
			TRC("found. ptr=%p", devlist[i]);
			return devlist[i];
		}
	}

	TRC("not found.");
	return NULL;
}

SaneDevice* SaneDevice::checkPointer(void* p)
{

	SaneDevice * devp = static_cast<SaneDevice*>(p);

	size_t i;

	for (i=0 ; i<devlist.size() ; i++) {
		if (devp == devlist[i]) {
			return devlist[i];
		}
	}

	return NULL;
}







SaneDevice::SaneDevice(
		SANE_String_Const name,
		SANE_String_Const vendor,
		SANE_String_Const model,
		SANE_String_Const type)
{
	strncpy(this->s_name_real, name, sizeof(this->s_name));
	for (int i; i<sizeof(this->s_name_real); i++) {
		if (this->s_name_real[i] == ':') {
			this->s_name[i] = '.';
		} else {
			this->s_name[i] = this->s_name_real[i];
		}
	}
	strncpy(this->s_vendor, vendor, sizeof(this->s_vendor));
	strncpy(this->s_model, model, sizeof(this->s_model));
	strncpy(this->s_type, type, sizeof(this->s_type));
	this->name = this->s_name;
	this->vendor = this->s_vendor;
	this->model = this->s_model;
	this->type = this->s_type;
	this->opened = false;
	this->device_ptr = NULL;
}

SaneDevice::~SaneDevice()
{
	INFOl("~SaneDevice:")
	if (this->opened) {
		this->close();
	}
	INFOl("~SaneDevice: OK")
}

SANE_Status SaneDevice::open()
{
	SANE_Status sts;

	this->device_ptr = AbstractDevice::newInstance(this);
	if (this->device_ptr == NULL) {
		ERR(0, "unknown model. model=%s", this->model);
		return SANE_STATUS_INVAL;
	}

	sts = this->device_ptr->configure();
	if (sts != SANE_STATUS_GOOD) {
		delete this->device_ptr;
		this->device_ptr = NULL;
		return sts;
	}

	sts = this->device_ptr->open_device();
	if (sts != SANE_STATUS_GOOD) {
		delete this->device_ptr;
		this->device_ptr = NULL;
		return sts;
	}

	this->device_ptr->setSaneHandle(this);

	this->opened = true;

	return SANE_STATUS_GOOD;
}

void SaneDevice::close()
{
	this->device_ptr->close_device();
	this->device_ptr->setSaneHandle(NULL);
	TRC("delete device instance.");
	delete this->device_ptr;
	TRC("delete device instance OK.");
	this->device_ptr = NULL;
	this->opened = false;
}

const SANE_Option_Descriptor * SaneDevice::get_option_descriptor(SANE_Int option)
{
	if (!this->opened) {
		ERR(0, "device not open");
		return 0;
	}
	return this->device_ptr->get_option_descriptor(option);
}

SANE_Status SaneDevice::control_option(SANE_Int option, SANE_Action action, void *value, SANE_Int * info)
{
	if (!this->opened) {
		ERR(0, "device not open");
		return SANE_STATUS_INVAL;
	}

	return this->device_ptr->control_option(option, action, value, info);
}

SANE_Status SaneDevice::get_parameters(SANE_Parameters * params)
{
	if (!this->opened) {
		ERR(0, "device not open");
		return SANE_STATUS_INVAL;
	}

	return this->device_ptr->get_parameters(params);
}

SANE_Status SaneDevice::start()
{
	if (!this->opened) {
		ERR(0, "device not open");
		return SANE_STATUS_INVAL;
	}

	return this->device_ptr->start();
}

SANE_Status SaneDevice::read(SANE_Byte * data, SANE_Int max_length, SANE_Int * length)
{
	if (!this->opened) {
		ERR(0, "device not open");
		return SANE_STATUS_INVAL;
	}

	return this->device_ptr->read(data, max_length, length);
}

void SaneDevice::cancel()
{
	if (!this->opened) {
		ERR(0, "device not open");
		return;
	}

	this->device_ptr->cancel();
}

SANE_Status SaneDevice::set_io_mode(SANE_Bool non_blocking)
{
	if (!this->opened) {
		ERR(0, "device: not open");
		return SANE_STATUS_INVAL;
	}

	return this->device_ptr->set_io_mode(non_blocking);
}

SANE_Status SaneDevice::get_select_fd(SANE_Int * fd)
{
	if (!this->opened) {
		ERR(0, "sane_get_select_fd: not open");
		return SANE_STATUS_INVAL;
	}

	return this->device_ptr->get_select_fd(fd);
}

};

