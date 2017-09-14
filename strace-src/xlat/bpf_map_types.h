/* Generated by ./xlat/gen.sh from ./xlat/bpf_map_types.in; do not edit. */
#if !(defined(BPF_MAP_TYPE_UNSPEC) || (defined(HAVE_DECL_BPF_MAP_TYPE_UNSPEC) && HAVE_DECL_BPF_MAP_TYPE_UNSPEC))
# define BPF_MAP_TYPE_UNSPEC 0
#endif
#if !(defined(BPF_MAP_TYPE_HASH) || (defined(HAVE_DECL_BPF_MAP_TYPE_HASH) && HAVE_DECL_BPF_MAP_TYPE_HASH))
# define BPF_MAP_TYPE_HASH 1
#endif
#if !(defined(BPF_MAP_TYPE_ARRAY) || (defined(HAVE_DECL_BPF_MAP_TYPE_ARRAY) && HAVE_DECL_BPF_MAP_TYPE_ARRAY))
# define BPF_MAP_TYPE_ARRAY 2
#endif
#if !(defined(BPF_MAP_TYPE_PROG_ARRAY) || (defined(HAVE_DECL_BPF_MAP_TYPE_PROG_ARRAY) && HAVE_DECL_BPF_MAP_TYPE_PROG_ARRAY))
# define BPF_MAP_TYPE_PROG_ARRAY 3
#endif
#if !(defined(BPF_MAP_TYPE_PERF_EVENT_ARRAY) || (defined(HAVE_DECL_BPF_MAP_TYPE_PERF_EVENT_ARRAY) && HAVE_DECL_BPF_MAP_TYPE_PERF_EVENT_ARRAY))
# define BPF_MAP_TYPE_PERF_EVENT_ARRAY 4
#endif
#if !(defined(BPF_MAP_TYPE_PERCPU_HASH) || (defined(HAVE_DECL_BPF_MAP_TYPE_PERCPU_HASH) && HAVE_DECL_BPF_MAP_TYPE_PERCPU_HASH))
# define BPF_MAP_TYPE_PERCPU_HASH 5
#endif
#if !(defined(BPF_MAP_TYPE_PERCPU_ARRAY) || (defined(HAVE_DECL_BPF_MAP_TYPE_PERCPU_ARRAY) && HAVE_DECL_BPF_MAP_TYPE_PERCPU_ARRAY))
# define BPF_MAP_TYPE_PERCPU_ARRAY 6
#endif
#if !(defined(BPF_MAP_TYPE_STACK_TRACE) || (defined(HAVE_DECL_BPF_MAP_TYPE_STACK_TRACE) && HAVE_DECL_BPF_MAP_TYPE_STACK_TRACE))
# define BPF_MAP_TYPE_STACK_TRACE 7
#endif
#if !(defined(BPF_MAP_TYPE_CGROUP_ARRAY) || (defined(HAVE_DECL_BPF_MAP_TYPE_CGROUP_ARRAY) && HAVE_DECL_BPF_MAP_TYPE_CGROUP_ARRAY))
# define BPF_MAP_TYPE_CGROUP_ARRAY 8
#endif
#if !(defined(BPF_MAP_TYPE_LRU_HASH) || (defined(HAVE_DECL_BPF_MAP_TYPE_LRU_HASH) && HAVE_DECL_BPF_MAP_TYPE_LRU_HASH))
# define BPF_MAP_TYPE_LRU_HASH 9
#endif
#if !(defined(BPF_MAP_TYPE_LRU_PERCPU_HASH) || (defined(HAVE_DECL_BPF_MAP_TYPE_LRU_PERCPU_HASH) && HAVE_DECL_BPF_MAP_TYPE_LRU_PERCPU_HASH))
# define BPF_MAP_TYPE_LRU_PERCPU_HASH 10
#endif
#if !(defined(BPF_MAP_TYPE_LPM_TRIE) || (defined(HAVE_DECL_BPF_MAP_TYPE_LPM_TRIE) && HAVE_DECL_BPF_MAP_TYPE_LPM_TRIE))
# define BPF_MAP_TYPE_LPM_TRIE 11
#endif
#if !(defined(BPF_MAP_TYPE_ARRAY_OF_MAPS) || (defined(HAVE_DECL_BPF_MAP_TYPE_ARRAY_OF_MAPS) && HAVE_DECL_BPF_MAP_TYPE_ARRAY_OF_MAPS))
# define BPF_MAP_TYPE_ARRAY_OF_MAPS 12
#endif
#if !(defined(BPF_MAP_TYPE_HASH_OF_MAPS) || (defined(HAVE_DECL_BPF_MAP_TYPE_HASH_OF_MAPS) && HAVE_DECL_BPF_MAP_TYPE_HASH_OF_MAPS))
# define BPF_MAP_TYPE_HASH_OF_MAPS 13
#endif

#ifdef IN_MPERS

# error static const struct xlat bpf_map_types in mpers mode

#else

static
const struct xlat bpf_map_types[] = {
 XLAT(BPF_MAP_TYPE_UNSPEC),
 XLAT(BPF_MAP_TYPE_HASH),
 XLAT(BPF_MAP_TYPE_ARRAY),
 XLAT(BPF_MAP_TYPE_PROG_ARRAY),
 XLAT(BPF_MAP_TYPE_PERF_EVENT_ARRAY),
 XLAT(BPF_MAP_TYPE_PERCPU_HASH),
 XLAT(BPF_MAP_TYPE_PERCPU_ARRAY),
 XLAT(BPF_MAP_TYPE_STACK_TRACE),
 XLAT(BPF_MAP_TYPE_CGROUP_ARRAY),
 XLAT(BPF_MAP_TYPE_LRU_HASH),
 XLAT(BPF_MAP_TYPE_LRU_PERCPU_HASH),
 XLAT(BPF_MAP_TYPE_LPM_TRIE),
 XLAT(BPF_MAP_TYPE_ARRAY_OF_MAPS),
 XLAT(BPF_MAP_TYPE_HASH_OF_MAPS),
 XLAT_END
};

#endif /* !IN_MPERS */
