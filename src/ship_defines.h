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
 * @struct Ship
 * @brief Holds information about ship
 *
 * Contains structs of information and GPS information
 */
struct Ship {
	// Info
	gint64 imo; /**< Ship IMO number */
	gchar *name; /**< Ship name */
	gint64 mmsi; /**< Ship MMSI number */
	gfloat course; /**< Course in degress */
	gfloat speed; /**< Speed in km/h */
	gchar *comment; /**< APRS comment or AIS destination and ETA */
	gint heading; /**< Heading */
	gfloat length; /**< Ship length in meters */
	gfloat width; /**< Ship width in meters */
	gfloat draught; /**< Ship draught in meters */
	gint ref_front; /**< AIS reference distance from the front */
	gint ref_left; /**< AIS reference distance from the left */
	gchar *path; /**< Packet path */
	gchar class; /**< Class of station identifier (a: ARPS, w: Web..) */
	gchar type; /**< Target type (a: AIS, i: ARPS item, o: ARPS object, w: weather station) */
	gchar *srccall; /**< Source callsign */
	gchar *dstcall; /**< APRS packet destination callsign */
	gint vessel_class; /**< AIS class code */
	gint navstat; /**< AIS navigational status code */

	// GPS
	time_t time; /**< Time when the target first reported this position */
	time_t lasttime; /**< Time when the target last reported this position */
	gdouble latitude; /**< Latitude in decimal degrees, north is positive */
	gdouble longitude; /**< Longitude in decimal degrees, east is positive */
};

#endif //SHIPSOFTWAREBACKEND_SHIP_DEFINES_H
