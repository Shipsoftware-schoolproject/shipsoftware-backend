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

#include <json-glib/json-glib.h>
#include <string.h>
#include "json.h"

static gboolean _get_node(const gchar *member, const gchar *json,
			  JsonNode **node)
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

static gboolean _get_node_from_array(const gchar *member, const gchar *json,
				     const gint64 index, JsonNode **node)
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

gboolean json_read_int(const gchar *member, const gchar *json, gint64 *value)
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

gboolean json_read_string(const gchar *member, const gchar *json, gchar **value)
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

gint64 json_read_entry_int(const gchar *member, const gchar *json,
			   const gint64 index)
{
	gint64 ret = 0;
	JsonNode *node;
	GType node_type;

	if (_get_node_from_array(member, json, index, &node)) {
		node_type = json_node_get_value_type(node);

		switch (node_type) {
			case G_TYPE_INT:
			case G_TYPE_UINT:
			case G_TYPE_LONG:
			case G_TYPE_ULONG:
			case G_TYPE_INT64:
			case G_TYPE_UINT64:
				ret = json_node_get_int(node);
				break;
			case G_TYPE_STRING:
				ret = g_ascii_strtoll(json_node_get_string(node),
						      NULL, 10);
				break;
			default:
				break;
		}
		json_node_free(node);
	}

	return ret;
}

gdouble json_read_entry_double(const gchar *member, const gchar *json,
			       const gint64 index)
{
	gdouble ret = 0.0;
	JsonNode *node;
	GType node_type;

	if (_get_node_from_array(member, json, index, &node)) {
		node_type = json_node_get_value_type(node);

		switch (node_type) {
			case G_TYPE_INT:
			case G_TYPE_UINT:
			case G_TYPE_LONG:
			case G_TYPE_ULONG:
			case G_TYPE_INT64:
			case G_TYPE_UINT64:
				ret = json_node_get_int(node);
				break;
			case G_TYPE_FLOAT:
			case G_TYPE_DOUBLE:
				ret = json_node_get_double(node);
				break;
			case G_TYPE_STRING:
				ret = g_ascii_strtod(json_node_get_string(node),
						     NULL);
				break;
			default:
				break;
		}
		json_node_free(node);
	}

	return ret;
}

gfloat json_read_entry_float(const gchar *member, const gchar *json,
			     const gint64 index)
{
	return (gfloat)json_read_entry_double(member, json, index);
}

gchar *json_read_entry_string(const gchar *member, const gchar *json,
			      const gint64 index)
{
	gchar *ret = NULL;
	JsonNode *node;
	GType node_type;

	if (_get_node_from_array(member, json, index, &node)) {
		node_type = json_node_get_value_type(node);

		switch (node_type) {
			case G_TYPE_INT:
			case G_TYPE_UINT:
			case G_TYPE_LONG:
			case G_TYPE_ULONG:
			case G_TYPE_INT64:
			case G_TYPE_UINT64:
				ret = g_strdup_printf("%" G_GUINT64_FORMAT,
						      json_node_get_int(node));
				break;
			case G_TYPE_FLOAT:
			case G_TYPE_DOUBLE:
				ret = g_strdup_printf("%f",
						      json_node_get_double(node));
				break;
			case G_TYPE_STRING:
				ret = g_strdup(json_node_get_string(node));
				break;
			default:
				break;
		}
		json_node_free(node);
	}

	return ret;
}

gchar json_read_entry_char(const gchar *member, const gchar *json,
			   const gint64 index)
{
	gchar *ret = NULL;
	JsonNode *node;
	GType node_type;

	if (_get_node_from_array(member, json, index, &node)) {
		node_type = json_node_get_value_type(node);

		switch (node_type) {
			case G_TYPE_INT:
			case G_TYPE_UINT:
			case G_TYPE_LONG:
			case G_TYPE_ULONG:
			case G_TYPE_INT64:
			case G_TYPE_UINT64:
				ret = g_strdup_printf("%" G_GUINT64_FORMAT,
						      json_node_get_int(node));
				break;
			case G_TYPE_FLOAT:
			case G_TYPE_DOUBLE:
				ret = g_strdup_printf("%f",
						      json_node_get_double(node));
				break;
			case G_TYPE_STRING:
				ret = g_strdup(json_node_get_string(node));
				break;
			default:
				break;
		}
		json_node_free(node);
	}

	return ret ? ret[0] : (gchar)'0';
}

gboolean save_json_file(const struct Config *config, gchar **error)
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
