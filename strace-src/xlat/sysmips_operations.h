/* Generated by ./xlat/gen.sh from ./xlat/sysmips_operations.in; do not edit. */

#ifdef IN_MPERS

# error static const struct xlat sysmips_operations in mpers mode

#else

static
const struct xlat sysmips_operations[] = {
#if defined(SETNAME) || (defined(HAVE_DECL_SETNAME) && HAVE_DECL_SETNAME)
  XLAT(SETNAME),
#endif
#if defined(FLUSH_CACHE) || (defined(HAVE_DECL_FLUSH_CACHE) && HAVE_DECL_FLUSH_CACHE)
  XLAT(FLUSH_CACHE),
#endif
#if defined(MIPS_FIXADE) || (defined(HAVE_DECL_MIPS_FIXADE) && HAVE_DECL_MIPS_FIXADE)
  XLAT(MIPS_FIXADE),
#endif
#if defined(MIPS_RDNVRAM) || (defined(HAVE_DECL_MIPS_RDNVRAM) && HAVE_DECL_MIPS_RDNVRAM)
  XLAT(MIPS_RDNVRAM),
#endif
#if defined(MIPS_ATOMIC_SET) || (defined(HAVE_DECL_MIPS_ATOMIC_SET) && HAVE_DECL_MIPS_ATOMIC_SET)
  XLAT(MIPS_ATOMIC_SET),
#endif
 XLAT_END
};

#endif /* !IN_MPERS */
