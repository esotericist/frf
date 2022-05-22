## primitives differences

this document is a list of primitives pulled from the protomuck muf manual. as this is simply tables of the names of code elements, i'm assuming this is fair use as per phone books in copyright law.

i'll be using this documentation to indicate what is implemented, what shouldn't be implemented, and what is being altered. it may lag behind the actual implementation in frf substantively, but i'll try not to let it stay that way.

notation:

* `✔️` indicates implemented as-is
* `✦` indicates implemented under a different name, but same semantics (alternate name should be provided)
* `⏏` indicates with substantive semantic changes (notes should be provided)
* `✉` indicates invalid/incoherent to implement at all (generally specific to muck frameworks, notes generally not provided)
* anything otherwise unadorned is either unimplemented, or undocumented.

groups are not in the same order as in the muf manual due to my opinions on reevance.

at some point i'll produce a specific-to-frf document of all primitives to account for the ones unique to frf.

#### stack prims
```markdown
 ✔️DUP        ✔️DUPN       LDUP         ✔️OVER       ✔️POP          
 ✔️ROT        ✔️ROTATE     SORT         ✔️SWAP       ✔️DEPTH     
 ✔️NUMBER?    ✉DBREF?    ✔️INT?         ✔️STRING?     ADDRESS?
  SOCKET?    ✉LOCK?      FLOAT?        VARIABLE?   ARRAY?   
 ✦MARK        }          CHECKARGS     REVERSE     LREVERSE 
 ✔️POPN       ✔️PICK      ✔️PUT          ✉DESCR?      SQL? 
  MARK?      ✔️NIP       ✔️TUCK
```
note: `MARK` in both codebases is actually spelled `{` in actual programs, mark is just the internal name

#### data conversion prims

```markdown
  ATOI     DBREF     INT    ✔️INTOSTR    ✔️CTOI     ITOC
  STOD     DTOS      FLOAT   STRTOF      FTOSTR   ITOH
  HTOI     FTOSTRC
```

#### variable prims
```markdown
 ✔️VAR     VARIABLE     VARIABLE?     LVAR     LOCALVAR
 ✔️@      ✔️!           ✉ME           ✉TRIGGER ✉COMMAND
 ✗DESCR  ✔️VAR!
```

#### control structure prims
```markdown
 ✔️EXIT         CALL       EXECUTE     READ        PUBLIC      SLEEP
  PID          ISPID?     KILL       ✔️IF         ✔️ELSE       ✔️THEN
 ✉QUEUE       ✔️FORK      ✉INTERP     ✉CALLER      ABORT      ✔️CASE
 ✉TRIG        ✉PROG      ✉PARSEMPI   ✉PARSEPROP              ✉CANCALL?
 ✔️BEGIN       ✔️UNTIL     ✔️REPEAT     ✔️WHILE      ✔️BREAK      ✔️CONTINUE
 ✉COMPILED?    TREAD      FOREACH     FOR        ✉PROPQUEUE  ✉ENVPROPQUEUE         
 ✉INSTANCES   ✉MAGECALL  ✉WIZCALL    ✉ARCHCALL   ✉BOYCALL    ✉GETPIDINFO
 ✉GETPIDS      TRY        CATCH       ENDCATCH   ✔️JMP        ✉ENQUEUE
 ✉PARSEPROPEX ✔️DEBUG_ON  ✔️DEBUG_OFF  ✔️DEBUG_LINE ✉SELFCALL    DEBUGGER_BREAK
  READ_WANTS_BLANKS       GET_READ_WANTS_BLANKS   READ_WANTS_BLANKS_TOGGLE
  READ_WANTS_BLANKS_ON    READ_WANTS_BLANKS_OFF   CATCH_DETAILED
```


#### math and logic prims

only ints implemented so far. floats (internally in the form of doubles) will come.

implementation detail: integers have two bits less than normal maxint, due to tagged pointer usage. this is unlikely to matter for most purposes, but should be born in mind.
```markdown
 ✔️+   ✔️-   ✔️*   ✔️/   ✔️%   ✔️<   ✔️>   ✔️=   ✔️!=   ✔️<=   ✔️>=    ^
  RANDOM     BITOR     BITXOR     BITSHIFT     ++     -- 
  CEIL       FLOOR     SQRT       PI           ROUND  
  SIN        COS       TAN        ASIN         ACOS   ATAN
  ATAN2      DIST3D    XYZ_TO_POLAR            POLAR_TO_XYZ  
  EXP        LOG       LOG10      FABS         POW    FRAND
  FMOD       MODF     ✔️AND       ✔️OR          ✔️NOT    ABS
  SRAND      GETSEED   SETSEED    SIGN         XOR    INF
  EPSILON
```


#### string prims

```markdown
 ✔️EXPLODE      ✔️INSTR       ✦INSTRING    ✔️RINSTR      ✦RINSTRING
 ✉PRONOUN_SUB  ✔️STRCMP      ✦STRINGCMP   ✔️STRINGPFX   ✦STRNCMP
 ✔️STRCAT       ✔️SUBST        STRIPTAIL    STRIPLEAD    STRIP
 ✔️STRCUT       ✔️SPLIT       ✔️RSPLIT      ✔️MIDSTR       SMATCH
 ✔️STRLEN       ✔️TOUPPER     ✔️TOLOWER      STRENCRYPT   STRDECRYPT
  FMTSTRING     TOKENSPLIT  ✉TEXTATTR    ✉PARSE_ANSI  ✉UNPARSE_ANSI
 ✉ESCAPE_ANSI  ✉ANSI_STRLEN ✉ANSI_STRCUT ✉ANSI_STRIP   STRIPSPACES
 ✉NAME-OK?     ✉PNAME-OK?    ANSI_MIDSTR ✉PARSE_NEON  ✉FLAG_2CHAR
 ✉POWER_2CHAR   UNESCAPE_URL ESCAPE_URL   SHA1HASH     MD5HASH
  REGMATCH      REGSUB       REGEXP       FMTSTRINGEX
```

some of the naming conventions are highly arbitrary, in particular the tendency to just write out a longer version of part of the name to indicate case insensitivity

i'm opting for `[orientation]action[count][case insensitivity]`, as shown below. i'm sure i'll change my mind someday.

* `STRNCMP` as `strcmpn`
* `STRINGCMP` as `strcmpi`
* `INSTRING` as `instri`
* `RINSTRING` as `rinstri`


#### array prims

```markdown
✔️ARRAY_MAKE          ARRAY_MAKE_DICT    ✔️ARRAY_INTERPRET
 ARRAY_EXPLODE      ✔️ARRAY_VALS          ARRAY_KEYS         ✔️ARRAY_COUNT
 ARRAY_FIRST         ARRAY_LAST          ARRAY_PREV          ARRAY_NEXT
✔️ARRAY_GETITEM      ✔️ARRAY_SETITEM      ✔️ARRAY_APPENDITEM    ARRAY_SORT
 ARRAY_GETRANGE      ARRAY_SETRANGE      ARRAY_INSERTRANGE  ✔️ARRAY?
✔️ARRAY_DELITEM       ARRAY_DELRANGE      ARRAY_NUNION        ARRAY_UNION
 ARRAY_NINTERSECT    ARRAY_INTERSECT     ARRAY_NDIFF         ARRAY_DIFF
✔️ARRAY_JOIN          ARRAY_REVERSE      ✉DESCR_ARRAY         DICTIONARY?
✉ARRAY_GET_PROPDIRS ✉ARRAY_GET_PROPVALS ✉ARRAY_PUT_PROPVALS ✉ARRAY_NOTIFY
✉ARRAY_GET_PROPLIST ✉ARRAY_PUT_PROPLIST ✔️ARRAY_INSERTITEM   ✉ARRAY_ANSI_NOTIFY
✉ARRAY_GET_REFLIST  ✉ARRAY_PUT_REFLIST  ✔️ARRAY_JOIN         ✉ARRAY_NOTIFY_HTML
✉CONTENTS_ARRAY     ✉EXITS_ARRAY         ARRAY_FINDVAL      ✉FIND_ARRAY
✉ENTRANCES_ARRAY    ✉ONLINE_ARRAY        ARRAY_EXCLUDEVAL   ✉REGFIND_ARRAY
✉GETOBJINFO         ✉GETDESCRINFO        ARRAY_EXTRACT      ✉ARRAY_FILTER_FLAGS
 ARRAY_MATCHVAL      ARRAY_MATCHKEY      ARRAY_FILTER_SMART ✉ARRAY_FILTER_PROP
 ARRAY_REGMATCHVAL   ARRAY_REGMATCHKEY   ARRAY_REGSUB       ✉ARRAY_REGFILTER_PROP
 ARRAY_CUT           ARRAY_COMPARE       ARRAY_SORT_INDEXED  ARRAY_NESTED_GET
 ARRAY_SUM          ✉ARRAY_GETLINKS      ARRAY_FMTSTRINGS    ARRAY_NESTED_SET
 ARRAY_STRING_FRAGMENT                                       ARRAY_NESTED_DEL

            }ARRAY     }LIST     }DICT     }JOIN    }CAT      }TELL    

            []         ->[]      []<-      [^]      [x]       [..]
                      *->[]     *[]<-     *[^]     *[x]
```


#### multitasking prims
the multitasking prims are mostly muck-specific, or are heavily informed by muck heritage. i doubt any of them will be implemented as-is.

```markdown
  PREEMPT       FOREGROUND      BACKGROUND     MODE         SETMODE
  PR_MODE       FG_MODE         BG_MODE        SLEEP
  EVENT_WAIT    EVENT_SEND      EVENT_COUNT    EVENT_EXISTS
  TIMER_START   TIMER_STOP      EVENT_WAITFOR  WATCHPID
  ONEVENT       INTERRUPT_LEVEL
```

#### time prims
```markdown
  TIME     SYSTIME     TIMESTAMPS     TIMESPLIT     TIMEFMT
  DATE     GMTOFFSET   SYSTIME_PRECISE
```

#### file prims
file primitives warrant distinct semantics if they're folded into the event system. that said, keeping this list here as a reference
```markdown
  FWRITE    FREAD    FREADN    FAPPEND    FCR      FREADTO
  BWRITE    BREAD    BAPPEND   FRM        FREN     MKDIR
  FPUBLISH  FSINFO   FSTATS    CURID      FSIZE    GETDIR
  RMDIR
```

#### socket prims
socket primitives, like file primitives, probably need restructuring in the context of events.
```markdown
  SOCKOPEN        SOCKCLOSE      SOCKSEND      SOCKRECV       NBSOCKOPEN
  SOCKCHECK       LSOCKOPEN      SOCKACCEPT    GET_SOCKINFO   SOCKET_SETUSER
  SET_SOCKOPT     SOCKTODESCR    SOCKSHUTDOWN  SOCKSECURE     SOCKUNSECURE
  SSL_SOCKACCEPT  UDPSEND        UDPOPEN       UDPCLOSE       UDP6SEND
  NBSOCK6OPEN     LSOCK6OPEN     SOCK6OPEN
```

#### fully muck specific prim groups
below this point are primitive sets that are fully muck-specific. it's highly unlikely anything in these groups sharing a name with anything implemented in frf will have the same purpose, let alone semantics.

they are kept in place purely for historical reference purposes (and also in case i ever decide to implement muck semantics in an frf application for some insane reason)

```markdown
System Prims:
  SYSPARM         SETSYSPARM     LOGSTATUS     VERSION     FORCE
  FORCE_LEVEL     MOTD_NOTIFY    COMPILE       UNCOMPILE   BANDWIDTH
  DUMP            DELTA          SHUTDOWN      RESTART     ARMAGEDDON
  SYSPARM_ARRAY   SUID
```

```markdown
Connection Prims:
  AWAKE?     ONLINE     CONCOUNT     CONDBREF     CONTIME
  CONIDLE    CONDESCR   CONHOST      CONBOOT      CONNOTIFY
  CONUSER    CONIPNUM   CONPORT      DESCRCON     NEXTDESCR 
  DESCRIPTORS      DESCR_SETUSER     DESCR        DESCRFLUSH
  DESCR_HTML?      DESCR_PUEBLO?     DESCR?       DESCR_WELCOME_USER
  DESCR_LOGOUT     DESCRDBREF        DESCRIDLE    DESCRTIME
  DESCRHOST        DESCRIPNUM        DESCRPORT    DESCRCONPORT
  DESCRLEASTIDLE   DESCRMOSTIDLE     FIRSTDESCR   LASTDESCR
  DESCRUSER        GETDESCRINFO      DESCRBOOT    DESCR_SET
  DESCR_FLAG?      DESCRBUFSIZE
```


```markdown
Locks Management:
  SETLOCKSTR     GETLOCKSTR     LOCKED?     PARSELOCK
  UNPARSELOCK    PRETTYLOCK     TESTLOCK    LOCK?
  ISLOCKED? 
```


```markdown
Property Prims:
  ADDPROP     GETPROP       SETPROP         ENVPROP       REMOVE_PROP
  GETPROPSTR  GETPROPVAL    GETPROPFVAL     ENVPROPSTR    NEXTPROP
  DESC        IDESC         HTMLDESC        IHTMLDESC     PROPDIR?
  SETDESC     SETSUCC       SETFAIL         SETDROP       SETOSUCC
  SETOFAIL    SETODROP      SUCC            FAIL          DROP
  OSUCC       OFAIL         ODROP           GETLOCKSTR  
  ANSIDESC    SETANSIDESC   IANSIDESC       SETIANSIDESC
  REFLIST_ADD REFLIST_FIND  REFLIST_DEL     COPYPROPS
```


```markdown
Notifying Prims (I/O):
  NOTIFY              NOTIFY_EXCLUDE              NOTIFY_EXCEPT
  ANSI_NOTIFY         ANSI_NOTIFY_EXCLUDE         ANSI_NOTIFY_EXCEPT
  NOTIFY_HTML         NOTIFY_HTML_EXCLUDE         NOTIFY_HTML_EXCEPT
  NOTIFY_HTML_NOCR    NOTIFY_HTML_EXCLUDE_NOCR    HTML_NOCR_EXCEPT
  NOTIFY_DESCRIPTOR   ARRAY_NOTIFY                DESCRFLUSH
  STOPMIDI            PLAYMIDI                    COMMANDTEXT
  ARRAY_ANSI_NOTIFY   ARRAY_NOTIFY_HTML           ANSI_NOTIFY_DESCRIPTOR
  NOTIFY_DESCRIPTOR_CHAR
```

```markdown
Data Base Prims:
  CONTENTS      EXITS       NEXT         NEXTOWNED      NEXTPLAYER
  SETNAME       SET         SETOWN       SETLINK        GETLINKS
  MATCH         RMATCH      PART_PMATCH  PMATCH         NEWPASSWORD
  NEWPLAYER     COPYPLAYER  TOADPLAYER   OBJMEM         SETPASSWORD
  GETLINK       LOCATION    OWNER        CONTROLS       NEXT_FLAG 
  FLAG?         MLEVEL      DBCMP        CHECKPASSWORD  NEXTOWNED_FLAG
  NEWROOM       NEWOBJECT   NEWEXIT      TIMESTAMPS     NEXTTHING_FLAG
  PENNIES       ADDPENNIES  NAME         UNPARSEOBJ     MOVEPENNIES
  COPYOBJ       RECYCLE     MOVETO       STATS          DBTOP
  OK?           THING?      EXIT?        PLAYER?        PROGRAM?   ROOM?
  NEXTPROGRAM   NEXTEXIT    NEXTROOM     NEXTTHING      NEXTPLAYER_FLAG
  NEXTENTRANCE  POWER?      ISPOWER?     NEWPROGRAM     NEXTPLAYER_POWER   
  PNAME-OK?     NAME-OK?    FINDNEXT     CONTENTS_ARRAY EXITS_ARRAY
  GETOBJINFO    FIND_ARRAY  FLAG_2CHAR   POWER_2CHAR    ENTRANCES_ARRAY
  ANSI_NAME     REFSTAMPS   TOUCH        USE            ANSI_UNPARSEOBJ
  ISFLAG?                   REGFINDNEXT                 GETLINKS_ARRAY
                                                        REGFIND_ARRAY
```

```markdown
MCP Prims:
  MCP_REGISTER    MCP_REGISTER EVENT   MCP_SUPPORTS       MCP_BIND
  MCP_SEND        GUI_AVAILABLE        GUI_DLOG_CREATE    GUI_DLOG_SHOW
  GUI_DLOG_CLOSE  GUI_CTRL_CREATE      GUI_VALUES_SET     GUI_VALUES_GET
  GUI_VALUE_GET   GUI_CTRL_COMMAND
```

```markdown
MUF Editing Prims:
  KILL_MACRO          INSERT_MACRO      GET_MACROS_ARRAY 
  PROGRAM_LINECOUNT   PROGRAM_GETLINES  PROGRAM_DELETELINES  
  PROGRAM_INSERTLINES

MySQL Prims:
  SQLQUERY    SQLCONNECT    SQLCLOSE    SQLPING    SQL?

ANSI Prims:
 ANSI_MIDSTR            ANSI_NAME              ANSI_NOTIFY
 ANSI_NOTIFY_DESCRIPTOR ANSI_NOTIFY_EXCEPT     ANSI_NOTIFY_EXCLUDE
 ANSI_STRCUT            ANSI_STRIP             ANSI_STRLEN
 ANSI_UNPARSEOBJ        ARRAY_ANSI_NOTIFY      ESCAPE_ANSI
 PARSE_ANSI             UNPARSE_ANSI

Webserver Prims:
 BASE64ENCODE           BASE64DECODE           HTTPDATA

Ignore Support Prims:
 MAX_IGNORES            IGNORE_ADD	       IGNORE_DEL
 IGNORING?              ARRAY_GET_IGNORES
```

