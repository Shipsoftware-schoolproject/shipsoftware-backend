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

#include <json-glib/json-glib.h>
#include <glib-object.h>
#include <string.h>
#include "json.h"
#include "config.h"

static gboolean _get_node(gchar *member, gchar *json, JsonNode **node)
{
	gboolean ret;
	JsonParser *parser;

	parser = json_parser_new();

	if (!json_parser_load_from_data(parser, json, strlen(json), NULL)) {
		ret = FALSE;
	} else {
		JsonNode *root = json_parser_get_root(parser);
		JsonObject *obj = json_node_get_object(root);
		JsonNode *_member = json_object_get_member(obj, member);

		if (!_member) {
			ret = FALSE;
		} else {
			*(node) = json_node_copy(_member);
			ret = TRUE;
		}
	}

	g_object_unref(parser);

	return ret;
}

static gboolean _get_node_from_array(gchar *member, gchar *json, gint64 index, JsonNode **node)
{
	gboolean ret;
	JsonParser *parser;

	parser = json_parser_new();

	if (!json_parser_load_from_data(parser, json, strlen(json), NULL)) {
		ret = FALSE;
	} else {
		JsonNode *root = json_parser_get_root(parser);
		JsonObject *obj = json_node_get_object(root);
		JsonArray *entries = json_object_get_array_member(obj, "entries");

		if (!entries) {
			ret = FALSE;
		} else {
			JsonObject *entry = json_array_get_object_element(entries, index);

			if (!entry) {
				ret = FALSE;
			} else {
				JsonNode *_member = json_object_get_member(entry, member);

				if (!_member) {
					ret = FALSE;
				} else {
					*(node) = json_node_copy(_member);
					ret = TRUE;
				}
			}
		}
	}

	g_object_unref(parser);

	return ret;
}

gboolean json_read_int(gchar *member, gchar *json, gint64 *value)
{
	JsonNode *node = json_node_alloc();

	if (_get_node(member, json, &node)) {
		gint64 val = json_node_get_int(node);
		memcpy(value, &val, sizeof(gint64));
		json_node_free(node);
	} else {
		return FALSE;
	}

	return TRUE;
}

gboolean json_read_string(gchar *member, gchar *json, gchar **value)
{
	JsonNode *node;

	if (_get_node(member, json, &node)) {
		*(value) = g_strdup(json_node_get_string(node));
		json_node_free(node);
	} else {
		return FALSE;
	}

	return TRUE;
}

gint64 json_read_entry_int(gchar *member, gchar *json, gint64 index)
{
	gint64 ret;
	JsonNode *node;

	if (_get_node_from_array(member, json, index, &node)) {
		ret = g_ascii_strtoll(json_node_get_string(node), NULL, 10);
		json_node_free(node);
	} else {
		ret = 0;
	}

	return ret;
}

gdouble json_read_entry_double(gchar *member, gchar *json, gint64 index)
{
	gdouble ret;
	JsonNode *node;

	if (_get_node_from_array(member, json, index, &node)) {
		// ARPS.fi returns first entry as a double but subsequent entries as a "string"
		// for: lat, lng, course; expect speed..
		if (g_strcmp0(member, "speed") == 0) {
			ret = json_node_get_double(node);
		} else {
			if (index < 1) {
				ret = json_node_get_double(node);
			} else {
				const gchar *val = json_node_get_string(node);
				ret = g_ascii_strtod(val, NULL);
			}
		}
		json_node_free(node);
	} else {
		ret = 0;
	}

	return ret;
}

gchar *json_read_entry_string(gchar *member, gchar *json, gint64 index)
{
	gchar *ret;
	JsonNode *node;

	if (_get_node_from_array(member, json, index, &node)) {
		ret = g_strdup(json_node_get_string(node));
		json_node_free(node);
	} else {
		ret = NULL;
	}

	return ret;
}

gboolean save_json_file(struct Config *config, gchar **error)
{
	gboolean ret;
	GError *_error;
	JsonBuilder *builder;
	JsonGenerator *generator;
	JsonNode *root;

	ret = FALSE;
	_error = NULL;

	builder = json_builder_new();
	json_builder_begin_object(builder);
	json_builder_set_member_name(builder, "database");
	json_builder_add_string_value(builder, config->db_name);
	json_builder_set_member_name(builder, "username");
	json_builder_add_string_value(builder, config->db_username);
	json_builder_set_member_name(builder, "password");
	json_builder_add_string_value(builder, config->db_password);
	json_builder_set_member_name(builder, "hostname");
	json_builder_add_string_value(builder, config->db_hostname);
	json_builder_set_member_name(builder, "api_key");
	json_builder_add_string_value(builder, config->api_key);
	json_builder_set_member_name(builder, "log_size");
	json_builder_add_int_value(builder, config->log_size);
	json_builder_end_object(builder);

	generator = json_generator_new();
	root = json_builder_get_root(builder);
	json_generator_set_pretty(generator, TRUE);
	json_generator_set_indent(generator, 4);
	json_generator_set_root(generator, root);

	if (!json_generator_to_file(generator, "configuration.json", &_error)) {
		*(error) = g_strdup(_error->message);
	} else {
		ret = TRUE;
	}

	json_node_free(root);
	g_object_unref(generator);
	g_object_unref(builder);

	return ret;
}
