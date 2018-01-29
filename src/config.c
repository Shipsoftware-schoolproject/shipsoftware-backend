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

#include <string.h>
#include <glib-object.h>
#include "config.h"
#include "json.h"

gboolean load_config(struct Config *config, gchar **contents, gchar **error)
{
	GError *_error;
	_error = NULL;

	config->db_name = NULL;
	config->db_username = NULL;
	config->db_password = NULL;
	config->db_hostname = NULL;
	config->api_key = NULL;
	config->log_size = 20;

	if (!g_file_get_contents("configuration.json", contents, NULL, &_error)) {
		*(error) = g_strdup(_error->message);
		return FALSE;
	}

	return TRUE;
}

gboolean parse_config(struct Config *config, const gchar *contents,
		      gchar **error)
{
	gchar *ret;
	gchar *database;
	gchar *username;
	gchar *password;
	gchar *hostname;
	gchar *api_key;
	gint64 log_size;

	ret = "";

	if (!json_read_string("database", contents, &database)) {
		ret = g_strconcat(ret, "Configuration is missing `database` entry!\n", NULL);
	} else {
		config->db_name = g_strdup(database);
	}

	if (!json_read_string("username", contents, &username)) {
		ret = g_strconcat(ret, "Configuration is missing `username` entry!\n", NULL);
	} else {
		config->db_username = g_strdup(username);
	}

	if (!json_read_string("password", contents, &password)) {
		ret = g_strconcat(ret, "Configuration is missing `password` entry!\n", NULL);
	} else {
		config->db_password = g_strdup(password);
	}

	if (!json_read_string("hostname", contents, &hostname)) {
		ret = g_strconcat(ret, "Configuration is missing `hostname` entry!\n", NULL);
	} else {
		config->db_hostname = g_strdup(hostname);
	}

	if (!json_read_string("api_key", contents, &api_key)) {
		ret = g_strconcat(ret, "Configuration is missing `api_key` entry!\n", NULL);
	} else {
		config->api_key = g_strdup(api_key);
	}

	if (json_read_int("log_size", contents, &log_size)) {
		if (log_size < 0) {
			log_size = 20;
		} else if (log_size > G_MAXINT64) {
			log_size = 20;
		}
		config->log_size = log_size;
	}

	if (ret[0] != '\0') {
		*(error) = g_strdup(ret);
		return FALSE;
	}

	return TRUE;
}

gboolean validate_config(const struct Config *config, gchar **error)
{
	char *ret;

	ret = "";

	if (strlen(config->db_name) < 1) {
		ret = g_strconcat(ret, "Database name too short\n", NULL);
	}

	if (strlen(config->db_username) < 1) {
		ret = g_strconcat(ret, "Database username is empty\n", NULL);
	}

	if (strlen(config->db_hostname) < 2) {
		ret = g_strconcat(ret, "Hostname too short\n", NULL);
	}

	if (strlen(config->api_key) < 21) {
		ret = g_strconcat(ret, "API key is too short, should be 21 characters\n", NULL);
	} else if (strlen(config->api_key) > 21) {
		ret = g_strconcat(ret, "API key is too long, should be 21 characters\n", NULL);
	}

	if (ret[0] != '\0') {
		*(error) = g_strdup(ret);
		return FALSE;
	}

	return TRUE;
}

gboolean save_config(const struct Config *config, gchar **error)
{
	gboolean ret;

	ret = FALSE;

	if (save_json_file(config, error)) {
		ret = TRUE;
	}

	return ret;
}
