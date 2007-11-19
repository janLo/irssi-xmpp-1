/*
 * $Id: loudmouth-tools.c,v 1.3 2007/11/19 13:14:26 cdidier Exp $
 *
 * Copyright (C) 2007 Colin DIDIER
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <string.h>
#include "loudmouth/loudmouth.h"

#include "module.h"
#include "settings.h"
#include "signals.h"

#include "xmpp-servers.h"
#include "xmpp-tools.h"

LmMessageNode *
lm_tools_message_node_find(LmMessageNode *node, const char *name,
    const char *attribute, const char *value)
{
	LmMessageNode *l;		
	const char *v;

	g_return_val_if_fail(name != NULL, NULL);
	g_return_val_if_fail(attribute != NULL, NULL);
	g_return_val_if_fail(value != NULL, NULL);

	if (node == NULL)
		return NULL;

	for (l = node->children; l != NULL; l = l->next)
		if (g_ascii_strcasecmp(l->name, name) == 0) {

			v = lm_message_node_get_attribute(l, attribute);
			if (v != NULL && strcmp(value, v) == 0)
				return l;
		}

	return NULL;
}

gboolean
lm_send(XMPP_SERVER_REC *server, LmMessage *message, GError **error)
{

	if (settings_get_bool("xmpp_raw_window")) {
		char *text =
		    xmpp_recode_in(lm_message_node_to_string(message->node));
		signal_emit("xmpp raw out", 2, server, text);
		g_free(text);
	}

	return lm_connection_send(server->lmconn, message, error);
}
