/*
 * Copyright © 2013 Guillaume Friloux <kuri@efl.so>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * @file send.c
 * @brief Contains functions to send logs to ES
 * @author Guillaume Friloux <kuri@efl.so>
 * @version 1.0
 *
 * Contains functions to send logs to ES
 */

#include "send.h"

/**
 * @fn int send_logmessage(struct logmessage *new_logmessage)
 * @brief This function will receive a logmessage to index,
 *  convert it into JSON data and send it to <a href=www.elasticsearch.com>ES</a>
 *
 * @param new_logmessage (struct logmessage *) Log message to index
 *
 * @see send_convJSON()
 * @see struct logmessage
 * @see send_toES
 *
 * @return 0
 */
int send_logmessage(struct logmessage *new_logmessage)
{
	Eina_List *l;
	char *tag      = NULL,
	     *tags     = NULL,
	     *jsondata = NULL;
	unsigned int size = 0,
	             i    = 0;

	// We check how much space we need to store tags
	EINA_LIST_FOREACH(new_logmessage->list_tags, l, tag)
	{
		size += strlen(tag) + 3;
	}

	tags = malloc( sizeof(char) * ( size + 1 ) );
	memset(tags, 0, size + 1);

	EINA_LIST_FOREACH(new_logmessage->list_tags, l, tag)
	{
		if( i > 0 )
			strcat(tags, ",");

		strcat(tags, "\"");
		strcat(tags, tag);
		strcat(tags, "\"");
		i++;
	}
	// We will build our JSON object from these values
	send_convJSON(new_logmessage->source_host, new_logmessage->source_path, new_logmessage->type, new_logmessage->message, tags, new_logmessage->timestamp, &jsondata);

	EINA_LOG_DOM_DBG(einadom_send, "JSON : %s", jsondata);

	send_toES(jsondata);

	free(jsondata);
	free(tags);
	return(0);
}

/**
 * @fn int send_convJSON(char *source_host, char *source_path, char *type, char *message, char *tags, char *timestamp, char **jsondata)
 * @brief This Function receives all needed vars to build the JSON data for <a href=www.elasticsearch.com>ES</a>.
 * @warning This function will malloc jsondata, so dont do it yourself, and dont forget to free it!
 *
 * @param source_host (char *) Host that sent the logfile
 * @param source_path (char *) Logfile name
 * @param type (char *) Type for this log message
 * @param message (char *) Log message
 * @param tags (char *) list of tags, they must be already parsed in format "Tag1","Tag2","Tag3"
 * @param timestamp (char *) Timestamp for the message
 * @param jsondata (char **) Buffer that we will malloc and fill with generated JSON message
 *
 * @return 0
 */
int send_convJSON(char *source_host, char *source_path, char *type, char *message, char *tags, char *timestamp, char **jsondata)
{
	char *Esource_host = NULL,
	     *Esource_path = NULL,
	     *Etype        = NULL,
	     *Emessage     = NULL;

	send_escape(source_host, &Esource_host);
	send_escape(source_path, &Esource_path);
	send_escape(type, &Etype);
	send_escape(message, &Emessage);

	*jsondata = malloc( sizeof(char) * (200 + strlen(Esource_host)*2 + strlen(Esource_path)*2 + strlen(Etype) + strlen(Emessage) + strlen(tags) + 30 + 1) );
	sprintf(*jsondata,
	        "{"
	        "	\"@source\" : \"file://%s%s\","
	        "	\"@type\" : \"%s\","
	        "	\"@tags\" : [ %s ],"
	        "	\"@fields\" : { },"
	        "	\"@message\" : \"%s\","
	        "	\"@timestamp\" : \"%s\","
	        "	\"@source_host\" : \"%s\","
	        "	\"@source_path\" : \"%s\""
	        "}",
	        Esource_host, Esource_path, Etype, tags, Emessage, timestamp, Esource_host, Esource_path);

	free(Esource_host);
	free(Esource_path);
	free(Etype);
	free(Emessage);

	return(0);
}


/**
 * @fn char *send_escape(char *src, char **dst)
 * @brief This Function will escape all double quote so we dont get a parsing problem
 * @warning This function will malloc dst, so dont do it yourself, and dont forget to free it!
 *
 * @param src (char *) buffer to escape
 * @param dst (char **) Escaped buffer
 *
 * @return Pointer to escaped buffer
 */
char *send_escape(char *src, char **dst)
{
	unsigned i = 0,
	         j = 0,
	         len = strlen(src);
	char tmp[len*2+1];

	memset(tmp, 0, sizeof(tmp));

	for( i = 0; i < len+1; i++)
	{
		if( src[i] == '"')
			tmp[j++] = '\\';
		tmp[j++] = src[i];
	}
	*dst = malloc( sizeof(char) * ( strlen(tmp) + 1 ) );
	strcpy(*dst, tmp);
	return(*dst);
}

/**
 * @fn int send_destroy(void)
 * @brief Inits the curl object, and set necessary params
 *
 * @return 0
 */
int send_init(void)
{
	curl = curl_easy_init();
	if( !curl )
	{
		EINA_LOG_DOM_ERR(einadom_send, "Can't init curl.");
		return(-1);
	}
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, EINA_TRUE);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, send_fromES);
	curl_easy_setopt(curl, CURLOPT_URL, global_ESserver);
	send_connected = EINA_TRUE;

	return(0);
}

/**
 * @fn int send_destroy(void)
 * @brief Destroys the curl object
 *
 * @return 0
 */
int send_destroy(void)
{
	curl_easy_cleanup(curl);
	return(0);
}

/**
 * @fn int send_toES(char *jsondata)
 * @brief This function will send a given JSON data to the configured
 *  JSON server
 *
 * @param jsondata (char *)
 *
 * @return 0
 */
int send_toES(char *jsondata)
{
/*  ecore_con_url_post doesnt return any error if it cant send
	which sucks for us. Waiting for patch coming for EFLs 1.1
	Ecore_Con_Url *ecu;
	Eina_Bool ret;

	ecore_con_url_init();
	ecu = ecore_con_url_new(global_ESserver);
	if( !ecu )
	{
		EINA_LOG_DOM_ERR(einadom_send, "Can't init ecore_con_url_new()");
		ecore_con_url_shutdown();
		return(-1);
	}

	ret = ecore_con_url_post(ecu, jsondata, -1, "text/xml");
	if( ret == EINA_FALSE )
	{
		EINA_LOG_DOM_ERR(einadom_send, "Can't send jsondata to ES");
		ecore_con_url_shutdown();
		return(-1);
	}

	ecore_con_url_free(ecu);
	ecore_con_url_shutdown();
*/
	if( send_connected == EINA_FALSE )
	{
		send_init();
	}

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsondata);
	res = curl_easy_perform(curl);
	if( res )
	{
		EINA_LOG_DOM_ERR(einadom_send, "Sending of JSON query failed, curl returned : %d", res);
		send_destroy();
		send_connected = EINA_FALSE;
	}

	return(0);
}

/**
 * @fn static size_t send_fromES(void *ptr, size_t size, size_t nmemb, void *data)
 * @brief This fonction will get response from <a href=www.elasticsearch.com>ES</A>
 *  after sending our JSON data. This function is a callback used by libcurl
 *
 * @param ptr (void *)  To be defined
 * @param size (size_t) To be defined
 * @param nmemb (size_t) To be defined
 * @param data (void *) To be defined
 *
 * @return Number of bytes received
 */
size_t send_fromES(void *ptr, size_t size, size_t nmemb, void *data)
{
	if( strncmp((char *)ptr, "{\"ok\":", 6) )
		EINA_LOG_DOM_ERR(einadom_send, "Indexing of JSON data failed");
	return(size * nmemb);
}
