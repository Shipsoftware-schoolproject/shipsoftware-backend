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

#include <unistd.h>
#include <string.h>
#include "api_thread.h"
#ifdef WITH_GUI
#include "main_window.h"
#endif
#include "api.h"
#include "json.h"

int RUNNING = 0;

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
			if (json_read_string("description", json, &description))
			{
				log_error(g_strconcat("API failed: ",
						      description, NULL));
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

gpointer api_thread(gpointer config)
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
	_config = g_slice_alloc(sizeof((struct Config *)config));
	if (!_config) {
		log_error(g_strdup("Failed to allocate memory for config!"));
		g_thread_exit(GINT_TO_POINTER(1));
		return NULL;
	}
	_config = g_slice_dup(struct Config, config);
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
				if (!api_get_loc(ships, _config->api_key, &json,
						 &error))
				{
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

			for (gint64 i = 0; i < entries; ++i) {
				struct Ship ship;
				ship.imo = json_read_entry_int("imo", json, i);
				ship.name = json_read_entry_string("name", json, i); ship.name = !ship.name ? "" : ship.name;
				ship.mmsi = json_read_entry_int("mmsi", json, i);
				ship.course = json_read_entry_float("course", json, i);
				ship.speed = json_read_entry_float("speed", json, i);
				ship.comment = json_read_entry_string("comment", json, i);
				ship.heading = (gint16)json_read_entry_int("heading", json, i);
				ship.length = json_read_entry_float("length", json, i);
				ship.width = json_read_entry_float("width", json, i);
				ship.draught = json_read_entry_float("draught", json, i);
				ship.ref_front = (gint16)json_read_entry_int("ref_front", json, i);
				ship.ref_left = (gint16)json_read_entry_int("ref_left", json, i);
				ship.path = json_read_entry_string("path", json, i);
				ship.class = json_read_entry_char("class", json, i);
				ship.type = json_read_entry_char("type", json, i);
				ship.srccall = json_read_entry_string("srccall", json, i);
				ship.dstcall = json_read_entry_string("dstcall", json, i);
				ship.vessel_class = (gint16)json_read_entry_int("vesselclass", json, i);
				ship.navstat = (gint8)json_read_entry_int("navstat", json, i);

				ship.time = json_read_entry_int("time", json, i);
				ship.lasttime = json_read_entry_int("lasttime", json, i);
				ship.latitude = json_read_entry_double("lat", json, i);
				ship.longitude = json_read_entry_double("lng", json, i);

				error = NULL;

				if (!db_update_ship_info(db, &ship, &error)) {
					log_error(g_strconcat(ship.name, ": UPDATE Ships failed, ", error, NULL));
				}

				if (!db_update_ship_gps(db, &ship, &error)) {
					log_error(g_strconcat(ship.name, ": UPDATE GPS failed, ", error, NULL));
				}

				if (!db_clean_ship_gps(db, &ship.imo, &error)) {
					log_error(g_strconcat(ship.name,
							      ": DELETE of old GPS records failed, ",
							      error,
							      NULL));
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
		_update_label(LABEL_RUNNING, TRUE,
			      g_strconcat("Running (next update in ", sec_left,
					  " seconds)", NULL));
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
