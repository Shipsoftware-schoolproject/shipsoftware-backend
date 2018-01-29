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

/**
 * @file database.h
 * @brief Wrapper around different database types
 * @details Provides simple interface to interact with database.
 * @author Tomi Lähteenmäki
 * @author Jori Jalkanen
 * @license This project is licensed under GNU General Public License, Version 2
 */

#ifndef DATABASE_H
#define DATABASE_H

extern struct Config *config;

/**
 * @struct Database
 * @brief Holds database related data
 * @details Holds properties of database connection
 */
struct Database {
	gpointer con; /**< Connection handle */
};

/**
 * @struct Route
 * @brief Holds data about ship route
 *
 * Holds data of ships departure and destination port, combined with
 * the ID of the route which is stored in database for that specific route.
 */
struct Route {
	gchar *departure; /**< Ship departure port */
	gchar *destination; /**< Ship destination port */
	gint64 id; /**< ShipRouteID in the database */
};

/**
 * Initialize database connection
 *
 * @param[in,out] db Struct of type Database()
 * @param[in] config Struct of type Config()
 * @param[out] error Pointer to gchar to store error message
 * @return gboolean Returns TRUE on success, otherwise FALSE
 */
gboolean db_init(struct Database *db, const struct Config *config,
		 gchar **error);

/**
 * Close connection
 *
 * @param[in] db Struct of type Database()
 * @return Nothing
 */
void db_close_con(struct Database *db);

/**
 * @brief Get MMSI's of ships
 *
 * Return all MMSI's of the ships in the database.
 *
 * @param[in] db Struct type of Database()
 * @param[out] ships Pointer to gchar
 * @param[out] error Pointer to gchar where to store error message
 * @return gboolean Returns TRUE on success, otherwise FALSE
 */
gboolean db_get_ships(const struct Database *db, gchar **ships, gchar **error);

/**
 * Update ship Course and Speed
 *
 * @param[in] db Struct of type Database()
 * @param[in] course
 * @param[in] speed
 * @param[in] mmsi Ship MMSI number
 * @param[out] error Pointer to gchar where to store error message
 * @return gboolean Returns TRUE on success, otherwise FALSE
 */
gboolean db_update_ship_course_speed(const struct Database *db,
				     const gfloat course, const gdouble speed,
				     const gint64 mmsi, gchar **error);

/**
 * Update GPS location in GPS table
 *
 * @param[in] db Struct of type Database()
 * @param[in] latitude
 * @param[in] longitude
 * @param[in] mmsi Ship MMSI number
 * @param[out] error Pointer to gchar where to store error message
 * @return gboolean Returns TRUE on success, otherwise FALSE
 */
gboolean db_update_gps_location(const struct Database *db, const gfloat latitude,
				const gfloat longitude, const gint64 mmsi,
				gchar **error);

/**
 * @brief Get route ID
 *
 * Get route ID for route where departure port is @p departure and destination
 * port is @p destination
 *
 * @param[in] db Struct of type Database()
 * @param[in,out] route Struct of type Route()
 * @param[out] error Pointer to gchar where to store error message
 * @return gboolean Returns TRUE on success, otherwise FALSE
 */
gboolean db_get_route_id(const struct Database *db, struct Route *route,
			 gchar **error);

/**
 * Update ship current route
 *
 * @param[in] db Struct of type Database()
 * @param[in] id Route ID
 * @param[in] mmsi Ship MMSI number
 * @param[out] error Pointer to gchar where to store error message
 * @return gboolean Returns TRUE on success, otherwise FALSE
 */
gboolean db_update_route(const struct Database *db, gint64 id,
			 const gint64 mmsi, gchar **error);

#endif
