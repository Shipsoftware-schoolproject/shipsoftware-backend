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
#include <mysql.h>
#include <string.h>
#include "database.h"
#include "config.h"

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

gboolean db_update_ship_course_speed(const struct Database *db,
				     const gfloat course, const gdouble speed,
				     const gint64 mmsi, gchar **error)
{
	gboolean ret;
	gchar *query;
	MYSQL_STMT *stmt;
	MYSQL_BIND bind[3];

	ret = FALSE;
	query = "UPDATE Ships SET Course = ?, ShipSpeed = ? WHERE MMSI = ?";

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
		bind[0].buffer_type = MYSQL_TYPE_FLOAT;
		bind[0].buffer = (char *)&course;
		bind[0].is_null = 0;
		bind[0].length = 0;

		bind[1].buffer_type = MYSQL_TYPE_DOUBLE;
		bind[1].buffer = (char *)&speed;
		bind[1].is_null = 0;
		bind[1].length = 0;

		bind[2].buffer_type = MYSQL_TYPE_LONG;
		bind[2].buffer = (char *)&mmsi;
		bind[2].is_null = 0;
		bind[2].length= 0;

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

static gboolean _get_ship_id(const struct Database *db, const gint64 mmsi,
			     gint32 *ship_id, gchar **error)
{
	gboolean ret;
	const gchar *query;
	MYSQL_STMT *stmt;
	MYSQL_BIND bind[1];
	MYSQL_BIND result[1];
	gint32 id;

	ret = FALSE;
	query = "SELECT ShipID FROM Ships WHERE MMSI = ?";
	stmt = mysql_stmt_init(db->con);
	if (!stmt) {
		*(error) = g_strdup("failed to prepare query, out of memory");
		return FALSE;
	}

	if (mysql_stmt_prepare(stmt, query, strlen(query))) {
		*(error) = g_strconcat("query prepare failed: " ,
				       mysql_stmt_error(stmt), NULL);
	} else {
		memset(bind, 0, sizeof(bind));
		bind[0].buffer_type = MYSQL_TYPE_LONG;
		bind[0].buffer = (char *)&mmsi;
		bind[0].is_null = 0;
		bind[0].length = 0;

		memset(result, 0, sizeof(result));
		result[0].buffer_type = MYSQL_TYPE_LONG;
		result[0].buffer = (char *)&id;
		result[0].length = 0;

		if (mysql_stmt_bind_param(stmt, bind)) {
			*(error) = g_strdup_printf(mysql_stmt_error(stmt), NULL);
		} else {
			if (mysql_stmt_bind_result(stmt, result)) {
				*(error) = g_strdup_printf(mysql_stmt_error(stmt), NULL);
			} else {
				if (mysql_stmt_execute(stmt)) {
					*(error) = g_strdup_printf(mysql_stmt_error(stmt));
				} else {
					if (mysql_stmt_store_result(stmt)) {
						*(error) = g_strdup_printf(mysql_stmt_error(stmt));
					} else {
						if (mysql_stmt_fetch(stmt) != 0) {
							*(error) = g_strdup("no results");
						} else {
							(*ship_id) = id;
							mysql_stmt_free_result(stmt);
							ret = TRUE;
						}
					}
				}
			}
		}
	}

	mysql_stmt_close(stmt);

	return ret;
}

gboolean db_update_gps_location(const struct Database *db, const gfloat latitude,
				const gfloat longitude, const gint64 mmsi,
				gchar **error)
{
	gboolean ret;
	const gchar *query;
	gint32 ship_id;
	MYSQL_STMT *stmt;
	MYSQL_BIND bind[3];

	ret = FALSE;
	query = "INSERT INTO GPS (ShipID, North, East) VALUES (?, ?, ?)";

	if (!_get_ship_id(db, mmsi, &ship_id, error)) {
		*(error) = g_strconcat("couldn't get ShipID: ", *(error), NULL);
		return FALSE;
	}

	stmt = mysql_stmt_init(db->con);
	if (!stmt) {
		*(error) = g_strdup("failed to prepare query, out of memory");
		return FALSE;
	}

	if (mysql_stmt_prepare(stmt, query, strlen(query))) {
		*(error) = g_strconcat("query prepare failed: " ,
				       mysql_stmt_error(stmt), NULL);
	} else {
		memset(bind, 0, sizeof(bind));
		bind[0].buffer_type = MYSQL_TYPE_LONG;
		bind[0].buffer = (void *)&ship_id;
		bind[0].is_null = 0;
		bind[0].length= 0;

		bind[1].buffer_type = MYSQL_TYPE_FLOAT;
		bind[1].buffer = (char *)&latitude;
		bind[1].is_null = 0;
		bind[1].length = 0;

		bind[2].buffer_type = MYSQL_TYPE_FLOAT;
		bind[2].buffer = (char *)&longitude;
		bind[2].is_null = 0;
		bind[2].length = 0;

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

	/**
	 * @todo Issue [#2](https://github.com/Shipsoftware-schoolproject/shipsoftware-sql/issues/1)
	 * needs to be fixed in [shipsoftware-sql](https://github.com/Shipsoftware-schoolproject/shipsoftware-sql)
	 * project. Until that, we use this ugly "work around"..
	 */
	if (ret) {
		ret = FALSE;
		gchar *count_query;
		gint64 num_rows;
		MYSQL_BIND result[1];

		count_query = g_strdup_printf("SELECT COUNT(*) FROM GPS WHERE ShipID = %" G_GINT32_FORMAT, ship_id);
		num_rows = 0;

		stmt = mysql_stmt_init(db->con);
		if (!stmt) {
			*(error) = g_strdup("failed to prepare query, out of memory");
			return FALSE;
		}

		if (mysql_stmt_prepare(stmt, count_query, strlen(count_query))) {
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
							*(error) = g_strconcat("failed to fetch results, ", mysql_stmt_error(stmt), NULL);
						} else {
							while (num_rows > 5) {
								gchar *query = g_strdup_printf("SELECT LogID FROM GPS WHERE ShipID = %" G_GINT32_FORMAT " ORDER BY LogID ASC LIMIT 1", ship_id);
								MYSQL_STMT *del_stmt;
								MYSQL_BIND del_result[1];
								gint32 log_id;

								log_id = 0;

								del_stmt = mysql_stmt_init(db->con);
								if (del_stmt) {
									if (!mysql_stmt_prepare(del_stmt, query, strlen(query))) {
										memset(del_result, 0, sizeof(del_result));
										del_result[0].buffer_type = MYSQL_TYPE_LONG;
										del_result[0].buffer = (char *)&log_id;
										del_result[0].length = 0;

										if (!mysql_stmt_bind_result(del_stmt, del_result)) {
											if (!mysql_stmt_execute(del_stmt)) {
												if (!mysql_stmt_store_result(del_stmt)) {
													if (mysql_stmt_fetch(del_stmt) == 0) {
														gchar *del_query = g_strdup_printf("DELETE FROM GPS WHERE LogID = %" G_GINT32_FORMAT, log_id);
														mysql_query(db->con, del_query);
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
								g_free(query);
							}
							ret = TRUE;
						}
						mysql_stmt_free_result(stmt);
					}
				}
			}
		}

		mysql_stmt_close(stmt);
		g_free(count_query);
	}

	return ret;
}

gboolean db_get_route_id(const struct Database *db, struct Route *route,
			 gchar **error)
{
	gboolean ret;
	gchar *query;
	MYSQL_STMT *stmt;
	MYSQL_BIND bind[2];
	MYSQL_BIND result[1];
	gint id;

	ret = FALSE;

	if (route->departure == NULL && route->destination != NULL) {
		*(error) = g_strdup("missing departure port");
		return FALSE;
	} else if (route->departure != NULL && route->destination == NULL) {
		*(error) = g_strdup("missing destination port");
		return FALSE;
	} else if (route->departure != NULL && route->destination != NULL) {
		query = "SELECT ShipRoutesID FROM ShipRoutes WHERE "
			"StartingPortID = (SELECT ShipPortID FROM ShipPorts WHERE Name LIKE ?) "
			"AND EndingPortID = (SELECT ShipPortID FROM ShipPorts WHERE Name LIKE ?)";
	} else {
		*(error) = g_strdup("no departure nor destination port");
		return FALSE;
	}

	stmt = mysql_stmt_init(db->con);
	if (!stmt) {
		*(error) = g_strdup("failed to prepare query, out of memory");
		return FALSE;
	}

	if (mysql_stmt_prepare(stmt, query, strlen(query))) {
		*(error) = g_strconcat("query prepare failed: " , mysql_stmt_error(stmt), NULL);
	} else {
		memset(bind, 0, sizeof(bind));
		bind[0].buffer_type = MYSQL_TYPE_STRING;
		bind[0].buffer = route->departure;
		bind[0].buffer_length = strlen(route->departure);

		bind[1].buffer_type = MYSQL_TYPE_STRING;
		bind[1].buffer = route->destination;
		bind[1].buffer_length = strlen(route->destination);

		memset(result, 0, sizeof(result));
		result[0].buffer_type = MYSQL_TYPE_LONG;
		result[0].buffer = (char *)&id;
		result[0].length = 0;

		if (mysql_stmt_bind_param(stmt, bind)) {
			*(error) = g_strdup_printf(mysql_stmt_error(stmt), NULL);
		} else {
			if (mysql_stmt_bind_result(stmt, result)) {
				*(error) = g_strdup_printf(mysql_stmt_error(stmt), NULL);
			} else {
				if (mysql_stmt_execute(stmt)) {
					*(error) = g_strdup_printf(mysql_stmt_error(stmt), NULL);
				} else {
					if (mysql_stmt_store_result(stmt)) {
						*(error) = g_strdup_printf(mysql_stmt_error(stmt), NULL);
					} else {
						gint result_ret = mysql_stmt_fetch(stmt);

						if (result_ret != 0) {
							*(error) = g_strdup("no results");
						} else {
							route->id = id;
							mysql_stmt_free_result(stmt);
							ret = TRUE;
						}
					}
				}
			}
		}
	}

	mysql_stmt_close(stmt);

	return ret;
}

gboolean db_update_route(const struct Database *db, const gint64 id,
			 const gint64 mmsi, gchar **error)
{
	const gchar *query;

	query = g_strconcat("UPDATE Ships SET ShipRoutesID = ",
			    g_strdup_printf("%" G_GINT64_FORMAT, id),
			    " WHERE MMSI = ", g_strdup_printf("%" G_GINT64_FORMAT, mmsi), NULL);
	if (mysql_query(db->con, query)) {
		*(error) = g_strconcat("query failed: ", mysql_error(db->con), NULL);
		return FALSE;
	}

	return TRUE;
}
