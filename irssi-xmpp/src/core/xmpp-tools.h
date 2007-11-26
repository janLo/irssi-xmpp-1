/* $Id: xmpp-tools.h,v 1.4 2007/11/26 12:55:07 cdidier Exp $ */

#ifndef __XMPP_TOOLS_H
#define __XMPP_TOOLS_H

#define xmpp_extract_nick(jid)						\
	xmpp_extract_resource(jid)

#define xmpp_extract_channel(jid)					\
	xmpp_strip_resource(jid)

__BEGIN_DECLS
char	*xmpp_recode_out(const char *);
char	*xmpp_recode_in(const char *);

char	*xmpp_extract_resource(const char *);
char	*xmpp_strip_resource(const char *);
char	*xmpp_extract_user(const char *);
char	*xmpp_extract_host(const char *);
gboolean xmpp_have_host(const char *);
gboolean xmpp_have_resource(const char *);
gboolean xmpp_priority_out_of_bound(const int);
gboolean xmpp_presence_changed(const int, const int, const char *,
	     const char *, const int, const int);
__END_DECLS

#endif
