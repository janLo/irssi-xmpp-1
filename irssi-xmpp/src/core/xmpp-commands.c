/*
 * $Id: xmpp-commands.c,v 1.47 2008/08/19 02:56:35 cdidier Exp $
 *
 * Copyright (C) 2007 Colin DIDIER
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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

#include <stdlib.h>

#include "module.h"
#include "channels.h"
#include "recode.h"
#include "settings.h"
#include "signals.h"
#include "window-item-def.h"

#include "xmpp-commands.h"
#include "xmpp-queries.h"
#include "xmpp-servers.h"
#include "register.h"
#include "rosters-tools.h"
#include "tools.h"

const char *xmpp_commands[] = {
	"away",
	"quote",
	"roster",
	"whois",
	NULL
};

const char *xmpp_command_roster[] = {
	"add",
	"remove",
	"name",
	"group",
	"accept",
	"deny",
	"subscribe",
	"unsubscribe",
	NULL
};

static char *
cmd_connect_get_line(const char *data)
{
	GHashTable *optlist;
	const char *port;
	char *line, *jid, *password, *network, *network_free, *host, *host_free;
	void *free_arg;

	line = host_free = network_free = NULL;
	if (!cmd_get_params(data, &free_arg, 2 | PARAM_FLAG_OPTIONS,
	    "xmppconnect", &optlist, &jid, &password))
		return NULL;
	if (*jid == '\0' || *password == '\0' || !xmpp_have_host(jid)) {
		cmd_params_free(free_arg);
		signal_emit("error command", 1,
		    GINT_TO_POINTER(CMDERR_NOT_ENOUGH_PARAMS));
		signal_stop();
		return NULL;
	}
	network = g_hash_table_lookup(optlist, "network");
	if (network == NULL || *network == '\0') {
		char *stripped = xmpp_strip_resource(jid);
		network = network_free = g_strconcat("xmpp:", stripped, NULL);
		g_free(stripped);
	}
	host = g_hash_table_lookup(optlist, "host");
	if (host == NULL || *host == '\0')
		host = host_free = xmpp_extract_host(jid);
	port = g_hash_table_lookup(optlist, "port");
	if (port == NULL)
		port = "0";
	line = g_strdup_printf("%s-xmppnet \"%s\" %s %d \"%s\" \"%s\"",
	    (g_hash_table_lookup(optlist, "ssl") != NULL) ? "-ssl " : "",
	    network, host, atoi(port), password, jid);
	g_free(network_free);
	g_free(host_free);
	cmd_params_free(free_arg);
	return line;
}

/* SYNTAX: XMPPCONNECT [-ssl] [-host <server>] [-port <port>]
 *                     <jid>[/<resource>] <password> */
static void
cmd_xmppconnect(const char *data, SERVER_REC *server, WI_ITEM_REC *item)
{
	char *line, *cmd_line;

	if ((line = cmd_connect_get_line(data)) == NULL)
		return;
	cmd_line = g_strconcat(settings_get_str("cmdchars"), "CONNECT ",
	    line, NULL);
	g_free(line);
	signal_emit("send command", 3, cmd_line, server, item);
	g_free(cmd_line);
}

/* SYNTAX: XMPPSERVER [-ssl] [-host <server>] [-port <port>]
 *                    <jid>[/<resource>] <password> */
static void
cmd_xmppserver(const char *data, SERVER_REC *server, WI_ITEM_REC *item)
{
	char *line, *cmd_line;

	if ((line = cmd_connect_get_line(data)) == NULL)
		return;
	cmd_line = g_strconcat(settings_get_str("cmdchars"), "SERVER ",
	    line, NULL);
	g_free(line);
	signal_emit("send command", 3, cmd_line, server, item);
	g_free(cmd_line);
}

/* SYNTAX: XMPPREGISTER [-ssl] [-host <server>] [-port <port>]
 *                      <jid> <password> */
static void
cmd_xmppregister(const char *data, SERVER_REC *server, WI_ITEM_REC *item)
{
	GHashTable *optlist;
	char *str, *jid, *username, *password, *host, *address;
	int port;
	void *free_arg;

	if (!cmd_get_params(data, &free_arg, 2 | PARAM_FLAG_OPTIONS,
	    "xmppconnect", &optlist, &jid, &password))
		return;
	if (*jid == '\0' || *password == '\0' || !xmpp_have_host(jid))
		cmd_param_error(CMDERR_NOT_ENOUGH_PARAMS);
	username = xmpp_extract_user(jid);
	host = xmpp_extract_host(jid);
	address = g_hash_table_lookup(optlist, "host");
	if (address == NULL || *address == '\0')
		address = host;
	port = str = g_hash_table_lookup(optlist, "port") ? atoi(str) : 0;
	xmpp_register(address, port, g_hash_table_lookup(optlist, "ssl") != NULL,
	    username, host, password);
	g_free(username);
	g_free(host);
	cmd_params_free(free_arg);
}

/* SYNTAX: XMPPUNREGISTER -yes */
static void
cmd_xmppunregister(const char *data, SERVER_REC *server, WI_ITEM_REC *item)
{
	GHashTable *optlist;
	LmMessage *lmsg;
	LmMessageNode *node;
	void *free_arg;

	CMD_XMPP_SERVER(server);
	if (!cmd_get_params(data, &free_arg, 0 | PARAM_FLAG_OPTIONS, 
	    "xmppunregister", &optlist))
		return;
	if (g_hash_table_lookup(optlist, "yes") == NULL)
		cmd_param_error(CMDERR_NOT_GOOD_IDEA);
	lmsg = lm_message_new_with_sub_type(NULL,
	    LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_SET);
	node = lm_message_node_add_child(lmsg->node, "query", NULL);
	lm_message_node_set_attribute(node, "xmlns", "jabber:iq:register");
	lm_message_node_add_child(node, "remove", NULL);
	signal_emit("xmpp send iq", 2, server, lmsg);
	lm_message_unref(lmsg);
	cmd_params_free(free_arg);
}

/* SYNTAX: XMPPPASSWD -yes <old_password> <new_password> */
static void
cmd_xmpppasswd(const char *data, SERVER_REC *server, WI_ITEM_REC *item)
{
	CMD_XMPP_SERVER(server);
	/* TODO */
}

static void
set_away(XMPP_SERVER_REC *server, const char *data)
{
	char **tmp;
	const char *reason;
	int show;

	if (!IS_XMPP_SERVER(server))
		return;
	g_strstrip((char *)data);
	tmp = g_strsplit(data, " ", 2);
	if (*data == '\0') {
		show = XMPP_PRESENCE_AVAILABLE;
	} else {
		show = xmpp_get_show(tmp[0]);
		if (show == XMPP_PRESENCE_AVAILABLE && g_ascii_strcasecmp(
		    xmpp_presence_show[XMPP_PRESENCE_ONLINE], tmp[0]) != 0) {
			show = xmpp_get_show(
			    settings_get_str("xmpp_default_away_mode"));
			reason = data;
		} else
			reason = tmp[1];
	}
	signal_emit("xmpp set presence", 4, server, show, reason,
	    server->priority);
	g_strfreev(tmp);
}

/* SYNTAX: AWAY [-one | -all] [away|dnd|xa|chat] [<reason>] */
static void
cmd_away(const char *data, XMPP_SERVER_REC *server)
{
	GHashTable *optlist;
	char *reason;
	void *free_arg;

	CMD_XMPP_SERVER(server);
	if (!cmd_get_params(data, &free_arg, 1 | PARAM_FLAG_OPTIONS |
	    PARAM_FLAG_GETREST, "away", &optlist, &reason))
		return;
	if (g_hash_table_lookup(optlist, "one") != NULL)
		set_away(server, reason);
	else 
		g_slist_foreach(servers, (GFunc)set_away, reason);
	cmd_params_free(free_arg);
}

/* SYNTAX: QUOTE <raw_command> */
static void
cmd_quote(const char *data, XMPP_SERVER_REC *server)
{
	char *recoded;

	CMD_XMPP_SERVER(server);
	if (*data == '\0')
		cmd_return_error(CMDERR_NOT_ENOUGH_PARAMS);
	g_strstrip((char *)data);
	if (*data == '\0')
		cmd_return_error(CMDERR_NOT_ENOUGH_PARAMS);
	signal_emit("xmpp xml out", 2, server, data);
	recoded = xmpp_recode_out(data);
	lm_connection_send_raw(server->lmconn, recoded, NULL);
	g_free(recoded);
}

/* SYNTAX: ROSTER */
static void
cmd_roster(const char *data, XMPP_SERVER_REC *server, WI_ITEM_REC *item)
{
	CMD_XMPP_SERVER(server);
	if (*data == '\0')
		signal_emit("xmpp roster show", 1, server);
	else
		command_runsub(xmpp_commands[XMPP_COMMAND_ROSTER], data,
		    server, item);
}

/* SYNTAX: ROSTER FULL */
static void
cmd_roster_full(const char *data, XMPP_SERVER_REC *server, WI_ITEM_REC *item)
{
	gboolean oldvalue;

	CMD_XMPP_SERVER(server);
	oldvalue = settings_get_bool("roster_show_offline");
	if (!oldvalue)
		settings_set_bool("roster_show_offline", TRUE);
	signal_emit("xmpp roster show", 1, server);
	if (!oldvalue)
		settings_set_bool("roster_show_offline", oldvalue);
}

/* SYNTAX: ROSTER ADD <jid> */
static void
cmd_roster_add(const char *data, XMPP_SERVER_REC *server)
{
	LmMessage *lmsg;
	LmMessageNode *query_node, *item_node;
	const char *jid;
	char *jid_recoded;
	void *free_arg;

	CMD_XMPP_SERVER(server);
	if (!cmd_get_params(data, &free_arg, 1, &jid))
		return;
	if (*jid == '\0') 
		cmd_param_error(CMDERR_NOT_ENOUGH_PARAMS);
	lmsg = lm_message_new_with_sub_type(NULL,
	    LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_SET);
	query_node = lm_message_node_add_child(lmsg->node, "query", NULL);
	lm_message_node_set_attribute(query_node, "xmlns", "jabber:iq:roster");
	jid_recoded = xmpp_recode_out(jid);
	item_node = lm_message_node_add_child(query_node, "item", NULL);
	lm_message_node_set_attribute(item_node, "jid", jid_recoded);
	signal_emit("xmpp send iq", 2, server, lmsg);
	lm_message_unref(lmsg);
	if (settings_get_bool("roster_add_send_subscribe")) {
		lmsg = lm_message_new_with_sub_type(jid_recoded,
		    LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_SUBSCRIBE);
		signal_emit("xmpp send presence", 2, server, lmsg);
		lm_message_unref(lmsg);
	}
	g_free(jid_recoded);
	cmd_params_free(free_arg);
}


/* SYNTAX: ROSTER REMOVE <jid> */
static void
cmd_roster_remove(const char *data, XMPP_SERVER_REC *server)
{
	LmMessage *lmsg;
	LmMessageNode *query_node, *item_node;
	XMPP_ROSTER_USER_REC *user;
	const char *jid;
	char *recoded;
	void *free_arg;

	CMD_XMPP_SERVER(server);
	if (!cmd_get_params(data, &free_arg, 1, &jid))
		return;
	if (*jid == '\0')
		cmd_param_error(CMDERR_NOT_ENOUGH_PARAMS);
	user = rosters_find_user(server->roster, jid, NULL, NULL);
	if (user == NULL) {
		signal_emit("xmpp not in roster", 2, server, jid);
		goto out;
	}
	lmsg = lm_message_new_with_sub_type(NULL,
	    LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_SET);
	query_node = lm_message_node_add_child(lmsg->node, "query", NULL);
	lm_message_node_set_attribute(query_node, "xmlns", "jabber:iq:roster");
	item_node = lm_message_node_add_child(query_node, "item", NULL);
	recoded = xmpp_recode_out(jid);
	lm_message_node_set_attribute(item_node, "jid", recoded);
	g_free(recoded);
	lm_message_node_set_attribute(item_node, "subscription", "remove");
	signal_emit("xmpp send iq", 2, server, lmsg);
	lm_message_unref(lmsg);

out:
	cmd_params_free(free_arg);
}

/* SYNTAX: ROSTER NAME <jid> [<name>] */
static void
cmd_roster_name(const char *data, XMPP_SERVER_REC *server)
{
	LmMessage *lmsg;
	LmMessageNode *query_node, *item_node;
	XMPP_ROSTER_USER_REC *user;
	XMPP_ROSTER_GROUP_REC *group;
	const char *jid, *name;
	char *recoded;
	void *free_arg;

	CMD_XMPP_SERVER(server);
	if (!cmd_get_params(data, &free_arg, 2 | PARAM_FLAG_GETREST, &jid,
	    &name))
		return;
	if (*jid == '\0')
		cmd_param_error(CMDERR_NOT_ENOUGH_PARAMS);
	user = rosters_find_user(server->roster, jid, &group, NULL);
	if (user == NULL) {
		signal_emit("xmpp not in roster", 2, server, jid);
		goto out;
	}
	lmsg = lm_message_new_with_sub_type(NULL,
	    LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_SET);
	query_node = lm_message_node_add_child(lmsg->node, "query", NULL);
	lm_message_node_set_attribute(query_node, "xmlns", "jabber:iq:roster");
	item_node = lm_message_node_add_child(query_node, "item", NULL);
	recoded = xmpp_recode_out(jid);
	lm_message_node_set_attribute(item_node, "jid", recoded);
	g_free(recoded);
	if (group->name != NULL) {
		recoded = xmpp_recode_out(group->name);
		lm_message_node_add_child(item_node, "group", recoded);
		g_free(recoded);
	}
	if (*name != '\0') {
		recoded = xmpp_recode_out(name);
		lm_message_node_set_attribute(item_node, "name", recoded);
		g_free(recoded);
	}
	signal_emit("xmpp send iq", 2, server, lmsg);
	lm_message_unref(lmsg);

out:
	cmd_params_free(free_arg);
}

/* SYNTAX: ROSTER GROUP <jid> [<group>] */
static void
cmd_roster_group(const char *data, XMPP_SERVER_REC *server)
{
	LmMessage *lmsg;
	LmMessageNode *query_node, *item_node;
	XMPP_ROSTER_USER_REC *user;
	XMPP_ROSTER_GROUP_REC *group;
	const char *jid, *group_name;
	char *recoded;
	void *free_arg;

	CMD_XMPP_SERVER(server);
	if (!cmd_get_params(data, &free_arg, 2 | PARAM_FLAG_GETREST, &jid,
	    &group_name))
		return;
	if (*jid == '\0')
		cmd_param_error(CMDERR_NOT_ENOUGH_PARAMS);
	user = rosters_find_user(server->roster, jid, &group, NULL);
	if (user == NULL) {
		signal_emit("xmpp not in roster", 2, server, jid);
		goto out;
	}
	lmsg = lm_message_new_with_sub_type(NULL,
	    LM_MESSAGE_TYPE_IQ, LM_MESSAGE_SUB_TYPE_SET);
	query_node = lm_message_node_add_child(lmsg->node, "query", NULL);
	lm_message_node_set_attribute(query_node, "xmlns", "jabber:iq:roster");
	item_node = lm_message_node_add_child(query_node, "item", NULL);
	recoded = xmpp_recode_out(jid);
	lm_message_node_set_attribute(item_node, "jid", recoded);
	g_free(recoded);
	if (*group_name != '\0') {
		recoded = xmpp_recode_out(group_name);
		lm_message_node_add_child(item_node, "group", recoded);
		g_free(recoded);
	}
	if (user->name != NULL) {
		recoded = xmpp_recode_out(user->name);
		lm_message_node_set_attribute(item_node, "name", recoded);
		g_free(recoded);
	}
	signal_emit("xmpp send iq", 2, server, lmsg);
	lm_message_unref(lmsg);

out:
	cmd_params_free(free_arg);
}

/* SYNTAX: ROSTER ACCEPT <jid> */
static void
cmd_roster_accept(const char *data, XMPP_SERVER_REC *server)
{
	LmMessage *lmsg;
	const char *jid;
	char *recoded;
	void *free_arg;

	CMD_XMPP_SERVER(server);
	if (!cmd_get_params(data, &free_arg, 1, &jid))
		return;
	if (*jid == '\0')
		cmd_param_error(CMDERR_NOT_ENOUGH_PARAMS);
	recoded = xmpp_recode_out(jid);
	lmsg = lm_message_new_with_sub_type(recoded,
	    LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_SUBSCRIBED);
	g_free(recoded);
	signal_emit("xmpp send presence", 2, server, lmsg);	
	lm_message_unref(lmsg);
	cmd_params_free(free_arg);
}

/* SYNTAX: ROSTER DENY <jid> */
static void
cmd_roster_deny(const char *data, XMPP_SERVER_REC *server)
{
	LmMessage *lmsg;
	const char *jid;
	char *recoded;
	void *free_arg;

	CMD_XMPP_SERVER(server);
	if (!cmd_get_params(data, &free_arg, 1, &jid))
		return;
	if (*jid == '\0')
		cmd_param_error(CMDERR_NOT_ENOUGH_PARAMS);
	recoded = xmpp_recode_out(jid);
	lmsg = lm_message_new_with_sub_type(recoded,
	    LM_MESSAGE_TYPE_PRESENCE,LM_MESSAGE_SUB_TYPE_UNSUBSCRIBED);
	g_free(recoded);
	signal_emit("xmpp send presence", 2, server, lmsg);
	lm_message_unref(lmsg);
	cmd_params_free(free_arg);
}

/* SYNTAX: ROSTER SUBSCRIBE <jid> [<reason>] */
static void
cmd_roster_subscribe(const char *data, XMPP_SERVER_REC *server)
{
	LmMessage *lmsg;
	const char *jid, *reason;
	char *recoded;
	void *free_arg;

	CMD_XMPP_SERVER(server);
	if (!cmd_get_params(data, &free_arg, 2 | PARAM_FLAG_GETREST, &jid,
	    &reason))
		return;
	if (*jid == '\0')
		cmd_param_error(CMDERR_NOT_ENOUGH_PARAMS);
	recoded = xmpp_recode_out(jid);
	lmsg = lm_message_new_with_sub_type(recoded,
	    LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_SUBSCRIBE);
	g_free(recoded);
	if (*reason != '\0') {
		recoded = xmpp_recode_out(reason);
		lm_message_node_add_child(lmsg->node, "status", recoded);
		g_free(recoded);
	}
	signal_emit("xmpp send presence", 2, server, lmsg);
	lm_message_unref(lmsg);
	cmd_params_free(free_arg);
}

/* SYNTAX: ROSTER UNSUBSCRIBE <jid> */
static void
cmd_roster_unsubscribe(const char *data, XMPP_SERVER_REC *server)
{
	LmMessage *lmsg;
	const char *jid;
	char *recoded;
	void *free_arg;

	CMD_XMPP_SERVER(server);
	if (!cmd_get_params(data, &free_arg, 1, &jid))
		return;
	if (*jid == '\0')
		cmd_param_error(CMDERR_NOT_ENOUGH_PARAMS);
	recoded = xmpp_recode_out(jid);
	lmsg = lm_message_new_with_sub_type(recoded,
	    LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_UNSUBSCRIBE);
	g_free(recoded);
	signal_emit("xmpp send presence", 2, server, lmsg);
	lm_message_unref(lmsg);
	cmd_params_free(free_arg);
}

/* SYNTAX: ME <message> */
static void
cmd_me(const char *data, XMPP_SERVER_REC *server, WI_ITEM_REC *item)
{
	const char *target;
	char *str, *recoded;
	int type;

	CMD_XMPP_SERVER(server);
	if (*data == '\0')
		return;
	g_strstrip((char *)data);
	if (*data == '\0')
		return;
	target = window_item_get_target(item);
	type = IS_CHANNEL(item) ? SEND_TARGET_CHANNEL : SEND_TARGET_NICK;
	if (type == SEND_TARGET_NICK)
		signal_emit("message xmpp own_action", 4, server, data, target,
		    SEND_TARGET_NICK);
	str = g_strconcat("/me ", data, NULL);
	recoded = recode_out(SERVER(server), str, target);
	g_free(str);
	server->send_message(SERVER(server), target, recoded, type);
	g_free(recoded);
}

void
xmpp_commands_init(void)
{
	command_bind("xmppconnect", NULL, (SIGNAL_FUNC)cmd_xmppconnect);
	command_bind("xmppserver", NULL, (SIGNAL_FUNC)cmd_xmppserver);
	command_bind("xmppregister", NULL, (SIGNAL_FUNC)cmd_xmppregister);
	command_bind("xmppunregister", NULL, (SIGNAL_FUNC)cmd_xmppunregister);
	command_bind("xmpppasswd", NULL, (SIGNAL_FUNC)cmd_xmpppasswd);
	command_bind_xmpp("away", NULL, (SIGNAL_FUNC)cmd_away);
	command_bind_xmpp("quote", NULL, (SIGNAL_FUNC)cmd_quote);
	command_bind_xmpp("roster", NULL, (SIGNAL_FUNC)cmd_roster);
	command_bind_xmpp("roster full", NULL, (SIGNAL_FUNC)cmd_roster_full);
	command_bind_xmpp("roster add", NULL, (SIGNAL_FUNC)cmd_roster_add);
	command_bind_xmpp("roster remove", NULL,
	    (SIGNAL_FUNC)cmd_roster_remove);
	command_bind_xmpp("roster name", NULL, (SIGNAL_FUNC)cmd_roster_name);
	command_bind_xmpp("roster group", NULL, (SIGNAL_FUNC)cmd_roster_group);
	command_bind_xmpp("roster accept", NULL,
	    (SIGNAL_FUNC)cmd_roster_accept);
	command_bind_xmpp("roster deny", NULL,
	    (SIGNAL_FUNC)cmd_roster_deny);
	command_bind_xmpp("roster subscribe", NULL,
	    (SIGNAL_FUNC)cmd_roster_subscribe);
	command_bind_xmpp("roster unsubscribe", NULL,
	    (SIGNAL_FUNC)cmd_roster_unsubscribe);
	command_bind_xmpp("me", NULL, (SIGNAL_FUNC)cmd_me);

	command_set_options("connect", "+xmppnet");
	command_set_options("server add", "-xmppnet");
	command_set_options("xmppconnect", "ssl -network -host @port");
	command_set_options("xmppunregister", "yes");

	settings_add_str("xmpp", "xmpp_default_away_mode", "away");
	settings_add_bool("xmpp_roster", "roster_add_send_subscribe", TRUE);
}

void
xmpp_commands_deinit(void)
{
	command_unbind("xmppconnect", (SIGNAL_FUNC)cmd_xmppconnect);
	command_unbind("xmppserver", (SIGNAL_FUNC)cmd_xmppserver);
	command_unbind("xmppregister", (SIGNAL_FUNC)cmd_xmppregister);
	command_unbind("xmppunregister", (SIGNAL_FUNC)cmd_xmppunregister);
	command_unbind("xmpppasswd", (SIGNAL_FUNC)cmd_xmpppasswd);
	command_unbind("away", (SIGNAL_FUNC)cmd_away);
	command_unbind("quote", (SIGNAL_FUNC)cmd_quote);
	command_unbind("roster", (SIGNAL_FUNC)cmd_roster);
	command_unbind("roster full", (SIGNAL_FUNC)cmd_roster_full);
	command_unbind("roster add", (SIGNAL_FUNC)cmd_roster_add);
	command_unbind("roster remove", (SIGNAL_FUNC)cmd_roster_remove);
	command_unbind("roster name", (SIGNAL_FUNC)cmd_roster_name);
	command_unbind("roster group", (SIGNAL_FUNC)cmd_roster_group);
	command_unbind("roster accept", (SIGNAL_FUNC)cmd_roster_accept);
	command_unbind("roster deny", (SIGNAL_FUNC)cmd_roster_deny);
	command_unbind("roster subscribe", (SIGNAL_FUNC)cmd_roster_subscribe);
	command_unbind("roster unsubscribe",
	    (SIGNAL_FUNC)cmd_roster_unsubscribe);
	command_unbind("me", (SIGNAL_FUNC)cmd_me);
}
