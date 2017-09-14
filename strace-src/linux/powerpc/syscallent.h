/*
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1995-2017 The strace developers.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

[  0] = { 0,	0,		SEN(restart_syscall),		"restart_syscall"	},
[  1] = { 1,	TP|SE,		SEN(exit),			"exit"			},
[  2] = { 0,	TP,		SEN(fork),			"fork"			},
[  3] = { 3,	TD,		SEN(read),			"read"			},
[  4] = { 3,	TD,		SEN(write),			"write"			},
[  5] = { 3,	TD|TF,		SEN(open),			"open"			},
[  6] = { 1,	TD,		SEN(close),			"close"			},
[  7] = { 3,	TP,		SEN(waitpid),			"waitpid"		},
[  8] = { 2,	TD|TF,		SEN(creat),			"creat"			},
[  9] = { 2,	TF,		SEN(link),			"link"			},
[ 10] = { 1,	TF,		SEN(unlink),			"unlink"		},
[ 11] = { 3,	TF|TP|SE|SI,	SEN(execve),			"execve"		},
[ 12] = { 1,	TF,		SEN(chdir),			"chdir"			},
[ 13] = { 1,	0,		SEN(time),			"time"			},
[ 14] = { 3,	TF,		SEN(mknod),			"mknod"			},
[ 15] = { 2,	TF,		SEN(chmod),			"chmod"			},
[ 16] = { 3,	TF,		SEN(chown),			"lchown"		},
[ 17] = { 0,	TM,		SEN(break),			"break"			},
[ 18] = { 2,	TF|TST|TSTA,	SEN(oldstat),			"oldstat"		},
[ 19] = { 3,	TD,		SEN(lseek),			"lseek"			},
[ 20] = { 0,	NF,		SEN(getpid),			"getpid"		},
[ 21] = { 5,	TF,		SEN(mount),			"mount"			},
[ 22] = { 1,	TF,		SEN(umount),			"umount"		},
[ 23] = { 1,	0,		SEN(setuid),			"setuid"		},
[ 24] = { 0,	NF,		SEN(getuid),			"getuid"		},
[ 25] = { 1,	0,		SEN(stime),			"stime"			},
[ 26] = { 4,	0,		SEN(ptrace),			"ptrace"		},
[ 27] = { 1,	0,		SEN(alarm),			"alarm"			},
[ 28] = { 2,	TD|TFST|TSTA,	SEN(oldfstat),			"oldfstat"		},
[ 29] = { 0,	TS,		SEN(pause),			"pause"			},
[ 30] = { 2,	TF,		SEN(utime),			"utime"			},
[ 31] = { 2,	0,		SEN(stty),			"stty"			},
[ 32] = { 2,	0,		SEN(gtty),			"gtty"			},
[ 33] = { 2,	TF,		SEN(access),			"access"		},
[ 34] = { 1,	0,		SEN(nice),			"nice"			},
[ 35] = { 0,	0,		SEN(ftime),			"ftime"			},
[ 36] = { 0,	0,		SEN(sync),			"sync"			},
[ 37] = { 2,	TS,		SEN(kill),			"kill"			},
[ 38] = { 2,	TF,		SEN(rename),			"rename"		},
[ 39] = { 2,	TF,		SEN(mkdir),			"mkdir"			},
[ 40] = { 1,	TF,		SEN(rmdir),			"rmdir"			},
[ 41] = { 1,	TD,		SEN(dup),			"dup"			},
[ 42] = { 1,	TD,		SEN(pipe),			"pipe"			},
[ 43] = { 1,	0,		SEN(times),			"times"			},
[ 44] = { 0,	0,		SEN(prof),			"prof"			},
[ 45] = { 1,	TM|SI,		SEN(brk),			"brk"			},
[ 46] = { 1,	0,		SEN(setgid),			"setgid"		},
[ 47] = { 0,	NF,		SEN(getgid),			"getgid"		},
[ 48] = { 2,	TS,		SEN(signal),			"signal"		},
[ 49] = { 0,	NF,		SEN(geteuid),			"geteuid"		},
[ 50] = { 0,	NF,		SEN(getegid),			"getegid"		},
[ 51] = { 1,	TF,		SEN(acct),			"acct"			},
[ 52] = { 2,	TF,		SEN(umount2),			"umount2"		},
[ 53] = { 0,	0,		SEN(lock),			"lock"			},
[ 54] = { 3,	TD,		SEN(ioctl),			"ioctl"			},
[ 55] = { 3,	TD,		SEN(fcntl),			"fcntl"			},
[ 56] = { 0,	0,		SEN(mpx),			"mpx"			},
[ 57] = { 2,	0,		SEN(setpgid),			"setpgid"		},
[ 58] = { 2,	0,		SEN(ulimit),			"ulimit"		},
[ 59] = { 1,	0,		SEN(oldolduname),		"oldolduname"		},
[ 60] = { 1,	NF,		SEN(umask),			"umask"			},
[ 61] = { 1,	TF,		SEN(chroot),			"chroot"		},
[ 62] = { 2,	TSFA,		SEN(ustat),			"ustat"			},
[ 63] = { 2,	TD,		SEN(dup2),			"dup2"			},
[ 64] = { 0,	NF,		SEN(getppid),			"getppid"		},
[ 65] = { 0,	0,		SEN(getpgrp),			"getpgrp"		},
[ 66] = { 0,	0,		SEN(setsid),			"setsid"		},
[ 67] = { 3,	TS,		SEN(sigaction),			"sigaction"		},
[ 68] = { 0,	TS,		SEN(sgetmask),			"sgetmask"		},
[ 69] = { 1,	TS,		SEN(ssetmask),			"ssetmask"		},
[ 70] = { 2,	0,		SEN(setreuid),			"setreuid"		},
[ 71] = { 2,	0,		SEN(setregid),			"setregid"		},
[ 72] = { 1,	TS,		SEN(sigsuspend),		"sigsuspend"		},
[ 73] = { 1,	TS,		SEN(sigpending),		"sigpending"		},
[ 74] = { 2,	0,		SEN(sethostname),		"sethostname"		},
[ 75] = { 2,	0,		SEN(setrlimit),			"setrlimit"		},
[ 76] = { 2,	0,		SEN(getrlimit),			"getrlimit"		},
[ 77] = { 2,	0,		SEN(getrusage),			"getrusage"		},
[ 78] = { 2,	0,		SEN(gettimeofday),		"gettimeofday"		},
[ 79] = { 2,	0,		SEN(settimeofday),		"settimeofday"		},
[ 80] = { 2,	0,		SEN(getgroups),			"getgroups"		},
[ 81] = { 2,	0,		SEN(setgroups),			"setgroups"		},
[ 82] = { 1,	TD,		SEN(oldselect),			"select"		},
[ 83] = { 2,	TF,		SEN(symlink),			"symlink"		},
[ 84] = { 2,	TF|TLST|TSTA,	SEN(oldlstat),			"oldlstat"		},
[ 85] = { 3,	TF,		SEN(readlink),			"readlink"		},
[ 86] = { 1,	TF,		SEN(uselib),			"uselib"		},
[ 87] = { 2,	TF,		SEN(swapon),			"swapon"		},
[ 88] = { 4,	0,		SEN(reboot),			"reboot"		},
[ 89] = { 3,	TD,		SEN(readdir),			"readdir"		},
[ 90] = { 6,	TD|TM|SI,	SEN(mmap),			"mmap"			},
[ 91] = { 2,	TM|SI,		SEN(munmap),			"munmap"		},
[ 92] = { 2,	TF,		SEN(truncate),			"truncate"		},
[ 93] = { 2,	TD,		SEN(ftruncate),			"ftruncate"		},
[ 94] = { 2,	TD,		SEN(fchmod),			"fchmod"		},
[ 95] = { 3,	TD,		SEN(fchown),			"fchown"		},
[ 96] = { 2,	0,		SEN(getpriority),		"getpriority"		},
[ 97] = { 3,	0,		SEN(setpriority),		"setpriority"		},
[ 98] = { 4,	0,		SEN(profil),			"profil"		},
[ 99] = { 2,	TF|TSF|TSFA,	SEN(statfs),			"statfs"		},
[100] = { 2,	TD|TFSF|TSFA,	SEN(fstatfs),			"fstatfs"		},
[101] = { 3,	0,		SEN(ioperm),			"ioperm"		},
[102] = { 2,	TD,		SEN(socketcall),		"socketcall"		},
[103] = { 3,	0,		SEN(syslog),			"syslog"		},
[104] = { 3,	0,		SEN(setitimer),			"setitimer"		},
[105] = { 2,	0,		SEN(getitimer),			"getitimer"		},
[106] = { 2,	TF|TST|TSTA,	SEN(stat),			"stat"			},
[107] = { 2,	TF|TLST|TSTA,	SEN(lstat),			"lstat"			},
[108] = { 2,	TD|TFST|TSTA,	SEN(fstat),			"fstat"			},
[109] = { 1,	0,		SEN(olduname),			"olduname"		},
[110] = { 1,	0,		SEN(iopl),			"iopl"			},
[111] = { 0,	0,		SEN(vhangup),			"vhangup"		},
[112] = { 0,	0,		SEN(idle),			"idle"			},
[113] = { 5,	0,		SEN(vm86),			"vm86"			},
[114] = { 4,	TP,		SEN(wait4),			"wait4"			},
[115] = { 1,	TF,		SEN(swapoff),			"swapoff"		},
[116] = { 1,	0,		SEN(sysinfo),			"sysinfo"		},
[117] = { 6,	TI,		SEN(ipc),			"ipc"			},
[118] = { 1,	TD,		SEN(fsync),			"fsync"			},
[119] = { 0,	TS,		SEN(sigreturn),			"sigreturn"		},
[120] = { 5,	TP,		SEN(clone),			"clone"			},
[121] = { 2,	0,		SEN(setdomainname),		"setdomainname"		},
[122] = { 1,	0,		SEN(uname),			"uname"			},
[123] = { 5,	0,		SEN(printargs),			"modify_ldt"		},
[124] = { 1,	0,		SEN(adjtimex),			"adjtimex"		},
[125] = { 3,	TM|SI,		SEN(mprotect),			"mprotect"		},
[126] = { 3,	TS,		SEN(sigprocmask),		"sigprocmask"		},
[127] = { 2,	0,		SEN(create_module),		"create_module"		},
[128] = { 3,	0,		SEN(init_module),		"init_module"		},
[129] = { 2,	0,		SEN(delete_module),		"delete_module"		},
[130] = { 1,	0,		SEN(get_kernel_syms),		"get_kernel_syms"	},
[131] = { 4,	TF,		SEN(quotactl),			"quotactl"		},
[132] = { 1,	0,		SEN(getpgid),			"getpgid"		},
[133] = { 1,	TD,		SEN(fchdir),			"fchdir"		},
[134] = { 2,	0,		SEN(bdflush),			"bdflush"		},
[135] = { 3,	0,		SEN(sysfs),			"sysfs"			},
[136] = { 1,	NF,		SEN(personality),		"personality"		},
[137] = { 5,	0,		SEN(afs_syscall),		"afs_syscall"		},
[138] = { 1,	NF,		SEN(setfsuid),			"setfsuid"		},
[139] = { 1,	NF,		SEN(setfsgid),			"setfsgid"		},
[140] = { 5,	TD,		SEN(llseek),			"_llseek"		},
[141] = { 3,	TD,		SEN(getdents),			"getdents"		},
[142] = { 5,	TD,		SEN(select),			"_newselect"		},
[143] = { 2,	TD,		SEN(flock),			"flock"			},
[144] = { 3,	TM,		SEN(msync),			"msync"			},
[145] = { 3,	TD,		SEN(readv),			"readv"			},
[146] = { 3,	TD,		SEN(writev),			"writev"		},
[147] = { 1,	0,		SEN(getsid),			"getsid"		},
[148] = { 1,	TD,		SEN(fdatasync),			"fdatasync"		},
[149] = { 1,	0,		SEN(sysctl),			"_sysctl"		},
[150] = { 2,	TM,		SEN(mlock),			"mlock"			},
[151] = { 2,	TM,		SEN(munlock),			"munlock"		},
[152] = { 1,	TM,		SEN(mlockall),			"mlockall"		},
[153] = { 0,	TM,		SEN(munlockall),		"munlockall"		},
[154] = { 2,	0,		SEN(sched_setparam),		"sched_setparam"	},
[155] = { 2,	0,		SEN(sched_getparam),		"sched_getparam"	},
[156] = { 3,	0,		SEN(sched_setscheduler),	"sched_setscheduler"	},
[157] = { 1,	0,		SEN(sched_getscheduler),	"sched_getscheduler"	},
[158] = { 0,	0,		SEN(sched_yield),		"sched_yield"		},
[159] = { 1,	0,		SEN(sched_get_priority_max),	"sched_get_priority_max"},
[160] = { 1,	0,		SEN(sched_get_priority_min),	"sched_get_priority_min"},
[161] = { 2,	0,		SEN(sched_rr_get_interval),	"sched_rr_get_interval"	},
[162] = { 2,	0,		SEN(nanosleep),			"nanosleep"		},
[163] = { 5,	TM|SI,		SEN(mremap),			"mremap"		},
[164] = { 3,	0,		SEN(setresuid),			"setresuid"		},
[165] = { 3,	0,		SEN(getresuid),			"getresuid"		},
[166] = { 5,	0,		SEN(query_module),		"query_module"		},
[167] = { 3,	TD,		SEN(poll),			"poll"			},
[168] = { 3,	0,		SEN(nfsservctl),		"nfsservctl"		},
[169] = { 3,	0,		SEN(setresgid),			"setresgid"		},
[170] = { 3,	0,		SEN(getresgid),			"getresgid"		},
[171] = { 5,	0,		SEN(prctl),			"prctl"			},
[172] = { 0,	TS,		SEN(rt_sigreturn),		"rt_sigreturn"		},
[173] = { 4,	TS,		SEN(rt_sigaction),		"rt_sigaction"		},
[174] = { 4,	TS,		SEN(rt_sigprocmask),		"rt_sigprocmask"	},
[175] = { 2,	TS,		SEN(rt_sigpending),		"rt_sigpending"		},
[176] = { 4,	TS,		SEN(rt_sigtimedwait),		"rt_sigtimedwait"	},
[177] = { 3,	TS,		SEN(rt_sigqueueinfo),		"rt_sigqueueinfo"	},
[178] = { 2,	TS,		SEN(rt_sigsuspend),		"rt_sigsuspend"		},
[179] = { 6,	TD,		SEN(pread),			"pread64"		},
[180] = { 6,	TD,		SEN(pwrite),			"pwrite64"		},
[181] = { 3,	TF,		SEN(chown),			"chown"			},
[182] = { 2,	TF,		SEN(getcwd),			"getcwd"		},
[183] = { 2,	0,		SEN(capget),			"capget"		},
[184] = { 2,	0,		SEN(capset),			"capset"		},
[185] = { 2,	TS,		SEN(sigaltstack),		"sigaltstack"		},
[186] = { 4,	TD|TN,		SEN(sendfile),			"sendfile"		},
[187] = { 5,	TN,		SEN(getpmsg),			"getpmsg"		},
[188] = { 5,	TN,		SEN(putpmsg),			"putpmsg"		},
[189] = { 0,	TP,		SEN(vfork),			"vfork"			},
[190] = { 2,	0,		SEN(getrlimit),			"ugetrlimit"		},
[191] = { 5,	TD,		SEN(readahead),			"readahead"		},
[192] = { 6,	TD|TM|SI,	SEN(mmap_4koff),		"mmap2"			},
[193] = { 4,	TF,		SEN(truncate64),		"truncate64"		},
[194] = { 4,	TD,		SEN(ftruncate64),		"ftruncate64"		},
[195] = { 2,	TF|TST|TSTA,	SEN(stat64),			"stat64"		},
[196] = { 2,	TF|TLST|TSTA,	SEN(lstat64),			"lstat64"		},
[197] = { 2,	TD|TFST|TSTA,	SEN(fstat64),			"fstat64"		},
[198] = { 5,	0,		SEN(printargs),			"pciconfig_read"	},
[199] = { 5,	0,		SEN(printargs),			"pciconfig_write"	},
[200] = { 3,	0,		SEN(printargs),			"pciconfig_iobase"	},
[201] = { 6,	0,		SEN(printargs),			"multiplexer"		},
[202] = { 3,	TD,		SEN(getdents64),		"getdents64"		},
[203] = { 2,	TF,		SEN(pivotroot),			"pivot_root"		},
[204] = { 3,	TD,		SEN(fcntl64),			"fcntl64"		},
[205] = { 3,	TM,		SEN(madvise),			"madvise"		},
[206] = { 3,	TM,		SEN(mincore),			"mincore"		},
[207] = { 0,	NF,		SEN(gettid),			"gettid"		},
[208] = { 2,	TS,		SEN(kill),			"tkill"			},
[209] = { 5,	TF,		SEN(setxattr),			"setxattr"		},
[210] = { 5,	TF,		SEN(setxattr),			"lsetxattr"		},
[211] = { 5,	TD,		SEN(fsetxattr),			"fsetxattr"		},
[212] = { 4,	TF,		SEN(getxattr),			"getxattr"		},
[213] = { 4,	TF,		SEN(getxattr),			"lgetxattr"		},
[214] = { 4,	TD,		SEN(fgetxattr),			"fgetxattr"		},
[215] = { 3,	TF,		SEN(listxattr),			"listxattr"		},
[216] = { 3,	TF,		SEN(listxattr),			"llistxattr"		},
[217] = { 3,	TD,		SEN(flistxattr),		"flistxattr"		},
[218] = { 2,	TF,		SEN(removexattr),		"removexattr"		},
[219] = { 2,	TF,		SEN(removexattr),		"lremovexattr"		},
[220] = { 2,	TD,		SEN(fremovexattr),		"fremovexattr"		},
[221] = { 6,	0,		SEN(futex),			"futex"			},
[222] = { 3,	0,		SEN(sched_setaffinity),		"sched_setaffinity"	},
[223] = { 3,	0,		SEN(sched_getaffinity),		"sched_getaffinity"	},
[224] = { },
[225] = { 5,	0,		SEN(printargs),			"tuxcall"		},
[226] = { 4,	TD|TN,		SEN(sendfile64),		"sendfile64"		},
[227] = { 2,	TM,		SEN(io_setup),			"io_setup"		},
[228] = { 1,	TM,		SEN(io_destroy),		"io_destroy"		},
[229] = { 5,	0,		SEN(io_getevents),		"io_getevents"		},
[230] = { 3,	0,		SEN(io_submit),			"io_submit"		},
[231] = { 3,	0,		SEN(io_cancel),			"io_cancel"		},
[232] = { 1,	0,		SEN(set_tid_address),		"set_tid_address"	},
[233] = { 6,	TD,		SEN(fadvise64),			"fadvise64"		},
[234] = { 1,	TP|SE,		SEN(exit),			"exit_group"		},
[235] = { 4,	0,		SEN(lookup_dcookie),		"lookup_dcookie"	},
[236] = { 1,	TD,		SEN(epoll_create),		"epoll_create"		},
[237] = { 4,	TD,		SEN(epoll_ctl),			"epoll_ctl"		},
[238] = { 4,	TD,		SEN(epoll_wait),		"epoll_wait"		},
[239] = { 5,	TM|SI,		SEN(remap_file_pages),		"remap_file_pages"	},
[240] = { 3,	0,		SEN(timer_create),		"timer_create"		},
[241] = { 4,	0,		SEN(timer_settime),		"timer_settime"		},
[242] = { 2,	0,		SEN(timer_gettime),		"timer_gettime"		},
[243] = { 1,	0,		SEN(timer_getoverrun),		"timer_getoverrun"	},
[244] = { 1,	0,		SEN(timer_delete),		"timer_delete"		},
[245] = { 2,	0,		SEN(clock_settime),		"clock_settime"		},
[246] = { 2,	0,		SEN(clock_gettime),		"clock_gettime"		},
[247] = { 2,	0,		SEN(clock_getres),		"clock_getres"		},
[248] = { 4,	0,		SEN(clock_nanosleep),		"clock_nanosleep"	},
[249] = { 2,	0,		SEN(printargs),			"swapcontext"		},
[250] = { 3,	TS,		SEN(tgkill),			"tgkill"		},
[251] = { 2,	TF,		SEN(utimes),			"utimes"		},
[252] = { 3,	TF|TSF|TSFA,	SEN(statfs64),			"statfs64"		},
[253] = { 3,	TD|TFSF|TSFA,	SEN(fstatfs64),			"fstatfs64"		},
[254] = { 6,	TD,		SEN(fadvise64_64),		"fadvise64_64"		},
[255] = { 1,	0,		SEN(printargs),			"rtas"			},
[256] = { 5,	0,		SEN(printargs),			"sys_debug_setcontext"	},
[257] = { 5,	0,		SEN(vserver),			"vserver"		},
[258] = { 4,	TM,		SEN(migrate_pages),		"migrate_pages"		},
[259] = { 6,	TM,		SEN(mbind),			"mbind"			},
[260] = { 5,	TM,		SEN(get_mempolicy),		"get_mempolicy"		},
[261] = { 3,	TM,		SEN(set_mempolicy),		"set_mempolicy"		},
[262] = { 4,	0,		SEN(mq_open),			"mq_open"		},
[263] = { 1,	0,		SEN(mq_unlink),			"mq_unlink"		},
[264] = { 5,	0,		SEN(mq_timedsend),		"mq_timedsend"		},
[265] = { 5,	0,		SEN(mq_timedreceive),		"mq_timedreceive"	},
[266] = { 2,	0,		SEN(mq_notify),			"mq_notify"		},
[267] = { 3,	0,		SEN(mq_getsetattr),		"mq_getsetattr"		},
[268] = { 4,	0,		SEN(kexec_load),		"kexec_load"		},
[269] = { 5,	0,		SEN(add_key),			"add_key"		},
[270] = { 4,	0,		SEN(request_key),		"request_key"		},
[271] = { 5,	0,		SEN(keyctl),			"keyctl"		},
[272] = { 5,	TP,		SEN(waitid),			"waitid"		},
[273] = { 3,	0,		SEN(ioprio_set),		"ioprio_set"		},
[274] = { 2,	0,		SEN(ioprio_get),		"ioprio_get"		},
[275] = { 0,	TD,		SEN(inotify_init),		"inotify_init"		},
[276] = { 3,	TD,		SEN(inotify_add_watch),		"inotify_add_watch"	},
[277] = { 2,	TD,		SEN(inotify_rm_watch),		"inotify_rm_watch"	},
[278] = { 3,	0,		SEN(printargs),			"spu_run"		},
[279] = { 4,	0,		SEN(printargs),			"spu_create"		},
[280] = { 6,	TD,		SEN(pselect6),			"pselect6"		},
[281] = { 5,	TD,		SEN(ppoll),			"ppoll"			},
[282] = { 1,	TP,		SEN(unshare),			"unshare"		},
[283] = { 6,	TD,		SEN(splice),			"splice"		},
[284] = { 4,	TD,		SEN(tee),			"tee"			},
[285] = { 4,	TD,		SEN(vmsplice),			"vmsplice"		},
[286] = { 4,	TD|TF,		SEN(openat),			"openat"		},
[287] = { 3,	TD|TF,		SEN(mkdirat),			"mkdirat"		},
[288] = { 4,	TD|TF,		SEN(mknodat),			"mknodat"		},
[289] = { 5,	TD|TF,		SEN(fchownat),			"fchownat"		},
[290] = { 3,	TD|TF,		SEN(futimesat),			"futimesat"		},
[291] = { 4,	TD|TF|TFST|TSTA,SEN(fstatat64),			"fstatat64"		},
[292] = { 3,	TD|TF,		SEN(unlinkat),			"unlinkat"		},
[293] = { 4,	TD|TF,		SEN(renameat),			"renameat"		},
[294] = { 5,	TD|TF,		SEN(linkat),			"linkat"		},
[295] = { 3,	TD|TF,		SEN(symlinkat),			"symlinkat"		},
[296] = { 4,	TD|TF,		SEN(readlinkat),		"readlinkat"		},
[297] = { 3,	TD|TF,		SEN(fchmodat),			"fchmodat"		},
[298] = { 3,	TD|TF,		SEN(faccessat),			"faccessat"		},
[299] = { 3,	0,		SEN(get_robust_list),		"get_robust_list"	},
[300] = { 2,	0,		SEN(set_robust_list),		"set_robust_list"	},
[301] = { 6,	TM,		SEN(move_pages),		"move_pages"		},
[302] = { 3,	0,		SEN(getcpu),			"getcpu"		},
[303] = { 6,	TD,		SEN(epoll_pwait),		"epoll_pwait"		},
[304] = { 4,	TD|TF,		SEN(utimensat),			"utimensat"		},
[305] = { 3,	TD|TS,		SEN(signalfd),			"signalfd"		},
[306] = { 2,	TD,		SEN(timerfd_create),		"timerfd_create"	},
[307] = { 1,	TD,		SEN(eventfd),			"eventfd"		},
[308] = { 6,	TD,		SEN(sync_file_range2),		"sync_file_range2"	},
[309] = { 6,	TD,		SEN(fallocate),			"fallocate"		},
[310] = { 3,	0,		SEN(subpage_prot),		"subpage_prot"		},
[311] = { 4,	TD,		SEN(timerfd_settime),		"timerfd_settime"	},
[312] = { 2,	TD,		SEN(timerfd_gettime),		"timerfd_gettime"	},
[313] = { 4,	TD|TS,		SEN(signalfd4),			"signalfd4"		},
[314] = { 2,	TD,		SEN(eventfd2),			"eventfd2"		},
[315] = { 1,	TD,		SEN(epoll_create1),		"epoll_create1"		},
[316] = { 3,	TD,		SEN(dup3),			"dup3"			},
[317] = { 2,	TD,		SEN(pipe2),			"pipe2"			},
[318] = { 1,	TD,		SEN(inotify_init1),		"inotify_init1"		},
[319] = { 5,	TD,		SEN(perf_event_open),		"perf_event_open"	},
[320] = { 5,	TD,		SEN(preadv),			"preadv"		},
[321] = { 5,	TD,		SEN(pwritev),			"pwritev"		},
[322] = { 4,	TP|TS,		SEN(rt_tgsigqueueinfo),		"rt_tgsigqueueinfo"	},
[323] = { 2,	TD,		SEN(fanotify_init),		"fanotify_init"		},
[324] = { 6,	TD|TF,		SEN(fanotify_mark),		"fanotify_mark"		},
[325] = { 4,	0,		SEN(prlimit64),			"prlimit64"		},
[326] = { 3,	TN,		SEN(socket),			"socket"		},
[327] = { 3,	TN,		SEN(bind),			"bind"			},
[328] = { 3,	TN,		SEN(connect),			"connect"		},
[329] = { 2,	TN,		SEN(listen),			"listen"		},
[330] = { 3,	TN,		SEN(accept),			"accept"		},
[331] = { 3,	TN,		SEN(getsockname),		"getsockname"		},
[332] = { 3,	TN,		SEN(getpeername),		"getpeername"		},
[333] = { 4,	TN,		SEN(socketpair),		"socketpair"		},
[334] = { 4,	TN,		SEN(send),			"send"			},
[335] = { 6,	TN,		SEN(sendto),			"sendto"		},
[336] = { 4,	TN,		SEN(recv),			"recv"			},
[337] = { 6,	TN,		SEN(recvfrom),			"recvfrom"		},
[338] = { 2,	TN,		SEN(shutdown),			"shutdown"		},
[339] = { 5,	TN,		SEN(setsockopt),		"setsockopt"		},
[340] = { 5,	TN,		SEN(getsockopt),		"getsockopt"		},
[341] = { 3,	TN,		SEN(sendmsg),			"sendmsg"		},
[342] = { 3,	TN,		SEN(recvmsg),			"recvmsg"		},
[343] = { 5,	TN,		SEN(recvmmsg),			"recvmmsg"		},
[344] = { 4,	TN,		SEN(accept4),			"accept4"		},
[345] = { 5,	TD|TF,		SEN(name_to_handle_at),		"name_to_handle_at"	},
[346] = { 3,	TD,		SEN(open_by_handle_at),		"open_by_handle_at"	},
[347] = { 2,	0,		SEN(clock_adjtime),		"clock_adjtime"		},
[348] = { 1,	TD,		SEN(syncfs),			"syncfs"		},
[349] = { 4,	TN,		SEN(sendmmsg),			"sendmmsg"		},
[350] = { 2,	TD,		SEN(setns),			"setns"			},
[351] = { 6,	0,		SEN(process_vm_readv),		"process_vm_readv"	},
[352] = { 6,	0,		SEN(process_vm_writev),		"process_vm_writev"	},
[353] = { 3,	TD,		SEN(finit_module),		"finit_module"		},
[354] = { 5,	0,		SEN(kcmp),			"kcmp"			},
[355] = { 3,	0,		SEN(sched_setattr),		"sched_setattr"		},
[356] = { 4,	0,		SEN(sched_getattr),		"sched_getattr"		},
[357] = { 5,	TD|TF,		SEN(renameat2),			"renameat2"		},
[358] = { 3,	0,		SEN(seccomp),			"seccomp"		},
[359] = { 3,	0,		SEN(getrandom),			"getrandom"		},
[360] = { 2,	TD,		SEN(memfd_create),		"memfd_create"		},
[361] = { 3,	TD,		SEN(bpf),			"bpf"			},
[362] = { 5,	TD|TF|TP|SE|SI,	SEN(execveat),			"execveat"		},
[363] = { 0,	0,		SEN(printargs),			"switch_endian"		},
[364] = { 1,	TD,		SEN(userfaultfd),		"userfaultfd"		},
[365] = { 2,	0,		SEN(membarrier),		"membarrier"		},
[366] = { 3,	TI,		SEN(semop),			"semop"			},
[367] = { 3,	TI,		SEN(semget),			"semget"		},
[368] = { 4,	TI,		SEN(semctl),			"semctl"		},
[369] = { 4,	TI,		SEN(semtimedop),		"semtimedop"		},
[370] = { 4,	TI,		SEN(msgsnd),			"msgsnd"		},
[371] = { 5,	TI,		SEN(msgrcv),			"msgrcv"		},
[372] = { 2,	TI,		SEN(msgget),			"msgget"		},
[373] = { 3,	TI,		SEN(msgctl),			"msgctl"		},
[374] = { 3,	TI|TM|SI,	SEN(shmat),			"shmat"			},
[375] = { 1,	TI|TM|SI,	SEN(shmdt),			"shmdt"			},
[376] = { 3,	TI,		SEN(shmget),			"shmget"		},
[377] = { 3,	TI,		SEN(shmctl),			"shmctl"		},
[378] = { 3,	TM,		SEN(mlock2),			"mlock2"		},
[379] = { 6,	TD,		SEN(copy_file_range),		"copy_file_range"	},
[380] = { 6,	TD,		SEN(preadv2),			"preadv2"		},
[381] = { 6,	TD,		SEN(pwritev2),			"pwritev2"		},
[382] = { 5,	TD,		SEN(kexec_file_load),		"kexec_file_load"	},
[383] = { 5,	TD|TF|TSTA,	SEN(statx),			"statx"			},

#define SYS_socket_subcall	400
#include "subcall.h"
