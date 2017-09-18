/****************************************************************************
 * Copyright (c) 2017 Tomi Lähteenmäki <lihis@lihis.net>                	*
 *                                                                          *
 * This program is free software; you can redistribute it and/or modify 	*
 * it under the terms of the GNU General Public License as published by 	*
 * the Free Software Foundation; either version 2 of the License, or    	*
 * (at your option) any later version.                                  	*
 *                                                                          *
 * This program is distributed in the hope that it will be useful,      	*
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       	*
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        	*
 * GNU General Public License for more details.                         	*
 *                                                                          *
 * You should have received a copy of the GNU General Public License    	*
 * along with this program; if not, write to the Free Software          	*
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,           	*
 * MA 02110-1301, USA.                                                  	*
 ****************************************************************************/

#ifdef WITH_GUI
	#include <gtk/gtk.h>
	#include "main_window.h"
	#include "dialogs.h"
#else
	#include <stdio.h>
	#include <glib.h>
	#include <string.h>
	#include "config.h"
	#include "api_thread.h"
	#include "version.h"
#endif

#ifdef __WIN32__
	#include <windows.h>
#endif

#ifndef WITH_GUI
struct Config *config;

static void print_version()
{
	g_print("shipsoftware_backend version %s\n", ShipSoftwareBackend_VERSION);
}

static void print_help()
{
	g_print("Usage: shipsoftware_backend [OPTION]\n");
	g_print("\n");
	g_print("Without options the program will start and run until stopped.\n");
	g_print("\n");
	g_print(" -H --help\t\t\tPrint this help and exit\n");
	g_print("    --version\t\t\tPrint program version and exit\n");
	g_print("\nProgram was compiled without GUI support\n");
}
#endif

int main(int argc, char *argv[])
{
	gint status;

#ifdef WITH_GUI
	GtkApplication *app;

#ifdef __WIN32__
	SetEnvironmentVariable("GTK_CSD", "0");
#endif

	app = gtk_application_new("org.shipsoftwarebackend", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate_mainwindow), NULL);
	status = g_application_run(G_APPLICATION(app), argc, argv);

	g_object_unref(app);
#else
	gchar *config_contents;
	gchar *error;
	gint config_valid;
	GThread *thread;

	if (argc > 1) {
		if (strcmp(argv[1], "-H") == 0 || strcmp(argv[1], "--help") == 0) {
			print_help();
			return 0;
		} else if (strcmp(argv[1], "--version") == 0) {
			print_version();
			return 0;
		} else {
			g_printerr("Invalid option `%s`!\n", argv[1]);
			return 1;
		}
	}

	config_valid = 0;
	config = g_slice_alloc(sizeof(*config));

	if (!load_config(config, &config_contents, &error)) {
		log_error(g_strconcat("Could not load configuration! ", error, NULL));
		g_free(error);
	} else {
		if (!parse_config(config, config_contents, &error)) {
			log_error(g_strconcat("Invalid configuration! \n", error, NULL));
			g_free(error);
		} else {
			if (!validate_config(config, &error)) {
				log_error(g_strconcat("Invalid configuration!\n", error, NULL));
				g_free(error);
			} else {
				config_valid = 1;
			}
		}
	}

	if (config_valid) {
		RUNNING = 1;
		log_message(g_strdup("Started"));
		thread = g_thread_new("api_thread", api_thread, &status);
		status = GPOINTER_TO_INT(g_thread_join(thread));
		g_thread_unref(thread);
	}

	g_slice_free1(sizeof(*config), config);
#endif

	return status;
}
