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

#include <mp/mp.h>
#include <mp/utils/file_utils.h>

#include <ei/ei.h>

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

/**
 * The purpose of this example is to show how to
 * update an existing memory plugin. The existing
 * plugin may be a plugin created by create_empty_plugin_example
 * or by create_new_plugin_example.
 */
int main(int argc, char **argv) {
	mp_memory_plugin *plugin;
	int plugin_id;
	typedef void(*hello_world_func)(void);
	hello_world_func hello_world;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <plugin_id> <new_plugin_path>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	ei_init();
	plugin = NULL;

	if (!mp_is_file_exists(argv[2])) {
		ei_stacktrace_push_msg("Specified new plugin file '%s' doesn't exist", argv[2]);
		goto clean_up;
	}

	srand(time(0));
	plugin_id = atoi(argv[1]);

	/* Load plugin from id */
	ei_logger_info("Loading memory plugin from id %d", plugin_id);
	if (!(plugin = mp_memory_plugin_load(plugin_id))) {
		ei_stacktrace_push_msg("Failed to load plugin with id %d", plugin_id);
		goto clean_up;
	}
	ei_logger_info("Memory plugin loaded");

	ei_logger_info("Updating memory plugin from another library stored in disk");
	if (!mp_memory_plugin_update_from_file(plugin, argv[2])) {
		ei_stacktrace_push_msg("Failed to update content plugin from new plugin");
		goto clean_up;
	}
	ei_logger_info("Memory plugin updated");

	/* Get a function from the new plugin to check the difference */
	ei_logger_info("Getting function hello_world() from new memory plugin...");
	if (!(hello_world = mp_memory_plugin_get_function(plugin, "hello_world"))) {
		ei_stacktrace_push_msg("Failed to get hello_world function from plugin of id", plugin_id);
		goto clean_up;
	}
	ei_logger_info("hello_world function retrieved");

	hello_world();

clean_up:
	if (plugin) {
		/* Unload the plugin */
		mp_memory_plugin_unload(plugin);
	}
	if (ei_stacktrace_is_filled()) {
		ei_logger_stacktrace("Error occurs with the following stacktrace:");
	}
	ei_uninit();
	return EXIT_SUCCESS;
}
