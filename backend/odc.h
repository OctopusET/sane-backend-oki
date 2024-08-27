/*

 This file is part of the Scanner backend for SANE.

 This program is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the Free
 Software Foundation; either version 2 of the License, or (at your option)
 any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 more details.

 You should have received a copy of the GNU General Public License along with
 this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 Place, Suite 330, Boston, MA  02111-1307  USA

*/
/*
 * odc.h
 *
 * Copyright (c) 2012 Oki Data Corporation. All rights reserved.
*/
#ifndef odc_h
#define odc_h

#include <locale.h>
#include <libintl.h>
#define BACKEND_TEXTDOMAIN "sane-backend-"STRINGIFY(BACKEND_NAME)
#define BACKEND_I18N(text) dgettext(BACKEND_TEXTDOMAIN, text)

#include "../include/sane/sane.h"
extern "C" int sttrclevel(SANE_Status st);

#endif /* odc_h */
