
#include "datatypes.h"

SGLIB_DEFINE_SORTED_LIST_FUNCTIONS(iListType, ILIST_COMPARATOR, next_ptr)
SGLIB_DEFINE_SORTED_LIST_FUNCTIONS(sListType, SLIST_COMPARATOR, next_ptr)
SGLIB_DEFINE_HASHED_CONTAINER_FUNCTIONS(sListType, slist_hash_size, slist_hash_function )


struct slist* alloc_slist() {
    return GC_malloc( sizeof( struct slist ));
}


unsigned int slist_hash_function(sListType *e) {
    // cribbed from http://www.cse.yorku.ca/~oz/hash.html
    int c;
    unsigned long hash = 5381;
    size_t len = sdslen( e->s );
    sds str = e->s;
    for( int i = 0; i < len; i++ ) {
        c = str[i];
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}
