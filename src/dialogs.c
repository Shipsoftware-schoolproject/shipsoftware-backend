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

#include "dialogs.h"
#include "version.h"

void show_about_dialog(gpointer parent)
{
	GtkWidget *dialog;
	const gchar *program_name;
	const gchar *copyright;
	const gchar *comment;
	const gchar *authors[] = { "Tomi Lähteenmäki", "Jori Jalkanen", NULL };
	const gchar *website_label;
	const gchar *website;

	dialog = gtk_about_dialog_new();
	program_name = "ShipSoftware-backend";
	copyright = "Copyright \xc2\xa9 2018 Tomi Lähteenmäki, Jori Jalkanen";
	comment = "School project made in VAMK.";
	website_label = "Source at Github";
	website = "https://github.com/Shipsoftware-schoolproject";

	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), program_name);
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), ShipSoftwareBackend_VERSION);
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), copyright);
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), comment);
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(dialog), authors);
	gtk_about_dialog_set_website_label(GTK_ABOUT_DIALOG(dialog), website_label);
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), website);
	gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), gdk_pixbuf_new_from_xpm_data(logo_xpm));

	gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
	gtk_window_set_destroy_with_parent(GTK_WINDOW(parent), TRUE);
	gtk_window_set_title(GTK_WINDOW(dialog), "About");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

int show_yesno_dialog(GtkWidget *parent, const char *title, const char *message)
{
	gint ret;
	GtkDialogFlags flags;
	GtkWidget *dialog;

	flags = GTK_DIALOG_DESTROY_WITH_PARENT;
	dialog = gtk_message_dialog_new(GTK_WINDOW(parent), flags,
					GTK_MESSAGE_ERROR, GTK_BUTTONS_YES_NO,
					message);

	gtk_window_set_title(GTK_WINDOW(dialog), title);

	ret = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	return ret;
}

void show_ok_dialog(GtkWidget *parent, const char *title, const char *message)
{
	GtkDialogFlags flags;
	GtkWidget *dialog;

	flags = GTK_DIALOG_DESTROY_WITH_PARENT;
	dialog = gtk_message_dialog_new(GTK_WINDOW(parent), flags,
					GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
					message);

	gtk_window_set_title(GTK_WINDOW(dialog), title);

	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}
