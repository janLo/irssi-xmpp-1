/* $Id: xmpp-protocol.h,v 1.9 2007/08/28 18:40:28 cdidier Exp $ */

#ifndef __XMPP_PROTOCOL_H
#define __XMPP_PROTOCOL_H

#define XMPP_PROTOCOL_LEVEL 1

__BEGIN_DECLS
void	xmpp_send_message_chat(XMPP_SERVER_REC *, const char *,
	    const char *);
void	xmpp_set_presence(XMPP_SERVER_REC *, const int, const char *,
	    const int);
void	xmpp_register_handlers(XMPP_SERVER_REC *);

void	xmpp_protocol_init(void);
void	xmpp_protocol_deinit(void);
__END_DECLS

#endif
