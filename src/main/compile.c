
#include "datatypes.h"
#include "compile.h"
#include "prims.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm.h"

int errorstate = 0;

#define pmode P->compilestate->parsemode
#define vset P->current_codestream->vars

uintptr_t tag_prim (void *v) {
    return ( (uintptr_t)(void *) v ) | 3;
}

void append_prim( proc *P, size_t v ) {
    void * ptr = atomtoprim( P->node, v );
    append_cp( P, tag_prim( ptr ) );
}

void append_word( proc *P, void * v  ) {
    append_cp(P, (uintptr_t)(void *) v );
}

int64_t parse_atomtovar( proc *P, size_t atom ) {
    iListType *it = alloc_ilist();
    it->first.a_val = atom;
    struct ilist *tmp = sglib_hashed_iListType_find_member( P->compilestate->vartable, it );
    if(tmp) {
    return tmp->second.a_val;
    }
    return -1;
}

void record_var(proc *P, sfs varname, size_t varnum ) {
    size_t varatom = stringtoatom( sfstolower( sfstrim( varname, " ") ) );
    iListType *newword = alloc_ilist();
    newword->first.a_val = varatom;
    newword->second.a_val = varnum;
    sglib_hashed_iListType_add( P->compilestate->vartable, newword );
    vset->vars[vset->count].name = varatom;
    vset->vars[vset->count].dp.u_val = 1;
    vset = grow_variable_set( vset );
    
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


#define flowstack       P->compilestate->flowcontrolstack
#define flowtop         P->compilestate->flowcontroltop       
#define flowtopatom     flowstack[flowtop].flowatom
#define flowtopcell     flowstack[flowtop].celltarget

bool searchflowstack( proc *P, size_t searchatom ) {
    for(size_t i = flowtop ; i > 0 ; i-- ) {
        if ( flowstack[i].flowatom == searchatom ) {
            return true;
        }
    }
    return false;
}

void addflowframe(proc *P, size_t thisatom ) {
    flowtop++;
    flowstack[flowtop].flowatom = thisatom;
    flowstack[flowtop].celltarget = P->current_codestream->instructioncount;
}

void popflowtop(proc *P ) {
    flowstack[flowtop].flowatom = 0;
    flowstack[flowtop].celltarget = 0;
    flowtop--;
}

void dropflowframe(proc *P, size_t stackitem ) {
    for ( size_t i = stackitem ; i < flowtop ; i++ ) {
        flowstack[flowtop].flowatom = flowstack[flowtop + 1].flowatom;
        flowstack[flowtop].celltarget = flowstack[flowtop + 1].celltarget;
    }
    flowtop--;
}

/* P, firstcell.i, lastcell.i, keyop.i, targetcell.i */ 
void updatecells(proc *P, size_t firstcell, size_t lastcell, uintptr_t keyop, size_t targetcell ) {
    for ( size_t i = firstcell ; i < lastcell ; i++ ) {
        if( P->current_codestream->codestream[i].u_val == keyop && P->current_codestream->codestream[i+1].u_val ==  keyop ) {
            P->current_codestream->codestream[i+1].u_val = targetcell;
        }
    }
}

void unexpectedprim( proc *P, size_t foundatom, size_t previousatom ) {
    printf( "error: unexpected %s after %s\n", atomtostring( foundatom ), atomtostring( previousatom )  );
}

bool checkflowcontrol(proc *P, size_t maybeop ) {
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
            void * ptr = atomtoprim( P->node, a_cjmp );
            P->current_codestream->codestream[pos_if].u_val = tag_prim( ptr ) ;
            P->current_codestream->codestream[pos_if+1].u_val = pos_then;
        } else if( flowtopatom == a_else ) {
            pos_else = flowtopcell;
            popflowtop ( P );
            pos_if = flowtopcell;
            popflowtop ( P );
            pos_then = P->current_codestream->instructioncount;
            P->current_codestream->codestream[pos_if].u_val = tag_prim( atomtoprim( P->node, a_cjmp ) );
            P->current_codestream->codestream[pos_if+1].u_val = pos_else+2;
            P->current_codestream->codestream[pos_else].u_val = tag_prim( atomtoprim( P->node, a_jmp ) );
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
            updatecells(P, loopstart, loopend, tag_prim( atomtoprim ( P->node, a_continue )  ), loopstart);
            updatecells(P, loopstart, loopend, tag_prim( atomtoprim ( P->node, a_break )  ), loopend+2);
            updatecells(P, loopstart, loopend, tag_prim( atomtoprim ( P->node, a_while )  ), loopend+2);
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
            updatecells(P, loopstart, loopend, tag_prim( atomtoprim ( P->node, a_continue )  ), loopstart);
            updatecells(P, loopstart, loopend, tag_prim( atomtoprim ( P->node, a_break )  ), loopend+2);
            updatecells(P, loopstart, loopend, tag_prim( atomtoprim ( P->node, a_while )  ), loopend+2);
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
atom(push_string)
atom(push_atom)
atom(push_var)

atom(call)
atom(exit)
atom(var)
atom2(var_store, var!)
atom2(exclaim, !)

atom(unexpected_variable)
atom(unrecognized_token)

// set in frf.c
// used for comparisons in tokenize
sfs numstring;
sfs opstring;

void tokenize( proc *P, sfs input ) {
    sfs c = sfsnewlen( input, 1 );
    if( sfsmatchcount ( numstring, c ) || ( sfslen(input) >=2 && sfsmatchcount( opstring, c ) ) ) {
        append_cp( P, ( (uintptr_t)(void *) atomtoprim( P->node, a_push_int  ) ) | 3 );
        append_cp( P, atoi( input ) );
        return;
    }

    input = sfstolower( input );
    size_t maybe_op = verifyatom( input );
    if( pmode.directive) {
        if( P->compilestate->keyword == a_var || P->compilestate->keyword == a_var_store) {
            if( maybe_op && parse_atomtovar(P, maybe_op) >= 0 ) {
                printf( "error: unexpected existing variable %s in variable declaration.\n", atomtostring( maybe_op ) );
            } else {
                record_var( P, input, vset->count );
            }
            pmode.directive = false;
            if(P->compilestate->keyword == a_var_store ) {
                append_prim( P, a_push_var );
                append_cp( P, vset->count -1 );
                append_prim( P, a_exclaim );
            }
            P->compilestate->keyword = 0;
            return;
        }
    } else if( maybe_op == a_var ) {
        pmode.directive = true;
        P->compilestate->keyword = a_var;
        return;
    } else if( maybe_op == a_var_store ) {
        pmode.directive = true;
        P->compilestate->keyword = a_var_store;
        return;
    }
    if( checkflowcontrol(P, maybe_op) ) {
        return;
    } else {
        if( maybe_op ) {
            int64_t maybe_var = parse_atomtovar( P, maybe_op );
            if( maybe_var >= 0 ) {
                append_prim( P, a_push_var);
                append_cp( P, maybe_var );

                return;
            }

            void * maybe_prim = atomtoprim( P->node, maybe_op );
            if( maybe_prim ) {
                append_prim( P, maybe_op );
                return;
            } else {
                void * maybe_word = atomtoword(P->node, maybe_op );
                if( maybe_word ) {
                    append_prim( P, a_call );
                    append_word( P, maybe_word );
                    return;
                }

            }
        }
    }

    printf( "unexpected token: %s\n", sfstrim( input, sfsnew( " " ) ) );
    process_reset(P, a_unrecognized_token);
 }

typedef void (*call_prim)(proc *p);

#define checktoken(s) do { sListType *elem = slist_find( P->node->definetable, (s));  \
                        if( elem) { \
                            inputstr = sfscatsfs( sfsnew((char *) (uintptr_t) elem->ptr), inputstr );  \
                            workingstring =sfsempty(); \
                        } else { \
                            tokenize(P, s);\
                        }\
                        } while(0)


#define continue_str workingstring = sfscatc( workingstring, nextchar )

sfs parse_atom( proc *P, sfs inputstr ) {
    sfs workingstring = sfsempty();
    sfs nextchar = sfsempty();
    while ( sfslen( inputstr ) )
    {
        nextchar = sfsnewlen( inputstr, 1 );
        inputstr = sfsright( inputstr, sfslen( inputstr) - 1 );
        
        size_t nextchar_a = verifyatom( nextchar );
        if( nextchar_a == a__squote ) {
            size_t stringatom = stringtoatom( workingstring );
            append_prim( P, a_push_atom );
            append_cp(P, stringatom );
            workingstring = sfsempty();
            pmode.atom = false;
            return inputstr;
        } else {
            continue_str;
        }
    }
    return inputstr;
}

sfs parse_string(proc *P,  sfs inputstr ) {
    size_t a__little_r = stringtoatom( sfsnew( "r" ));
    size_t a__big_r = stringtoatom( sfsnew( "R" ));
    size_t a__little_n = stringtoatom( sfsnew( "n" ));

    sfs workingstring = sfsempty();
    sfs nextchar = sfsempty();
    while ( sfslen( inputstr ) )
    {
        nextchar = sfsnewlen( inputstr, 1 );
        inputstr = sfsright( inputstr, sfslen( inputstr) - 1 );
        
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
                nextchar = sfsempty();
                pmode.escape = true;
            }
        } else if ( nextchar_a == a__dquote ) {
            if( pmode.escape ) {
                pmode.escape = false;
                continue_str;
                nextchar = sfsempty();
            } else {
                size_t stringatom = stringtoatom( workingstring );
                append_prim( P, a_push_string );
                append_cp(P, stringatom );
                workingstring = sfsempty();
                pmode.string = false;
                return inputstr;
            }
        } else {
            continue_str;
        }
    }
    return inputstr;
}


sfs parse_immed(proc *P,  sfs inputstr ) {
    sfs workingstring = sfsempty();
    sfs nextchar = sfsempty();
    while ( sfslen( inputstr ) )
    {
        if(P->errorstate) {
            return workingstring;
        }
        nextchar = sfsnewlen( inputstr, 1 );
        inputstr = sfsright( inputstr, sfslen( inputstr) - 1 );
        
        size_t nextchar_a = verifyatom( nextchar );
        if( nextchar_a == a__space || nextchar_a == a__newline ) {
            if( sfslen( workingstring) > 0 ) {
                checktoken( workingstring);
            }
            workingstring = sfsempty();
            
        } else if( nextchar_a == a__dsign ) {
            if( sfslen( workingstring) == 0 ) {
                pmode.directive = true;
                return sfscatc( nextchar, inputstr );
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
            if( sfslen(workingstring) > 0 ) {
                checktoken( workingstring);
            }
            workingstring = sfsempty();
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

sfs parse_compile(proc *P,  sfs inputstr ) {
    sfs workingstring = sfsempty();
    sfs nextchar = sfsempty();
    while ( sfslen( inputstr ) )
    {
        if(P->errorstate) {
            return workingstring;
        }
        nextchar = sfsnewlen( inputstr, 1 );
        inputstr = sfsright( inputstr, sfslen( inputstr) - 1 );
        
        size_t nextchar_a = verifyatom( nextchar );
        if( nextchar_a == a__space || nextchar_a == a__newline ) {
            if( sfslen( workingstring) > 0 ) {
                checktoken( workingstring);
            }
            workingstring = sfsempty();
            
        } else if( nextchar_a == a__dquote ) {
            pmode.string = true;
            return inputstr;
        } else if( nextchar_a == a__squote ) {
            pmode.atom = true;
            return inputstr;
        } else if( nextchar_a == a__parenl ) {
            if( sfslen(workingstring) > 0 ) {
                checktoken( workingstring);
            }
            workingstring = sfsempty();
            pmode.comment = true;
            return inputstr;
        } else if( nextchar_a == a__semicolon ) {
            if( sfslen( workingstring ) ) {
                printf( "unknown token: %s.\n", workingstring );
                // big error here
                return inputstr;
            } else {
                append_prim( P, a_exit );
                if( flowtop ) {
                    printf( "unexpected end of word\n" );
                    // more big error here
                } else {
                    sfs wordname = atomtostring( P->current_codestream->nameatom );
                    size_t icount = P->current_codestream->instructioncount;
                    size_t vcount = P->current_codestream->vars->count;
                    printf( "word added: %s, codestream size: %zu, variables: %zu.\n", wordname , icount, vcount );
                    if( vcount ) {
                        sfs vlist = sfsnew( "variable list: " );
                        for( size_t i = 0; i < vcount; i++ ) {
                            if( i> 0 ) {
                                vlist = sfscatc(vlist, ", " );
                            }
                            vlist = sfscatsfs(vlist, atomtostring(P->current_codestream->vars->vars[i].name));
                        }
                        printf( "%s\n", vlist);
                    }
                    pmode.compile = false;
                    popcallstackframe(P);
                }
                    sglib_hashed_iListType_init( P->compilestate->vartable );
            }
        } else {
            workingstring = sfscatc( workingstring, nextchar );
        }
    }
    return inputstr;
}

sfs parse_comment(proc *P,  sfs inputstr ) {
    sfs workingstring = sfsempty();
    sfs nextchar = sfsempty();
    while ( sfslen( inputstr ) )
    {
        nextchar = sfsnewlen( inputstr, 1 );
        inputstr = sfsright( inputstr, sfslen( inputstr) - 1 );
        
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

void define_newword( proc *P,  sfs inputstr ) {
    if( sfscmp( inputstr, sfsnew( ":" ) ) || sfscmp( inputstr, sfsnew( ";" ) ) ||
        sfscmp( inputstr, sfsnew( "@" ) ) || sfscmp( inputstr, sfsnew( "!" ) ) ||
        sfscmp( inputstr, sfsnew( "var" ) ) ) {
            // big error here invocation.
    }

    size_t wordname = newatom( inputstr );
    if( wordname ) {
        pushcallstackframe(P);
        P->current_codestream=newcodeset( P->node, 1024, wordname );
        printf( "compiling new word: %s.\n", atomtostring(wordname) );
    }
    
    
}

sfs parse_newword(proc *P,  sfs inputstr ) {
    sfs workingstring = sfsempty();
    sfs nextchar = sfsempty();
    while ( sfslen( inputstr ) )
    {
        nextchar = sfsnewlen( inputstr, 1 );
        inputstr = sfsright( inputstr, sfslen( inputstr) - 1 );
        
        size_t nextchar_a = verifyatom( nextchar );
        if( nextchar_a == a__parenl || nextchar_a == a__space ) {
                if( sfslen( workingstring ) > 0 && sfscmp( workingstring, sfsnew( " " ) ) ) {
                    define_newword( P, workingstring );
                }
                if(nextchar_a == a__parenl ) {
                    pmode.comment = true;
                }
            return inputstr;
        } else {
            continue_str;
        }
    }
    return inputstr;
}


void parse_line( proc *P, sfs input ) {
    sfs s_space = sfsnew( " " );
    input = sfstrim( sfstrim( input, s_space ), "\n" );
    sfs workingstring = sfscatc( input, s_space );
    while( sfslen( workingstring) > 0  && errorstate == 0  ) {
        if( pmode.comment ) {
            workingstring = parse_comment( P, workingstring );

        } else if ( pmode.atom ) {
            workingstring = parse_atom( P, workingstring );
        } else if ( pmode.directive ) {
            // workingstring = parse_directive( workingstring );
        } else if ( pmode.string ) {
            workingstring = parse_string( P, workingstring );
        } else if ( pmode.compile ) {
            if ( P->current_codestream->nameatom == 0 ) {
                workingstring = parse_newword( P, workingstring );
            } else {
                workingstring = parse_compile( P, workingstring );
            }
        } else {
            workingstring = parse_immed( P, workingstring );
        }
    }
}
