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
 * @file ship_defines.h
 * @brief Definitions which are needed by other files
 * @author Tomi Lähteenmäki
 * @license This project is licensed under GNU General Public License, Version 2
 */

#ifndef SHIPSOFTWAREBACKEND_SHIP_DEFINES_H
#define SHIPSOFTWAREBACKEND_SHIP_DEFINES_H

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

#endif //SHIPSOFTWAREBACKEND_SHIP_DEFINES_H
