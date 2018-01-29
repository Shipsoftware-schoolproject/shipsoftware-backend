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
 * @file config.h
 * @brief Configuration related functionalities
 * @details Provides configuration loading, saving and parsing functionality.
 * @author Tomi Lähteenmäki
 * @license This project is licensed under GNU General Public License, Version 2
 * @see @ref ConfigPage page for information of the structure of the @c
 * configuration.json
 */

#ifndef CONFIG_H
#define CONFIG_H

/**
 * @struct Config
 * @brief Struct to hold configuration
 *
 * Holds all configuration properties
 */
struct Config {
	const gchar *db_name; /**< Name of the database */
	const gchar *db_username; /**< Username to used to connect to the database */
	const gchar *db_password; /**< Password for the database user */
	const gchar *db_hostname; /**< Hostname of the database */
	const gchar *api_key; /**< aprs.fi API key */
	gint64 log_size; /**< Number of rows to keep in GUI listbox */
};

/**
 * @brief Load configuration file and return it as a string
 *
 * Load configuration file content and stores it as a gchar in the @c contents.
 *
 * @param[in,out] config Struct of type config()
 * @param[out] contents Pointer to ghcar where to store file contents
 * @param[out] error Pointer to ghcar where to store error message
 * @return gboolean TRUE if file contents was loaded successfully,
 * otherwise FALSE
 */
gboolean load_config(struct Config *config, gchar **contents, gchar **error);

/**
 * @brief Parses @c contents values and assigns those values to @c config struct
 *
 * Tries to parse properties from @c contents and assing those those to
 * @c config structure
 *
 * @param[in,out] config Struct of type config()
 * @param[in] contents JSON as ghcar
 * @param[out] error Pointer to gchar where to store error message
 * @return gboolean TRUE if all properties needed properties were parsed
 * from @c contents, otherwise FALSE
 * @note Struct config() properties should be valited with validate_config()
 */
gboolean parse_config(struct Config *config, const gchar *contents,
		      gchar **error);

/**
 * @brief Validates properties of @c config
 *
 * Validates @c config struct properties
 *
 * @param[in,out] config Struct of type config()
 * @param[out] error Pointer to ghcar where to store error message
 * @return gboolean TRUE if all properties are valid, otherwise FALSE
 */
gboolean validate_config(const struct Config *config, gchar **error);

#ifdef WITH_GUI
/**
 * @brief Saves configuration to file
 *
 * Saves @c config struct properties to the configuration file
 *
 * @param[in] config Struct of type config()
 * @param[out] error Pointer to ghcar where to store error message
 * @return gboolean TRUE if configuration was saved, otherwise FALSE
 */
gboolean save_config(const struct Config *config, gchar **error);
#endif

#endif
