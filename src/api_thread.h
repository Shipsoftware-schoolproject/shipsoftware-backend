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
 * @file api_thread.h
 * @brief Thread which gets data from API and inserts it to database
 * @details Provides the main functionality of the program; get's data from
 * aprs.fi and inserts it to database.
 * @author Tomi Lähteenmäki
 * @license This project is licensed under GNU General Public License, Version 2
 */

#ifndef API_THREAD_H
#define API_THREAD_H

#include <glib-object.h>

/**
 * @see Config()
 */
extern struct Config *config;
/**
 * @see Database()
 */
extern struct Database *db;

#ifdef WITH_GUI
/**
 * @see label_data
 */
extern struct label_data data;
#endif

int RUNNING; /**< Holds the thread running state */
GMutex MUTEX; /**< MUTEX */

/**
 * @brief Get current local time
 *
 * Return current local time in pretty format
 *
 * @return ghcar
 */
gchar *get_datetime();

/**
 * @brief Wrapper to print log messages
 *
 * Prints log messages; if built with GTK support, messages are prepended
 * to @p GTK_LIST_BOX, otherwise they are printed to stdout.
 *
 * @param[in] message
 * @return Nothing
 */
void log_message(gpointer message);

/**
 * @brief Wrapper to print log error messages
 *
 * Prints error log messages; if built with GTK support, messages are prepended
 * to @p GTK_LIST_BOX, otherwise they are printed to stderr.
 *
 * @param[in] message
 * @return Nothing
 */
void log_error(gpointer message);

/**
 * @brief The thread function
 *
 * This is the function which should be threaded, it handles the getting data
 * from the API and inserting it to the database. The thread will run until the
 * the @p RUNNING_STATE is not "0".
 *
 * @return gpointer
 * @note Returns always NULL
 */
gpointer api_thread();

#endif
