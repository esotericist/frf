#include <stdlib.h>
#include "prims.h"
#include "stack.h"
#include "vm.h"

// #region prim registration
void preregisterprim(void *thefunc, char *s)
{
    preprims = realloc(preprims, sizeof(struct preprim) * (numpreprims + 1));
    preprims[numpreprims].thefunc = thefunc;
    preprims[numpreprims].prim = s;
    numpreprims++;
}

extern sListType *atom_table[slist_hash_size];

void finalizeprims(struct node_state *N)
{
    for (int i = 0; i < numpreprims; i++)
    {
        uintptr_t new_prim_ptr = (uintptr_t)(void *)preprims[i].thefunc;
        sfs primname = sfstrim(sfsnew(preprims[i].prim), " ");
        size_t new_atom_val = stringtoatom(primname);
        iListType *new_prim_entry = alloc_ilist();
        iListType *new_atom_entry = alloc_ilist();
        new_prim_entry->first.p_val = new_prim_ptr;
        new_prim_entry->second.a_val = new_atom_val;
        sglib_hashed_iListType_add(N->primtoatomtable, new_prim_entry);

        new_atom_entry->first.a_val = new_atom_val;
        new_atom_entry->second.p_val = new_prim_ptr;
        sglib_hashed_iListType_add(N->atomtoprimtable, new_atom_entry);
    }
}
// #endregion

#pragma GCC push_options
#pragma GCC optimize("align-functions=16")

// #region raw value prims

prim(push_int)
{
    size_t pos = P->currentop++;
    struct code_point *cp = &P->current_codestream->codestream[pos];
    push_int( cp_get_int(cp));
}

prim(push_atom)
{
    size_t pos = P->currentop++;
    push_atom( cp_get_atom(&P->current_codestream->codestream[pos]));
}

prim(push_string)
{
    size_t pos = P->currentop++;
    push_string( cp_get_string(&P->current_codestream->codestream[pos]));
}

prim(push_var) {
    size_t pos = P->currentop++;
    push_var( cp_get_int(&P->current_codestream->codestream[pos]) );
}

prim2(push_stackmark, { ) {
    push_stackmark
}

prim2(stackrange, } ) {
    size_t range, pos;
    for(size_t i = dcount -1; i >= 0; i-- ) {
        if(dp_is_stackmark( dstack[i])) {
            pos = i;
            break;
        }
    }
    for(size_t i = pos; i < dcount -1; i++ ) {
        dstack[i] = dstack[i+1];
    }
    dcount--;
    range = dcount - pos; 
    push_int(range);

}

prim2( store_var, ! ) {
    require_var v = pop_var;
    if( v >= 0 ) {
        needstack(1)
        struct datapoint d = pop_dp(P);
        P->current_varset->vars[v].dp.u_val = d.u_val;
    } else {
        runtimefault( "error in %zu: invalid variable" )
    }
}

prim2( fetch_var, @ ) {
    require_var v = pop_var;
    if( v >= 0 ) {
        push_dp(P, P->current_varset->vars[v].dp );
    } else {
        runtimefault( "error in %zu: invalid variable" )
    }
}

prim2(checkisint, int? ) {
    bool t = dp_is_int( topdp );
    push_bool( t );
}

// todo: add floats
prim2(checkisnum, number? ) {
    bool t = dp_is_int( topdp );
    push_bool( t );
}

prim2(checkisstr, string? ) {
    bool t = dp_is_string( topdp );
    push_bool( t );
}

prim2(checkisatom, atom? ) {
    bool t = dp_is_atom( topdp );
    push_bool( t );
}

prim2(checkisarray, array?) {
    bool t = dp_is_array( topdp );
    push_bool( t );
}

prim2(checkistuple, tuple?) {
    bool t = dp_is_tuple( topdp );
    push_bool( t );
}

// #endregion

// #region stackprims
prim(dup)
{
    needstack(1)
        push_dp(P, topdp);
}

prim(over)
{
    needstack(2)
        push_dp(P, dstack[dcount - 2]);
}

prim(pick)
{
    require_int count = pop_int;
    if (count < 1)
    {
        stackfault(a_expected_positive_integer)
    }
    needstack(count)
        push_dp(P, dstack[dcount - count]);
}

prim(depth)
{
    int64_t v = P->d->top;
    push_int( v );
}

prim(swap)
{
    needstack(2) struct datapoint second = (pop_dp(P));
    struct datapoint first = (pop_dp(P));
    push_dp(P, second);
    push_dp(P, first);
}

void rotate(proc *P, int64_t count)
{
    if (count < 0)
    {
        count = -count;
        needstack(count) struct datapoint top = dstack[dcount - 1];
        for (size_t i = 0; i < count; i++)
        {
            dstack[dcount - i] = dstack[dcount - i - 1];
        }
        dstack[dcount - count] = top;
    }
    else
    {
        needstack(count) struct datapoint bottom = dstack[dcount - count];
        for (size_t i = count - 1; i > 0; i--)
        {
            dstack[dcount - i - 1] = dstack[dcount - i];
        }
        dstack[dcount - 1] = bottom;
    }
}

prim(rotate)
{
    require_int count = pop_int;
    rotate(P, count);
}

prim(rot)
{
    rotate(P, 3);
}

prim(pop)
{
    needstack(1)
        pop_dp(P);
}

prim(popn)
{
    require_int count = pop_int;
    needstack(count) if (count >= 0)
    {
        for (size_t i = 0; i < count; i++)
        {
            pop_dp(P);
        }
    }
}

prim(dupn) {
    require_int count = pop_int;
    size_t offset = dcount - count;
    needstack(count) if( count >= 0) {
        for (size_t i = 0; i < count; i++)
        {
            push_dp(P, dstack[offset + i]);
        }
    }
}

prim(nip) {
    p_swap(P);
    pop_dp(P);   
}

prim(tuck) {
    p_dup(P);
    rotate(P, -3);
}

prim(put) {
    require_int count = pop_int;
    needstack(count)
    rotate(P, - (count + 1));
    rotate(P, count);
    pop_dp(P);   

}

// #endregion

// #region flow control prims

// unconditional jump primarily used by the compiler
// pulls its address from the next cell
prim(jmp)
{
    size_t pos = P->currentop++;
    struct code_point *cp = &P->current_codestream->codestream[pos];
    P->currentop = cp_get_int(cp);
}

// conditional jump, primarly used by the compiler
// uses the top entry of the stack for truth, pulls its address from the next cell
prim(cjmp)
{
    size_t pos = P->currentop++;
    require_bool val = pop_bool;
    if (!val)
    {
        struct code_point *cp = &P->current_codestream->codestream[pos];
        P->currentop = cp_get_int(cp);
    }
}

prim(continue)
{
    P->currentop = cp_get_int(&P->current_codestream->codestream[P->currentop]);
}

prim(break)
{
    P->currentop = cp_get_int(&P->current_codestream->codestream[P->currentop]);
}

prim(while)
{
    if (pop_bool)
    {
        P->currentop++;
    }
    else
    {
        P->currentop = P->current_codestream->codestream[P->currentop].u_val;
    }
}

prim(call)
{
    struct code_set *targetword = (void *)(uintptr_t)P->current_codestream->codestream[P->currentop].p_val;
    P->currentop++;
    pushcallstackframe(P);
    if (!P->errorstate)
    {
        P->current_codestream = targetword;
        P->current_varset = grow_variable_set( targetword->vars );
        P->current_varset->count--;
        P->currentop = 0;
    }
}

prim(exit)
{
    popcallstackframe(P);
}

prim(pid) {
    push_int(P->pid)
}

atom(killed)

prim(kill) {
    require_int i1 = pop_int;
    iListType *tmp= alloc_ilist();
    tmp->first.a_val = i1;
    if( i1 > 0 ) {
        tmp = sglib_hashed_iListType_find_member( P->node->process_table, tmp );
        if( tmp ) {
            push_int(1);
            process_kill( (proc *)(uintptr_t) tmp->second.p_val, a_killed, sfsempty() );
        } else {
            push_int(0);
        }
    }
}

prim(fork) {
    proc *new_P = newprocess( P->node );
    new_P->current_codestream = P->current_codestream;
    new_P->currentop = P->currentop;
    if( P->current_varset ) {
        new_P->current_varset = grow_variable_set(P->current_varset);
        new_P->current_varset->count--;
    }
    new_P->debugmode = P->debugmode;
    new_P->max_slice_ops = P->max_slice_ops;
    for( size_t i = 0; i < P->d->top ; i++ ) {
        push_dp(new_P, P->d->stack[i] );
    }
    for( size_t i = 0; i < P->c->top ; i++ ) {
        copyframe( &P->c->stack[i] , &new_P->c->stack[i] );
    }
    push_int(new_P->pid);
    P = new_P;
    push_int(0);
    
}

// #endregion

// #region math prims
prim2(add, +)
{
    require_int second = pop_int;
    require_int first = pop_int;
    push_int( first + second );
}

prim2(minus, -)
{
    require_int second = pop_int;
    require_int first = pop_int;
    push_int( first - second );
}

prim2(mult, *)
{
    require_int second = pop_int;
    require_int first = pop_int;
    push_int( first * second );
}

prim2(div, /)
{
    require_int second = pop_int;
    require_int first = pop_int;
    push_int( first / second );
}

prim2(modulo, %)
{
    require_int second = pop_int;
    require_int first = pop_int;
    push_int( first % second );
}
// #endregion

// #region logic prims
prim2(isequalto, =)
{
    require_int second = pop_int;
    require_int first = pop_int;
    push_bool( first == second );
}

prim2(isnotsequalto, !=)
{
    require_int second = pop_int;
    require_int first = pop_int;
    push_bool( first != second );
}

prim2(isgreaterthan, >)
{
    require_int second = pop_int;
    require_int first = pop_int;
    push_bool( first > second );
}

prim2(islessthan, <)
{
    require_int second = pop_int;
    require_int first = pop_int;
    push_bool( first < second );
}

prim2(isgreaterorequal, >=)
{
    require_int second = pop_int;
    require_int first = pop_int;
    push_bool( first >= second );
}

prim2(islesserorequal, <=)
{
    require_int second = pop_int;
    require_int first = pop_int;
    push_bool( first <= second );
}

prim(not )
{
    needstack(1)
        push_bool( !pop_bool );
}

prim(or)
{
    require_bool second = pop_bool;
    require_bool first = pop_bool;
    push_bool( first || second );
}

prim(and)
{
    require_bool second = pop_bool;
    require_bool first = pop_bool;
    push_bool( first && second );
}
// #endregion

// #region string prims

prim(strlen) {
    require_string s = pop_string;
    push_int( sfslen(s) );
}

prim(strcat)
{
    require_string s2 = pop_string;
    require_string s1 = pop_string;
    push_string( sfscatsfs(s1, s2) );
}

prim(strcmp)
{
    require_string s2 = pop_string;
    require_string s1 = pop_string;
    push_int( sfscmp(s1, s2) );
}

// MUF called this 'strncmp', but renaming it to match 'popn'
prim(strcmpn)
{
    require_int len = pop_int;
    require_string s2 = pop_string;
    require_string s1 = pop_string;
    push_int( sfscmp(sfsnewlen(s1, len), sfsnewlen(s2, len)) );
}


// MUF used 'stringcmp', but that's terrible notation, and i think it should be like 'strcmpn'
prim(strcmpi)
{
    require_string s2 = pop_string;
    require_string s1 = pop_string;
    push_int( sfscmp(sfstolower(s1), sfstolower(s2)) );
}

prim(stringpfx) {
    require_string s2 = pop_string;
    require_string s1 = pop_string;
    push_bool( sfscmp(sfsnewlen(s1, sfslen(s2)), s2) == 0 );
    
}

prim(strcut)
{
    require_int index = pop_int;
    require_string str = pop_string;
    size_t len = sfslen(str);
    if (index < 0)
    {
        index = 0;
    }
    if (index > len)
    {
        index = len;
    }
    push_string( sfsnewlen(str, index));
    push_string( sfsright(str, len - index));
}

prim(midstr) {
    require_int i2 = pop_int;
    require_int i1 = pop_int - 1;
    require_string s = pop_string;
    size_t len = sfslen(s);
    if(i1 < 0 ) {
        i1 = 0;
    }
    if(i2 > len) {
        i2 = len;
    }
    push_string( sfsrange( s, i1, i1 + i2 - 1 ));
}

prim(instr) {
    require_string s2 = pop_string;
    require_string s1 = pop_string;
    push_int( sfsinstr( s1, s2, false ) );
}

// case insensitive instr (was 'instring' in muf)
prim(instri) {
    require_string s2 = pop_string;
    require_string s1 = pop_string;
    push_int( sfsinstr( sfstolower(s1), sfstolower(s2), false ) );
}

// reverse instr
prim(rinstr) {
    require_string s2 = pop_string;
    require_string s1 = pop_string;
    push_int( sfsinstr( s1, s2, true ) );
}

// case insensitive rinstr (was 'rinstring' in muf)
prim(rinstri) {
    require_string s2 = pop_string;
    require_string s1 = pop_string;
    push_int( sfsinstr( sfstolower(s1), sfstolower(s2), true ) );
}

prim(explode) {
    require_string s2 = pop_string;
    require_string s1 = pop_string;
    size_t count;
    sfs *strings = sfssplit( s1, s2, &count );
    for( size_t i = count; i > 0; --i ) {
        push_string( strings[i-1]);
    }
    push_int( count);
}

prim(split) {
    require_string s2 = pop_string;
    require_string s1 = pop_string;
    size_t index = sfsinstr( s1, s2, false );
    if( index ) {
        push_string( sfsnewlen( s1, index -1 ));
        push_string( sfsright( s1, sfslen(s1) - index ));
    } else {
        push_string( s1);
        push_string( sfsempty());
    }
}

prim(rsplit) {
    require_string s2 = pop_string;
    require_string s1 = pop_string;
    size_t index = sfsinstr( s1, s2, true );
    if( index ) {
        push_string( sfsnewlen( s1, index -1 ));
        push_string( sfsright( s1, sfslen(s1) - index ));
    } else {
        push_string( s1);
        push_string( sfsempty());
    }
}

prim(tolower) {
    require_string s = pop_string;
    push_string( sfstolower(s));
}

prim(toupper) {
    require_string s = pop_string;
    push_string( sfstoupper(s));
}

prim(subst) {
    require_string s3 = pop_string;
    require_string s2 = pop_string;
    require_string s1 = pop_string;
    push_string( sfssubst( s1, s3, s2 ));
}

// #endregion

// #region conversion prims

prim(intostr)
{
    require_int num = pop_int;
    push_string( sfsfromlonglong(num));
}

prim(ctoi)
{
    require_string str = pop_string;
    push_int( str[0]);
}
// #endregion

// #region debugging
prim(debug_on)
{
    P->debugmode = true;
}

prim(debug_off)
{
    P->debugmode = false;
}

prim(debug_line)
{
    printf("%s\n", dump_stack(P));
}

// #endregion