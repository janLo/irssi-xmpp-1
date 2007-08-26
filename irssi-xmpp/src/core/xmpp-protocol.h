/* $Id: xmpp-protocol.h,v 1.8 2007/08/26 16:14:47 cdidier Exp $ */

#ifndef __XMPP_PROTOCOL_H
#define __XMPP_PROTOCOL_H

#define XMPP_PROTOCOL_LEVEL 1

#define XMPP_RECODE_IN 0
#define XMPP_RECODE_OUT 1

#define XMPP_PRIORITY_MIN -128
#define XMPP_PRIORITY_MAX 127

__BEGIN_DECLS
char   *xmpp_recode(const char *, const int);

char		*xmpp_jid_get_username(const char *);
char		*xmpp_jid_get_ressource(const char *);
char		*xmpp_jid_strip_ressource(const char *);
gboolean	 xmpp_jid_have_ressource(const char *);
gboolean	 xmpp_jid_have_address(const char *);
gboolean	 xmpp_priority_out_of_bound(const int);
gboolean	 xmpp_presence_changed(const int, const int, const char *,
		     const char *, const int, const int);

void	xmpp_send_message_chat(XMPP_SERVER_REC *, const char *,
	    const char *);
void	xmpp_set_presence(XMPP_SERVER_REC *, const int, const char *,
	    const int);
void	xmpp_register_handlers(XMPP_SERVER_REC *);

void	xmpp_protocol_init(void);
void	xmpp_protocol_deinit(void);
__END_DECLS

#endif
