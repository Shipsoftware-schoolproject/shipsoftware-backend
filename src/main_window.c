/****************************************************************************
 * Copyright (c) 2018 Tomi Lähteenmäki <lihis@lihis.net>                    *
 *                                                                          *
 * This program is free software; you can redistribute it and/or modify     *
 * it under the terms of the GNU General Public License as published by     *
 * the Free Software Foundation; either version 2 of the License, or        *
 * (at your option) any later version.                                      *
 *                                                                          *
 * This program is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 * GNU General Public License for more details.                             *
 *                                                                          *
 * You should have received a copy of the GNU General Public License        *
 * along with this program; if not, write to the Free Software              *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,               *
 * MA 02110-1301, USA.                                                      *
 ****************************************************************************/

#include <glib-object.h>
#include "config.h"
#include "main_window.h"
#include "api_thread.h"
#include "dialogs.h"
#include "config_window.h"
#include "logo.xpm"

#define COLOR_GREEN "#008000"
#define COLOR_RED "#FF0000"

struct Config *config;
GMutex MUTEX;
GtkWidget *LABEL_RUNNING;
GtkWidget *LABEL_LAST_UPDATED;
GtkWidget *BUTTON_START;
GtkWidget *LISTBOX_LOGS;

gboolean add_log_row(void *message)
{
	GtkWidget *row;
	GtkWidget *item;
	GList *rows;

	row = gtk_list_box_row_new();
	gtk_widget_set_tooltip_text(GTK_WIDGET(row), get_datetime());
	item = gtk_label_new(message);

	gtk_label_set_xalign(GTK_LABEL(item), 0);
	gtk_container_add(GTK_CONTAINER(row), item);

	g_mutex_lock(&MUTEX);
	gtk_list_box_insert(GTK_LIST_BOX(LISTBOX_LOGS), GTK_WIDGET(row), 0);

	rows = gtk_container_get_children(GTK_CONTAINER(LISTBOX_LOGS));
	if (g_list_length(rows) > config->log_size) {
		while (g_list_length(rows) > config->log_size) {
			GList *last_row = g_list_last(rows);
			gtk_container_remove(GTK_CONTAINER(LISTBOX_LOGS),
					     last_row->data);
			rows = g_list_remove(rows, last_row->data);
		}
	}
	g_free(message);
	g_mutex_unlock(&MUTEX);

	gtk_widget_show_all(row);

	return FALSE;
}

void clear_logs()
{
	GList *rows;

	g_mutex_lock(&MUTEX);
	rows = gtk_container_get_children(GTK_CONTAINER(LISTBOX_LOGS));

	while (rows) {
		gtk_container_remove(GTK_CONTAINER(LISTBOX_LOGS),
				     GTK_WIDGET(rows->data));
		rows = rows->next;
	}
	g_mutex_unlock(&MUTEX);

	g_list_free(rows);
}

gboolean set_button_text(void *text)
{
	gtk_widget_set_sensitive(BUTTON_START, TRUE);
	gtk_button_set_label(GTK_BUTTON(BUTTON_START), text);

	return FALSE;
}

gboolean update_label(gpointer data)
{
	char *color;
	const char *format = "<span foreground=\"%s\">%s</span>";
	struct label_data *args = data;

	if (args->color) {
		color = COLOR_GREEN;
	} else {
		color = COLOR_RED;
	}

	if (args->text == NULL) {
		args->text = gtk_label_get_text(GTK_LABEL(args->widget));
	}

	gtk_label_set_markup(GTK_LABEL(args->widget),
			     g_markup_printf_escaped(format, color, args->text));

	g_slice_free1(sizeof(*args), args);

	return FALSE;
}

void start_clicked()
{
	GThread *thread;
	int _running;

	g_mutex_lock(&MUTEX);
	gtk_widget_set_sensitive(BUTTON_START, FALSE);
	_running = RUNNING;
	g_mutex_unlock(&MUTEX);

	if (_running) {
		g_mutex_lock(&MUTEX);
		gtk_widget_set_sensitive(BUTTON_START, FALSE);
		RUNNING = 0;
		g_mutex_unlock(&MUTEX);
	} else {
		g_mutex_lock(&MUTEX);
		RUNNING = 1;
		g_mutex_unlock(&MUTEX);
		thread = g_thread_new("api_thread", api_thread, config);
		g_thread_unref(thread);
	}
}

static gboolean keypress_event(GtkWidget *widget, GdkEventKey *event)
{
	switch(event->keyval) {
		case GDK_KEY_q:
			if (event->state & GDK_CONTROL_MASK) {
				gtk_widget_destroy(widget);
			}
			break;
		default:
			break;
	}

	return FALSE;
}

void activate_mainwindow(GtkApplication *app)
{
	GtkWidget *window;
	GtkWidget *menubar;
	GtkWidget *menubar_filemenu;
	GtkWidget *menubar_helpmenu;
	GtkWidget *filemenu;
	GtkWidget *filemenu_conf;
	GtkWidget *filemenu_quit;
	GtkWidget *helpmenu;
	GtkWidget *helpmenu_about;
	GtkWidget *running;
	GtkWidget *last_updated;
	const char *format;
	char *text;
	GtkWidget *status_area;
	GtkWidget *button_area;
	GtkWidget *log_area;
	GtkWidget *scroll_container;
	GtkWidget *bottom_area;
	GtkWidget *button_clear_log;
	GtkWidget *main_box;
	gchar *config_contents;
	gchar *error;
	gchar *error_message;
	int config_valid;

	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "ShipSoftware-backend");
	gtk_window_set_default_size(GTK_WINDOW(window), 600, 380);
	gtk_window_set_icon(GTK_WINDOW(window),
			    gdk_pixbuf_new_from_xpm_data(logo_xpm));

	menubar = gtk_menu_bar_new();
	menubar_filemenu = gtk_menu_item_new_with_mnemonic("_File");
	menubar_helpmenu = gtk_menu_item_new_with_mnemonic("_Help");

	filemenu = gtk_menu_new();
	filemenu_conf = gtk_menu_item_new_with_mnemonic("_Configuration");
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), filemenu_conf);
	filemenu_quit = gtk_menu_item_new_with_mnemonic("_Quit");
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), filemenu_quit);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menubar_filemenu), filemenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menubar_filemenu);

	helpmenu = gtk_menu_new();
	helpmenu_about = gtk_menu_item_new_with_mnemonic("_About");
	gtk_menu_shell_append(GTK_MENU_SHELL(helpmenu), helpmenu_about);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menubar_helpmenu), helpmenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menubar_helpmenu);

	running = gtk_label_new("Running:");
	last_updated = gtk_label_new("Last Updated:");

	format = "<span foreground=\"%s\">%s</span>";
	LABEL_RUNNING = gtk_label_new(NULL);
	text = g_markup_printf_escaped(format, COLOR_RED, "Not running");
	gtk_label_set_markup(GTK_LABEL(LABEL_RUNNING), text);
	LABEL_LAST_UPDATED = gtk_label_new(NULL);
	text = g_markup_printf_escaped(format, COLOR_RED, "Never");
	gtk_label_set_markup(GTK_LABEL(LABEL_LAST_UPDATED), text);
	g_free(text);

	status_area = gtk_fixed_new();
	gtk_fixed_put(GTK_FIXED(status_area), running, 10, 5);
	gtk_fixed_put(GTK_FIXED(status_area), LABEL_RUNNING, 110, 5);
	gtk_fixed_put(GTK_FIXED(status_area), last_updated, 10, 40);
	gtk_fixed_put(GTK_FIXED(status_area), LABEL_LAST_UPDATED, 110, 40);

	button_area = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	BUTTON_START = gtk_button_new_with_label("Start");
	gtk_widget_set_sensitive(BUTTON_START, FALSE);
	button_clear_log = gtk_button_new_with_label("Clear log");
	gtk_box_pack_start(GTK_BOX(button_area), BUTTON_START, FALSE, FALSE, 10);
	gtk_box_pack_start(GTK_BOX(button_area),
			   gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0), TRUE,
			   FALSE, 0);
	gtk_box_pack_start(GTK_BOX(button_area), button_clear_log, FALSE, FALSE, 10);

	log_area = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	LISTBOX_LOGS = gtk_list_box_new();
	bottom_area = gtk_fixed_new();
	scroll_container = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scroll_container), LISTBOX_LOGS);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_container),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_fixed_put(GTK_FIXED(bottom_area),
		      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5), 0, 0);

	main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	gtk_box_pack_start(GTK_BOX(main_box), menubar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(main_box), status_area, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(main_box), button_area, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(log_area), scroll_container, TRUE, TRUE, 10);
	gtk_box_pack_start(GTK_BOX(main_box), log_area, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(main_box), bottom_area, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), main_box);

	g_signal_connect_swapped(filemenu_conf, "activate", G_CALLBACK(show_config_window), window);
	g_signal_connect_swapped(filemenu_quit, "activate", G_CALLBACK(gtk_widget_destroy), window);
	g_signal_connect_swapped(helpmenu_about, "activate", G_CALLBACK(show_about_dialog), window);
	g_signal_connect(BUTTON_START, "clicked", G_CALLBACK(start_clicked), NULL);
	g_signal_connect(button_clear_log, "clicked", G_CALLBACK(clear_logs), NULL);
	g_signal_connect(G_OBJECT(window), "key_press_event", G_CALLBACK(keypress_event), NULL);

	gtk_widget_show_all(window);

	config_valid = 0;
	config = g_slice_alloc(sizeof(*config));
	if (!load_config(config, &config_contents, &error)) {
		error_message = g_strdup("Could not load configuration");
	} else {
		if (!parse_config(config, config_contents, &error)) {
			error_message = g_strdup("Invalid configuration");
		} else {
			if (!validate_config(config, &error)) {
				error_message = g_strdup("Invalid configuration");
			} else {
				config_valid = 1;
			}
		}
	}

	if (config_valid) {
		gtk_widget_set_sensitive(BUTTON_START, TRUE);
	} else {
		if (show_yesno_dialog(window, "Error", g_strconcat(error, "\n\nDo you want to open configuration editor?", NULL)) == GTK_RESPONSE_YES) {
			if (show_config_window(window) != GTK_RESPONSE_OK) {
				config_valid = 0;
				show_ok_dialog(window, "Error", "You must edit configuration before you can start the program!");
			} else {
				config_valid = 1;
				gtk_widget_set_sensitive(BUTTON_START, TRUE);
			}
		} else {
			config_valid = 0;
			show_ok_dialog(window, "Error", "You must edit configuration before you can start the program!");
		}
		g_free(error);

		if (!config_valid) {
			log_error(error_message);
		}
	}
}
