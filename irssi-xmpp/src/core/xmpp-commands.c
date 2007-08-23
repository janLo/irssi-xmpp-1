/*
 * $Id: xmpp-commands.c,v 1.10 2007/08/23 17:43:32 cdidier Exp $
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
#include "settings.h"

#include "xmpp-servers.h"
#include "xmpp-commands.h"
/*#include "xmpp-channels.h"*/
#include "xmpp-protocol.h"
#include "xmpp-rosters.h"

const gchar *xmpp_commands[] = {
    "away",
    "quote",
    "roster",
    "whois",
    NULL
};

const gchar *xmpp_command_roster[] = {
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

static void
cmd_away(const char *data, XMPP_SERVER_REC *server)
{
    gchar **str, **str_prefixed = NULL;
    const gchar *show, *reason;
    gboolean default_mode = FALSE;

    CMD_XMPP_SERVER(server);

    str = g_strsplit(data, " ", 2);

    if (data[0] == '\0')
         show = NULL;
    else if ((g_ascii_strcasecmp(str[0], "-one") != 0
            && g_ascii_strcasecmp(str[0], "-all") != 0)) {
        show = str[0];
        reason = str[1];
    } else if (str[1] != NULL) {
        str_prefixed = g_strsplit(str[1], " ", 2);
        show = str_prefixed[0];
        reason = str_prefixed[1];
    }

parse_cmd_away:

    if ((show == NULL) || (show[0] == '\0'))
        xmpp_set_presence(server, XMPP_PRESENCE_AVAILABLE, NULL);

    else if (g_ascii_strcasecmp(show,
            xmpp_presence_show[XMPP_PRESENCE_CHAT]) == 0)
        xmpp_set_presence(server, XMPP_PRESENCE_CHAT, reason);

    else if (g_ascii_strcasecmp(show,
            xmpp_presence_show[XMPP_PRESENCE_DND]) == 0)
        xmpp_set_presence(server, XMPP_PRESENCE_DND, reason);

    else if (g_ascii_strcasecmp(show,
            xmpp_presence_show[XMPP_PRESENCE_XA]) == 0)
        xmpp_set_presence(server, XMPP_PRESENCE_XA, reason);

    else if (g_ascii_strcasecmp(show,
            xmpp_presence_show[XMPP_PRESENCE_AWAY]) == 0)
        xmpp_set_presence(server, XMPP_PRESENCE_AWAY, reason);

    else if (default_mode)
        xmpp_set_presence(server, XMPP_PRESENCE_AWAY, reason);

    else {
        reason = (str_prefixed != NULL) ? str[1] : data;
        show = settings_get_str("xmpp_default_away_mode");
        default_mode = TRUE;

        goto parse_cmd_away;
    }

    if (str_prefixed != NULL)
        g_strfreev(str_prefixed);
    g_strfreev(str);
}

static void
cmd_quote(const char *data, XMPP_SERVER_REC *server)
{
    CMD_XMPP_SERVER(server);

    lm_connection_send_raw(server->lmconn, data, NULL);
}



static void
cmd_roster_show_users(gpointer data, gpointer user_data)
{
    XMPP_SERVER_REC *server = XMPP_SERVER(user_data);
    XmppRosterUser *user = (XmppRosterUser *) data;

    if (xmpp_roster_show_user(user))
        signal_emit("xmpp roster nick", 2, server, user);
}

static void
cmd_roster_show_groups(gpointer data, gpointer user_data)
{
    XMPP_SERVER_REC *server = XMPP_SERVER(user_data);
    XmppRosterGroup *group = (XmppRosterGroup *) data;
    GSList *user_list;
    gboolean show_group = FALSE;

    user_list = group->users;
    while (!show_group && user_list)
    {
        if (xmpp_roster_show_user((XmppRosterUser *) user_list->data))
            show_group = TRUE;

        user_list = user_list->next;
    }

    if (show_group) {
        group->users = g_slist_sort(group->users,
            (GCompareFunc) xmpp_sort_user_func);

    	signal_emit("xmpp roster group", 2, server, group->name);

        g_slist_foreach(group->users, (GFunc) cmd_roster_show_users, server);
    }
}

static void
cmd_roster(const char *data, XMPP_SERVER_REC *server)
{
    LmMessage *msg;
    LmMessageNode *root, *query, *item;
    XmppRosterUser *user;
    XmppRosterGroup *group;
    gchar **str;
    gchar *name, *group_name, *status_recoded, *jid_recoded = NULL;
    gboolean empty_param = FALSE;
    GError *error = NULL;

    CMD_XMPP_SERVER(server);

    /* /ROSTER */
    if (data[0] == '\0') {
    	signal_emit("xmpp begin of roster", 1, server);
        g_slist_foreach(server->roster, (GFunc) cmd_roster_show_groups, server);
    	signal_emit("xmpp end of roster", 1, server);

        return;
    }

    str = g_strsplit(data, " ", 3);

    if (!str[1] || str[1][0] == '\0') {
        signal_emit("error command", 2,
            GINT_TO_POINTER(CMDERR_NOT_ENOUGH_PARAMS),
            xmpp_commands[XMPP_COMMAND_ROSTER]);
        goto cmd_roster_end;
    }

    if (!xmpp_jid_have_address(str[1])) {
        signal_emit("error command", 2,
            GINT_TO_POINTER(CMDERR_OPTION_UNKNOWN), str[1]);
        goto cmd_roster_end;
    }

    jid_recoded = xmpp_recode(str[1], XMPP_RECODE_OUT);

    /* /ROSTER ADD <jid> */
    if (g_ascii_strcasecmp(str[0],
            xmpp_command_roster[XMPP_COMMAND_ROSTER_PARAM_ADD]) == 0) {

        msg = lm_message_new_with_sub_type(NULL , LM_MESSAGE_TYPE_IQ,
                LM_MESSAGE_SUB_TYPE_SET);
        root = lm_message_get_node(msg);

        query = lm_message_node_add_child(root, "query", NULL);
        lm_message_node_set_attribute(query, "xmlns", "jabber:iq:roster");

        item = lm_message_node_add_child(query, "item", NULL);
        lm_message_node_set_attribute(item, "jid", jid_recoded);

        lm_connection_send(server->lmconn, msg, &error);
        lm_message_unref(msg);

        if (settings_get_bool("roster_add_send_subscribe")) {
            msg = lm_message_new_with_sub_type(str[1], LM_MESSAGE_TYPE_PRESENCE,
                LM_MESSAGE_SUB_TYPE_SUBSCRIBE);
            lm_connection_send(server->lmconn, msg, &error);
            lm_message_unref(msg);
        }

    /* /ROSTER REMOVE <jid> */
    } else if (g_ascii_strcasecmp(str[0],
            xmpp_command_roster[XMPP_COMMAND_ROSTER_PARAM_REMOVE]) == 0) {

        user = xmpp_find_user_from_groups(server->roster, str[1], NULL);
        if (!user) {
            signal_emit("xmpp not in roster", 2, server, str[1]);
            goto cmd_roster_end;
        }

        msg = lm_message_new_with_sub_type(NULL , LM_MESSAGE_TYPE_IQ,
                LM_MESSAGE_SUB_TYPE_SET);
        root = lm_message_get_node(msg);

        query = lm_message_node_add_child(root, "query", NULL);
        lm_message_node_set_attribute(query, "xmlns", "jabber:iq:roster");

        item = lm_message_node_add_child(query, "item", NULL);
        lm_message_node_set_attribute(item, "jid", jid_recoded);
        lm_message_node_set_attribute(item, "subscription", "remove");

        lm_connection_send(server->lmconn, msg, &error);
        lm_message_unref(msg);

    /* /ROSTER RENAME <jid> <name> */
    } else if (g_ascii_strcasecmp(str[0], 
            xmpp_command_roster[XMPP_COMMAND_ROSTER_PARAM_NAME]) == 0) {

        user = xmpp_find_user_from_groups(server->roster, str[1], &group);
        if (!user) {
            signal_emit("xmpp not in roster", 2, server, str[1]);
            goto cmd_roster_end;
        }

        if (!str[2] || str[2][0] == '\0')
            empty_param = TRUE;

        msg = lm_message_new_with_sub_type(NULL , LM_MESSAGE_TYPE_IQ,
                LM_MESSAGE_SUB_TYPE_SET);
        root = lm_message_get_node(msg);

        query = lm_message_node_add_child(root, "query", NULL);
        lm_message_node_set_attribute(query, "xmlns", "jabber:iq:roster");

        item = lm_message_node_add_child(query, "item", NULL);
        lm_message_node_set_attribute(item, "jid", jid_recoded);

        if (group->name) {
            group_name = xmpp_recode(group->name, XMPP_RECODE_OUT);
            lm_message_node_add_child(item, "group", group_name);
            g_free(group_name);
        }

        if (!empty_param) {
            name = xmpp_recode(str[2], XMPP_RECODE_OUT);
            lm_message_node_set_attribute(item, "name", name);
            g_free(name);
        }

        lm_connection_send(server->lmconn, msg, &error);
        lm_message_unref(msg);

    /* /ROSTER MOVE <jid> <group> */
    } else if (g_ascii_strcasecmp(str[0],
            xmpp_command_roster[XMPP_COMMAND_ROSTER_PARAM_GROUP]) == 0) {

        user = xmpp_find_user_from_groups(server->roster, str[1], NULL);
        if (!user) {
            signal_emit("xmpp not in roster", 2, server, str[1]);
            goto cmd_roster_end;
        }

        if (!str[2] || str[2][0] == '\0')
            empty_param = TRUE;

        msg = lm_message_new_with_sub_type(NULL , LM_MESSAGE_TYPE_IQ,
                LM_MESSAGE_SUB_TYPE_SET);
        root = lm_message_get_node(msg);

        query = lm_message_node_add_child(root, "query", NULL);
        lm_message_node_set_attribute(query, "xmlns", "jabber:iq:roster");

        item = lm_message_node_add_child(query, "item", NULL);
        lm_message_node_set_attribute(item, "jid", jid_recoded);

        if (!empty_param) {
            group_name = xmpp_recode(str[2], XMPP_RECODE_OUT);
            lm_message_node_add_child(item, "group", group_name);
            g_free(group_name);
        }

        if (user->name) {
            name = xmpp_recode(user->name, XMPP_RECODE_OUT);
            lm_message_node_set_attribute(item, "name", user->name);
            g_free(group_name);
        }

        lm_connection_send(server->lmconn, msg, &error);
        lm_message_unref(msg);

    /* /ROSTER ACCEPT <jid> */
    } else if (g_ascii_strcasecmp(str[0],
            xmpp_command_roster[XMPP_COMMAND_ROSTER_PARAM_ACCEPT]) == 0 ) {

        msg = lm_message_new_with_sub_type(jid_recoded,
            LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_SUBSCRIBED);
        lm_connection_send(server->lmconn, msg, &error);
        lm_message_unref(msg);

    /* /ROSTER DENY <jid> */
    } else if (g_ascii_strcasecmp(str[0],
            xmpp_command_roster[XMPP_COMMAND_ROSTER_PARAM_DENY]) == 0 ) {

        msg = lm_message_new_with_sub_type(jid_recoded,
            LM_MESSAGE_TYPE_PRESENCE,LM_MESSAGE_SUB_TYPE_UNSUBSCRIBED);
        lm_connection_send(server->lmconn, msg, &error);
        lm_message_unref(msg);

    /* /ROSTER SUBSCRIBE <jid> <status> */
    } else if (g_ascii_strcasecmp(str[0],
            xmpp_command_roster[XMPP_COMMAND_ROSTER_PARAM_SUBSCRIBE]) == 0 ) {

        msg = lm_message_new_with_sub_type(jid_recoded,
            LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_SUBSCRIBE);

        if (str[2]) {
            status_recoded = xmpp_recode(str[2], XMPP_RECODE_OUT);
            lm_message_node_add_child(lm_message_get_node(msg), "status",
                status_recoded);
            g_free(status_recoded);
        }

        lm_connection_send(server->lmconn, msg, &error);
        lm_message_unref(msg);

    /* /ROSTER UNSUBSCRIBE <jid> */
    } else if (g_ascii_strcasecmp(str[0],
            xmpp_command_roster[XMPP_COMMAND_ROSTER_PARAM_UNSUBSCRIBE]) == 0 ) {

        msg = lm_message_new_with_sub_type(jid_recoded,
            LM_MESSAGE_TYPE_PRESENCE, LM_MESSAGE_SUB_TYPE_UNSUBSCRIBE);
        lm_connection_send(server->lmconn, msg, &error);
        lm_message_unref(msg);

    } else
        signal_emit("error command", 2,
            GINT_TO_POINTER(CMDERR_OPTION_UNKNOWN), str[0]);

    if (error) {
        signal_emit("xmpp message error", 3, server, str[1], error->message);
        g_free(error);
    }

cmd_roster_end:
    g_free(jid_recoded);
    g_strfreev(str);
}

void
cmd_whois(const char *data, XMPP_SERVER_REC *server)
{
    LmMessage *msg;
    LmMessageNode *vcard_node;
    gchar *jid_recoded, *jid = NULL;

    CMD_XMPP_SERVER(server);

    if (data[0] == '\0') {
        signal_emit("error command", 2,
            GINT_TO_POINTER(CMDERR_NOT_ENOUGH_PARAMS),
            xmpp_commands[XMPP_COMMAND_WHOIS]);
        return;
    }

    if (!xmpp_jid_have_address(data)) {
        signal_emit("error command", 2,
            GINT_TO_POINTER(CMDERR_OPTION_UNKNOWN), data);
        return;
    }

    jid = xmpp_jid_strip_ressource(data);
    jid_recoded = xmpp_recode(jid, XMPP_RECODE_OUT);

    msg = lm_message_new_with_sub_type(jid_recoded, LM_MESSAGE_TYPE_IQ,
        LM_MESSAGE_SUB_TYPE_GET);
    vcard_node =lm_message_node_add_child(lm_message_get_node(msg), "vCard",
        NULL);
    lm_message_node_set_attribute(vcard_node, "xmlns", "vcard-temp");

    lm_connection_send(server->lmconn, msg, NULL);
    lm_message_unref(msg);

    g_free(jid);
    g_free(jid_recoded);
}

void
xmpp_commands_init(void)
{
    command_bind_xmpp("away", NULL, (SIGNAL_FUNC) cmd_away);
    command_bind_xmpp("quote", NULL, (SIGNAL_FUNC) cmd_quote);
    command_bind_xmpp("roster", NULL, (SIGNAL_FUNC) cmd_roster);
    command_bind_xmpp("whois", NULL, (SIGNAL_FUNC) cmd_whois);

    command_set_options("connect", "+xmppnet");
	command_set_options("server add", "-xmppnet");
}

void
xmpp_commands_deinit(void)
{
     command_unbind("away", (SIGNAL_FUNC) cmd_away);
     command_unbind("quote", (SIGNAL_FUNC) cmd_quote);
     command_unbind("roster", (SIGNAL_FUNC) cmd_roster);
     command_unbind("whois", (SIGNAL_FUNC) cmd_whois);
}
