#ifndef SFS_H_
#define SFS_H_
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

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
sfs sfsnewlen( const char *init, size_t initlen );

sfs sfsright( sfs s, size_t len);

// result = sfs + (len characters from start of cstring t)
sfs sfscatlen(sfs s, const void *t, size_t len);

// result = sfs s + c string t
sfs sfscatc(sfs s, const char *t);

// result = sfs s + sfs t
sfs sfscatsfs(sfs s, const sfs t);

// result = sfs s without leading/trailing characters contained in c string cset
sfs sfstrim(sfs s, const char *cset);
sfs sfstrimlead(sfs s, const char *cset);
sfs sfstrimtail(sfs s, const char *cset);

sfs sfstolower(sfs s);
sfs sfstoupper(sfs s);

int sfscmp(const sfs s1, const sfs s2);
sfs sfsfromlonglong(long long value);

sfs sfscatfmt(sfs s, char const *fmt, ...);
sfs sfsrange(sfs s, ssize_t start, ssize_t end);

sfs sfscatvprintf(sfs s, const char *fmt, va_list ap);
sfs sfscatprintf(sfs s, const char *fmt, ...);

size_t sfsinstr( sfs strtosearch, sfs key, bool reverse );
size_t sfsmatchcount( sfs strtosearch, sfs key );
sfs sfssubst( sfs s, sfs sep, sfs repl );

sfs *sfssplitlen(const char *s, ssize_t len, const char *sep, int seplen, size_t *count);
sfs *sfssplit(const sfs s, sfs sep, size_t *count);


static inline size_t sfslen(const sfs s) {
    return ((struct sfshdr *)((s)-(hdrlen)))->len;
}

static inline void sfssetlen( sfs s, size_t newlen ) {
    ((struct sfshdr *)((s)-(hdrlen)))->len = newlen;
}

#endif