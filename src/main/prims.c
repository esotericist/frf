#include "prims.h"
#include "stdlib.h"

void preregisterprim( void *thefunc,  char *s) {
    preprims = realloc( preprims, sizeof( struct preprim) * (numpreprims + 1));
    preprims[numpreprims].thefunc = thefunc;
    preprims[numpreprims].prim = s;
    numpreprims++;
}

void finalizeprims() {
    for(int i = 0; i < numpreprims ; i++ ) {
        //*(preprims[i].thefunc) = 
        newatom( sdsnew( preprims[i].prim ) );
    }
    free(preprims);
}

prim(dup) {
    
}
