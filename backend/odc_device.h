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

#ifndef ODC_DEVICE_H_
#define ODC_DEVICE_H_
extern "C" {
#include "../include/sane/sanei.h"
#include "../include/sane/saneopts.h"
}

#include "../driver/devel/boundary.h"

#define ODC_COM_NAME_NUM_OPTIONS SANE_NAME_NUM_OPTIONS
#define ODC_COM_TITLE_NUM_OPTIONS SANE_TITLE_NUM_OPTIONS
#define ODC_COM_DESC_NUM_OPTIONS SANE_DESC_NUM_OPTIONS

#define ODC_COM_NAME_STANDARD SANE_NAME_STANDARD
#define ODC_COM_TITLE_STANDARD BACKEND_I18N("Standard")	
#define ODC_COM_DESC_STANDARD BACKEND_I18N("Source, mode and resolution options")	
#define ODC_COM_NAME_GEOMETRY SANE_NAME_GEOMETRY
#define ODC_COM_TITLE_GEOMETRY BACKEND_I18N("Selected range")	
#define ODC_COM_DESC_GEOMETRY BACKEND_I18N("Scan area and media size options")	
#define ODC_COM_NAME_ENHANCEMENT SANE_NAME_ENHANCEMENT
#define ODC_COM_TITLE_ENHANCEMENT BACKEND_I18N("Enhancement")	
#define ODC_COM_DESC_ENHANCEMENT BACKEND_I18N("Image modification options")	
#define ODC_COM_NAME_ADVANCED SANE_NAME_ADVANCED
#define ODC_COM_TITLE_ADVANCED BACKEND_I18N("Advanced")	
#define ODC_COM_DESC_ADVANCED BACKEND_I18N("Hardware specific options")	
#define ODC_COM_NAME_SENSORS SANE_NAME_SENSORS
#define ODC_COM_TITLE_SENSORS BACKEND_I18N("Sensors")	
#define ODC_COM_DESC_SENSORS BACKEND_I18N("Scanner sensors and buttons")	


namespace device {

extern const char* LTC_DEFAULT;

struct Option
{
	SANE_Option_Descriptor desc;	
	union
	{
	  SANE_Bool b;					
	  SANE_Word w;					
	  SANE_Word *wa;					
	  SANE_String s;					
	} val;								
	SANE_Bool loaded;				

	Option();
	virtual ~Option();

	virtual SANE_Status set_auto() = 0;

	virtual SANE_Status set_value(void *value, SANE_Int * info) = 0;

	virtual SANE_Status get_value(void *value) = 0;

	void set_value_b(SANE_Bool value) {this->val.b = value;}	
	void set_value_w(SANE_Word value) {this->val.w = value;}	
	void set_value_s(SANE_String value) {this->val.s = value;}

	void clear_constraint();
	void set_constraint(SANE_String_Const *string_list, SANE_String_Const initval = NULL);
	void set_constraint(const SANE_Word *word_list);
	void set_constraint(const SANE_Word *word_list, SANE_Word initval);
	void set_constraint(const SANE_Range *range);
	void set_constraint(const SANE_Range *range, SANE_Word initval);

	void set_active() {this->desc.cap &= ~SANE_CAP_INACTIVE;}
	void set_inactive() {this->desc.cap |= SANE_CAP_INACTIVE;}
	void set_hard_select() {this->desc.cap &= ~SANE_CAP_SOFT_SELECT; this->desc.cap |= SANE_CAP_HARD_SELECT;}
	void set_soft_select() {this->desc.cap &= ~SANE_CAP_HARD_SELECT; this->desc.cap |= SANE_CAP_SOFT_SELECT;}

	bool is_active() {return (SANE_OPTION_IS_ACTIVE(this->desc.cap) == SANE_TRUE) ? true : false;}
	bool is_settable() {return (SANE_OPTION_IS_SETTABLE(this->desc.cap) == SANE_TRUE) ? true : false;}
};


struct OptionBool : public Option
{
	OptionBool(
			SANE_String_Const name,
			SANE_String_Const title,
			SANE_String_Const desc,
			SANE_Bool value = SANE_FALSE
	);

	virtual SANE_Status set_auto();
	virtual SANE_Status set_value(void *value, SANE_Int * info);
	virtual SANE_Status get_value(void *value);

	void set_info(SANE_Int i) {this->infoval=i;}
protected:
	SANE_Int infoval;
};


struct OptionInt : public Option
{
	OptionInt(
			SANE_String_Const name,
			SANE_String_Const title,
			SANE_String_Const desc,
			SANE_Word value,
			SANE_Unit unit = SANE_UNIT_NONE
	);
	OptionInt(
			SANE_String_Const name,
			SANE_String_Const title,
			SANE_String_Const desc,
			const SANE_Word *word_list,
			SANE_Word value,
			SANE_Unit unit = SANE_UNIT_NONE
	);
	OptionInt(
			SANE_String_Const name,
			SANE_String_Const title,
			SANE_String_Const desc,
			const SANE_Range *range,
			SANE_Word value,
			SANE_Unit unit = SANE_UNIT_NONE
	);

	virtual SANE_Status set_auto();
	virtual SANE_Status set_value(void *value, SANE_Int * info);
	virtual SANE_Status get_value(void *value);

	void set_info(SANE_Int i) {this->infoval=i;}
protected:
	SANE_Int infoval;
};


struct OptionFixed : public Option
{
	OptionFixed(
			SANE_String_Const name,
			SANE_String_Const title,
			SANE_String_Const desc,
			SANE_Word value,
			SANE_Unit unit = SANE_UNIT_NONE
	);
	OptionFixed(
			SANE_String_Const name,
			SANE_String_Const title,
			SANE_String_Const desc,
			const SANE_Word *word_list,
			SANE_Word value,
			SANE_Unit unit = SANE_UNIT_NONE
	);
	OptionFixed(
			SANE_String_Const name,
			SANE_String_Const title,
			SANE_String_Const desc,
			const SANE_Range *range,
			SANE_Word value,
			SANE_Unit unit = SANE_UNIT_NONE
	);

	virtual SANE_Status set_auto();
	virtual SANE_Status set_value(void *value, SANE_Int * info);
	virtual SANE_Status get_value(void *value);

	void set_info(SANE_Int i) {this->infoval=i;}
protected:
	SANE_Int infoval;
};


struct OptionString : public Option
{
	OptionString(
			SANE_String_Const name,
			SANE_String_Const title,
			SANE_String_Const desc,
			SANE_Int maxlength
	);
	OptionString(
			SANE_String_Const name,
			SANE_String_Const title,
			SANE_String_Const desc,
			SANE_String_Const *string_list,
			SANE_String_Const initval = NULL
	);
	~OptionString();

	virtual SANE_Status set_auto();
	virtual SANE_Status set_value(void *value, SANE_Int * info);
	virtual SANE_Status get_value(void *value);

	void set_info(SANE_Int i) {this->infoval=i;}
protected:
	SANE_Int infoval;
};


struct OptionButton : public Option
{
	OptionButton(
			SANE_String_Const name,
			SANE_String_Const title,
			SANE_String_Const desc
	);

	virtual SANE_Status set_auto();
	virtual SANE_Status set_value(void *value, SANE_Int * info);
	virtual SANE_Status get_value(void *value);
};


struct OptionGroup : public Option
{
	OptionGroup(
			SANE_String_Const name,
			SANE_String_Const title,
			SANE_String_Const desc
	);

	virtual SANE_Status set_auto();
	virtual SANE_Status set_value(void *value, SANE_Int * info);
	virtual SANE_Status get_value(void *value);
};



struct OptNumOpts : public OptionInt
{
	OptNumOpts(SANE_Word numopts);
};




struct AbstractDevice
{
	static AbstractDevice* newInstance(SANE_Device* sane_device);


	AbstractDevice(SANE_Device* sane_device);
	virtual ~AbstractDevice();
	void setSaneHandle(SANE_Handle h) {this->handle = h;}
	SANE_Handle getSaneHandle() {return this->handle;}

	virtual SANE_Status configure();

	virtual SANE_Status open_device();
	virtual void close_device();
	virtual SANE_Status calibration();
	const SANE_Option_Descriptor * get_option_descriptor (SANE_Int option);
	SANE_Status control_option (SANE_Int option, SANE_Action action, void *value, SANE_Int * info);
	virtual SANE_Status get_parameters (SANE_Parameters * params);
	virtual SANE_Status start ();
	virtual SANE_Status read (SANE_Byte * data, SANE_Int max_length, SANE_Int * length);
	virtual void cancel ();
	virtual SANE_Status set_io_mode (SANE_Bool non_blocking);
	virtual SANE_Status get_select_fd (SANE_Int * fd);


protected:
	SANE_Device* sane_device;	
	SANE_Handle handle;	
	SANE_Int number_of_options;	
	Option** options;	
	SANE_Parameters params;	
	SANE_Bool open;	

	OptionGroup o_com_grp_standard;	
	OptionGroup o_com_grp_geometry;	
	OptionGroup o_com_grp_enhancement;	
	OptionGroup o_com_grp_advanced;	
	OptionGroup o_com_grp_sensors;	

	void print_options();
	SANE_Status alloc_options_mem(SANE_Int num);
};


struct SaneDevice : public SANE_Device
{
	SaneDevice(
			SANE_String_Const name,
			SANE_String_Const vendor,
			SANE_String_Const model,
			SANE_String_Const type);
	~SaneDevice();
	SANE_Status open();
	void close();
	bool is_opened() {return this->opened;}
	const SANE_Option_Descriptor * get_option_descriptor(SANE_Int option);
	SANE_Status control_option(SANE_Int option, SANE_Action action, void *value, SANE_Int * info);
	SANE_Status get_parameters(SANE_Parameters * params);
	SANE_Status start();
	SANE_Status read(SANE_Byte * data, SANE_Int max_length, SANE_Int * length);
	void cancel();
	SANE_Status set_io_mode(SANE_Bool non_blocking);
	SANE_Status get_select_fd(SANE_Int * fd);

	const char* get_real_name() {return this->s_name_real;}

	static SANE_Status initDeviceList();
	static void cleanDeviceList();
	static const SaneDevice** getDevices();
	static SaneDevice* findDevice(const char* name);
	static SaneDevice* checkPointer(void* p);

private:
	char s_name_real[32];
	char s_name[32];
	char s_vendor[32];
	char s_model[32];
	char s_type[32];
	bool opened;	
	AbstractDevice* device_ptr;	

	static odc::PointerList<SaneDevice> devlist;	
};

void DBG_ShowProperties(const odc::Properties* prop);
void DBG_InfoToString(SANE_Int info, char* out);

};

#endif /* ODC_DEVICE_H_ */
