#include "atoms.h"
#include <stdlib.h>

size_t atomcount = 0;

// atoms are per the erlang concept. integer representations of strings.
// note that, like erlang, the definition of a given atom persists for the lifetime of the
// node, and atoms are never released.
// unbounded creation of atoms will eventually kill the things. at present, a limit of
// 32768 atoms is enforced by the rigidity of sglib; hash maps cannot be resized at
// run-time as of yet. eventually i want to have this expand as needed, but just as with
// erlang, unbounded atom creation will always eventually kill the crab.

sListType *atom_table[slist_hash_size];
sds atom_strings[slist_hash_size];

// in normal usage, atoms-as-datatype are restricted to alphanumeric (lowercase only), period,
// slash, and underscore. all other characters are dropped. string should be converted to
// lowercase in most cases.
// we technically can use atoms to hold arbitrary strings, but that's not typical behavior
sds sanitizeatomstring( sds str ) {
    int i;
    char c;
    sds outstr = sdsempty();
    for( i = 0; i < sdslen( str) ; i++ ) {
        c = str[i];
        switch (c)
        {
            case '.':
            case  '/':
            case '0' ... '9':
            case '_':
            case 'a' ... 'z': {
                sdscat(outstr, sdsnewlen( &c, 1 ) );
                continue;
            }
        }
    }
    return outstr;
}

// look up the value for a string you expect already exists as an atom 
// but don't want to ensure exists
size_t verifyatom( sds str ) {
    if (str_eq( sdsempty(), str )) {
        return 0;
    }
    struct slist *it = alloc_slist();
    it->s = str;
    struct slist *tmp = sglib_hashed_sListType_find_member( atom_table, it );
    if( tmp == NULL ) {
        return 0;
    }
    return tmp->data;
}

// ensure a string exists as an atom. use with care: this does not perform any validation.
// normally you want to use newatom() to ensure the atom meets our specifications;
// in particular case insensitive contexts (primary atom uses) need to be converted to lowercase.
size_t stringtoatom( sds str ) {
    int i = verifyatom( str );
    if( i ) {
        return i;
    }
    sListType *newatom = alloc_slist();
    newatom->s = str;
    newatom->data = atomcount++;
    sglib_hashed_sListType_add( atom_table, newatom );
    atom_strings[newatom->data] = str;
    return newatom->data;
}

// standard atom creation/lookup mechanism. 
// registers a string as an atom if it isn't already registered, returns existing value if it already is.
// leading/trailing spaces are stripped, and string is converted to lowercase because standard atoms
// are intended to be effectively case insensitive (but always displayed as lowercase when printed).
size_t newatom( sds str ) {
    str = sdstrim(  str , " " );
    sdstolower( str ); 
    return stringtoatom( sanitizeatomstring( str ) ) ;
}

sds atomtostring( size_t atom ) {
    return atom_strings[ atom ];
}


// used as part of a registration mechanism so i can specify atoms in .c files before functions
// which need those atoms defined.
// see atoms.h for more details on pre-registration
void preregisteratom( size_t *var,  char *s) {
    preatoms = realloc( preatoms, sizeof( struct preatom) * (numpreatoms + 1));
    preatoms[numpreatoms].thevar = var;
    preatoms[numpreatoms].atom = s;
    numpreatoms++;
}

// sets up baseline atom definitions we know we need, including pre-registered atoms
void atoms_init() {
    stringtoatom( sdsempty() );
    for(int i = 0; i < numpreatoms ; i++ ) {
        *(preatoms[i].thevar) = newatom( sdsnew( preatoms[i].atom ) );
    }
    free(preatoms);
    a__dsign = stringtoatom( sdsnew( "$" ) );
    a__dquote = stringtoatom( sdsnew( "\"" ) );
    a__squote = stringtoatom( sdsnew( "'" ) );
    a__backslash = stringtoatom( sdsnew( "\\" ) );
    a__colon = stringtoatom( sdsnew( ":" ) );
    a__semicolon = stringtoatom( sdsnew( ";" ) );
    a__parenl = stringtoatom( sdsnew( "(" ) );
    a__parenr = stringtoatom( sdsnew( ")" ) );
    a__space = stringtoatom( sdsnew( " " ) );
}