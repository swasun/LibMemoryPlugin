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

#ifndef LIBMEMORYPLUGIN_MEMORY_PLUGIN_STRUCT_H
#define LIBMEMORYPLUGIN_MEMORY_PLUGIN_STRUCT_H

#include <mp/utils/bool.h>

#include <smo/smo.h>

#include <ms/ms.h>

#include <stddef.h>

typedef struct {
	int id;
	ms_slot *slot;
	smo_handle *object_handle;
} mp_memory_plugin;

mp_memory_plugin *mp_memory_plugin_create_new_from_buffer(unsigned char *data, size_t size);

mp_memory_plugin *mp_memory_plugin_create_new_from_file(const char *file_name);

mp_memory_plugin *mp_memory_plugin_create_empty();

void mp_memory_plugin_destroy(mp_memory_plugin *plugin);

int mp_memory_plugin_save(mp_memory_plugin *plugin, const char *target_path);

mp_memory_plugin *mp_memory_plugin_load(int id);

void mp_memory_plugin_unload(mp_memory_plugin *plugin);

void *mp_memory_plugin_get_function(mp_memory_plugin *plugin, const char *function_name);

bool mp_memory_plugin_update_from_buffer(mp_memory_plugin *plugin, unsigned char *data, size_t size);

bool mp_memory_plugin_update_from_file(mp_memory_plugin *plugin, const char *file_name);

bool mp_memory_plugin_release(mp_memory_plugin *plugin);

#endif
