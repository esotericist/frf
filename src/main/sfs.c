#include <gc.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include "sfs.h"


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


sfs alloc_sfs( size_t initlen ) {
    sfs s;
    void *sh;
    sh = GC_malloc_atomic( hdrlen + initlen + 1 );
    if( sh == NULL ) {
        return NULL;
    }
    memset(sh, 0, hdrlen+initlen+1);
    s = (char*)sh+hdrlen;
    sfssetlen( s, initlen );
    return s;
}

sfs sfsnewlen( const char *init, size_t initlen ) {
    sfs s = alloc_sfs( initlen );
    if (initlen && init)
        memcpy(s, init, initlen);
    s[initlen] = '\0';
    return s;
}

sfs sfsright( sfs s, size_t len ) {
    size_t offset = sfslen( s ) - len;
    if( offset < 0 ) {
        len = sfslen( s);
        offset = 0;
    }
    sfs new_s = alloc_sfs( len );
    memcpy( new_s, s + offset, len );
    return new_s;
}

sfs sfsnew( const char *init ) {
    size_t initlen = (init == NULL) ? 0 : strlen(init);
    return sfsnewlen(init, initlen);

}
sfs sfsempty( void ) {
    return sfsnewlen("",0);
}

sfs sfscatlen(sfs s, const void *t, size_t secondlen) {
    size_t firstlen = sfslen( s );
    sfs new_s = alloc_sfs( firstlen + secondlen );
    memcpy( new_s, s, firstlen );
    memcpy( new_s + firstlen, t, secondlen );
    new_s[firstlen + secondlen] = '\0';
    sfssetlen( new_s, firstlen + secondlen );
    return new_s;
}

sfs sfscatc(sfs s, const char *t) {
    return sfscatlen(s, t, strlen(t));
}

sfs sfscatsfs(sfs s, const sfs t) {
    return sfscatlen( s, t, sfslen( t) );
}

sfs sfstrim(sfs s, const char *cset) {
    sfs new_s = alloc_sfs( sfslen( s ) );
    char *start, *end, *sp, *ep;
    size_t len;

    sp = start = s;
    ep = end = s+sfslen(s)-1;
    while(sp <= end && strchr(cset, *sp)) sp++;
    while(ep > sp && strchr(cset, *ep)) ep--;
    len = (sp > ep) ? 0 : ((ep-sp)+1);
    memcpy(new_s, sp, len);
    new_s[len] = '\0';
    sfssetlen(new_s,len);
    return new_s;
}


sfs sfstolower(sfs s) {
    sfs new_s = alloc_sfs( sfslen (s) );
    size_t len = sfslen(s), j;
    for (j = 0; j < len; j++) new_s[j] = tolower(s[j]);
    return new_s;
}

sfs sfstoupper(sfs s) {
    sfs new_s = alloc_sfs( sfslen (s) );
    size_t len = sfslen(s), j;
    for (j = 0; j < len; j++) new_s[j] = toupper(s[j]);
    return new_s;
}


int sfscmp(const sfs s1, const sfs s2) {
    size_t l1, l2, minlen;
    l1 = sfslen(s1);
    l2 = sfslen(s2);
    minlen = (l1 < l2) ? l1 : l2;
    if( l1 > l2 ) {
        return s1[minlen];
    } else if( l1 < l2 ) {
        return - (s2[minlen]);
    }
    for( size_t i = 0; i < minlen ; i++ ) {
        if( s1[i] != s2[i] ) {
            return s2[i] - s1[i];
        }
    }
    return 0;
}

/* Helper for sfscatlonglong() doing the actual number -> string
 * conversion. 's' must point to a string with room for at least
 * SFS_LLSTR_SIZE bytes.
 *
 * The function returns the length of the null-terminated string
 * representation stored at 's'. */
#define SFS_LLSTR_SIZE 21
int sfsll2str(char *s, long long value) {
    char *p, aux;
    unsigned long long v;
    size_t l;

    /* Generate the string representation, this method produces
     * an reversed string. */
    v = (value < 0) ? -value : value;
    p = s;
    do {
        *p++ = '0'+(v%10);
        v /= 10;
    } while(v);
    if (value < 0) *p++ = '-';

    /* Compute length and add null term. */
    l = p-s;
    *p = '\0';

    /* Reverse the string. */
    p--;
    while(s < p) {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }
    return l;
}

/* Identical sfsll2str(), but for unsigned long long type. */
int sfsull2str(char *s, unsigned long long v) {
    char *p, aux;
    size_t l;

    /* Generate the string representation, this method produces
     * an reversed string. */
    p = s;
    do {
        *p++ = '0'+(v%10);
        v /= 10;
    } while(v);

    /* Compute length and add null term. */
    l = p-s;
    *p = '\0';

    /* Reverse the string. */
    p--;
    while(s < p) {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }
    return l;
}

sfs sfsfromlonglong(long long value) {
    char buf[SFS_LLSTR_SIZE];
    int len = sfsll2str(buf,value);
    return sfsnewlen(buf,len);
}

sfs sfsrange( const sfs s, ssize_t start, ssize_t end) {
    size_t newlen, len = sfslen(s);
    sfs new_s;    

    if (len == 0) return NULL;
    if (start < 0) {
        start = len+start;
        if (start < 0) start = 0;
    }
    if (end < 0) {
        end = len+end;
        if (end < 0) end = 0;
    }
    newlen = (start > end) ? 0 : (end-start)+1;
    if (newlen != 0) {
        if (start >= (ssize_t)len) {
            newlen = 0;
        } else if (end >= (ssize_t)len) {
            end = len-1;
            newlen = (start > end) ? 0 : (end-start)+1;
        }
    } else {
        start = 0;
    }
    new_s = alloc_sfs( newlen );
    if( newlen ) {
        memcpy(new_s, s+start, newlen);
        new_s[newlen] = 0;
        sfssetlen(new_s,newlen);
    }
    return new_s;
}

sfs sfscatfmt(sfs s, char const *fmt, ...);

/* Like sfscatprintf() but gets va_list instead of being variadic. */
sfs sfscatvprintf(sfs s, const char *fmt, va_list ap) {
    va_list cpy;
    char staticbuf[1024], *buf = staticbuf, *t;
    size_t buflen = strlen(fmt)*2;

    /* We try to start using a static buffer for speed.
     * If not possible we revert to heap allocation. */
    if (buflen > sizeof(staticbuf)) {
        buf = GC_malloc_atomic(buflen);
        if (buf == NULL) return NULL;
    } else {
        buflen = sizeof(staticbuf);
    }

    /* Try with buffers two times bigger every time we fail to
     * fit the string in the current buffer size. */
    while(1) {
        buf[buflen-2] = '\0';
        va_copy(cpy,ap);
        vsnprintf(buf, buflen, fmt, cpy);
        va_end(cpy);
        if (buf[buflen-2] != '\0') {
            if (buf != staticbuf) GC_free(buf);
            buflen *= 2;
            buf = GC_malloc_atomic(buflen);
            if (buf == NULL) return NULL;
            continue;
        }
        break;
    }

    /* Finally concat the obtained string to the SFS string and return it. */
    t = sfscatc(s, buf);
    if (buf != staticbuf) GC_free(buf);
    return t;
}

/* Append to the sfs string 's' a string obtained using printf-alike format
 * specifier.
 *
 * After the call, the modified sfs string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call.
 *
 * Example:
 *
 * s = sfsnew("Sum is: ");
 * s = sfscatprintf(s,"%d+%d = %d",a,b,a+b).
 *
 * Often you need to create a string from scratch with the printf-alike
 * format. When this is the need, just use sfsempty() as the target string:
 *
 * s = sfscatprintf(sfsempty(), "... your format ...", args);
 */
sfs sfscatprintf(sfs s, const char *fmt, ...) {
    va_list ap;
    char *t;
    va_start(ap, fmt);
    t = sfscatvprintf(s,fmt,ap);
    va_end(ap);
    return t;
}


size_t sfsinstr( sfs strtosearch, sfs key, bool reverse ) {
    size_t keylen = sfslen(key);
    size_t searchedlen = sfslen(strtosearch);
    if( keylen > searchedlen) {
        return 0;
    }
    size_t start = 0, end = (searchedlen - keylen) + 1, inc = 1;
    if( reverse ) {
        start = end;
        end = 0;
        inc = -1;
    }
    for( size_t i = start; i != end ; i = i + inc ) {
        if( memcmp( key, strtosearch + i, keylen ) == 0 ) {
            return i + 1;
        }
    }

    return 0;
}

size_t sfsmatchcount( sfs strtosearch, sfs key ) {
    size_t keylen = sfslen(key);
    size_t searchedlen = sfslen(strtosearch);
    size_t count = 0;
    if( keylen > searchedlen) {
        return 0;
    }
    for( size_t i = 0; i < (searchedlen - keylen + 1) ; i++ ) {
        if( memcmp( key, strtosearch + i, keylen ) == 0 ) {
            count++;
        }
    }
    return count;
} 


/* Split 's' with separator in 'sep'. An array
 * of sfs strings is returned. *count will be set
 * by reference to the number of tokens returned.
 *
 * On out of memory, zero length string, zero length
 * separator, NULL is returned.
 *
 * Note that 'sep' is able to split a string using
 * a multi-character separator. For example
 * sfssplit("foo_-_bar","_-_"); will return two
 * elements "foo" and "bar".
 *
 * This version of the function is binary-safe but
 * requires length arguments. sdssplit() is just the
 * same function but for zero-terminated strings.
 */
sfs *sfssplitlen(const char *s, ssize_t len, const char *sep, int seplen, size_t *count) {
    int elements = 0, slots = 5;
    long start = 0, j;
    sfs *tokens;

    if (seplen < 1 || len < 0) return NULL;

    tokens = GC_malloc(sizeof(sfs)*slots);
    if (tokens == NULL) return NULL;

    if (len == 0) {
        *count = 0;
        return tokens;
    }
    for (j = 0; j < (len-(seplen-1)); j++) {
        /* make sure there is room for the next element and the final one */
        if (slots < elements+2) {
            sfs *newtokens;

            slots *= 2;
            newtokens = GC_realloc(tokens,sizeof(sfs)*slots);
            if (newtokens == NULL) goto cleanup;
            tokens = newtokens;
        }
        /* search the separator */
        if ((seplen == 1 && *(s+j) == sep[0]) || (memcmp(s+j,sep,seplen) == 0)) {
            tokens[elements] = sfsnewlen(s+start,j-start);
            if (tokens[elements] == NULL) goto cleanup;
            elements++;
            start = j+seplen;
            j = j+seplen-1; /* skip the separator */
        }
    }
    /* Add the final element. We are sure there is room in the tokens array. */
    tokens[elements] = sfsnewlen(s+start,len-start);
    if (tokens[elements] == NULL) goto cleanup;
    elements++;
    *count = elements;
    return tokens;

cleanup:
    {
        return NULL;
    }
}

sfs *sfssplit(const sfs s, sfs sep, size_t *count) {
    return sfssplitlen( s, sfslen( s), sep, sfslen( sep ), count );
}