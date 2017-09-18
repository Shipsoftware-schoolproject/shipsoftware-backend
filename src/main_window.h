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
 * @file main_window.h
 * @brief Main window for the program.
 * @details This provides the main window for the program if it's compiled
 * with GUI support.
 * @author Tomi Lähteenmäki
 * @license This project is licensed under GNU General Public License, Version 2
 */

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <gtk/gtk.h>
#include "config.h"

/**
 * @struct label_data
 * @brief Struct to hold data for label
 *
 * Holds needed data to update GTK_LABEL with wanted properties
 */
struct label_data {
	GtkWidget *widget; /**< GTK_LABEL which properties to change */
	gboolean color; /**< Text color */
	const char *text; /**< Text which to put in the label */
};

int RUNNING;
GMutex MUTEX; /**< Shared mutex between main_window() and api_thread() */
GtkWidget *LISTBOX_LOGS; /**< Listbox for the logs */
GtkWidget *LABEL_RUNNING; /**< Label for running state */
GtkWidget *BUTTON_START; /**< Start button */
GtkWidget *LABEL_LAST_UPDATED; /**< Last updated label */

/**
 * @brief Prepends row with message to GTK_LIST_BOX
 *
 * Prepends a row with @c message to GTK_LIST_BOX and sets tooltip with
 * current date and time.
 *
 * @param[in] message Message for the row
 * @return gboolean
 * @note Returns always FALSE
 * @note @p message will be freed with g_free()
 */
gboolean add_log_row(void *message);

/**
 * Clears all rows from GTK_LIST_BOX
 *
 * @return Nothing
 */
void clear_logs();

/**
 * Sets START_BUTTON text to supplied text
 *
 * @param[in] text
 * @return gboolean
 * @note Returns always FALSE
 */
gboolean set_button_text(void *text);

/**
 * Updates label
 *
 * @param[in] data Struct of type label_data()
 * @return gboolean
 * @note Retuns always FALSE
 */
gboolean update_label(gpointer data);

/**
 * Toggle @ref RUNNING variable
 *
 * @return Nothing
 */
void start_clicked();

/**
 * @brief Constructs and displays main window and loads configuration
 *
 * Constructs main window and loads configuration. If there were errors
 * while loading or parsing the configuration, a dialog is created asking
 * does the user want to open config_window().
 *
 * If user does not want to open config_window(), program cannot be started.
 * Otherwise START_BUTTON is made clickable.
 *
 * @param[in] app GtkApplication
 * @return Nothing
 */
void activate_mainwindow(GtkApplication *app);

#endif
