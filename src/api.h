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

/**
 * @file api.h
 * @brief aprs.fi API caller
 * @details Functions to get data from aprs.fi API
 * @author Tomi Lähteenmäki
 * @license This project is licensed under GNU General Public License, Version 2
 * @note API_URL is defined in api.c with @c @#DEFINE!
 * @see arps.fi API documentation https://aprs.fi/page/api
 */

#ifndef API_H
#define API_H

/**
 * @brief Get location data of the ships
 *
 * Returns ships location data in JSON in a gchar.
 *
 * @param[in] name Identifier(s) of ships
 * @param[in] api_key API key
 * @param[out] data JSON as gchar
 * @param[out] error Pointer to gchar where to store error message
 * @return gboolean TRUE on success, otherwise FALSE
 * @warning @p data is not touched if API or something fails! Meaning it's
 * stays uninitialized if it was uninitialized before calling this function.
 * @note @p data should be freed with g_free() after it's not needed anymore.
 */
gboolean api_get_loc(const gchar *name, const gchar *api_key, gchar **data, gchar **error);

#endif
