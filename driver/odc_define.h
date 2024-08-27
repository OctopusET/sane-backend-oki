/*
 * odc_define.h
 *
 * Copyright (c) 2012 Oki Data Corporation. All rights reserved.
 */

#ifndef ODC_DEFINE_H_
#define ODC_DEFINE_H_

#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

#define STRINGIFY1(x)	#x
#define STRINGIFY(x)	STRINGIFY1(x)


#define OEMNAME STRINGIFY(OEM)



#include <locale.h>
#include <libintl.h>
#define DRV_TEXTDOMAIN OEMNAME"mfpdrv"
#define DRV_I18N(text) dgettext(DRV_TEXTDOMAIN, text)


#define HT (0x09)	
#define LF (0x0a)	
#define FF (0x0c)	
#define CR (0x0d)	
#define ESC (0x1b)	


#define MM_INCH 	(25.4)



#define MAX_VENDORNAME_LEN		(48)

#define MAX_VENDORCODE_LEN		(16)

#define MAX_MODELNAME_LEN		(32)

#define MAX_DEVICENAME_LEN		(96)

#define MAX_DEVTYPENAME_LEN	(64)

#define MAX_HOSTNAME_LEN		(256)

#define MAX_LTC_LEN				(16)

#define MAX_PROPNAME_LEN 		(32)
#define MAX_PORPSTR_LEN 		(224)


#define PCSCAN_DEFAULT_SCANER_PORT	(9967)
#define PCSCAN_DEFAULT_LOCAL_PORT		(9968)
#define PORT9100_PORT		(9100)


#define DEF_SCAN_RESOLUTION_MIN	(50.0)
#define DEF_SCAN_RESOLUTION_MAX	(1200.0)
#define DEF_SCAN_RESOLUTION_QUANT	(1.0)


#include "devel/libstatus.h"


#endif /* ODC_DEFINE_H_ */
