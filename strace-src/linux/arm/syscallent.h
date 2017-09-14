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
[ 16] = { 3,	TF,		SEN(chown16),			"lchown"		},
[ 17] = { 0,	TM,		SEN(break),			"break"			},
[ 18] = { 2,	TF|TST|TSTA,	SEN(oldstat),			"oldstat"		},
[ 19] = { 3,	TD,		SEN(lseek),			"lseek"			},
[ 20] = { 0,	NF,		SEN(getpid),			"getpid"		},
[ 21] = { 5,	TF,		SEN(mount),			"mount"			},
[ 22] = { 1,	TF,		SEN(umount),			"umount"		},
[ 23] = { 1,	0,		SEN(setuid16),			"setuid"		},
[ 24] = { 0,	NF,		SEN(getuid16),			"getuid"		},
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
[ 46] = { 1,	0,		SEN(setgid16),			"setgid"		},
[ 47] = { 0,	NF,		SEN(getgid16),			"getgid"		},
[ 48] = { 2,	TS,		SEN(signal),			"signal"		},
[ 49] = { 0,	NF,		SEN(geteuid16),			"geteuid"		},
[ 50] = { 0,	NF,		SEN(getegid16),			"getegid"		},
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
[ 70] = { 2,	0,		SEN(setreuid16),		"setreuid"		},
[ 71] = { 2,	0,		SEN(setregid16),		"setregid"		},
[ 72] = { 3,	TS,		SEN(sigsuspend),		"sigsuspend"		},
[ 73] = { 1,	TS,		SEN(sigpending),		"sigpending"		},
[ 74] = { 2,	0,		SEN(sethostname),		"sethostname"		},
[ 75] = { 2,	0,		SEN(setrlimit),			"setrlimit"		},
[ 76] = { 2,	0,		SEN(getrlimit),			"getrlimit"		},
[ 77] = { 2,	0,		SEN(getrusage),			"getrusage"		},
[ 78] = { 2,	0,		SEN(gettimeofday),		"gettimeofday"		},
[ 79] = { 2,	0,		SEN(settimeofday),		"settimeofday"		},
[ 80] = { 2,	0,		SEN(getgroups16),		"getgroups"		},
[ 81] = { 2,	0,		SEN(setgroups16),		"setgroups"		},
[ 82] = { 1,	TD,		SEN(oldselect),			"select"		},
[ 83] = { 2,	TF,		SEN(symlink),			"symlink"		},
[ 84] = { 2,	TF|TLST|TSTA,	SEN(oldlstat),			"oldlstat"		},
[ 85] = { 3,	TF,		SEN(readlink),			"readlink"		},
[ 86] = { 1,	TF,		SEN(uselib),			"uselib"		},
[ 87] = { 2,	TF,		SEN(swapon),			"swapon"		},
[ 88] = { 4,	0,		SEN(reboot),			"reboot"		},
[ 89] = { 3,	TD,		SEN(readdir),			"readdir"		},
[ 90] = { 1,	TD|TM|SI,	SEN(old_mmap),			"mmap"			},
[ 91] = { 2,	TM|SI,		SEN(munmap),			"munmap"		},
[ 92] = { 2,	TF,		SEN(truncate),			"truncate"		},
[ 93] = { 2,	TD,		SEN(ftruncate),			"ftruncate"		},
[ 94] = { 2,	TD,		SEN(fchmod),			"fchmod"		},
[ 95] = { 3,	TD,		SEN(fchown16),			"fchown"		},
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
[113] = { 5,	0,		SEN(printargs),			"syscall"		},
[114] = { 4,	TP,		SEN(wait4),			"wait4"			},
[115] = { 1,	TF,		SEN(swapoff),			"swapoff"		},
[116] = { 1,	0,		SEN(sysinfo),			"sysinfo"		},
[117] = { 6,	TI,		SEN(ipc),			"ipc"			},
[118] = { 1,	TD,		SEN(fsync),			"fsync"			},
[119] = { 0,	TS,		SEN(sigreturn),			"sigreturn"		},
[120] = { 5,	TP,		SEN(clone),			"clone"			},
[121] = { 2,	0,		SEN(setdomainname),		"setdomainname"		},
[122] = { 1,	0,		SEN(uname),			"uname"			},
[123] = { 3,	0,		SEN(modify_ldt),		"modify_ldt"		},
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
[138] = { 1,	NF,		SEN(setfsuid16),		"setfsuid"		},
[139] = { 1,	NF,		SEN(setfsgid16),		"setfsgid"		},
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
[164] = { 3,	0,		SEN(setresuid16),		"setresuid"		},
[165] = { 3,	0,		SEN(getresuid16),		"getresuid"		},
[166] = { 5,	0,		SEN(vm86),			"vm86"			},
[167] = { 5,	0,		SEN(query_module),		"query_module"		},
[168] = { 3,	TD,		SEN(poll),			"poll"			},
[169] = { 3,	0,		SEN(nfsservctl),		"nfsservctl"		},
[170] = { 3,	0,		SEN(setresgid16),		"setresgid"		},
[171] = { 3,	0,		SEN(getresgid16),		"getresgid"		},
[172] = { 5,	0,		SEN(prctl),			"prctl"			},
[173] = { 0,	TS,		SEN(rt_sigreturn),		"rt_sigreturn"		},
[174] = { 4,	TS,		SEN(rt_sigaction),		"rt_sigaction"		},
[175] = { 4,	TS,		SEN(rt_sigprocmask),		"rt_sigprocmask"	},
[176] = { 2,	TS,		SEN(rt_sigpending),		"rt_sigpending"		},
[177] = { 4,	TS,		SEN(rt_sigtimedwait),		"rt_sigtimedwait"	},
[178] = { 3,	TS,		SEN(rt_sigqueueinfo),		"rt_sigqueueinfo"	},
[179] = { 2,	TS,		SEN(rt_sigsuspend),		"rt_sigsuspend"		},
[180] = { 6,	TD,		SEN(pread),			"pread64"		},
[181] = { 6,	TD,		SEN(pwrite),			"pwrite64"		},
[182] = { 3,	TF,		SEN(chown16),			"chown"			},
[183] = { 2,	TF,		SEN(getcwd),			"getcwd"		},
[184] = { 2,	0,		SEN(capget),			"capget"		},
[185] = { 2,	0,		SEN(capset),			"capset"		},
[186] = { 2,	TS,		SEN(sigaltstack),		"sigaltstack"		},
[187] = { 4,	TD|TN,		SEN(sendfile),			"sendfile"		},
[188] = { 5,	TN,		SEN(getpmsg),			"getpmsg"		},
[189] = { 5,	TN,		SEN(putpmsg),			"putpmsg"		},
[190] = { 0,	TP,		SEN(vfork),			"vfork"			},
[191] = { 2,	0,		SEN(getrlimit),			"ugetrlimit"		},
[192] = { 6,	TD|TM|SI,	SEN(mmap_4koff),		"mmap2"			},
[193] = { 4,	TF,		SEN(truncate64),		"truncate64"		},
[194] = { 4,	TD,		SEN(ftruncate64),		"ftruncate64"		},
[195] = { 2,	TF|TST|TSTA,	SEN(stat64),			"stat64"		},
[196] = { 2,	TF|TLST|TSTA,	SEN(lstat64),			"lstat64"		},
[197] = { 2,	TD|TFST|TSTA,	SEN(fstat64),			"fstat64"		},
[198] = { 3,	TF,		SEN(chown),			"lchown32"		},
[199] = { 0,	NF,		SEN(getuid),			"getuid32"		},
[200] = { 0,	NF,		SEN(getgid),			"getgid32"		},
[201] = { 0,	NF,		SEN(geteuid),			"geteuid32"		},
[202] = { 0,	NF,		SEN(geteuid),			"getegid32"		},
[203] = { 2,	0,		SEN(setreuid),			"setreuid32"		},
[204] = { 2,	0,		SEN(setregid),			"setregid32"		},
[205] = { 2,	0,		SEN(getgroups),			"getgroups32"		},
[206] = { 2,	0,		SEN(setgroups),			"setgroups32"		},
[207] = { 3,	TD,		SEN(fchown),			"fchown32"		},
[208] = { 3,	0,		SEN(setresuid),			"setresuid32"		},
[209] = { 3,	0,		SEN(getresuid),			"getresuid32"		},
[210] = { 3,	0,		SEN(setresgid),			"setresgid32"		},
[211] = { 3,	0,		SEN(getresgid),			"getresgid32"		},
[212] = { 3,	TF,		SEN(chown),			"chown32"		},
[213] = { 1,	0,		SEN(setuid),			"setuid32"		},
[214] = { 1,	0,		SEN(setgid),			"setgid32"		},
[215] = { 1,	NF,		SEN(setfsuid),			"setfsuid32"		},
[216] = { 1,	NF,		SEN(setfsgid),			"setfsgid32"		},
[217] = { 3,	TD,		SEN(getdents64),		"getdents64"		},
[218] = { 2,	TF,		SEN(pivotroot),			"pivot_root"		},
[219] = { 3,	TM,		SEN(mincore),			"mincore"		},
[220] = { 3,	TM,		SEN(madvise),			"madvise"		},
[221] = { 3,	TD,		SEN(fcntl64),			"fcntl64"		},
[222 ... 223] = { },
[224] = { 0,	NF,		SEN(gettid),			"gettid"		},
[225] = { 5,	TD,		SEN(readahead),			"readahead"		},
[226] = { 5,	TF,		SEN(setxattr),			"setxattr"		},
[227] = { 5,	TF,		SEN(setxattr),			"lsetxattr"		},
[228] = { 5,	TD,		SEN(fsetxattr),			"fsetxattr"		},
[229] = { 4,	TF,		SEN(getxattr),			"getxattr"		},
[230] = { 4,	TF,		SEN(getxattr),			"lgetxattr"		},
[231] = { 4,	TD,		SEN(fgetxattr),			"fgetxattr"		},
[232] = { 3,	TF,		SEN(listxattr),			"listxattr"		},
[233] = { 3,	TF,		SEN(listxattr),			"llistxattr"		},
[234] = { 3,	TD,		SEN(flistxattr),		"flistxattr"		},
[235] = { 2,	TF,		SEN(removexattr),		"removexattr"		},
[236] = { 2,	TF,		SEN(removexattr),		"lremovexattr"		},
[237] = { 2,	TD,		SEN(fremovexattr),		"fremovexattr"		},
[238] = { 2,	TS,		SEN(kill),			"tkill"			},
[239] = { 4,	TD|TN,		SEN(sendfile64),		"sendfile64"		},
[240] = { 6,	0,		SEN(futex),			"futex"			},
[241] = { 3,	0,		SEN(sched_setaffinity),		"sched_setaffinity"	},
[242] = { 3,	0,		SEN(sched_getaffinity),		"sched_getaffinity"	},
[243] = { 2,	TM,		SEN(io_setup),			"io_setup"		},
[244] = { 1,	TM,		SEN(io_destroy),		"io_destroy"		},
[245] = { 5,	0,		SEN(io_getevents),		"io_getevents"		},
[246] = { 3,	0,		SEN(io_submit),			"io_submit"		},
[247] = { 3,	0,		SEN(io_cancel),			"io_cancel"		},
[248] = { 1,	TP|SE,		SEN(exit),			"exit_group"		},
[249] = { 4,	0,		SEN(lookup_dcookie),		"lookup_dcookie"	},
[250] = { 1,	TD,		SEN(epoll_create),		"epoll_create"		},
[251] = { 4,	TD,		SEN(epoll_ctl),			"epoll_ctl"		},
[252] = { 4,	TD,		SEN(epoll_wait),		"epoll_wait"		},
[253] = { 5,	TM|SI,		SEN(remap_file_pages),		"remap_file_pages"	},
[254 ... 255] = { },
[256] = { 1,	0,		SEN(set_tid_address),		"set_tid_address"	},
[257] = { 3,	0,		SEN(timer_create),		"timer_create"		},
[258] = { 4,	0,		SEN(timer_settime),		"timer_settime"		},
[259] = { 2,	0,		SEN(timer_gettime),		"timer_gettime"		},
[260] = { 1,	0,		SEN(timer_getoverrun),		"timer_getoverrun"	},
[261] = { 1,	0,		SEN(timer_delete),		"timer_delete"		},
[262] = { 2,	0,		SEN(clock_settime),		"clock_settime"		},
[263] = { 2,	0,		SEN(clock_gettime),		"clock_gettime"		},
[264] = { 2,	0,		SEN(clock_getres),		"clock_getres"		},
[265] = { 4,	0,		SEN(clock_nanosleep),		"clock_nanosleep"	},
[266] = { 3,	TF|TSF|TSFA,	SEN(statfs64),			"statfs64"		},
[267] = { 3,	TD|TFSF|TSFA,	SEN(fstatfs64),			"fstatfs64"		},
[268] = { 3,	TS,		SEN(tgkill),			"tgkill"		},
[269] = { 2,	TF,		SEN(utimes),			"utimes"		},
[270] = { 6,	TD,		SEN(fadvise64_64),		"fadvise64_64"		},
[271] = { 3,	0,		SEN(printargs),			"pciconfig_iobase"	},
[272] = { 5,	0,		SEN(printargs),			"pciconfig_read"	},
[273] = { 5,	0,		SEN(printargs),			"pciconfig_write"	},
[274] = { 4,	0,		SEN(mq_open),			"mq_open"		},
[275] = { 1,	0,		SEN(mq_unlink),			"mq_unlink"		},
[276] = { 5,	0,		SEN(mq_timedsend),		"mq_timedsend"		},
[277] = { 5,	0,		SEN(mq_timedreceive),		"mq_timedreceive"	},
[278] = { 2,	0,		SEN(mq_notify),			"mq_notify"		},
[279] = { 3,	0,		SEN(mq_getsetattr),		"mq_getsetattr"		},
[280] = { 5,	TP,		SEN(waitid),			"waitid"		},
[281] = { 3,	TN,		SEN(socket),			"socket"		},
[282] = { 3,	TN,		SEN(bind),			"bind"			},
[283] = { 3,	TN,		SEN(connect),			"connect"		},
[284] = { 2,	TN,		SEN(listen),			"listen"		},
[285] = { 3,	TN,		SEN(accept),			"accept"		},
[286] = { 3,	TN,		SEN(getsockname),		"getsockname"		},
[287] = { 3,	TN,		SEN(getpeername),		"getpeername"		},
[288] = { 4,	TN,		SEN(socketpair),		"socketpair"		},
[289] = { 4,	TN,		SEN(send),			"send"			},
[290] = { 6,	TN,		SEN(sendto),			"sendto"		},
[291] = { 4,	TN,		SEN(recv),			"recv"			},
[292] = { 6,	TN,		SEN(recvfrom),			"recvfrom"		},
[293] = { 2,	TN,		SEN(shutdown),			"shutdown"		},
[294] = { 5,	TN,		SEN(setsockopt),		"setsockopt"		},
[295] = { 5,	TN,		SEN(getsockopt),		"getsockopt"		},
[296] = { 3,	TN,		SEN(sendmsg),			"sendmsg"		},
[297] = { 3,	TN,		SEN(recvmsg),			"recvmsg"		},
[298] = { 3,	TI,		SEN(semop),			"semop"			},
[299] = { 3,	TI,		SEN(semget),			"semget"		},
[300] = { 4,	TI,		SEN(semctl),			"semctl"		},
[301] = { 4,	TI,		SEN(msgsnd),			"msgsnd"		},
[302] = { 5,	TI,		SEN(msgrcv),			"msgrcv"		},
[303] = { 2,	TI,		SEN(msgget),			"msgget"		},
[304] = { 3,	TI,		SEN(msgctl),			"msgctl"		},
[305] = { 3,	TI|TM|SI,	SEN(shmat),			"shmat"			},
[306] = { 1,	TI|TM|SI,	SEN(shmdt),			"shmdt"			},
[307] = { 3,	TI,		SEN(shmget),			"shmget"		},
[308] = { 3,	TI,		SEN(shmctl),			"shmctl"		},
[309] = { 5,	0,		SEN(add_key),			"add_key"		},
[310] = { 4,	0,		SEN(request_key),		"request_key"		},
[311] = { 5,	0,		SEN(keyctl),			"keyctl"		},
[312] = { 4,	TI,		SEN(semtimedop),		"semtimedop"		},
[313] = { 5,	0,		SEN(vserver),			"vserver"		},
[314] = { 3,	0,		SEN(ioprio_set),		"ioprio_set"		},
[315] = { 2,	0,		SEN(ioprio_get),		"ioprio_get"		},
[316] = { 0,	TD,		SEN(inotify_init),		"inotify_init"		},
[317] = { 3,	TD,		SEN(inotify_add_watch),		"inotify_add_watch"	},
[318] = { 2,	TD,		SEN(inotify_rm_watch),		"inotify_rm_watch"	},
[319] = { 6,	TM,		SEN(mbind),			"mbind"			},
[320] = { 5,	TM,		SEN(get_mempolicy),		"get_mempolicy"		},
[321] = { 3,	TM,		SEN(set_mempolicy),		"set_mempolicy"		},
[322] = { 4,	TD|TF,		SEN(openat),			"openat"		},
[323] = { 3,	TD|TF,		SEN(mkdirat),			"mkdirat"		},
[324] = { 4,	TD|TF,		SEN(mknodat),			"mknodat"		},
[325] = { 5,	TD|TF,		SEN(fchownat),			"fchownat"		},
[326] = { 3,	TD|TF,		SEN(futimesat),			"futimesat"		},
[327] = { 4,	TD|TF|TFST|TSTA,SEN(fstatat64),			"fstatat64"		},
[328] = { 3,	TD|TF,		SEN(unlinkat),			"unlinkat"		},
[329] = { 4,	TD|TF,		SEN(renameat),			"renameat"		},
[330] = { 5,	TD|TF,		SEN(linkat),			"linkat"		},
[331] = { 3,	TD|TF,		SEN(symlinkat),			"symlinkat"		},
[332] = { 4,	TD|TF,		SEN(readlinkat),		"readlinkat"		},
[333] = { 3,	TD|TF,		SEN(fchmodat),			"fchmodat"		},
[334] = { 3,	TD|TF,		SEN(faccessat),			"faccessat"		},
[335] = { 6,	TD,		SEN(pselect6),			"pselect6"		},
[336] = { 5,	TD,		SEN(ppoll),			"ppoll"			},
[337] = { 1,	TP,		SEN(unshare),			"unshare"		},
[338] = { 2,	0,		SEN(set_robust_list),		"set_robust_list"	},
[339] = { 3,	0,		SEN(get_robust_list),		"get_robust_list"	},
[340] = { 6,	TD,		SEN(splice),			"splice"		},
[341] = { 6,	TD,		SEN(sync_file_range2),		"sync_file_range2"	},
[342] = { 4,	TD,		SEN(tee),			"tee"			},
[343] = { 4,	TD,		SEN(vmsplice),			"vmsplice"		},
[344] = { 6,	TM,		SEN(move_pages),		"move_pages"		},
[345] = { 3,	0,		SEN(getcpu),			"getcpu"		},
[346] = { 6,	TD,		SEN(epoll_pwait),		"epoll_pwait"		},
[347] = { 4,	0,		SEN(kexec_load),		"kexec_load"		},
[348] = { 4,	TD|TF,		SEN(utimensat),			"utimensat"		},
[349] = { 3,	TD|TS,		SEN(signalfd),			"signalfd"		},
[350] = { 2,	TD,		SEN(timerfd_create),		"timerfd_create"	},
[351] = { 1,	TD,		SEN(eventfd),			"eventfd"		},
[352] = { 6,	TD,		SEN(fallocate),			"fallocate"		},
[353] = { 4,	TD,		SEN(timerfd_settime),		"timerfd_settime"	},
[354] = { 2,	TD,		SEN(timerfd_gettime),		"timerfd_gettime"	},
[355] = { 4,	TD|TS,		SEN(signalfd4),			"signalfd4"		},
[356] = { 2,	TD,		SEN(eventfd2),			"eventfd2"		},
[357] = { 1,	TD,		SEN(epoll_create1),		"epoll_create1"		},
[358] = { 3,	TD,		SEN(dup3),			"dup3"			},
[359] = { 2,	TD,		SEN(pipe2),			"pipe2"			},
[360] = { 1,	TD,		SEN(inotify_init1),		"inotify_init1"		},
[361] = { 5,	TD,		SEN(preadv),			"preadv"		},
[362] = { 5,	TD,		SEN(pwritev),			"pwritev"		},
[363] = { 4,	TP|TS,		SEN(rt_tgsigqueueinfo),		"rt_tgsigqueueinfo"	},
[364] = { 5,	TD,		SEN(perf_event_open),		"perf_event_open"	},
[365] = { 5,	TN,		SEN(recvmmsg),			"recvmmsg"		},
[366] = { 4,	TN,		SEN(accept4),			"accept4"		},
[367] = { 2,	TD,		SEN(fanotify_init),		"fanotify_init"		},
[368] = { 6,	TD|TF,		SEN(fanotify_mark),		"fanotify_mark"		},
[369] = { 4,	0,		SEN(prlimit64),			"prlimit64"		},
[370] = { 5,	TD|TF,		SEN(name_to_handle_at),		"name_to_handle_at"	},
[371] = { 3,	TD,		SEN(open_by_handle_at),		"open_by_handle_at"	},
[372] = { 2,	0,		SEN(clock_adjtime),		"clock_adjtime"		},
[373] = { 1,	TD,		SEN(syncfs),			"syncfs"		},
[374] = { 4,	TN,		SEN(sendmmsg),			"sendmmsg"		},
[375] = { 2,	TD,		SEN(setns),			"setns"			},
[376] = { 6,	0,		SEN(process_vm_readv),		"process_vm_readv"	},
[377] = { 6,	0,		SEN(process_vm_writev),		"process_vm_writev"	},
[378] = { 5,	0,		SEN(kcmp),			"kcmp"			},
[379] = { 3,	TD,		SEN(finit_module),		"finit_module"		},
[380] = { 3,	0,		SEN(sched_setattr),		"sched_setattr"		},
[381] = { 4,	0,		SEN(sched_getattr),		"sched_getattr"		},
[382] = { 5,	TD|TF,		SEN(renameat2),			"renameat2"		},
[383] = { 3,	0,		SEN(seccomp),			"seccomp"		},
[384] = { 3,	0,		SEN(getrandom),			"getrandom"		},
[385] = { 2,	TD,		SEN(memfd_create),		"memfd_create"		},
[386] = { 3,	TD,		SEN(bpf),			"bpf"			},
[387] = { 5,	TD|TF|TP|SE|SI,	SEN(execveat),			"execveat"		},
[388] = { 1,	TD,		SEN(userfaultfd),		"userfaultfd"		},
[389] = { 2,	0,		SEN(membarrier),		"membarrier"		},
[390] = { 3,	TM,		SEN(mlock2),			"mlock2"		},
[391] = { 6,	TD,		SEN(copy_file_range),		"copy_file_range"	},
[392] = { 6,	TD,		SEN(preadv2),			"preadv2"		},
[393] = { 6,	TD,		SEN(pwritev2),			"pwritev2"		},
[394] = { 4,	TM|SI,		SEN(pkey_mprotect),		"pkey_mprotect"		},
[395] = { 2,	0,		SEN(pkey_alloc),		"pkey_alloc"		},
[396] = { 1,	0,		SEN(pkey_free),			"pkey_free"		},
[397] = { 5,	TD|TF|TSTA,	SEN(statx),			"statx"			},

#ifdef __ARM_EABI__
# define ARM_FIRST_SHUFFLED_SYSCALL 400
#else
# define SYS_socket_subcall	400
# include "subcall.h"
# define ARM_FIRST_SHUFFLED_SYSCALL (SYS_ipc_subcall + SYS_ipc_nsubcalls)
#endif /* !__ARM_EABI__ */

/*
 * __ARM_NR_cmpxchg (0x000ffff0).
 * Remapped by shuffle_scno() to be directly after ordinary syscalls
 * in this table.
 */
[ARM_FIRST_SHUFFLED_SYSCALL    ] = { 5,	0,	SEN(printargs),	"cmpxchg"		},

/*
 * ARM specific syscalls. Encoded with scno 0x000f00xx.
 * Remapped by shuffle_scno() to be directly after __ARM_NR_cmpxchg.
 */
[ARM_FIRST_SHUFFLED_SYSCALL+1+0] = { },
[ARM_FIRST_SHUFFLED_SYSCALL+1+1] = { 5,	0,	SEN(printargs),	"breakpoint"		},
[ARM_FIRST_SHUFFLED_SYSCALL+1+2] = { 5,	0,	SEN(printargs),	"cacheflush"		},
[ARM_FIRST_SHUFFLED_SYSCALL+1+3] = { 5,	0,	SEN(printargs),	"usr26"			},
[ARM_FIRST_SHUFFLED_SYSCALL+1+4] = { 5,	0,	SEN(printargs),	"usr32"			},
[ARM_FIRST_SHUFFLED_SYSCALL+1+5] = { 5,	0,	SEN(printargs),	"set_tls"		},
#define ARM_LAST_SPECIAL_SYSCALL 5
