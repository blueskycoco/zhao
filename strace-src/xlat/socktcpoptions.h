/* Generated by ./xlat/gen.sh from ./xlat/socktcpoptions.in; do not edit. */

#ifdef IN_MPERS

# error static const struct xlat socktcpoptions in mpers mode

#else

static
const struct xlat socktcpoptions[] = {
#if defined(TCP_NODELAY) || (defined(HAVE_DECL_TCP_NODELAY) && HAVE_DECL_TCP_NODELAY)
  XLAT(TCP_NODELAY),
#endif
#if defined(TCP_MAXSEG) || (defined(HAVE_DECL_TCP_MAXSEG) && HAVE_DECL_TCP_MAXSEG)
  XLAT(TCP_MAXSEG),
#endif
#if defined(TCP_CORK) || (defined(HAVE_DECL_TCP_CORK) && HAVE_DECL_TCP_CORK)
  XLAT(TCP_CORK),
#endif
#if defined(TCP_KEEPIDLE) || (defined(HAVE_DECL_TCP_KEEPIDLE) && HAVE_DECL_TCP_KEEPIDLE)
  XLAT(TCP_KEEPIDLE),
#endif
#if defined(TCP_KEEPINTVL) || (defined(HAVE_DECL_TCP_KEEPINTVL) && HAVE_DECL_TCP_KEEPINTVL)
  XLAT(TCP_KEEPINTVL),
#endif
#if defined(TCP_KEEPCNT) || (defined(HAVE_DECL_TCP_KEEPCNT) && HAVE_DECL_TCP_KEEPCNT)
  XLAT(TCP_KEEPCNT),
#endif
#if defined(TCP_SYNCNT) || (defined(HAVE_DECL_TCP_SYNCNT) && HAVE_DECL_TCP_SYNCNT)
  XLAT(TCP_SYNCNT),
#endif
#if defined(TCP_LINGER2) || (defined(HAVE_DECL_TCP_LINGER2) && HAVE_DECL_TCP_LINGER2)
  XLAT(TCP_LINGER2),
#endif
#if defined(TCP_DEFER_ACCEPT) || (defined(HAVE_DECL_TCP_DEFER_ACCEPT) && HAVE_DECL_TCP_DEFER_ACCEPT)
  XLAT(TCP_DEFER_ACCEPT),
#endif
#if defined(TCP_WINDOW_CLAMP) || (defined(HAVE_DECL_TCP_WINDOW_CLAMP) && HAVE_DECL_TCP_WINDOW_CLAMP)
  XLAT(TCP_WINDOW_CLAMP),
#endif
#if defined(TCP_INFO) || (defined(HAVE_DECL_TCP_INFO) && HAVE_DECL_TCP_INFO)
  XLAT(TCP_INFO),
#endif
#if defined(TCP_QUICKACK) || (defined(HAVE_DECL_TCP_QUICKACK) && HAVE_DECL_TCP_QUICKACK)
  XLAT(TCP_QUICKACK),
#endif
#if defined(TCP_CONGESTION) || (defined(HAVE_DECL_TCP_CONGESTION) && HAVE_DECL_TCP_CONGESTION)
  XLAT(TCP_CONGESTION),
#endif
#if defined(TCP_MD5SIG) || (defined(HAVE_DECL_TCP_MD5SIG) && HAVE_DECL_TCP_MD5SIG)
  XLAT(TCP_MD5SIG),
#endif
#if defined(TCP_COOKIE_TRANSACTIONS) || (defined(HAVE_DECL_TCP_COOKIE_TRANSACTIONS) && HAVE_DECL_TCP_COOKIE_TRANSACTIONS)
  XLAT(TCP_COOKIE_TRANSACTIONS),
#endif
#if defined(TCP_THIN_LINEAR_TIMEOUTS) || (defined(HAVE_DECL_TCP_THIN_LINEAR_TIMEOUTS) && HAVE_DECL_TCP_THIN_LINEAR_TIMEOUTS)
  XLAT(TCP_THIN_LINEAR_TIMEOUTS),
#endif
#if defined(TCP_THIN_DUPACK) || (defined(HAVE_DECL_TCP_THIN_DUPACK) && HAVE_DECL_TCP_THIN_DUPACK)
  XLAT(TCP_THIN_DUPACK),
#endif
#if defined(TCP_USER_TIMEOUT) || (defined(HAVE_DECL_TCP_USER_TIMEOUT) && HAVE_DECL_TCP_USER_TIMEOUT)
  XLAT(TCP_USER_TIMEOUT),
#endif
#if defined(TCP_REPAIR) || (defined(HAVE_DECL_TCP_REPAIR) && HAVE_DECL_TCP_REPAIR)
  XLAT(TCP_REPAIR),
#endif
#if defined(TCP_REPAIR_QUEUE) || (defined(HAVE_DECL_TCP_REPAIR_QUEUE) && HAVE_DECL_TCP_REPAIR_QUEUE)
  XLAT(TCP_REPAIR_QUEUE),
#endif
#if defined(TCP_QUEUE_SEQ) || (defined(HAVE_DECL_TCP_QUEUE_SEQ) && HAVE_DECL_TCP_QUEUE_SEQ)
  XLAT(TCP_QUEUE_SEQ),
#endif
#if defined(TCP_REPAIR_OPTIONS) || (defined(HAVE_DECL_TCP_REPAIR_OPTIONS) && HAVE_DECL_TCP_REPAIR_OPTIONS)
  XLAT(TCP_REPAIR_OPTIONS),
#endif
#if defined(TCP_FASTOPEN) || (defined(HAVE_DECL_TCP_FASTOPEN) && HAVE_DECL_TCP_FASTOPEN)
  XLAT(TCP_FASTOPEN),
#endif
#if defined(TCP_TIMESTAMP) || (defined(HAVE_DECL_TCP_TIMESTAMP) && HAVE_DECL_TCP_TIMESTAMP)
  XLAT(TCP_TIMESTAMP),
#endif
#if defined(TCP_NOTSENT_LOWAT) || (defined(HAVE_DECL_TCP_NOTSENT_LOWAT) && HAVE_DECL_TCP_NOTSENT_LOWAT)
  XLAT(TCP_NOTSENT_LOWAT),
#endif
#if defined(TCP_CC_INFO) || (defined(HAVE_DECL_TCP_CC_INFO) && HAVE_DECL_TCP_CC_INFO)
  XLAT(TCP_CC_INFO),
#endif
#if defined(TCP_SAVE_SYN) || (defined(HAVE_DECL_TCP_SAVE_SYN) && HAVE_DECL_TCP_SAVE_SYN)
  XLAT(TCP_SAVE_SYN),
#endif
#if defined(TCP_SAVED_SYN) || (defined(HAVE_DECL_TCP_SAVED_SYN) && HAVE_DECL_TCP_SAVED_SYN)
  XLAT(TCP_SAVED_SYN),
#endif
#if defined(TCP_REPAIR_WINDOW) || (defined(HAVE_DECL_TCP_REPAIR_WINDOW) && HAVE_DECL_TCP_REPAIR_WINDOW)
  XLAT(TCP_REPAIR_WINDOW),
#endif
#if defined(TCP_FASTOPEN_CONNECT) || (defined(HAVE_DECL_TCP_FASTOPEN_CONNECT) && HAVE_DECL_TCP_FASTOPEN_CONNECT)
  XLAT(TCP_FASTOPEN_CONNECT),
#endif
 XLAT_END
};

#endif /* !IN_MPERS */
