/*****************************************************************************
* Copyright (C) 2018 by Charly Lamothe										 *
*																		     *
* This file is part of LibMemoryPlugin.                                      *
*																			 *
*   LibMemoryPlugin is free software: you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by	 *
*   the Free Software Foundation, either version 3 of the License, or		 *
*   (at your option) any later version.										 *
*																			 *
*   LibMemoryPlugin is distributed in the hope that it will be useful,		 *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of			 *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the			 *
*   GNU General Public License for more details.							 *
*																			 *
*   You should have received a copy of the GNU General Public License        *
*   along with LibMemoryPlugin.  If not, see <http://www.gnu.org/licenses/>. *
******************************************************************************/

#include <mp/utils/file_utils.h>
#include <mp/utils/alloc.h>

#include <ei/ei.h>

#include <Windows.h>
#include <stdio.h>

bool mp_is_file_exists(const char *file_name) {
#if defined(__unix__)
	struct stat st;
#elif defined(_WIN32) || defined(_WIN64)
	DWORD dw_attrib;
#else
#error "OS not supported"
#endif

#if defined(__unix__)
	if (stat(file_name, &st) == 0) {
		return S_ISREG(st.st_mode);
	}
	return false;
#elif defined(_WIN32) || defined(_WIN64)
	dw_attrib = GetFileAttributesA(file_name);
	if (dw_attrib != INVALID_FILE_ATTRIBUTES &&
		dw_attrib != FILE_ATTRIBUTE_DIRECTORY) {
		return true;
	}
#endif

	return false;
}
