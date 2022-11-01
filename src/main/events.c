#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include "stack.h"
#include "vm.h"
#include "events.h"
#include "structures.h"
#include "prims.h"

uv_loop_t *uvloop;

// definitions for various event things
typedef struct timer {
    sfs timer_handle;
    proc *P;
} event_timer;

sListType *timer_table[slist_hash_size];

// swiped from https://stackoverflow.com/a/47229318 on 2021/11/18
//
/* The original code is public domain -- Will Hartung 4/9/09 */
/* Modifications, public domain as well, by Antti Haapala, 11/10/17
   - Switched to getc on 5/23/19 */

ssize_t getline(char **lineptr, size_t *n, FILE *stream) {
    size_t pos;
    int c;

    if (lineptr == NULL || stream == NULL || n == NULL) {
        errno = EINVAL;
        return -1;
    }

    c = getc(stream);
    if (c == EOF) {
        return -1;
    }

    if (*lineptr == NULL) {
        *lineptr = GC_malloc(128);
        if (*lineptr == NULL) {
            return -1;
        }
        *n = 128;
    }

    pos = 0;
    while(c != EOF) {
        if (pos + 1 >= *n) {
            size_t new_size = *n + (*n >> 2);
            if (new_size < 128) {
                new_size = 128;
            }
            char *new_ptr = GC_realloc(*lineptr, new_size);
            if (new_ptr == NULL) {
                return -1;
            }
            *n = new_size;
            *lineptr = new_ptr;
        }

        ((unsigned char *)(*lineptr))[pos ++] = c;
        if (c == '\n') {
            break;
        }
        c = getc(stream);
    }

    (*lineptr)[pos] = '\0';
    return pos;
}

void events_initialization() {
    uvloop = GC_malloc( sizeof(uv_loop_t));
    uv_loop_init(uvloop);
    sglib_hashed_sListType_init(timer_table);
}

void events_teardown() {
    uv_loop_close(uvloop);
    GC_free(uvloop);
}

void events_run() {
    uv_run(uvloop, UV_RUN_DEFAULT);
}

#pragma GCC push_options
#pragma GCC optimize("align-functions=16")

// #region libuv helper prims

// stub: eventually we want a monotonic thing like erlang, but that's work
prim(now) {
    push_int(uv_hrtime());
}

// #endregion

// #region event prims

atom(event_timer)
atom(complete)

void event_timer_callback( uv_timer_t *req ) {
    sListType *elem = (sListType *)(void *)req->data;
    proc *target_P = (proc *)(uintptr_t) elem->ptr;

    struct array_span *arr = newarrayspan(3);
    dp_put_atom(&arr->elems[0], a_event_timer);
    dp_put_string(&arr->elems[1], elem->s);
    dp_put_atom(&arr->elems[2], a_complete);

    process_addmessage( target_P, arr);

    /* sglib_hashed_sListType_delete(timer_table, elem);
    sglib_hashed_sListType_add(timer_table, elem);
    */
}


prim(event_timer) {
    require_string handle = pop_string;
    require_int delay_in_ms = pop_int;
    uv_timer_t *timer_handle = GC_malloc(sizeof(uv_timer_t));
    uv_timer_init(uvloop, timer_handle);
    uv_timer_start(timer_handle,event_timer_callback, delay_in_ms, 0);

    sListType *timer = alloc_slist();
    timer->s = handle;
    timer->ptr = (uintptr_t)(proc *) P;
    
    timer_handle->data = timer;
}

// #endregion

// #region ipc prims


atom(message)

prim(message_send) {
    needstack(2)
    struct datapoint val = pop_dp(P);
    require_int target_pid = pop_int;

    proc * target_P = process_from_pid(P->node, target_pid);
    if( target_P == NULL ) {
        runtimefault( "error in %zu: process doesn't exist" )
    }

    struct array_span *arr = newarrayspan(3);
    dp_put_atom(&arr->elems[0], a_message);
    dp_put_int(&arr->elems[1], P->pid);
    arr->elems[2].u_val = val.u_val;

    process_addmessage( target_P, arr);
}

atom(empty)

prim(message_receive) {
    if(process_messagecount(P) > 0) {
        push_tup(process_fetchmessage(P));
    } else {
        push_atom(a_empty);
    } 
}

prim(message_waitfor) {
    if(process_messagecount(P) == 0) { 
        process_setinactive(P);
        P->currentop--;
    } else {
        push_int(process_messagecount(P));
    }    
}

prim(message_count) {
    push_int(process_messagecount(P));
}


// #endregion

// #region file manipulation prims


// ( s:filename -- i:size )
prim(fsize) {
    require_string filename = pop_string;
    struct stat statbuf;
    if( stat( filename, &statbuf) == -1 ) {
        runtimefault("error in %zu: unable to open file");
    }
    push_int(statbuf.st_size);
}

// ( -- s:cwd )
prim(fcwd) {
    char buffer[BUFSIZ];
    if( getcwd(buffer, BUFSIZ) ) {
        push_string( sfsnew( buffer ) );
    }
}


// (s:filename -- )
prim(frm) {
    require_string filename = pop_string;
    int result = remove( filename );
    push_int(result);
}

// (s:original s:newname -- )
prim(fmv) {
    require_string newname = pop_string;
    require_string original = pop_string;
    int result = rename( original, newname );
    push_int(result);
}

// (s:pathname -- arr/int )
prim(fgetdir) {
    require_string pathname = pop_string;
    DIR *thedir = opendir( pathname );
    struct dirent * entry;
    struct array_span *arr = newarrayspan(0);
    if(thedir != NULL) {
        while(( entry = readdir(thedir)) != NULL ) {
            sfs fname = sfsnew(entry->d_name);
            sfs fullname = sfscatprintf( pathname, "/%s", fname  );
            struct stat statbuf;
            stat( fullname, &statbuf);
            if( statbuf.st_mode & S_IFDIR) {
                fname = sfscatc(fname, "/" );
            } 
            arr = grow_span(arr);
            struct datapoint dp;
            dp_put_string( &dp, fname );
            arr->elems[arr->size - 1] = dp;
        }
        push_arr(arr);
    } else {
        push_int(0);
    }
}

// (s:inputstring s:filename -- i:finaloffset )
prim(fappend) {
    require_string filename = pop_string;
    require_string inputstring = pop_string;
    FILE *thefile = fopen( filename, "ab" );
    if( thefile == NULL ) {
        sfs error = sfscatprintf(sfsempty(), "error in %%zu: unable to open file: %s\n", filename );
        runtimefault( error );
    } else {
        fseek( thefile, 0, SEEK_END  );
        fprintf( thefile, "%s", inputstring );
    }
    fclose(thefile);
    struct stat statbuf;
    stat( filename, &statbuf);
    push_int( statbuf.st_size );
}


// (s:inputstring s:filename i:initialoffset -- i:finaloffset )
prim(fwrite) {
    require_int initialoffset = pop_int;
    require_string filename = pop_string;
    require_string inputstring = pop_string;
    FILE *thefile = fopen( filename, "wb" );
    if( thefile == NULL ) {
        sfs error = sfscatprintf(sfsempty(), "error in %%zu: unable to open file: %s\n", filename );
        runtimefault( error );
    } else {
        fseek( thefile, initialoffset, SEEK_SET );
        fprintf( thefile, "%s", inputstring );
        
    }
    fclose(thefile);
    struct stat statbuf;
    stat( filename, &statbuf);
    push_int( statbuf.st_size );
}

// ( s:filename i:initialoffset s:delimiter -- i:finaloffset s:outputstring )
prim(freadto) {
    require_string delimiter = pop_string;
    require_int initialoffset = pop_int;
    require_string filename = pop_string;
    if (!sfscmp( delimiter, sfsnew("$EOF$") )) {
        delimiter = sfsempty();
    } else {

    }
    struct stat statbuf;
    if( stat( filename, &statbuf) == -1 ) {
        runtimefault("error in %zu: unable to open file");
    }
    if( initialoffset > statbuf.st_size ) {
        push_string(sfsempty());
        push_int(statbuf.st_size);
    }
    sfs finalstring = sfsempty();
    size_t finaloffset = 0;
    FILE *thefile = fopen( filename, "r" );
    if( thefile == NULL ) {
        sfs error = sfscatprintf(sfsempty(), "error in %%zu: unable to open file: %s\n", filename );
        runtimefault( error );
    } else {
        char buffer[BUFSIZ];
        fseek( thefile, initialoffset, SEEK_SET  );
        size_t i = 0;
        do{
            int c = fgetc(thefile);
            if( feof(thefile) || c == delimiter[0] ) {
                if( i < BUFSIZ ) {
                    buffer[i] = 0;
                }
                finalstring = sfscatsfs(finalstring, sfsnew(buffer));
                finaloffset = initialoffset + i + 1;
                break;
            }
            buffer[i] = c;
            i++;
            if(i == BUFSIZ ) {
                finalstring = sfscatsfs(finalstring, sfsnewlen(buffer, i - 1));
                initialoffset = initialoffset + i;
                i = 0;
            }
        } while ( true );
        fclose(thefile);
    }
    push_string(finalstring);
    push_int(finaloffset);
}

prim(readline) {
    char *instring = NULL;
    size_t buffer_size = 0;
    ssize_t read_size;

    read_size = getline( &instring, &buffer_size, stdin );
    if( read_size > 0 ) {
        push_string(sfsnewlen( instring, read_size ));
    } else {
        push_string(sfsempty());
    }
}



prim2(print, .)
{
    needstack(1) struct datapoint dp = pop_dp(P);
    size_t type = checktype(dp);
    if (type == a_type_string)
    {
        printf("%s", dp_get_string(dp));
    }
    else
    {
        printf("%s", formatobject(P, dp));
    }
}
// #endregion
