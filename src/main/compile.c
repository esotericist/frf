
#include "datatypes.h"
#include "compile.h"
#include "prims.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int errorstate = 0;

#define pmode P->parsemode

size_t strcount( sds key, sds searched ) {
    size_t keylen = sdslen(key);
    size_t searchedlen = sdslen(searched);
    size_t count = 0;
    if( keylen > searchedlen) {
        return 0;
    }
    for( size_t i = 0; i < (searchedlen - keylen + 1) ; i++ ) {
        if( memcmp( key, searched + i, keylen ) == 0 ) {
            count++;
        }
    }
    return count;
} 

uintptr_t tag_prim (void *v) {
    return ( (uintptr_t)(void *) v ) | 3;
}

void append_prim( struct process_state *P, size_t v ) {
    void * ptr = fetchprim( P->node, v );
    append_cp( P, tag_prim( ptr ) );
}


atom(if)
atom(else)  
atom(then)
atom(begin)
atom(until)
atom(repeat)
atom(continue)
atom(break)
atom(while)
atom(jmp)
atom(cjmp)


#define flowtopatom     P->flowcontrolstack[P->flowcontroltop].flowatom
#define flowtopcell     P->flowcontrolstack[P->flowcontroltop].celltarget

bool searchflowstack( struct process_state *P, size_t searchatom ) {
    for(size_t i = P->flowcontroltop ; i > 0 ; i-- ) {
        if ( P->flowcontrolstack[i].flowatom == searchatom ) {
            return true;
        }
    }
    return false;
}

void addflowframe(struct process_state *P, size_t thisatom ) {
    P->flowcontroltop++;
    P->flowcontrolstack[P->flowcontroltop].flowatom = thisatom;
    P->flowcontrolstack[P->flowcontroltop].celltarget = P->current_codestream->instructioncount;
}

void popflowtop(struct process_state *P ) {
    P->flowcontrolstack[P->flowcontroltop].flowatom = 0;
    P->flowcontrolstack[P->flowcontroltop].celltarget = 0;
    P->flowcontroltop--;
}

void dropflowframe(struct process_state *P, size_t stackitem ) {
    for ( size_t i = stackitem ; i < P->flowcontroltop ; i++ ) {
        P->flowcontrolstack[P->flowcontroltop].flowatom = P->flowcontrolstack[P->flowcontroltop + 1].flowatom;
        P->flowcontrolstack[P->flowcontroltop].celltarget = P->flowcontrolstack[P->flowcontroltop + 1].celltarget;
    }
    P->flowcontroltop--;
}

/* P, firstcell.i, lastcell.i, keyop.i, targetcell.i */ 
void updatecells(struct process_state *P, size_t firstcell, size_t lastcell, uintptr_t keyop, size_t targetcell ) {
    for ( size_t i = firstcell ; i < lastcell ; i++ ) {
        if( P->current_codestream->codestream[i].u_val == keyop && P->current_codestream->codestream[i+1].u_val ==  keyop ) {
            P->current_codestream->codestream[i+1].u_val = targetcell;
        }
    }
}

void unexpectedprim( struct process_state *P, size_t foundatom, size_t previousatom ) {
    printf( "error: unexpected %s after %s\n", atomtostring( foundatom ), atomtostring( previousatom )  );
}

bool checkflowcontrol(struct process_state *P, size_t maybeop ) {
    size_t pos_if, pos_then, pos_else, loopstart, loopend;
    if( maybeop == a_if ) {
        addflowframe(P, a_if );
        append_cp(P, 0);
        append_cp(P, 0);
    } else if ( maybeop == a_else ) {
        if( flowtopatom == a_if ) {
            addflowframe(P, a_else );
            append_cp(P, 0);
            append_cp(P, 0);
        } else {
            unexpectedprim( P, maybeop, flowtopatom );
        }
    } else if ( maybeop == a_then ) {
        if( flowtopatom == a_if ) {
            pos_if = flowtopcell;
            pos_then = P->current_codestream->instructioncount;
            popflowtop( P );
            void * ptr = fetchprim( P->node, a_cjmp );
            P->current_codestream->codestream[pos_if].u_val = tag_prim( ptr ) ;
            P->current_codestream->codestream[pos_if+1].u_val = pos_then;
        } else if( flowtopatom == a_else ) {
            pos_else = flowtopcell;
            popflowtop ( P );
            pos_if = flowtopcell;
            popflowtop ( P );
            pos_then = P->current_codestream->instructioncount;
            P->current_codestream->codestream[pos_if].u_val = tag_prim( fetchprim( P->node, a_cjmp ) );
            P->current_codestream->codestream[pos_if+1].u_val = pos_else+2;
            P->current_codestream->codestream[pos_else].u_val = tag_prim( fetchprim( P->node, a_jmp ) );
            P->current_codestream->codestream[pos_else+1].u_val = pos_then;
        } else {
            unexpectedprim( P, maybeop, flowtopatom );
        }
    } else if ( maybeop == a_begin ) {
        addflowframe( P, a_begin );
    } else if ( maybeop == a_until ) {
        if( flowtopatom == a_begin ) {
            loopstart = flowtopcell;
            loopend = P->current_codestream->instructioncount;
            popflowtop( P );
            append_prim( P, a_cjmp );
            append_cp( P, loopstart );
            updatecells(P, loopstart, loopend, tag_prim( fetchprim ( P->node, a_continue )  ), loopstart);
            updatecells(P, loopstart, loopend, tag_prim( fetchprim ( P->node, a_break )  ), loopend+2);
            updatecells(P, loopstart, loopend, tag_prim( fetchprim ( P->node, a_while )  ), loopend+2);
         } else {
            unexpectedprim( P, maybeop, flowtopatom );
        }
    } else if ( maybeop == a_repeat ) {
        if( flowtopatom == a_begin ) {
            loopstart = flowtopcell;
            loopend = P->current_codestream->instructioncount;
            popflowtop( P );
            append_prim( P, a_jmp );
            append_cp( P, loopstart );
            updatecells(P, loopstart, loopend, tag_prim( fetchprim ( P->node, a_continue )  ), loopstart);
            updatecells(P, loopstart, loopend, tag_prim( fetchprim ( P->node, a_break )  ), loopend+2);
            updatecells(P, loopstart, loopend, tag_prim( fetchprim ( P->node, a_while )  ), loopend+2);
         } else {
            unexpectedprim( P, a_repeat, flowtopatom );
        }
    } else if ( maybeop == a_while ) {
        if( searchflowstack(P, a_begin )) {
            append_prim( P, a_while );
            append_prim( P, a_while );
        } else {
            unexpectedprim( P, maybeop, flowtopatom );
        }
    } else if ( maybeop == a_continue ) {
        if( searchflowstack(P, a_begin )) {
            append_prim( P, a_continue );
            append_prim( P, a_continue );
        } else {
            unexpectedprim( P, maybeop, flowtopatom );
        }
    } else if ( maybeop == a_break ) {
        if( searchflowstack(P, a_begin )) {
            append_prim( P, a_break );
            append_prim( P, a_break );
        } else {
            unexpectedprim( P, maybeop, flowtopatom );
        }
    } else {
        return false;
    }

    return true;
}


atom(push_int)
atom(pop_int)
atom(push_string)
atom(pop_string)

// set in frf.c
// used for comparisons in tokenize
sds numstring;
sds opstring;

void tokenize( struct process_state *P, sds inputstr ) {
}

typedef void (*call_prim)(struct process_state *p);

void checktoken( struct process_state *P, sds input) {

    sds c = sdsnewlen( input, 1 );
    if( strcount ( c, numstring ) || ( sdslen(input) >=2 && strcount( c, opstring ) ) ) {
        // uintptr_t push_i = );
        // uintptr_t pop_i = (uintptr_t)(void *) fetchprim( P->node, a_pop_int );
        append_cp( P, ( (uintptr_t)(void *) fetchprim( P->node, a_push_int  ) ) | 3 );
        append_cp( P, atoi( input ) );
        return;
    }

    sdstolower( input );
    size_t maybe_op = verifyatom( input );
    if( checkflowcontrol(P, maybe_op) ) {
        return;
    } else {
        if( maybe_op ) {
            void * maybe_prim = fetchprim( P->node, maybe_op );
            if( maybe_prim ) {
                append_prim( P, maybe_op );
                return;
            }
        }
    }

    printf( "? %s ?\n", sdstrim( input, sdsnew( " " ) ) );
    
}

sds split_string( sds inputstr, ssize_t index )
{   
    if( index < 0 ) {
        index = sdslen( inputstr ) + index;
    }
    sds outputstr = sdsnewlen( inputstr, index );
    sdsrange( inputstr, index, sdslen ( inputstr) );
    return outputstr;
}

#define continue_str workingstring = sdscat( workingstring, nextchar )

sds parse_whozit( sds inputstr ) {
    sds workingstring = sdsempty();
    sds nextchar = sdsempty();
    while ( sdslen( inputstr ) )
    {
        nextchar = split_string( inputstr, 1 );
        if( 1 ) {

        } else {
            continue_str;
        }
    }
    return inputstr;
}

sds parse_string(struct process_state *P,  sds inputstr ) {
    size_t a__little_r = stringtoatom( sdsnew( "r" ));
    size_t a__big_r = stringtoatom( sdsnew( "R" ));
    size_t a__little_n = stringtoatom( sdsnew( "n" ));

    sds workingstring = sdsempty();
    sds nextchar = sdsempty();
    while ( sdslen( inputstr ) )
    {
        nextchar = split_string( inputstr, 1 );
        size_t nextchar_a = verifyatom( nextchar );

        if( (nextchar_a == a__little_r ) || (nextchar_a == a__big_r ) || ( nextchar_a == a__little_n ) ) {
            if( pmode.escape ) {
                pmode.escape = false;
                nextchar = "\n";
            }
            continue_str;
        } else if ( nextchar_a == a__backslash ) {
            if( pmode.escape ) {
                pmode.escape = false;
                continue_str;
            } else {
                nextchar = sdsempty();
                pmode.escape = true;
            }
        } else if ( nextchar_a == a__dquote ) {
            if( pmode.escape ) {
                pmode.escape = false;
                continue_str;
                nextchar = sdsempty();
            } else {
                size_t stringatom = stringtoatom( workingstring );
                append_prim( P, a_push_string );
                append_cp(P, stringatom );
                workingstring = sdsempty();
                pmode.string = false;
                return inputstr;
            }
        } else {
            continue_str;
        }
    }
    return inputstr;
}


sds parse_immed(struct process_state *P,  sds inputstr ) {
    sds workingstring = sdsempty();
    sds nextchar = sdsempty();
    while ( sdslen( inputstr ) )
    {
        nextchar = split_string( inputstr, 1 );
        size_t nextchar_a = verifyatom( nextchar );
        if( nextchar_a == a__space || nextchar_a == a__newline ) {
            if( sdslen( workingstring) > 0 ) {
                checktoken( P, workingstring);
            }
            workingstring = sdsempty();
            
        } else if( nextchar_a == a__dsign ) {
            if( sdslen( workingstring) == 0 ) {
                pmode.directive = true;
                return sdscat( nextchar, inputstr );
            } else {
                continue_str;
            }
        } else if( nextchar_a == a__dquote ) {
            pmode.string = true;
            return inputstr;
        } else if( nextchar_a == a__squote ) {
            pmode.atom = true;
            return inputstr;
        } else if( nextchar_a == a__parenl ) {
            if( sdslen(workingstring) > 0 ) {
                checktoken( P, workingstring);
            }
            workingstring = sdsempty();
            pmode.comment = true;
            return inputstr;
        } else if( nextchar_a == a__colon ) {
            pmode.compile = true;
            return inputstr;
        } else {
            continue_str;
        }
    }
    return inputstr;
}

sds parse_compile(struct process_state *P,  sds inputstr ) {
    sds workingstring = sdsempty();
    sds nextchar = sdsempty();
    while ( sdslen( inputstr ) )
    {
        nextchar = split_string( inputstr, 1 );
        size_t nextchar_a = verifyatom( nextchar );
        if( nextchar_a == a__space || nextchar_a == a__newline ) {
            if( sdslen( workingstring) > 0 ) {
                checktoken( P, workingstring);
            }
            workingstring = sdsempty();
            
        } else if( nextchar_a == a__dquote ) {
            pmode.string = true;
            return inputstr;
        } else if( nextchar_a == a__squote ) {
            pmode.atom = true;
            return inputstr;
        } else if( nextchar_a == a__parenl ) {
            if( sdslen(workingstring) > 0 ) {
                checktoken( P, workingstring);
            }
            workingstring = sdsempty();
            pmode.comment = true;
            return inputstr;
        } else if( nextchar_a == a__semicolon ) {
            // word finalization
        } else {
            workingstring = sdscat( workingstring, nextchar );
        }
    }
    return inputstr;
}

sds parse_comment(struct process_state *P,  sds inputstr ) {
    sds workingstring = sdsempty();
    sds nextchar = sdsempty();
    while ( sdslen( inputstr ) )
    {
        nextchar = split_string( inputstr, 1 );
        size_t nextchar_a = verifyatom( nextchar );
        if( nextchar_a == a__parenr ) {
            pmode.comment = false;
            return inputstr;
        } else {
            continue_str;
        }
    }
    return inputstr;
}


void parse_line( struct process_state *P, sds input ) {
    sds s_space = sdsnew( " " );
    input = sdstrim( sdstrim( input, s_space ), "\n" );
    sds workingstring = sdsdup(input);
    workingstring = sdscat( workingstring, s_space );
    while( sdslen( workingstring) > 0 /* and errorstate == 0 */ ) {
        if( pmode.comment ) {
            workingstring = parse_comment( P, workingstring );

        } else if ( pmode.atom ) {
            // workingstring = parse_atom( workingstring );
        } else if ( pmode.directive ) {
            // workingstring = parse_directive( workingstring );
        } else if ( pmode.string ) {
            workingstring = parse_string( P, workingstring );
        } else if ( pmode.compile ) {
            if ( 0 /* word has name? */ ) {
            // workingstring = parse_newword( workingstring );
            } else {
            workingstring = parse_compile( P, workingstring );
            }
        } else {
            workingstring = parse_immed( P, workingstring );
        }
    }
}
