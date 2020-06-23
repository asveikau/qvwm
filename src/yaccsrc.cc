/* A Bison parser, made by GNU Bison 1.875a.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     VARIABLE = 258,
     MENU = 259,
     SHORTCUT = 260,
     APP = 261,
     KEY = 262,
     IND = 263,
     EXITDLG = 264,
     STARTUP = 265,
     ACC = 266,
     VAR = 267,
     STRING = 268,
     FUNC = 269,
     PLUS = 270,
     MINUS = 271
   };
#endif
#define VARIABLE 258
#define MENU 259
#define SHORTCUT 260
#define APP 261
#define KEY 262
#define IND 263
#define EXITDLG 264
#define STARTUP 265
#define ACC 266
#define VAR 267
#define STRING 268
#define FUNC 269
#define PLUS 270
#define MINUS 271




/* Copy the first part of user declarations.  */
#line 21 "yaccsrc.yy"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "main.h"
#include "qvwm.h"
#include "parse.h"
#include "util.h"

extern int line;
extern char filename[256];
extern "C" {
extern int yylex();
extern int yyerror(const char* error);
}

#define YYDEBUG 0


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 41 "yaccsrc.yy"
typedef union YYSTYPE {
  char* str;
  MenuElem* mItem;
  AttrStream* aStream;
  unsigned int modifier;
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 135 "yaccsrc.cc"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 147 "yaccsrc.cc"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  40
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   91

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  20
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  24
/* YYNRULES -- Number of rules. */
#define YYNRULES  69
/* YYNRULES -- Number of states. */
#define YYNSTATES  93

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   271

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    18,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    17,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    19,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned char yyprhs[] =
{
       0,     0,     3,     5,     6,     9,    11,    14,    16,    19,
      21,    24,    26,    29,    31,    34,    36,    39,    41,    44,
      46,    49,    51,    54,    56,    59,    61,    65,    69,    72,
      74,    78,    82,    88,    91,    93,   100,   105,   111,   115,
     122,   127,   133,   137,   140,   142,   145,   149,   151,   157,
     161,   164,   166,   170,   174,   177,   180,   182,   186,   190,
     194,   196,   199,   201,   204,   207,   209,   211,   214,   216
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      21,     0,    -1,    22,    -1,    -1,    23,    22,    -1,    23,
      -1,     3,    24,    -1,     3,    -1,     6,    30,    -1,     6,
      -1,     4,    26,    -1,     4,    -1,     5,    28,    -1,     5,
      -1,     7,    35,    -1,     7,    -1,     8,    38,    -1,     8,
      -1,     9,    33,    -1,     9,    -1,    10,    40,    -1,    10,
      -1,    11,    42,    -1,    11,    -1,    25,    24,    -1,    25,
      -1,    12,    17,    13,    -1,    12,    17,    12,    -1,    27,
      26,    -1,    27,    -1,    13,    13,    13,    -1,    13,    13,
      14,    -1,    13,    13,    15,    26,    16,    -1,    29,    28,
      -1,    29,    -1,    13,    13,    13,    12,    18,    12,    -1,
      13,    13,    13,    12,    -1,    13,    13,    13,    18,    12,
      -1,    13,    13,    13,    -1,    13,    13,    14,    12,    18,
      12,    -1,    13,    13,    14,    12,    -1,    13,    13,    14,
      18,    12,    -1,    13,    13,    14,    -1,    31,    30,    -1,
      31,    -1,    13,    32,    -1,    12,    18,    32,    -1,    12,
      -1,    12,    17,    13,    18,    32,    -1,    12,    17,    13,
      -1,    34,    33,    -1,    34,    -1,    12,    13,    14,    -1,
      12,    13,    13,    -1,    12,    13,    -1,    36,    35,    -1,
      36,    -1,    12,    37,    13,    -1,    12,    37,    14,    -1,
      12,    19,    37,    -1,    12,    -1,    39,    38,    -1,    39,
      -1,    13,    13,    -1,    41,    40,    -1,    41,    -1,    13,
      -1,    43,    42,    -1,    43,    -1,    13,    12,    12,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned char yyrline[] =
{
       0,    59,    59,    60,    63,    64,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    87,    88,    91,    92,    95,    96,
      99,   100,   101,   104,   105,   108,   110,   112,   114,   116,
     118,   120,   122,   126,   127,   130,   133,   134,   135,   136,
     139,   140,   143,   144,   145,   148,   149,   152,   153,   156,
     157,   160,   161,   164,   167,   168,   171,   174,   175,   178
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "VARIABLE", "MENU", "SHORTCUT", "APP", 
  "KEY", "IND", "EXITDLG", "STARTUP", "ACC", "VAR", "STRING", "FUNC", 
  "PLUS", "MINUS", "'='", "','", "'|'", "$accept", "qvwmrc", "sessions", 
  "session", "vars", "var", "items", "item", "scs", "sc", "apps", "app", 
  "stream", "dlgitems", "dlgitem", "keys", "key", "mod", "inds", "ind", 
  "cmds", "cmd", "accs", "acc", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,    61,    44,   124
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    20,    21,    21,    22,    22,    23,    23,    23,    23,
      23,    23,    23,    23,    23,    23,    23,    23,    23,    23,
      23,    23,    23,    23,    24,    24,    25,    25,    26,    26,
      27,    27,    27,    28,    28,    29,    29,    29,    29,    29,
      29,    29,    29,    30,    30,    31,    32,    32,    32,    32,
      33,    33,    34,    34,    34,    35,    35,    36,    36,    37,
      37,    38,    38,    39,    40,    40,    41,    42,    42,    43
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     0,     2,     1,     2,     1,     2,     1,
       2,     1,     2,     1,     2,     1,     2,     1,     2,     1,
       2,     1,     2,     1,     2,     1,     3,     3,     2,     1,
       3,     3,     5,     2,     1,     6,     4,     5,     3,     6,
       4,     5,     3,     2,     1,     2,     3,     1,     5,     3,
       2,     1,     3,     3,     2,     2,     1,     3,     3,     3,
       1,     2,     1,     2,     2,     1,     1,     2,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       3,     7,    11,    13,     9,    15,    17,    19,    21,    23,
       0,     2,     5,     0,     6,    25,     0,    10,    29,     0,
      12,    34,     0,     8,    44,     0,    14,    56,     0,    16,
      62,     0,    18,    51,    66,    20,    65,     0,    22,    68,
       1,     4,     0,    24,     0,    28,     0,    33,    47,    45,
      43,    60,     0,    55,    63,    61,    54,    50,    64,     0,
      67,    27,    26,    30,    31,     0,    38,    42,     0,     0,
       0,    57,    58,    53,    52,    69,     0,    36,     0,    40,
       0,    49,    46,    59,    32,     0,    37,     0,    41,     0,
      35,    39,    48
};

/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,    10,    11,    12,    14,    15,    17,    18,    20,    21,
      23,    24,    49,    32,    33,    26,    27,    52,    29,    30,
      35,    36,    38,    39
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -69
static const yysigned_char yypact[] =
{
      -1,     8,    -2,    19,    20,    22,    23,    25,    26,    27,
      14,   -69,    -1,    18,   -69,     8,    28,   -69,    -2,    29,
     -69,    19,    31,   -69,    20,    32,   -69,    22,    33,   -69,
      23,    35,   -69,    25,   -69,   -69,    26,    37,   -69,    27,
     -69,   -69,    10,   -69,     2,   -69,    11,   -69,     9,   -69,
     -69,    34,    15,   -69,   -69,   -69,    17,   -69,   -69,    38,
     -69,   -69,   -69,   -69,   -69,    -2,     0,     1,    39,    31,
      32,   -69,   -69,   -69,   -69,   -69,    40,    36,    43,    41,
      45,    42,   -69,   -69,   -69,    46,   -69,    49,   -69,    31,
     -69,   -69,   -69
};

/* YYPGOTO[NTERM-NUM].  */
static const yysigned_char yypgoto[] =
{
     -69,   -69,    50,   -69,    30,   -69,   -18,   -69,    44,   -69,
      47,   -69,   -68,     5,   -69,    24,   -69,    -7,    48,   -69,
      51,   -69,    52,   -69
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
{
      45,    82,     1,     2,     3,     4,     5,     6,     7,     8,
       9,    16,    77,    79,    40,    63,    64,    65,    78,    80,
      13,    92,    61,    62,    66,    67,    68,    69,    71,    72,
      73,    74,    19,    22,    25,    42,    28,    31,    57,    34,
      37,    44,    46,    48,    51,    43,    54,    76,    56,    59,
      75,    53,    81,    70,    85,    86,    84,    88,    90,    87,
      89,    91,    41,    83,     0,    47,     0,     0,     0,     0,
       0,    50,     0,     0,     0,     0,     0,     0,    55,     0,
       0,     0,     0,     0,     0,     0,     0,    58,     0,     0,
       0,    60
};

static const yysigned_char yycheck[] =
{
      18,    69,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    13,    12,    12,     0,    13,    14,    15,    18,    18,
      12,    89,    12,    13,    13,    14,    17,    18,    13,    14,
      13,    14,    13,    13,    12,    17,    13,    12,    33,    13,
      13,    13,    13,    12,    12,    15,    13,    65,    13,    12,
      12,    27,    13,    19,    18,    12,    16,    12,    12,    18,
      18,    12,    12,    70,    -1,    21,    -1,    -1,    -1,    -1,
      -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    36,    -1,    -1,
      -1,    39
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      21,    22,    23,    12,    24,    25,    13,    26,    27,    13,
      28,    29,    13,    30,    31,    12,    35,    36,    13,    38,
      39,    12,    33,    34,    13,    40,    41,    13,    42,    43,
       0,    22,    17,    24,    13,    26,    13,    28,    12,    32,
      30,    12,    37,    35,    13,    38,    13,    33,    40,    12,
      42,    12,    13,    13,    14,    15,    13,    14,    17,    18,
      19,    13,    14,    13,    14,    12,    26,    12,    18,    12,
      18,    13,    32,    37,    16,    18,    12,    18,    12,    18,
      12,    12,    32
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrlab1


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (cinluded).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylineno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylineno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 59 "yaccsrc.yy"
    { DoAllSetting(); }
    break;

  case 3:
#line 60 "yaccsrc.yy"
    { DoAllSetting(); }
    break;

  case 10:
#line 71 "yaccsrc.yy"
    { CompleteMenu(yyvsp[-1].str, yyvsp[0].mItem); }
    break;

  case 12:
#line 73 "yaccsrc.yy"
    { CompleteMenu(yyvsp[-1].str, yyvsp[0].mItem); }
    break;

  case 18:
#line 79 "yaccsrc.yy"
    { CompleteMenu(yyvsp[-1].str, yyvsp[0].mItem); }
    break;

  case 26:
#line 91 "yaccsrc.yy"
    { AssignVariable(yyvsp[-2].str, yyvsp[0].str); }
    break;

  case 27:
#line 92 "yaccsrc.yy"
    { AssignVariable(yyvsp[-2].str, yyvsp[0].str); }
    break;

  case 28:
#line 95 "yaccsrc.yy"
    { yyval.mItem = ChainMenuItem(yyvsp[-1].mItem, yyvsp[0].mItem); }
    break;

  case 29:
#line 96 "yaccsrc.yy"
    { yyval.mItem = ChainMenuItem(yyvsp[0].mItem, NULL); }
    break;

  case 30:
#line 99 "yaccsrc.yy"
    { yyval.mItem = MakeExecItem(yyvsp[-2].str, yyvsp[-1].str, yyvsp[0].str); }
    break;

  case 31:
#line 100 "yaccsrc.yy"
    { yyval.mItem = MakeFuncItem(yyvsp[-2].str, yyvsp[-1].str, yyvsp[0].str); }
    break;

  case 32:
#line 101 "yaccsrc.yy"
    { yyval.mItem = MakeDirItem(yyvsp[-4].str, yyvsp[-3].str, yyvsp[-1].mItem); }
    break;

  case 33:
#line 104 "yaccsrc.yy"
    { yyval.mItem = ChainMenuItem(yyvsp[-1].mItem, yyvsp[0].mItem); }
    break;

  case 34:
#line 105 "yaccsrc.yy"
    { yyval.mItem = ChainMenuItem(yyvsp[0].mItem, NULL); }
    break;

  case 35:
#line 109 "yaccsrc.yy"
    { yyval.mItem = MakeDesktopItem(yyvsp[-5].str, yyvsp[-4].str, yyvsp[-3].str, yyvsp[-2].str, yyvsp[0].str); }
    break;

  case 36:
#line 111 "yaccsrc.yy"
    { yyval.mItem = MakeDesktopItem(yyvsp[-3].str, yyvsp[-2].str, yyvsp[-1].str, yyvsp[0].str, NULL); }
    break;

  case 37:
#line 113 "yaccsrc.yy"
    { yyval.mItem = MakeDesktopItem(yyvsp[-4].str, yyvsp[-3].str, yyvsp[-2].str, NULL, yyvsp[0].str); }
    break;

  case 38:
#line 115 "yaccsrc.yy"
    { yyval.mItem = MakeDesktopItem(yyvsp[-2].str, yyvsp[-1].str, yyvsp[0].str, NULL, NULL); }
    break;

  case 39:
#line 117 "yaccsrc.yy"
    { yyval.mItem = MakeDesktopFuncItem(yyvsp[-5].str, yyvsp[-4].str, yyvsp[-3].str, yyvsp[-2].str, yyvsp[0].str); }
    break;

  case 40:
#line 119 "yaccsrc.yy"
    { yyval.mItem = MakeDesktopFuncItem(yyvsp[-3].str, yyvsp[-2].str, yyvsp[-1].str, yyvsp[0].str, NULL); }
    break;

  case 41:
#line 121 "yaccsrc.yy"
    { yyval.mItem = MakeDesktopFuncItem(yyvsp[-4].str, yyvsp[-3].str, yyvsp[-2].str, NULL, yyvsp[0].str); }
    break;

  case 42:
#line 123 "yaccsrc.yy"
    { yyval.mItem = MakeDesktopFuncItem(yyvsp[-2].str, yyvsp[-1].str, yyvsp[0].str, NULL, NULL); }
    break;

  case 45:
#line 130 "yaccsrc.yy"
    { CreateAppHash(yyvsp[-1].str, yyvsp[0].aStream); }
    break;

  case 46:
#line 133 "yaccsrc.yy"
    { yyval.aStream = MakeStream(yyvsp[-2].str, NULL, yyvsp[0].aStream); }
    break;

  case 47:
#line 134 "yaccsrc.yy"
    { yyval.aStream = MakeStream(yyvsp[0].str, NULL, NULL); }
    break;

  case 48:
#line 135 "yaccsrc.yy"
    { yyval.aStream = MakeStream(yyvsp[-4].str, yyvsp[-2].str, yyvsp[0].aStream); }
    break;

  case 49:
#line 136 "yaccsrc.yy"
    { yyval.aStream = MakeStream(yyvsp[-2].str, yyvsp[0].str, NULL); }
    break;

  case 50:
#line 139 "yaccsrc.yy"
    { yyval.mItem = ChainMenuItem(yyvsp[-1].mItem, yyvsp[0].mItem); }
    break;

  case 51:
#line 140 "yaccsrc.yy"
    { yyval.mItem = ChainMenuItem(yyvsp[0].mItem, NULL); }
    break;

  case 52:
#line 143 "yaccsrc.yy"
    { yyval.mItem = MakeDlgFuncItem(yyvsp[-2].str, yyvsp[-1].str, yyvsp[0].str); }
    break;

  case 53:
#line 144 "yaccsrc.yy"
    { yyval.mItem = MakeDlgItem(yyvsp[-2].str, yyvsp[-1].str, yyvsp[0].str); }
    break;

  case 54:
#line 145 "yaccsrc.yy"
    { yyval.mItem = MakeDlgFuncItem(yyvsp[-1].str, yyvsp[0].str, NULL); }
    break;

  case 57:
#line 152 "yaccsrc.yy"
    { CreateSCKey(yyvsp[-2].str, yyvsp[-1].modifier, yyvsp[0].str); }
    break;

  case 58:
#line 153 "yaccsrc.yy"
    { CreateSCKeyFunc(yyvsp[-2].str, yyvsp[-1].modifier, yyvsp[0].str); }
    break;

  case 59:
#line 156 "yaccsrc.yy"
    { yyval.modifier = MakeModifier(yyvsp[-2].str, yyvsp[0].modifier); }
    break;

  case 60:
#line 157 "yaccsrc.yy"
    { yyval.modifier = MakeModifier(yyvsp[0].str, 0); }
    break;

  case 63:
#line 164 "yaccsrc.yy"
    { CreateIndicator(yyvsp[-1].str, yyvsp[0].str); }
    break;

  case 66:
#line 171 "yaccsrc.yy"
    { if (!restart) ExecCommand(yyvsp[0].str); }
    break;

  case 69:
#line 178 "yaccsrc.yy"
    { CreateAccessory(yyvsp[-2].str, yyvsp[-1].str, yyvsp[0].str); }
    break;


    }

/* Line 999 of yacc.c.  */
#line 1304 "yaccsrc.cc"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("syntax error, unexpected ") + 1;
	  yysize += yystrlen (yytname[yytype]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
	  /* Pop the error token.  */
          YYPOPSTACK;
	  /* Pop the rest of the stack.  */
	  while (yyss < yyssp)
	    {
	      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
	      yydestruct (yystos[*yyssp], yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval);
      yychar = YYEMPTY;

    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      yyvsp--;
      yystate = *--yyssp;

      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 181 "yaccsrc.yy"

int yyerror(const char* error)
{
  QvwmError("%s: %d: %s", filename, line, error);
  QvwmError("Restarting with the minimum configuration...");

  // restart without parsing any configuration files
  RestartQvwm(True);
}

