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

#include "config_window.h"
#include "config.h"
#include "dialogs.h"
#include <string.h>

static gboolean trigger_save(struct new_config *new_config_data, gchar **error)
{
	gboolean ret;

	ret = FALSE;

	struct Config *new_config = g_slice_alloc(sizeof(*new_config_data));
	new_config->db_name = gtk_entry_get_text(GTK_ENTRY(new_config_data->db_name));
	new_config->db_username = gtk_entry_get_text(GTK_ENTRY(new_config_data->db_username));
	new_config->db_password = gtk_entry_get_text(GTK_ENTRY(new_config_data->db_password));
	new_config->db_hostname = gtk_entry_get_text(GTK_ENTRY(new_config_data->db_hostname));
	new_config->api_key = gtk_entry_get_text(GTK_ENTRY(new_config_data->api_key));
	new_config->log_size = g_ascii_strtoll(gtk_entry_get_text(GTK_ENTRY(new_config_data->log_size)), NULL, 10);

	if (validate_config(new_config, error)) {
		if (save_config(new_config, error)) {
			ret = TRUE;
			g_slice_free1(sizeof(*config), config);
			config = g_slice_copy(sizeof(*config), new_config);
			config->db_name = g_memdup(new_config->db_name, strlen(new_config->db_name) + 1);
			config->db_username = g_memdup(new_config->db_username, strlen(new_config->db_username) + 1);
			config->db_password = g_memdup(new_config->db_password, strlen(new_config->db_password) + 1);
			config->db_hostname = g_memdup(new_config->db_hostname, strlen(new_config->db_hostname) + 1);
			config->api_key = g_memdup(new_config->api_key, strlen(new_config->api_key) + 1);
		}
	}

	g_slice_free1(sizeof(*new_config), new_config);

	return ret;
}

static void apply_config(struct new_config *config)
{
	gchar *error;

	if (!trigger_save(config, &error)) {
		show_ok_dialog(config->window, "Error", error);
	} else {
		show_ok_dialog(config->window, "Success", "Configuration saved");
		if (RUNNING == 1) {
			show_ok_dialog(config->window, "Warning", "Configuration will be applied after the backend is restarted.");
		}
	}
}

static void ok_clicked(struct new_config *config)
{
	gchar *error;

	if (!trigger_save(config, &error)) {
		show_ok_dialog(config->window, "Error", error);
	} else {
		if (RUNNING == 1) {
			show_ok_dialog(config->window, "Warning", "Configuration will be applied after the backend is restarted.");
		}
		gtk_dialog_response(GTK_DIALOG(config->window), GTK_RESPONSE_OK);
	}
}

static void close_dialog(GtkWidget *window)
{
	gtk_dialog_response(GTK_DIALOG(window), GTK_RESPONSE_CANCEL);
}

gint show_config_window(gpointer parent)
{
	gint ret;
	GtkWidget *window;
	GtkWidget *content_area;
	GtkWidget *main_box;
	GtkWidget *container_box;
	GtkWidget *left_box;
	GtkWidget *right_box;
	GtkWidget *database_frame;
	GtkWidget *api_frame;
	GtkWidget *misc_frame;
	GtkWidget *database_box;
	GtkWidget *api_box;
	GtkWidget *misc_box;
	GtkWidget *button_box;
	GtkWidget *btn_ok;
	GtkWidget *btn_apply;
	GtkWidget *btn_cancel;

	// Database
	GtkWidget *lbl_db_name;
	GtkWidget *txt_db_name;
	GtkWidget *lbl_username;
	GtkWidget *txt_username;
	GtkWidget *lbl_password;
	GtkWidget *txt_password;
	GtkWidget *lbl_hostname;
	GtkWidget *txt_hostname;

	// API
	GtkWidget *lbl_api_key;
	GtkWidget *txt_api_key;

	// Misc
	GtkWidget *lbl_log_size;
	GtkWidget *txt_log_size;

	window = gtk_dialog_new();
	content_area = gtk_dialog_get_content_area(GTK_DIALOG(window));

	main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	container_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	left_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	right_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	database_frame = gtk_frame_new(" Database");
	api_frame = gtk_frame_new(" API");
	misc_frame = gtk_frame_new(" Miscalleneous");

	database_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	api_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	misc_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

	gtk_container_set_border_width(GTK_CONTAINER(database_box), 5);
	gtk_container_set_border_width(GTK_CONTAINER(api_box), 5);
	gtk_container_set_border_width(GTK_CONTAINER(misc_box), 5);

	btn_ok = gtk_button_new_with_label("Ok");
	btn_apply = gtk_button_new_with_label("Apply");
	btn_cancel = gtk_button_new_with_label("Cancel");

	gtk_widget_set_vexpand(GTK_WIDGET(main_box), TRUE);
	gtk_container_add(GTK_CONTAINER(content_area), main_box);
	gtk_container_add(GTK_CONTAINER(main_box), container_box);
	gtk_box_pack_start(GTK_BOX(container_box), left_box, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(container_box), gtk_box_new(GTK_ORIENTATION_VERTICAL, 0), TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(container_box), right_box, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(left_box), database_frame, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(right_box), api_frame, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(right_box), misc_frame, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(main_box), gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(main_box), button_box, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(button_box), gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0), TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(button_box), btn_ok, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(button_box), btn_cancel, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(button_box), btn_apply, FALSE, FALSE, 2);
	gtk_container_add(GTK_CONTAINER(database_frame), database_box);
	gtk_container_add(GTK_CONTAINER(api_frame), api_box);
	gtk_container_add(GTK_CONTAINER(misc_frame), misc_box);

	// Database
	lbl_db_name = gtk_label_new("Database name:");
	gtk_label_set_xalign(GTK_LABEL(lbl_db_name), 0);
	txt_db_name = gtk_entry_new();
	if (config->db_name != NULL) {
		gtk_entry_set_text(GTK_ENTRY(txt_db_name), config->db_name);
	}

	lbl_username = gtk_label_new("Username:");
	gtk_label_set_xalign(GTK_LABEL(lbl_username), 0);
	txt_username = gtk_entry_new();
	if (config->db_username != NULL) {
		gtk_entry_set_text(GTK_ENTRY(txt_username), config->db_username);
	}

	lbl_password = gtk_label_new("Password:");
	gtk_label_set_xalign(GTK_LABEL(lbl_password), 0);
	txt_password = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(txt_password), FALSE);
	if (config->db_password != NULL) {
		gtk_entry_set_text(GTK_ENTRY(txt_password), config->db_password);
	}

	lbl_hostname = gtk_label_new("Hostname:");
	gtk_label_set_xalign(GTK_LABEL(lbl_hostname), 0);
	txt_hostname = gtk_entry_new();
	if (config->db_hostname != NULL) {
		gtk_entry_set_text(GTK_ENTRY(txt_hostname), config->db_hostname);
	}

	gtk_box_pack_start(GTK_BOX(database_box), lbl_db_name, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(database_box), txt_db_name, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(database_box), lbl_username, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(database_box), txt_username, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(database_box), lbl_password, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(database_box), txt_password, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(database_box), lbl_hostname, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(database_box), txt_hostname, FALSE, FALSE, 5);

	// API
	lbl_api_key = gtk_label_new("API key:");
	gtk_label_set_xalign(GTK_LABEL(lbl_api_key), 0);
	txt_api_key = gtk_entry_new();
	if (config->api_key != NULL) {
		gtk_entry_set_text(GTK_ENTRY(txt_api_key), config->api_key);
	}

	gtk_box_pack_start(GTK_BOX(api_box), lbl_api_key, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(api_box), txt_api_key, FALSE, FALSE, 5);

	// Misc
	lbl_log_size = gtk_label_new("Log size:");
	gtk_label_set_xalign(GTK_LABEL(lbl_log_size), 0);
	txt_log_size = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(txt_log_size), g_strdup_printf("%" G_GINT64_FORMAT, config->log_size));

	gtk_box_pack_start(GTK_BOX(misc_box), lbl_log_size, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(misc_box), txt_log_size, FALSE, FALSE, 5);

	// Config struct
	struct new_config *_config = g_slice_alloc(sizeof(*_config));
	_config->window = window;
	_config->db_name = txt_db_name;
	_config->db_username = txt_username;
	_config->db_password = txt_password;
	_config->db_hostname = txt_hostname;
	_config->api_key = txt_api_key;
	_config->log_size = txt_log_size;

	// Window
	gtk_window_set_modal(GTK_WINDOW(window), TRUE);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	gtk_window_set_default_size(GTK_WINDOW(window), 400, 410);
	gtk_container_set_border_width(GTK_CONTAINER(window), 10);
	gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(parent));
	gtk_window_set_destroy_with_parent(GTK_WINDOW(parent), TRUE);
	gtk_window_set_title(GTK_WINDOW(window), "Configuration");

	g_signal_connect_swapped(btn_ok, "clicked", G_CALLBACK(ok_clicked), _config);
	g_signal_connect_swapped(btn_apply, "clicked", G_CALLBACK(apply_config), _config);
	g_signal_connect_swapped(btn_cancel, "clicked", G_CALLBACK(close_dialog), window);

	gtk_widget_show_all(main_box);

	ret = gtk_dialog_run(GTK_DIALOG(window));

	g_slice_free1(sizeof(*_config), _config);
	gtk_widget_destroy(window);

	return ret;
}
