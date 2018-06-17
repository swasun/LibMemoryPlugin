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

#include <mp/api/memory_plugin.h>
#include <mp/utils/alloc.h>

#include <ei/ei.h>

#include <stdio.h>

#define EMPTY_SLOT_DATA "<AVAILABLE>"
#define EMPTY_SLOT_SIZE 11
#define RANDOM_ID_TRIES_NUMBER 10
#define MIN_SLOT_ID 100
#define MAX_SLOT_ID 200

mp_memory_plugin *mp_memory_plugin_create_new_from_buffer(unsigned char *data, size_t size) {
	mp_memory_plugin *plugin;

	ei_check_parameter_or_return(data);
	ei_check_parameter_or_return(size > 0);

	mp_safe_alloc(plugin, mp_memory_plugin, 1);
	plugin->object_handle = NULL;
	plugin->id = -1;
	if (!(plugin->slot = ms_slot_create_from_memory(data, size))) {
		ei_stacktrace_push_msg("Failed to create slot from buffer");
		mp_safe_free(plugin);
		return NULL;
	}

	return plugin;
}

mp_memory_plugin *mp_memory_plugin_create_new_from_file(const char *file_name) {
	mp_memory_plugin *plugin;

	ei_check_parameter_or_return(file_name);

	mp_safe_alloc(plugin, mp_memory_plugin, 1);
	plugin->object_handle = NULL;
	plugin->id = -1;
	if (!(plugin->slot = ms_slot_create_from_file(file_name))) {
		ei_stacktrace_push_msg("Failed to create slot from file");
		mp_safe_free(plugin);
		return NULL;
	}

	return plugin;
}

mp_memory_plugin *mp_memory_plugin_create_empty() {
	mp_memory_plugin *plugin;

	mp_safe_alloc(plugin, mp_memory_plugin, 1);
	plugin->object_handle = NULL;
	plugin->id = -1;
	if (!(plugin->slot = ms_slot_create_from_memory(EMPTY_SLOT_DATA, EMPTY_SLOT_SIZE))) {
		ei_stacktrace_push_msg("Failed to create slot from buffer");
		mp_safe_free(plugin);
		return NULL;
	}

	return plugin;
}

void mp_memory_plugin_destroy(mp_memory_plugin *plugin) {
	if (plugin) {
		smo_handle_destroy(plugin->object_handle);
		ms_slot_destroy(plugin->slot);
		mp_safe_free(plugin);
	}
}

int mp_memory_plugin_save(mp_memory_plugin *plugin, const char *target_path) {
	int *ids, ids_number, current_id, id, i, j;
	bool id_already_exist;

	ei_check_parameter_or_return(plugin);
	ei_check_parameter_or_return(plugin->slot);

	id = -1;

	if (!(ids = ms_slot_find_ids_from_file(target_path, &ids_number))) {
		/* @todo how to handle this special case ? */
		if (ei_stacktrace_is_filled()) {
			ei_logger_stacktrace("Failed to list existing slot ids");
			return -1;
		}
		id = MIN_SLOT_ID;
	}
	else {
		/**
		 * Calculate a new id in range (MIN_SLOT_ID, MAX_SLOT_ID)
		 * excluding the ids already used.
		 *
		 * @todo reduce the complexity
		 */
		for (i = 0; i < RANDOM_ID_TRIES_NUMBER; i++) {
			id = (rand() % (MAX_SLOT_ID - MIN_SLOT_ID + 1)) + MIN_SLOT_ID;
			id_already_exist = false;
			for (j = 0; j < ids_number; j++) {
				current_id = ids[j];
				if (current_id == id) {
					id_already_exist = true;
					break;
				}
			}
			if (!id_already_exist) {
				id = current_id;
				break;
			}
		}
		mp_safe_free(ids);
	}

	if (id == -1) {
		ei_stacktrace_push_msg("No more slot available. Release plugin or increase the max number of plugins");
		return -1;
	}

	if (!ms_slot_save_to_file(plugin->slot, id, target_path)) {
		ei_stacktrace_push_msg("Failed to save slot to specified target file");
		return -1;
	}

	return id;
}

mp_memory_plugin *mp_memory_plugin_load(int id) {
	mp_memory_plugin *plugin;
	ms_slot *loaded_slot;
	smo_handle *object_handle_opened;
	char *string_id;

	if (id < MIN_SLOT_ID || id > MAX_SLOT_ID) {
		ei_stacktrace_push_msg("Specified id %d is out of bound of (%d, %d)", id, MIN_SLOT_ID, MAX_SLOT_ID);
		return NULL;
	}
	
	if (!(loaded_slot = ms_slot_load_from_memory(id))) {
		ei_stacktrace_push_msg("Failed to load slot with id %d", id);
		return NULL;
	}

	mp_safe_alloc(string_id, char, 3);
	sprintf(string_id, "%d", id);

	if (!(object_handle_opened = smo_open(string_id, ms_slot_get_data(loaded_slot), ms_slot_get_size(loaded_slot)))) {
		ei_stacktrace_push_msg("Failed to load object handle from loaded slot");
		ms_slot_destroy(loaded_slot);
		mp_safe_free(string_id);
		return NULL;
	}

	mp_safe_free(string_id);

	mp_safe_alloc(plugin, mp_memory_plugin, 1);
	plugin->slot = loaded_slot;
	plugin->object_handle = object_handle_opened;
	plugin->id = id;

	return plugin;
}

void mp_memory_plugin_unload(mp_memory_plugin *plugin) {
	if (!plugin) {
		ei_logger_warn("Specified ptr is null. Already unload ?");
		return;
	}

	if (!plugin->object_handle) {
		ei_logger_warn("Object handle ptr of specified plugin is null. Already unload ?");
	}
	else if (!smo_close(plugin->object_handle)) {
		ei_logger_error("Failed to close object handle");
	}
	plugin->object_handle = NULL;

	if (!plugin->slot) {
		ei_logger_warn("Slot ptr of specified plugin is null. Already unload ?");
	}
	else {
		ms_slot_destroy(plugin->slot);
		plugin->slot = NULL;
	}

	mp_safe_free(plugin);
}

void *mp_memory_plugin_get_function(mp_memory_plugin *plugin, const char *function_name) {
	void *function_ptr;

	if (!plugin) {
		ei_stacktrace_push_msg("Specified plugin ptr is null");
		return NULL;
	}

	if (!plugin->object_handle) {
		ei_stacktrace_push_msg("Object handle ptr of specified plugin is null");
		return NULL;
	}

	if (!function_name) {
		ei_stacktrace_push_msg("Specified function name ptr is null");
		return NULL;
	}

	if (!(function_ptr = smo_get_function(plugin->object_handle, function_name))) {
		ei_stacktrace_push_msg("Failed to find speicified function in loaded shared object handle");
		return NULL;
	}

	return function_ptr;
}

bool mp_memory_plugin_update_from_buffer(mp_memory_plugin *plugin, unsigned char *data, size_t size) {
	char *string_id;

	ei_check_parameter_or_return(plugin);
	ei_check_parameter_or_return(data);
	ei_check_parameter_or_return(size > 0);

	if (plugin->object_handle) {
		if (!smo_close(plugin->object_handle)) {
			ei_stacktrace_push_msg("Failed to close object handle");
			return false;
		}
	}

	ms_slot_destroy(plugin->slot);

	if (!(plugin->slot = ms_slot_create_from_memory(data, size))) {
		ei_stacktrace_push_msg("Failed to create new slot from memory");
		return false;
	}

	mp_safe_alloc(string_id, char, 3);
	sprintf(string_id, "%d", plugin->id);

	if (!(plugin->object_handle = smo_open(string_id, ms_slot_get_data(plugin->slot), ms_slot_get_size(plugin->slot)))) {
		ei_stacktrace_push_msg("Failed to load new object handle");
		mp_safe_free(string_id);
		return false;
	}

	mp_safe_free(string_id);

	return true;
}

bool mp_memory_plugin_update_from_file(mp_memory_plugin *plugin, const char *file_name) {
	char *string_id;

	ei_check_parameter_or_return(plugin);
	ei_check_parameter_or_return(file_name);

	if (plugin->object_handle) {
		if (!smo_close(plugin->object_handle)) {
			ei_stacktrace_push_msg("Failed to close object handle");
			return false;
		}
	}

	ms_slot_destroy(plugin->slot);

	if (!(plugin->slot = ms_slot_create_from_file(file_name))) {
		ei_stacktrace_push_msg("Failed to create new slot from memory");
		return false;
	}

	mp_safe_alloc(string_id, char, 3);
	sprintf(string_id, "%d", plugin->id);

	if (!(plugin->object_handle = smo_open(string_id, ms_slot_get_data(plugin->slot), ms_slot_get_size(plugin->slot)))) {
		ei_stacktrace_push_msg("Failed to load new object handle");
		mp_safe_free(string_id);
		return false;
	}

	mp_safe_free(string_id);

	return true;
}

bool mp_memory_plugin_release(mp_memory_plugin *plugin) {
	bool result;
	int slot_id;

	ei_check_parameter_or_return(plugin);

	if (plugin->object_handle) {
		if (!smo_close(plugin->object_handle)) {
			ei_stacktrace_push_msg("Failed to close object handle");
			return false;
		}
	}

	slot_id = plugin->id;

	ms_slot_destroy(plugin->slot);

	if (!(plugin->slot = ms_slot_create_from_memory(EMPTY_SLOT_DATA, EMPTY_SLOT_SIZE))) {
		ei_stacktrace_push_msg("Failed to create empty slot from memory");
		return false;
	}

	if (!(result = ms_slot_save_to_memory(plugin->slot, slot_id))) {
		ei_stacktrace_push_msg("Failed to save empty plugin");
	}

	ms_slot_destroy(plugin->slot);

	return result;
}
