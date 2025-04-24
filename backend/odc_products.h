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

#ifndef ODC_FX750_H_
#define ODC_FX750_H_
#include "odc.h"
#include "odc_device.h"

#include "../driver/devel/boundary.h"
#include "../driver/devel/scandifc.h"

#define FX750_NAME_SCAN_SOURCE SANE_NAME_SCAN_SOURCE
#define FX750_TITLE_SCAN_SOURCE BACKEND_I18N("Scan source")	
#define FX750_DESC_SCAN_SOURCE BACKEND_I18N("Selects the scan source. If \"Auto\" is selected, and there is a document in the ADF, scanning is implemented from the ADF. If there is no document in the ADF, scanning is implemented from the flatbed.")	
#define FX750_VALUE_SCAN_SOURCE_AUTO BACKEND_I18N("Auto")
#define FX750_VALUE_SCAN_SOURCE_FLATBED BACKEND_I18N ("Flatbed")
#define FX750_VALUE_SCAN_SOURCE_ADF BACKEND_I18N ("Automatic Document Feeder (ADF)")


#define FX750_NAME_SCAN_MODE SANE_NAME_SCAN_MODE
#define FX750_TITLE_SCAN_MODE BACKEND_I18N("Color mode")	
#define FX750_DESC_SCAN_MODE BACKEND_I18N("Selects color mode.")	
#define FX750_VALUE_SCAN_MODE_COLOR BACKEND_I18N("Color")	
#define FX750_VALUE_SCAN_MODE_GRAY BACKEND_I18N("Gray")	
#define FX750_VALUE_SCAN_MODE_BW BACKEND_I18N ("Black and white")
#define FX750_VALUE_SCAN_MODE_HALFTONE BACKEND_I18N("Halftone")	


#define FX750_NAME_SCAN_RESOLUTION SANE_NAME_SCAN_RESOLUTION
#define FX750_TITLE_SCAN_RESOLUTION BACKEND_I18N("Scan resolution")	
#define FX750_DESC_SCAN_RESOLUTION BACKEND_I18N("Determines the scan image sharpness. Normally, the higher the resolution (i.e., the greater the numerical value), the sharper the image, but greater scan time, memory, and disk space are required. Scan using resolution settings that conform to your objective.")	
#define FX750_NAME_SCAN_X_RESOLUTION SANE_NAME_SCAN_X_RESOLUTION		
#define FX750_TITLE_SCAN_X_RESOLUTION BACKEND_I18N("X-resolution")	
#define FX750_DESC_SCAN_X_RESOLUTION BACKEND_I18N("Sets the horizontal resolution of the scanned image.")	
#define FX750_NAME_SCAN_Y_RESOLUTION SANE_NAME_SCAN_Y_RESOLUTION		
#define FX750_TITLE_SCAN_Y_RESOLUTION BACKEND_I18N("Y-resolution")	
#define FX750_DESC_SCAN_Y_RESOLUTION BACKEND_I18N("Sets the vertical resolution of the scanned image.")	


#define FX750_NAME_PREVIEW_MODE SANE_NAME_PREVIEW
#define FX750_TITLE_PREVIEW_MODE SANE_TITLE_PREVIEW
#define FX750_DESC_PREVIEW_MODE SANE_DESC_PREVIEW


#define FX750_NAME_DUPSCAN_GROUP "duplex-scanning-group"
#define FX750_TITLE_DUPSCAN_GROUP BACKEND_I18N("Duplex Scanning")
#define FX750_NAME_DUPSCAN "duplex-scanning-onoff"
#define FX750_TITLE_DUPSCAN BACKEND_I18N("Duplex Scanning")
#define FX750_DESC_DUPSCAN BACKEND_I18N("Duplex scanning can be selected when the scanning method is set to ADF. You can also set the bookbinding position of the document.")
#define FX750_NAME_BINDING_DIR "binding-direction"
#define FX750_TITLE_BINDING_DIR BACKEND_I18N("Binding direction")
#define FX750_DESC_BINDING_DIR BACKEND_I18N("Choose the direction of the original.")
#define FX750_VALUE_BINDING_RIGHTLEFT BACKEND_I18N("Right and Left Binding")
#define FX750_VALUE_BINDING_TOP BACKEND_I18N("Top Binding")


#define FX750_NAME_FILTER_GROUP "filter-group"
#define FX750_TITLE_FILTER_GROUP BACKEND_I18N("Filter")


#define FX750_NAME_SHARPNESS "sharpness"
#define FX750_TITLE_SHARPNESS BACKEND_I18N("Sharpness")
#define FX750_DESC_SHARPNESS BACKEND_I18N("Raise the contrast of the pixels adjacent to the image to sharpen fuzzy photos. Enabled when color mode is set to either gray scale or color.")
#define FX750_VALUE_SHARPNESS_NONE BACKEND_I18N("None")
#define FX750_VALUE_SHARPNESS_SHARPEN BACKEND_I18N("Sharpen")
#define FX750_VALUE_SHARPNESS_SHARPEN_MORE BACKEND_I18N("Sharpen More")


#define FX750_NAME_BGELIMINAT "bg-eliminat-level"
#define FX750_TITLE_BGELIMINAT BACKEND_I18N("Background Elimination")
#define FX750_DESC_BGELIMINAT BACKEND_I18N("Remove the mixed color of the scanned image on which the corresponding parts shall be white.")
#define FX750_VALUE_BGELIMINAT_LEVEL0 BACKEND_I18N("Level0")
#define FX750_VALUE_BGELIMINAT_LEVEL1 BACKEND_I18N("Level1")
#define FX750_VALUE_BGELIMINAT_LEVEL2 BACKEND_I18N("Level2")
#define FX750_VALUE_BGELIMINAT_LEVEL3 BACKEND_I18N("Level3")
#define FX750_VALUE_BGELIMINAT_LEVEL4 BACKEND_I18N("Level4")
#define FX750_VALUE_BGELIMINAT_LEVEL5 BACKEND_I18N("Level5")
#define FX750_VALUE_BGELIMINAT_LEVEL6 BACKEND_I18N("Level6")


#define FX750_NAME_EDGEERACE "edge-erase-onoff"
#define FX750_TITLE_EDGEERACE BACKEND_I18N("Edge Erase")
#define FX750_DESC_EDGEERACE BACKEND_I18N("Mask edge areas of the document.(Erases shaded areas in the document.)")
#define FX750_NAME_EDGEERACE_RANGE "edge-erase-range"
#define FX750_TITLE_EDGEERACE_RANGE BACKEND_I18N("Edge Erase Range")
#define FX750_DESC_EDGEERACE_RANGE BACKEND_I18N("Set Edge Erase Range")


#define FX750_NAME_FONTSMOOTHING "font-smoothing-onoff"
#define FX750_TITLE_FONTSMOOTHING BACKEND_I18N("Font Smoothing")
#define FX750_DESC_FONTSMOOTHING BACKEND_I18N("Make text appear clearer. If  checkbox is enabled, text will appear smoother.")


#define FX750_NAME_MOIREELIMINATION "moire-elimination-onoff"
#define FX750_TITLE_MOIREELIMINATION BACKEND_I18N("Moire Elimination")
#define FX750_DESC_MOIREELIMINATION BACKEND_I18N("Enable this checkbox to eliminate moire (wavy lines).")


#define FX750_NAME_MARGIN_GROUP "margin-group"
#define FX750_TITLE_MARGIN_GROUP BACKEND_I18N("Margin")
#define FX750_NAME_MARGIN_RIGHT "r-margin"
#define FX750_TITLE_MARGIN_RIGHT BACKEND_I18N("Right margin")
#define FX750_DESC_MARGIN_RIGHT BACKEND_I18N("Set right margin")
#define FX750_NAME_MARGIN_BOTTOM "b-margin"
#define FX750_TITLE_MARGIN_BOTTOM BACKEND_I18N("Bottom margin")
#define FX750_DESC_MARGIN_BOTTOM BACKEND_I18N("Set bottom margin")


#define FX750_NAME_SCAN_TL_X SANE_NAME_SCAN_TL_X
#define FX750_TITLE_SCAN_TL_X BACKEND_I18N("Top-left x")	
#define FX750_DESC_SCAN_TL_X BACKEND_I18N("Top-left x position of scan area.")	
#define FX750_NAME_SCAN_TL_Y SANE_NAME_SCAN_TL_Y
#define FX750_TITLE_SCAN_TL_Y BACKEND_I18N("Top-left y")	
#define FX750_DESC_SCAN_TL_Y BACKEND_I18N("Top-left y position of scan area.")	
#define FX750_NAME_SCAN_BR_X SANE_NAME_SCAN_BR_X
#define FX750_TITLE_SCAN_BR_X BACKEND_I18N("Bottom-right x")	
#define FX750_DESC_SCAN_BR_X BACKEND_I18N("Bottom-right x position of scan area.")	
#define FX750_NAME_SCAN_BR_Y SANE_NAME_SCAN_BR_Y
#define FX750_TITLE_SCAN_BR_Y BACKEND_I18N("Bottom-right y")	
#define FX750_DESC_SCAN_BR_Y BACKEND_I18N("Bottom-right y position of scan area.")	


#define FX750_NAME_CALIBRATION "Calibration"
#define FX750_TITLE_CALIBRATION "Calibration"
#define FX750_DESC_CALIBRATION ""




#define RETRY_INTERVAL_USEC ((useconds_t)(250 * 1000)) 

namespace products {

struct DeviceBaseImpl : public device::AbstractDevice
{
	struct ScanResolution : public device::OptionFixed
	{
		ScanResolution(AbstractDevice* owner, const char* name, const char* title, const char* desc);
		virtual SANE_Status set_value(void *value, SANE_Int * info);
		virtual SANE_Status get_value(void *value);
		SANE_Range range;
	protected:
		AbstractDevice* owner;
	};

	struct ScanSource : public device::OptionString
	{
		ScanSource(AbstractDevice* owner);
		virtual SANE_Status set_value(void *value, SANE_Int * info);
	protected:
		AbstractDevice* owner;
	};

	struct DuplexScanningOnOff : public device::OptionBool
	{
		DuplexScanningOnOff(AbstractDevice* owner);
		virtual SANE_Status set_value(void *value, SANE_Int * info);
	protected:
		AbstractDevice* owner;
	};

	struct EdgeEraseOnOff : public device::OptionBool
	{
		EdgeEraseOnOff(AbstractDevice* owner);
		virtual SANE_Status set_value(void *value, SANE_Int * info);
	protected:
		AbstractDevice* owner;
	};


	struct CalibrationButton : public device::OptionButton
	{
		CalibrationButton(AbstractDevice* owner);
		virtual SANE_Status set_value(void *value, SANE_Int * info);
	protected:
		AbstractDevice* owner;
	};



	DeviceBaseImpl(SANE_Device* sane_device);
	virtual ~DeviceBaseImpl();

	virtual SANE_Status configure();

	virtual SANE_Status open_device();
	virtual void close_device();
	virtual SANE_Status calibration();

	virtual SANE_Status get_parameters (SANE_Parameters * params);
	virtual SANE_Status start ();
	virtual SANE_Status read (SANE_Byte * data, SANE_Int max_length, SANE_Int * length);
	virtual void cancel ();
	virtual SANE_Status set_io_mode (SANE_Bool non_blocking);
	virtual SANE_Status get_select_fd (SANE_Int * fd);

protected:
	static SANE_String_Const source_list[];			
	static SANE_String_Const mode_list[];			
	static SANE_Range resolution_range;				
	static SANE_Word resolution_list[];				

	static SANE_String_Const sharpness_list[];		
	static SANE_String_Const binding_list[];		
	static SANE_Range margin_range;					
	static SANE_Range edgeerase_range;				
	static SANE_String_Const bgeliminat_list[];		

	enum device_options
	{
		opt_num_opts = 0,

		opt_std_group,
		opt_scan_source,
		opt_mode,
		opt_resolution,
		opt_preview_mode,

		opt_dupscan_group,
		opt_duplex_scan_onoff,
		opt_binding,

		opt_filter_group,
		opt_sharpness,
		opt_bg_eliminat_level,
		opt_edge_erase_onoff,
		opt_edge_erase_range,
		opt_font_smoothing_onoff,
		opt_moire_elimination_onoff,

		opt_geometry_group,
		opt_tl_x,
		opt_tl_y,
		opt_br_x,
		opt_br_y,

#if defined(ENABLE_MARGIN)
		opt_margin_group,
		opt_margin_right,
		opt_margin_bottom,
#endif /*defined(ENABLE_MARGIN)*/


		num_options
	};
	device::Option* o_list[num_options];

	device::OptNumOpts o_numopts;

	ScanSource o_scan_source;
	device::OptionString o_mode;
	ScanResolution o_x_resolution;
	device::OptionBool o_preview_mode;

	device::OptionGroup o_dupscan_group;
	DuplexScanningOnOff o_duplex_scan_onoff;
	device::OptionString o_binding;

	device::OptionGroup o_filter_group;
	device::OptionString o_sharpness;
	device::OptionString o_bg_eliminat_level;
	EdgeEraseOnOff o_edge_erase_onoff;
	device::OptionFixed o_edge_erase_range;
	device::OptionBool o_font_smoothing_onoff;
	device::OptionBool o_moire_elimination_onoff;

	device::OptionFixed o_tlX;
	device::OptionFixed o_tlY;
	device::OptionFixed o_brX;
	device::OptionFixed o_brY;

#if defined(ENABLE_MARGIN)
	device::OptionGroup o_margin_group;
	device::OptionFixed o_margin_right;
	device::OptionFixed o_margin_bottom;
#endif /*defined(ENABLE_MARGIN)*/


	SANE_Range geometry_x_range;
	SANE_Range geometry_y_range;

	unsigned char buffer[4 + 1024*16];
	size_t buffer_offset;
	size_t buffer_received;

	scandif scif;


	bool use_adf;

	size_t scan_data_size;
	size_t trns_data_size;

	bool non_blocking;


	bool scanning;
	bool canceled;
	bool return_eof;


	int get_scan_source();
	int get_scan_mode();
	int get_option_duplex_scan();
	int get_option_sharpness();
	int get_option_bgelm();
	int get_option_edge_erase();

	SANE_Status build_scanreq(scand_msg_t* msg);
	SANE_Status update_sane_parameters(SANE_Parameters *p, unsigned mode, size_t w, size_t h, size_t bytes);

	static uint8_t statusLevel(SANE_Status s);
	static SANE_Status libsts2sanests(odc::LibStatus s);
};

};

#endif /* ODC_FX750_H_ */
