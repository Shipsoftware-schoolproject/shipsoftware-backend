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

#include <glib.h>
#include <mysql.h>
#include <string.h>
#include "database.h"

gboolean db_init(struct Database *db, const struct Config *config,
		 gchar **error)
{
	db->con = mysql_init(NULL);

	if (mysql_real_connect(db->con, config->db_hostname,
			       config->db_username, config->db_password,
			       NULL, 0, NULL, 0) == NULL)
	{
		*(error) = g_strconcat("Connection failed: ",
				       mysql_error(db->con), NULL);
		return FALSE;
	} else {
		if (mysql_query(db->con, g_strconcat("USE ", config->db_name,
						     NULL)))
		{
			*(error) = g_strconcat("Failed to select database: ",
					       mysql_error(db->con), NULL);
			return FALSE;
		}
	}

	return TRUE;
}

void db_close_con(struct Database *db)
{
	mysql_close(db->con);
}

gboolean db_get_ships(const struct Database *db, gchar **ships, gchar **error)
{
	gboolean ret;
	gchar *query;
	MYSQL_RES *result;
	gchar *_ships;

	ret = FALSE;
	query = "SELECT MMSI FROM Ships";
	_ships = NULL;

	if (mysql_query(db->con, query)) {
		*(error) = g_strconcat("query failed: ", mysql_error(db->con),
				       NULL);
	} else {
		result = mysql_store_result(db->con);

		if (!result) {
			*(error) = g_strconcat("couldn't get result set: ",
					       mysql_error(db->con), NULL);
		} else {
			guint64 num_rows = mysql_num_rows(result);

			if (num_rows == 0) {
				*(error) = g_strdup("there is no ships in the database");
			} else {
				for (unsigned int i = 0; i < num_rows; ++i) {
					MYSQL_ROW row = mysql_fetch_row(result);
					if (_ships == NULL) {
						_ships = row[0];
					} else {
						_ships = g_strconcat(_ships,
								     ",",
								     row[0],
								     NULL);
					}
				}
				*(ships) = g_strdup(_ships);
				g_free(_ships);
				ret = TRUE;
			}
		}
		mysql_free_result(result);
	}

	return ret;
}

gboolean db_update_ship_info(const struct Database *db, struct Ship *info,
			     gchar **error)
{
	gboolean ret;
	gchar *query;
	MYSQL_STMT *stmt;
	MYSQL_BIND bind[19];

	ret = FALSE;
	query = "UPDATE Ships SET IMO = ?, ShipName = ?, CommentText = ?, ShipLength = ?, Width = ?, Draught = ?, Course = ?, Heading = ?, ShipSpeed = ?, RefFront = ?, RefLeft = ?, PathText = ?, Iclass = ?, TargetType = ?, SrcCall = ?, DstCall = ?, VesselClass = ?, NavStat = ? WHERE MMSI = ?";

	stmt = mysql_stmt_init(db->con);
	if (!stmt) {
		*(error) = g_strdup("failed to prepare query, out of memory");
		return FALSE;
	}

	if (mysql_stmt_prepare(stmt, query, strlen(query))) {
		*(error) = g_strconcat("query prepare failed, " ,
				       mysql_stmt_error(stmt), NULL);
	} else {
		memset(bind, 0, sizeof(bind));

		bind[0].buffer_type = MYSQL_TYPE_LONG;
		bind[0].buffer = &info->imo;

		bind[1].buffer_type = MYSQL_TYPE_STRING;
		bind[1].buffer = info->name;
		bind[1].buffer_length = strlen(info->name);

		bind[2].buffer_type = MYSQL_TYPE_STRING;
		bind[2].buffer = info->comment;
		bind[2].buffer_length = strlen(info->comment);

		bind[3].buffer_type = MYSQL_TYPE_FLOAT;
		bind[3].buffer = &info->length;

		bind[4].buffer_type = MYSQL_TYPE_FLOAT;
		bind[4].buffer = &info->width;

		bind[5].buffer_type = MYSQL_TYPE_FLOAT;
		bind[5].buffer = &info->draught;

		bind[6].buffer_type = MYSQL_TYPE_FLOAT;
		bind[6].buffer = &info->course;

		bind[7].buffer_type = MYSQL_TYPE_LONG;
		bind[7].buffer = &info->heading;

		bind[8].buffer_type = MYSQL_TYPE_FLOAT;
		bind[8].buffer = &info->speed;

		bind[9].buffer_type = MYSQL_TYPE_LONG;
		bind[9].buffer = &info->ref_front;

		bind[10].buffer_type = MYSQL_TYPE_LONG;
		bind[10].buffer = &info->ref_left;

		bind[11].buffer_type = MYSQL_TYPE_STRING;
		bind[11].buffer = info->path;
		bind[11].buffer_length = strlen(info->path);

		bind[12].buffer_type = MYSQL_TYPE_STRING;
		bind[12].buffer = &info->class;
		bind[12].buffer_length = 1;

		bind[13].buffer_type = MYSQL_TYPE_STRING;
		bind[13].buffer = &info->type;
		bind[13].buffer_length = 1;

		bind[14].buffer_type = MYSQL_TYPE_STRING;
		bind[14].buffer = info->srccall;
		bind[14].buffer_length = strlen(info->srccall);

		bind[15].buffer_type = MYSQL_TYPE_STRING;
		bind[15].buffer = info->dstcall;
		bind[15].buffer_length = strlen(info->dstcall);

		bind[16].buffer_type = MYSQL_TYPE_LONG;
		bind[16].buffer = &info->vessel_class;

		bind[17].buffer_type = MYSQL_TYPE_LONG;
		bind[17].buffer = &info->navstat;

		bind[18].buffer_type = MYSQL_TYPE_LONG;
		bind[18].buffer = &info->mmsi;

		if (mysql_stmt_bind_param(stmt, bind)) {
			*(error) = g_strdup_printf(mysql_stmt_error(stmt), NULL);
		} else {
			if (mysql_stmt_execute(stmt)) {
				*(error) = g_strdup_printf(mysql_stmt_error(stmt));
			} else {
				ret = TRUE;
			}
		}
	}

	mysql_stmt_close(stmt);

	return ret;
}

gboolean db_update_ship_gps(const struct Database *db, struct Ship *info,
			    gchar **error)
{
	gboolean ret;
	const gchar *query;
	MYSQL_STMT *stmt;
	MYSQL_BIND bind[5];

	ret = FALSE;
	query = "INSERT INTO GPS (IMO, Lat, Lng, RealTime, LastTime) VALUES (?, ?, ?, ?, ?)";

	stmt = mysql_stmt_init(db->con);
	if (!stmt) {
		*(error) = g_strdup("failed to prepare query, out of memory");
		return FALSE;
	}

	if (mysql_stmt_prepare(stmt, query, strlen(query))) {
		*(error) = g_strconcat("query prepare failed: " ,
				       mysql_stmt_error(stmt), NULL);
	} else {
		struct tm unix_time;
		struct tm unix_lasttime;
		MYSQL_TIME sql_time;
		MYSQL_TIME sql_lasttime;

#ifdef __WIN32__
		gmtime_s(&unix_time, &info->time);
#else
		gmtime_r(&info->time, &unix_time);
#endif
		sql_time.year = 1900 + (guint)unix_time.tm_year;
		sql_time.month = 1 + (guint)unix_time.tm_mon;
		sql_time.day = (guint)unix_time.tm_mday;
		sql_time.hour = (guint)unix_time.tm_hour;
		sql_time.minute = (guint)unix_time.tm_min;
		sql_time.second = (guint)unix_time.tm_sec;
		sql_time.second_part = 0;
		sql_time.neg = 0;

#ifdef __WIN32__
		gmtime_s(&unix_lasttime, &info->lasttime);
#else
		gmtime_r(&info->lasttime, &unix_lasttime);
#endif
		sql_lasttime.year = 1900 + (guint)unix_lasttime.tm_year;
		sql_lasttime.month = 1 + (guint)unix_lasttime.tm_mon;
		sql_lasttime.day = (guint)unix_lasttime.tm_mday;
		sql_lasttime.hour = (guint)unix_lasttime.tm_hour;
		sql_lasttime.minute = (guint)unix_lasttime.tm_min;
		sql_lasttime.second = (guint)unix_lasttime.tm_sec;
		sql_lasttime.second_part = 0;
		sql_lasttime.neg = 0;

		memset(bind, 0, sizeof(bind));

		bind[0].buffer_type = MYSQL_TYPE_LONG;
		bind[0].buffer = &info->imo;

		bind[1].buffer_type = MYSQL_TYPE_DOUBLE;
		bind[1].buffer = &info->latitude;

		bind[2].buffer_type = MYSQL_TYPE_DOUBLE;
		bind[2].buffer = &info->longitude;

		bind[3].buffer_type = MYSQL_TYPE_DATETIME;
		bind[3].buffer = &sql_time;

		bind[4].buffer_type = MYSQL_TYPE_DATETIME;
		bind[4].buffer = &sql_lasttime;

		if (mysql_stmt_bind_param(stmt, bind)) {
			*(error) = g_strdup_printf(mysql_stmt_error(stmt), NULL);
		} else {
			if (mysql_stmt_execute(stmt)) {
				*(error) = g_strdup_printf(mysql_stmt_error(stmt), NULL);
			} else {
				ret = TRUE;
			}
		}
	}

	mysql_stmt_close(stmt);

	return ret;
}

gboolean db_clean_ship_gps(const struct Database *db, const gint64 *imo,
			   gchar **error)
{
	gboolean ret = FALSE;
	gchar *query;
	gint64 num_rows;
	MYSQL_STMT *stmt;
	MYSQL_BIND result[1];

	query = g_strdup_printf("SELECT COUNT(*) FROM GPS WHERE IMO = %" G_GINT64_FORMAT, *imo);
	num_rows = 0;

	stmt = mysql_stmt_init(db->con);
	if (!stmt) {
		*(error) = g_strdup("failed to prepare query, out of memory");
		return FALSE;
	}

	if (mysql_stmt_prepare(stmt, query, strlen(query))) {
		*(error) = g_strconcat("query prepare failed: " ,
				       mysql_stmt_error(stmt), NULL);
	} else {
		memset(result, 0, sizeof(result));
		result[0].buffer_type = MYSQL_TYPE_LONG;
		result[0].buffer = (char *)&num_rows;
		result[0].length = 0;

		if (mysql_stmt_bind_result(stmt, result)) {
			*(error) = g_strdup_printf(mysql_stmt_error(stmt), NULL);
		} else {
			if (mysql_stmt_execute(stmt)) {
				*(error) = g_strconcat("could not get number of GPS entries: ", mysql_stmt_error(stmt), NULL);
			} else {
				if (mysql_stmt_store_result(stmt)) {
					*(error) = g_strconcat(mysql_stmt_error(stmt), NULL);
				} else {
					if (mysql_stmt_fetch(stmt) != 0) {
						*(error) = g_strconcat("failed to fetch results, ",
								       mysql_stmt_error(stmt),
								       NULL);
					} else {
						ret = TRUE;
					}
					mysql_stmt_free_result(stmt);
				}
			}
		}
	}
	mysql_stmt_close(stmt);
	g_free(query);

	if (num_rows == 0)
		return ret;

	while (num_rows > 20) {
		gchar *id_query = g_strdup_printf("SELECT ID FROM GPS WHERE IMO = %" G_GINT64_FORMAT " ORDER BY ID ASC LIMIT 1", *imo);
		MYSQL_STMT *del_stmt;
		MYSQL_BIND del_result[1];
		gint32 log_id;

		log_id = 0;

		del_stmt = mysql_stmt_init(db->con);
		if (!del_stmt) {
			*(error) = g_strdup("failed to prepare query, out of memory");
			return FALSE;
		} else {
			if (mysql_stmt_prepare(del_stmt, id_query, strlen(id_query))) {
				*(error) = g_strconcat("query prepare failed: ",
						       mysql_stmt_error(del_stmt),
						       NULL);
			} else {
				memset(del_result, 0, sizeof(del_result));
				del_result[0].buffer_type = MYSQL_TYPE_LONG;
				del_result[0].buffer = (char *)&log_id;
				del_result[0].length = 0;

				if (mysql_stmt_bind_result(del_stmt, del_result)) {
					*(error) = g_strdup_printf(mysql_stmt_error(del_stmt), NULL);
				} else {
					if (mysql_stmt_execute(del_stmt)) {
						*(error) = g_strconcat("could not get GPS entry ID: ",
								       mysql_stmt_error(del_stmt), NULL);
					} else {
						if (mysql_stmt_store_result(del_stmt)) {
							*(error) = g_strconcat(mysql_stmt_error(del_stmt), NULL);
						} else {
							if (mysql_stmt_fetch(del_stmt)) {
								*(error) = g_strconcat(
									"failed to delete GPS location, ",
									mysql_stmt_error(del_stmt), NULL);
							} else {
								gchar *del_query = g_strdup_printf("DELETE FROM GPS WHERE ID = %" G_GINT32_FORMAT, log_id);
								if (mysql_query(db->con, del_query)) {
									*(error) = g_strconcat("query failed, ",
										mysql_stmt_error(del_stmt), NULL);
									ret = FALSE;
								} else {
									ret = TRUE;
								}
								g_free(del_query);
							}
							mysql_stmt_free_result(del_stmt);
						}
					}
				}
			}
			mysql_stmt_close(del_stmt);
		}
		--num_rows;
		g_free(id_query);
	}

	return ret;
}
