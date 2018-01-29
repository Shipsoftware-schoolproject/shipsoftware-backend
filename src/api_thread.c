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

#include <unistd.h>
#include <string.h>
#include "api_thread.h"
#ifdef WITH_GUI
#include "main_window.h"
#else
#include "config.h"
#endif
#include "api.h"
#include "database.h"
#include "json.h"

int RUNNING = 0;

static gchar *_substring(const gchar *text, gint64 offset, gsize len)
{
	gchar *ret = NULL;
	gint64 text_len;
	gchar *tmp;

	text_len = strlen(text);

	if ((offset + len) > text_len) {
		return NULL;
	} else {
		tmp = g_strndup(text + offset, (len));
		ret = g_strconcat(tmp, "%", NULL);
		g_free(tmp);
	}

	return ret;
}

static gboolean parse_route(struct Route *route, const gchar *text)
{
	gboolean ret;
	gchar *departure;
	gchar *destination;
	gint64 start_offset;

	ret = FALSE;
	departure = NULL;
	destination = NULL;
	start_offset = 0;

	for (size_t i = 0; i < strlen(text); ++i) {
		if (text[i] == '-') {
			if (!departure) {
				departure = _substring(text, 0, i);
				start_offset = (i + 1);
				ret = TRUE;
			} else {
				destination = _substring(text, start_offset, (i - start_offset));
				ret = TRUE;
				break;
			}
		}
	}

	route->departure = departure;
	route->destination = destination;

	return ret;
}

#ifdef WITH_GUI
static gboolean _update_label(gpointer widget, gboolean color, const char *text)
{
	struct label_data *data = g_slice_alloc(sizeof(*data));
	data->widget = widget;
	data->color = color;
	data->text = text;

	g_idle_add(update_label, data);

	return FALSE;
}
#endif

gchar *get_datetime()
{
	gchar *dt;
	GDateTime *gdt;

	gdt = g_date_time_new_now_local();
	dt = g_date_time_format(gdt, "%F %H:%M:%S");
	g_date_time_unref(gdt);

	return dt;
}

void log_message(gpointer message)
{
#ifdef WITH_GUI
	g_idle_add(add_log_row, message);
#else
	gchar *date_time;
	date_time = get_datetime();
	g_print("[%s]: %s\n", date_time, (char *)message);
	g_free(message);
	g_free(date_time);
#endif
}

void log_error(gpointer message)
{
#ifdef WITH_GUI
	g_idle_add(add_log_row, message);
#else
	g_printerr("[%s]: %s\n", get_datetime(), (char *)message);
	g_free(message);
#endif
}

static gboolean api_check_result(const gchar *json)
{
	gboolean ret = FALSE;
	gchar *result;

	if (json_read_string("result", json, &result)) {
		if (g_ascii_strcasecmp(result, "ok") != 0) {
			gchar *description;
			if (json_read_string("description", json, &description)) {
				log_error(g_strconcat("API failed: ", description, NULL));
			}
		} else {
			ret = TRUE;
		}
	} else {
		log_error("API returned invalid json!");
	}
	g_free(result);

	return ret;
}

gpointer api_thread()
{
	int sleep_time;
	int slept;
	int _running;
	struct Config *_config;
	gboolean terminate;

	sleep_time = 7200;
	slept = sleep_time;
	g_mutex_lock(&MUTEX);
	_running = RUNNING;
	_config = g_slice_alloc(sizeof(*config));
	g_memmove(_config, config, sizeof(*config));
	g_mutex_unlock(&MUTEX);
	terminate = FALSE;

#ifdef WITH_GUI
	g_idle_add(set_button_text, "Stop");
	_update_label(LABEL_RUNNING, TRUE, "Running");
#endif

	while (_running) {
		gchar *error;

		if (slept >= sleep_time) {
			#ifdef WITH_GUI
				_update_label(LABEL_RUNNING, TRUE, "Running (updating..)");
			#endif
			slept = 0;
			struct Database *db;
			gchar *ships;
			gchar *json;
			gboolean api_result;
			gint64 entries;

			error = NULL;
			db = g_slice_alloc(sizeof(*db));
			ships = NULL;
			json = NULL;
			api_result = FALSE;
			entries = -1;

			// Open connection
			if (!db_init(db, _config, &error)) {
				terminate = TRUE;
				log_error(error);
			} else {
				// Get ships MMSI's
				if (!db_get_ships(db, &ships, &error)) {
					log_error(error);
				}
			}

			// Get data from API
			if (ships) {
				if (!api_get_loc(ships, _config->api_key, &json, &error)) {
				   log_error(error);
				}
			}

			if (json) {
				api_result = api_check_result(json);
			}

			if (api_result) {
				if (!json_read_int("found", json, &entries)) {
					log_error(g_strdup("API did not return entries field!"));
				}
			}

			if (entries > 0) {
				for (gint64 i = 0; i < entries; ++i) {
					const gchar *name = json_read_entry_string("name", json, i); name = !name ? "" : name;
					const gint64 mmsi = json_read_entry_int("mmsi", json, i);
					const gfloat latitude = (gfloat)json_read_entry_double("lat", json, i);
					const gfloat longitude = (gfloat)json_read_entry_double("lng", json, i);
					const gdouble course = json_read_entry_double("course", json, i);
					const gdouble speed = json_read_entry_double("speed", json, i);
					const gchar *comment = json_read_entry_string("comment", json, i);
					struct Route *route;

					route = g_slice_alloc(sizeof(*route));
					error = NULL;

					if (!db_update_ship_course_speed(db, course, speed, mmsi, &error)) {
						log_error(g_strconcat(name, ": UPDATE Ships failed, ", error, NULL));
					}

					if (!db_update_gps_location(db, latitude, longitude, mmsi, &error)) {
						log_error(g_strconcat(name, ": UPDATE GPS failed, ", error, NULL));
					}

					if (!parse_route(route, comment)) {
						log_error(g_strconcat(name, ": couldn't parse route", NULL));
					} else {
						if (!db_get_route_id(db, route, &error)) {
							log_error(g_strconcat(name, ": failed to get Route ID, ", error, NULL));
						} else {
							if (!db_update_route(db, route->id, mmsi, &error)) {
								log_error(g_strconcat(name, ": UPDATE Course failed, ", error, NULL));
							}
						}
					}

					g_slice_free1(sizeof(*route), route);
				}
			}

#ifdef WITH_GUI
			if (!terminate)
				_update_label(LABEL_LAST_UPDATED, TRUE, g_date_time_format(g_date_time_new_now_local(), "%F %H:%M:%S"));
#endif

			if (ships)
				g_free(ships);

			if (json) {
				g_free(json);
			}

			db_close_con(db);
			g_slice_free1(sizeof(*db), db);
		}

		if (terminate) {
			_running = 0;
			g_mutex_lock(&MUTEX);
			RUNNING = 0;
			g_mutex_unlock(&MUTEX);
		} else {
			g_mutex_lock(&MUTEX);
			_running = RUNNING;
			g_mutex_unlock(&MUTEX);
		}

#ifdef WITH_GUI
		gchar *sec_left;
		sec_left = g_strdup_printf("%i", (sleep_time - slept));
		_update_label(LABEL_RUNNING, TRUE, g_strconcat("Running (next update in ", sec_left, " seconds)", NULL));
		g_free(sec_left);
#endif

		if (_running) {
			sleep(1);
			++slept;
		}
	}

#ifdef WITH_GUI
	g_idle_add(set_button_text, "Start");
	_update_label(LABEL_RUNNING, FALSE, "Not running");
	_update_label(LABEL_LAST_UPDATED, FALSE, NULL);
#endif

	g_slice_free1(sizeof(*_config), _config);

	g_thread_exit(GINT_TO_POINTER(terminate ? 1 : 0));

	return NULL;
}
