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
 * @file config_window.h
 * @brief Configuration dialog
 * @details Provides configuration window
 * @author Tomi Lähteenmäki
 * @license This project is licensed under GNU General Public License, Version 2
 */

#ifndef CONFIG_WINDOW_H
#define CONFIG_WINDOW_H

#include <gtk/gtk.h>

extern int RUNNING;
/**
 * @see Config()
 */
extern struct Config *config;

/**
 * @struct new_config
 * @brief Holds GtkWidget pointers to the widgets
 *
 * Holds GtkWidget pointers which are used to get data from them.
 */
struct new_config {
    GtkWidget *window; /**< The configuration window itself */
    GtkWidget *db_name; /**< GTK_ENTRY of database name */
    GtkWidget *db_username; /**< GTK_ENTRY of database username */
    GtkWidget *db_password; /**< GTK_ENTRY of user password */
    GtkWidget *db_hostname; /**< GTK_ENTRY of database hostname */
    GtkWidget *api_key; /**< GTK_ENTRY of API key */
    GtkWidget *log_size; /**< GTK_ENTRY of log size */
};

/**
 * @brief Shows configuration window
 *
 * Constructs configuration window and shows it.
 *
 * @param[in] parent
 * @return gint Returns gint which is type of GTK_RESPONSE
 * @note @p widget is not used
 * @note @p parent should be type of GtkWidget*
 */
gint show_config_window(gpointer parent);

#endif
