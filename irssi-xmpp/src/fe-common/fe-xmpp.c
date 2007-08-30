/*
 * $Id: fe-xmpp.c,v 1.10 2007/08/30 17:21:44 cdidier Exp $
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

#include "module.h"
#include "module-formats.h"
#include "signals.h"
#include "servers-setup.h"
#include "levels.h"
#include "printtext.h"
#include "themes.h"

#include "xmpp-servers.h"

#include "xmpp-completion.h"
#include "fe-xmpp-composing.h"
#include "fe-xmpp-rosters.h"
#include "fe-xmpp-queries.h"

/*static void
sig_print_error(XMPP_SERVER_REC *server, const char *msg)
{
	printformat_module(MODULE_NAME, server, NULL, MSGLEVEL_CRAP,
	    XMPPTXT_DEFAULT_ERROR, NULL, msg, NULL);
}*/

static void
event_server_status(XMPP_SERVER_REC *server, const char *msg)
{
	printformat_module(MODULE_NAME, server, NULL, MSGLEVEL_CRAP,
	    XMPPTXT_DEFAULT_EVENT, NULL, msg, NULL);
}

static void
event_message_error(XMPP_SERVER_REC *server, const char *jid)
{
	printformat_module(MODULE_NAME, server, NULL, MSGLEVEL_CRAP,
	    XMPPTXT_MESSAGE_NOT_DELIVERED, jid);
}

#ifdef DEBUG
static void
event_debug(XMPP_SERVER_REC *server, const char *msg)
{
	printtext(server, NULL, MSGLEVEL_CRAP, "%s", msg);
}
#endif

static void
sig_server_add_fill(SERVER_SETUP_REC *rec, GHashTable *optlist)
{
	char *value;

	value = g_hash_table_lookup(optlist, "xmppnet");
	if (value != NULL) {
		g_free_and_null(rec->chatnet);
		if (*value != '\0')
		    rec->chatnet = g_strdup(value);
	}
}

void
fe_xmpp_init(void)
{
	theme_register(fecommon_xmpp_formats);

	signal_add("xmpp message error", (SIGNAL_FUNC)event_message_error);
	signal_add("xmpp server status", (SIGNAL_FUNC)event_server_status);
	signal_add("server add fill", (SIGNAL_FUNC)sig_server_add_fill);
#ifdef DEBUG
	 signal_add("xmpp debug", (SIGNAL_FUNC)event_debug);
#endif

	/*fe_xmpp_composing_init();*/
	fe_xmpp_rosters_init();
	fe_xmpp_queries_init();
	xmpp_completion_init();

	module_register("xmpp", "fe");
}

void
fe_xmpp_deinit(void)
{
	signal_remove("xmpp message error", (SIGNAL_FUNC)event_message_error);
	signal_remove("xmpp server status", (SIGNAL_FUNC)event_server_status);
	signal_remove("server add fill", (SIGNAL_FUNC)sig_server_add_fill);
#ifdef DEBUG
	signal_remove("xmpp debug", (SIGNAL_FUNC)event_debug);
#endif

	/*fe_xmpp_composing_deinit();*/
	fe_xmpp_rosters_deinit();
	fe_xmpp_queries_deinit();
	xmpp_completion_deinit();

	theme_unregister();
}
