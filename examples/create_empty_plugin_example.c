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
#include <ueum/ueum.h>
#include <ei/ei.h>

#include <stdlib.h>
#include <time.h>

/**
 * The purpose of this example is to show how to create
 * an empty plugin and save it to a specified target program.
 * The goal of create an empty plugin is that the target program
 * will update the content with a real plugin during it's running.
 */
int main(int argc, char **argv) {
	mp_memory_plugin *plugin;
	int plugin_id;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <target_path>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	ei_init();

	plugin = NULL;

	if (!ueum_is_file_exists(argv[1])) {
		ei_stacktrace_push_msg("Specified plugin target file '%s' doesn't exist", argv[1]);
		goto clean_up;
	}

	srand((unsigned int)time(0));

	ei_logger_info("Creating empty plugin...");
	if ((plugin = mp_memory_plugin_create_empty()) == NULL) {
		ei_stacktrace_push_msg("Failed to create new plugin");
		goto clean_up;
	}
	ei_logger_info("Empty plugin created");

	/**
	* save the plugin into the target path (exec or shared object). It returned the
	* corresponding id of the created plugin.
	*/
	ei_logger_info("Saving plugin to target file...");
	if ((plugin_id = mp_memory_plugin_save(plugin, argv[1], NULL)) == -1) {
		ei_stacktrace_push_msg("Failed to save new plugin to %s", argv[1]);
		goto clean_up;
	}

	ei_logger_info("Succeed to save empty plugin with id %d", plugin_id);

clean_up:
	/* destroy only the object content and not the saved plugin */
	mp_memory_plugin_destroy(plugin);
	if (ei_stacktrace_is_filled()) {
		ei_logger_stacktrace("Error occurs with the following stacktrace:");
	}
	ei_uninit();
	return EXIT_SUCCESS;
}
