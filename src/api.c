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

#include <glib.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include "api.h"
#include "version.h"

#define API_URL "https://api.aprs.fi/api/get?"

/**
 * @brief Memory structure for cURL
 */
struct MemoryStruct {
  char *memory; /**< Memory pointer */
  size_t size; /**< Memory size */
};

static size_t copy_to_memory(const void *contents, const size_t size, const size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
	return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

gboolean api_get_loc(const gchar *name, const gchar *api_key, gchar **data, gchar **error)
{
	struct MemoryStruct chunk;
	CURL *curl;
	CURLcode res;
	const gchar *user_agent = g_strconcat("shipsoftware-backend-schoolproject/", ShipSoftwareBackend_VERSION, " (+https://github.com/Shipsoftware-schoolproject/shipsoftware-backend)", NULL);

	chunk.memory = malloc(1);
	chunk.size = 0;

	curl = curl_easy_init();
	if (!curl) {
		free(chunk.memory);
		*(error) = g_strdup("cURL failed");
		return FALSE;
	}

	curl_easy_setopt(curl, CURLOPT_URL, g_strconcat(API_URL, "name=", name, "&what=loc&apikey=", api_key, NULL));
	curl_easy_setopt(curl, CURLOPT_USERAGENT, user_agent);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, copy_to_memory);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		*(error) = g_strconcat("API failed: " , curl_easy_strerror(res), NULL);
		return FALSE;
	} else {
		*(data) = g_malloc(chunk.size);
		*(data) = g_strdup(chunk.memory);
	}

	free(chunk.memory);

	curl_easy_cleanup(curl);

	return TRUE;
}
