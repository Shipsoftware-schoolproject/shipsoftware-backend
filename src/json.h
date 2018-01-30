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
 * @file json.h
 * @brief JSON wrapper
 * @author Tomi Lähteenmäki
 * @details Wraps glib-json JSON parsing functionality for simpler usage.
 * @author Tomi Lähteenmäki
 * @license This project is licensed under GNU General Public License, Version 2
 */

#ifndef JSON_H
#define JSON_H

#include "config.h"

/**
 * Read INT value from member
 *
 * @param[in] member Key in JSON
 * @param[in] json JSON which to read
 * @param[out] value Pointer to gint where to store the value
 * @return gboolean TRUE if value was read successfully, otherwise FALSE
 */
gboolean json_read_int(const gchar *const member, const gchar *json,
		       gint64 *value);

/**
 * Read GCHAR from member
 *
 * @param[in] member Key in JSON
 * @param[in] json JSON which to read
 * @param[out] value Pointer to gchar where to store the value
 * @return gboolean TRUE if value was read successfully, otherwise FALSE
 */
gboolean json_read_string(const gchar *member, const gchar *json,
			  gchar **value);

/**
 * Read INT value from @c json @c member in "entries" array @c index
 *
 * @param[in] member Key in JSON
 * @param[in] json JSON which to read
 * @param[in] index Index in the array
 * @return gint64
 */
gint64 json_read_entry_int(const gchar *member, const gchar *json,
			   const gint64 index);

/**
 * Read DOUBLE value from @c json @c member in "entries" array @c index
 *
 * @param[in] member Key in JSON
 * @param[in] json JSON which to read
 * @param[in] index Index in the array
 * @return gdouble
 *
 * @note Use only to parse API response!
 */
gdouble json_read_entry_double(const gchar *member, const gchar *json,
			       const gint64 index);

/**
 * Read FLOAT value from @c json @c member in "entries" array @c index
 *
 * @param[in] member Key in JSON
 * @param[in] json JSON which to read
 * @param[in] index Index in the array
 * @return gfloat
 *
 * @note Use only to parse API response!
 */
gfloat json_read_entry_float(const gchar *member, const gchar *json,
			       const gint64 index);

/**
 * Read STRING value from @c json @c member in "entries" array @c index
 *
 * @param[in] member Key in JSON
 * @param[in] json JSON which to read
 * @param[in] index Index in the array
 * @return gchar*
 *
 * @note Use only to parse API response!
 */
gchar *json_read_entry_string(const gchar *member, const gchar *json,
			      const gint64 index);

/**
 * Read GCHAR from value @c json @c member in "entries" array @c index
 *
 * @param member Key in JSON
 * @param json JSON which to read
 * @param index Index in the array
 * @return gchar
 *
 * @note Use only to parse API response!
 */
gchar json_read_entry_char(const gchar *member, const gchar *json,
			   const gint64 index);

/**
 * Save config struct to file
 *
 * @param[in] config Struct of type config()
 * @param[out] error Pointer to gchar to store error message
 * @return gboolean
 */
gboolean save_json_file(const struct Config *config, gchar **error);

#endif
