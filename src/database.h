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

#include "config.h"
#include "ship_defines.h"

/**
 * @struct Database
 * @brief Holds database related data
 * @details Holds properties of database connection
 */
struct Database {
	gpointer con; /**< Connection handle */
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
 * Update ship information in Ships table
 *
 * @param[in] db Struct of type Database()
 * @param[in] info Struct of type Ship()
 * @param[out] error Pointer to gchar where to store error message
 * @return gboolean Returns TRUE on success, otherwise FALSE
 * @note IMO seems to vary from time to time, so ship info (including IMO) is
 * updated based on the MMSI
 */
gboolean db_update_ship_info(const struct Database *db,
			     struct Ship *info,
			     gchar **error);

/**
 * Update GPS information in GPS table
 *
 * @param[in] db Struct of type Database()
 * @param[in] info Struct of type Ship()
 * @param[out] error Pointer to gchar where to store error message
 * @return gboolean Returns TRUE on success, otherwise FALSE
 */
gboolean db_update_ship_gps(const struct Database *db,
			    struct Ship *info,
			    gchar **error);

/**
 * Remove old records from GPS table
 *
 * @param[in] db Struct of type Database()
 * @param[in] IMO Pointer ship IMO
 * @param[out] error Pointer to gchar where to store error message
 * @return gboolean Returns TRUE if no records to delete or delete success,
 * otherwise FALSE
 *
 * @note This could be removed if database had a trigger or event which
 * deletes old records when inserting new record into the table.
 */
gboolean db_clean_ship_gps(const struct Database *db, const gint64 *imo,
			   gchar **error);

#endif
