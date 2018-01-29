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
 * @file dialogs.h
 * @brief Simple dialogs
 * @details Shows dialogs to the user.
 * @author Tomi Lähteenmäki
 * @license This project is licensed under GNU General Public License, Version 2
 */

#ifndef DIALOGS_H
#define DIALOGS_H

#include <gtk/gtk.h>

/**
 * Logo logo.xmp
 */
extern const char *logo_xpm[];

/**
 * @brief Show About dialog
 *
 * Shows About dialog to user which contains information about the program
 *
 * @param[in] parent GTK_WINDOW where the dialog was triggered from
 * @return Nothing
 */
void show_about_dialog(gpointer parent);

/**
 * @brief Show dialog with YES/NO button
 *
 * Shows a dialog with YES / NO button and returns GTK_RESPONSE of which
 * button the user clicked
 *
 * @param[in] parent GTK_WINDOW where the dialog was triggered from
 * @param[in] title Dialogs title
 * @param[in] message Dialogs content
 * @return int GTK_RESPONSE_*
 */
int show_yesno_dialog(GtkWidget *parent, const char *title,
		      const char *message);

/**
 * @brief Show dialog with OK button
 *
 * Shows a dialog with OK button and a message
 *
 * @param[in] parent GTK_WINDOW where the dialog was triggered from
 * @param[in] title Dialogs title
 * @param[in] message Dialogs content
 * @return Nothing
 */
void show_ok_dialog(GtkWidget *parent, const char *title, const char *message);

#endif
