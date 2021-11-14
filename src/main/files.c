#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "stack.h"
#include "vm.h"
#include "files.h"
#include "structures.h"
#include "prims.h"


#pragma GCC push_options
#pragma GCC optimize("align-functions=16")


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
    FILE *thefile = fopen( filename, "a" );
    if( thefile == NULL ) {
        sfs error = sfscatprintf(sfsempty(), "error in %%zu: unable to open file: %s\n", filename );
        runtimefault( error );
    } else {
        fseek( thefile, 0, SEEK_END  );
        fprintf( thefile, "%s", inputstring );
    }
    fpos_t finaloffset;
    fgetpos( thefile, &finaloffset );
    fclose(thefile);
    push_int( finaloffset.__pos );
}


// (s:inputstring s:filename i:initialoffset -- i:finaloffset )
prim(fwrite) {
    require_int initialoffset = pop_int;
    require_string filename = pop_string;
    require_string inputstring = pop_string;
    FILE *thefile = fopen( filename, "w" );
    if( thefile == NULL ) {
        sfs error = sfscatprintf(sfsempty(), "error in %%zu: unable to open file: %s\n", filename );
        runtimefault( error );
    } else {
        fseek( thefile, initialoffset, SEEK_SET );
        fprintf( thefile, "%s", inputstring );
        
    }
    fpos_t finaloffset;
    fgetpos( thefile, &finaloffset  );
    fclose(thefile);
    push_int( finaloffset.__pos );
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

// #endregion
