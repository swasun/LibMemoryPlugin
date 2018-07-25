/******************************************************************************
 * Copyright (C) 2018 by Charly Lamothe                                       *
 *                                                                            *
 * This file is part of LibMemoryPlugin.                                      *
 *                                                                            *
 *   LibMemoryPlugin is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by     *
 *   the Free Software Foundation, either version 3 of the License, or        *
 *   (at your option) any later version.                                      *
 *                                                                            *
 *   LibMemoryPlugin is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU General Public License for more details.                             *
 *                                                                            *
 *   You should have received a copy of the GNU General Public License        *
 *   along with LibMemoryPlugin.  If not, see <http://www.gnu.org/licenses/>. *
 ******************************************************************************/

#include <mp/mp.h>
#include <ueum/ueum.h>
#include <ei/ei.h>

#if defined(WITH_CRYPTO)

#include <uecm/uecm.h>

#endif

#include <stdlib.h>
#include <time.h>

/**
* The purpose of this example is to show how to create
* a plugin from a library stored in disk, and save it in a
* target program.
* Example of usage: ./create_new_plugin_example load_plugin_example.exe hello_world_plugin.dll
* The returned id will be used to load or update the created plugin.
*/
int main(int argc, char **argv) {
    mp_memory_plugin *plugin;
    mp_entry *entry;
    int plugin_id;
    void *crypto_metadata;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <plugin_source_path> <target_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    ei_init();

#if defined(WITH_CRYPTO)
    uecm_init();
#endif

    plugin = NULL;
    entry = NULL;
    crypto_metadata = NULL;

    if (!ueum_is_file_exists(argv[1])) {
        ei_stacktrace_push_msg("Specified plugin source file '%s' doesn't exist", argv[1]);
        goto clean_up;
    }

    if (!ueum_is_file_exists(argv[2])) {
        ei_stacktrace_push_msg("Specified plugin target file '%s' doesn't exist", argv[2]);
        goto clean_up;
    }

    srand((unsigned int)time(0));

    entry = mp_entry_create();
    mp_entry_add_file(entry, argv[1]);
#if defined(WITH_CRYPTO)
    if ((crypto_metadata = (uecm_crypto_metadata *)uecm_crypto_metadata_create_default()) == NULL) {
        ei_stacktrace_push_msg("Failed to create random crypto metadata");
        goto clean_up;
    }
    if (!uecm_crypto_metadata_write((uecm_crypto_metadata *)crypto_metadata, "metadata", "uid", "password")) {
        ei_stacktrace_push_msg("Failed to write crypto metadata");
        goto clean_up;
    }
#endif

    /* Create new plugin from buffer */
    ei_logger_info("Creating new plugin from file...");
    if ((plugin = mp_memory_plugin_create_new(entry)) == NULL) {
        ei_stacktrace_push_msg("Failed to create new plugin");
        goto clean_up;
    }
    ei_logger_info("New plugin created");

    /**
    * Save the plugin into the target path (exec or shared object). It returned the
    * corresponding id of the created plugin.
    */
    ei_logger_info("Saving plugin to target file...");
    if ((plugin_id = mp_memory_plugin_save(plugin, argv[2], crypto_metadata)) == -1) {
        ei_stacktrace_push_msg("Failed to save new plugin to %s", argv[2]);
        goto clean_up;
    }

    ei_logger_info("Succeed to save new plugin with id %d", plugin_id);

clean_up:
    mp_entry_destroy(entry);
    /* Destroy only the object content and not the saved plugin */
    mp_memory_plugin_destroy(plugin);
#if defined(WITH_CRYPTO)
    uecm_crypto_metadata_destroy((uecm_crypto_metadata *)crypto_metadata);
    uecm_uninit();
#endif
    if (ei_stacktrace_is_filled()) {
        ei_logger_stacktrace("Error occurs with the following stacktrace:");
    }
    ei_uninit();
    return EXIT_SUCCESS;
}
