#include "prims.h"
#include "stdlib.h"
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

prim(push_int)
{
    size_t pos = P->currentop++;
    struct code_point *cp = &P->current_codestream->codestream[pos];
    push_int(P, cp_get_int(cp));
}

prim(push_atom)
{
    size_t pos = P->currentop++;
    push_atom(P, cp_get_atom(&P->current_codestream->codestream[pos]));
}

prim(push_string)
{
    size_t pos = P->currentop++;
    push_string(P, cp_get_string(&P->current_codestream->codestream[pos]));
}

// #region stackprims
prim(dup)
{
    needstack(1)
        push_dp(P, topdp);
}

prim(over)
{
    needstack(2)
        push_dp(P, &dstack[dcount - 2]);
}

prim(pick)
{
    require_int count = pop_int(P);
    if (count < 1)
    {
        stackfault(a_expected_positive_integer)
    }
    needstack(count)
        push_dp(P, &dstack[dcount - count]);
}

prim(depth)
{
    push_int(P, P->d->top);
}

prim(swap)
{
    needstack(2) struct datapoint second = (pop_dp(P));
    struct datapoint first = (pop_dp(P));
    push_dp(P, &second);
    push_dp(P, &first);
}

void rotate(struct process_state *P, int64_t count)
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
    require_int count = pop_int(P);
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
    needstack(1)
        require_int count = pop_int(P);
    needstack(count) if (count >= 0)
    {
        for (size_t i = 0; i < count; i++)
        {
            pop_dp(P);
        }
    }
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
    require_bool val = pop_bool(P);
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
    if (pop_bool(P))
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
        P->currentop = 0;
    }
}

prim(exit)
{
    popcallstackframe(P);
}
// #endregion

// #region math prims
prim2(add, +)
{
    require_int second = pop_int(P);
    require_int first = pop_int(P);
    push_int(P, first + second);
}

prim2(minus, -)
{
    require_int second = pop_int(P);
    require_int first = pop_int(P);
    push_int(P, first - second);
}

prim2(mult, *)
{
    require_int second = pop_int(P);
    require_int first = pop_int(P);
    push_int(P, first * second);
}

prim2(div, /)
{
    require_int second = pop_int(P);
    require_int first = pop_int(P);
    push_int(P, first / second);
}

prim2(modulo, %)
{
    require_int second = pop_int(P);
    require_int first = pop_int(P);
    push_int(P, first % second);
}
// #endregion

// #region logic prims
prim2(isequalto, =)
{
    require_int second = pop_int(P);
    require_int first = pop_int(P);
    push_bool(P, first == second);
}

prim2(isnotsequalto, !=)
{
    require_int second = pop_int(P);
    require_int first = pop_int(P);
    push_bool(P, first != second);
}

prim2(isgreaterthan, >)
{
    require_int second = pop_int(P);
    require_int first = pop_int(P);
    push_bool(P, first > second);
}

prim2(islessthan, <)
{
    require_int second = pop_int(P);
    require_int first = pop_int(P);
    push_bool(P, first < second);
}

prim2(isgreaterorequal, >=)
{
    require_int second = pop_int(P);
    require_int first = pop_int(P);
    push_bool(P, first >= second);
}

prim2(islesserorequal, <=)
{
    require_int second = pop_int(P);
    require_int first = pop_int(P);
    push_bool(P, first <= second);
}

prim(not )
{
    needstack(1)
        push_bool(P, !pop_bool(P));
}

prim(or)
{
    require_bool second = pop_bool(P);
    require_bool first = pop_bool(P);
    push_bool(P, first || second);
}

prim(and)
{
    require_bool second = pop_bool(P);
    require_bool first = pop_bool(P);
    push_bool(P, first && second);
}
// #endregion

// #region strings
prim(intostr)
{
    require_int num = pop_int(P);
    push_string(P, sfsfromlonglong(num));
}

prim(strcat)
{
    require_string s2 = pop_string(P);
    require_string s1 = pop_string(P);
    push_string(P, sfscatsfs(s1, s2));
}

prim(strcmp)
{
    require_string s2 = pop_string(P);
    require_string s1 = pop_string(P);
    push_int(P, sfscmp(s1, s2));
}

// MUF called this 'strncmp', but renaming it to match 'popn'
prim(strcmpn)
{
    require_int len = pop_int(P);
    require_string s2 = pop_string(P);
    require_string s1 = pop_string(P);
    push_int(P, sfscmp(sfsnewlen(s1, len), sfsnewlen(s2, len)));
}


// MUF used 'stringcmp', but that's terrible notation, and i think it should be like 'strcmpn'
prim(strcmpi)
{
    require_string s2 = pop_string(P);
    require_string s1 = pop_string(P);
    push_int(P, sfscmp(sfstolower(s1), sfstolower(s2)));
}

prim(strcut)
{
    require_int index = pop_int(P);
    require_string str = pop_string(P);
    size_t len = sfslen(str);
    if (index < 0)
    {
        index = 0;
    }
    if (index > len)
    {
        index = len;
    }
    push_string(P, sfsnewlen(str, index));
    push_string(P, sfsright(str, len - index));
}

prim(instr) {
    require_string s2 = pop_string(P);
    require_string s1 = pop_string(P);
    push_int( P, sfsinstr( s1, s2, false ) );
}

// case insensitive instr (was 'instring' in muf)
prim(instri) {
    require_string s2 = pop_string(P);
    require_string s1 = pop_string(P);
    push_int( P, sfsinstr( sfstolower(s1), sfstolower(s2), false ) );
}

// reverse instr
prim(rinstr) {
    require_string s2 = pop_string(P);
    require_string s1 = pop_string(P);
    push_int( P, sfsinstr( s1, s2, true ) );
}

// case insensitive rinstr (was 'rinstring' in muf)
prim(rinstri) {
    require_string s2 = pop_string(P);
    require_string s1 = pop_string(P);
    push_int( P, sfsinstr( sfstolower(s1), sfstolower(s2), true ) );
}

prim(explode) {
    require_string s2 = pop_string(P);
    require_string s1 = pop_string(P);
    size_t count;
    sfs *strings = sfssplit( s1, s2, &count );
    for( size_t i = count; i > 0; --i ) {
        push_string(P, strings[i-1]);
    }
    push_int(P, count);
}

// #endregion

// #region conversion prims
prim(ctoi)
{
    require_string str = pop_string(P);
    push_int(P, str[0]);
}
// #endregion

// #region debugging
prim(debugon)
{
    P->debugmode = true;
}

prim(debugoff)
{
    P->debugmode = false;
}

prim(dumpstack)
{
    printf("%s\n", dump_stack(P));
}

prim2(print, .)
{
    needstack(1) struct datapoint dp = pop_dp(P);
    size_t type = checktype(&dp);
    if (type == a_type_string)
    {
        printf("%s", dp_get_string(&dp));
    }
    else
    {
        printf("%s", formatobject(P->node, &dp));
    }
}
// #endregion