#ifndef SFS_H_
#define SFS_H_
#include <stdint.h>
#include <stdarg.h>

/* 
 * simple functional strings library, by esotericist (eso at esotericist dot org)
 * because i realized after i started on this project i needed immutable strings
 * but i started with a dynamic string library instead
 * and didn't want to rearchitect it all from scratch
 * 
 * some portions derived from (and with continuance of licensing thereof):
 * SDSLib 2.0 -- A C dynamic strings library
 *
 * fetched on 2021/09/30 from: 
 * https://github.com/antirez/sds/tree/fb463145c9c245636feb28b5aac0fc897e16f67e
 *
 * Copyright (c) 2006-2015, Salvatore Sanfilippo <antirez at gmail dot com>
 * Copyright (c) 2015, Oran Agra
 * Copyright (c) 2015, Redis Labs, Inc
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

struct __attribute__ ((__packed__)) sfshdr {
    size_t len;
    char buf[];
};

#define hdrlen sizeof( struct sfshdr )

typedef char *sfs;

sfs sfsempty( void );
sfs sfsnew( const char *init );
static inline sfs sfsdup( sfs s ) { return s; }
sfs sfsnewlen( const char *init, size_t initlen );

sfs sfsright( sfs s, size_t len);

sfs sfscatlen(sfs s, const void *t, size_t len);
sfs sfscatc(sfs s, const char *t);
sfs sfscatsfs(sfs s, const sfs t);
sfs sfstrim(sfs s, const char *cset);
void sfstolower(sfs s);

int sfscmp(const sfs s1, const sfs s2);
sfs sfsfromlonglong(long long value);

sfs sfscatfmt(sfs s, char const *fmt, ...);
sfs sfsrange(sfs s, ssize_t start, ssize_t end);

sfs sfscatvprintf(sfs s, const char *fmt, va_list ap);
sfs sfscatprintf(sfs s, const char *fmt, ...);


static inline size_t sfslen(const sfs s) {
    return ((struct sfshdr *)((s)-(hdrlen)))->len;
}

static inline void sfssetlen( sfs s, size_t newlen ) {
    ((struct sfshdr *)((s)-(hdrlen)))->len = newlen;
}

#define sds sfs

#define sdsnew sfsnew
#define sdsdup sfsdup
#define sdsnewlen sfsnewlen
#define sdstrim sfstrim
#define sdscat sfscatc
#define sdscatlen sfscatlen
#define sdscatsds sfscatsfs
#define sdsfromlonglong sfsfromlonglong
#define sdsempty sfsempty
#define sdscmp sfscmp
#define sdsrange sfsrange
#define sdstolower sfstolower
#define sdslen sfslen
#define sdscatprintf sfscatprintf

#endif