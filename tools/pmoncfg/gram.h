/* A Bison parser, made by GNU Bison 3.7.6.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_GRAM_H_INCLUDED
# define YY_YY_GRAM_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    AND = 258,                     /* AND  */
    AT = 259,                      /* AT  */
    ATTACH = 260,                  /* ATTACH  */
    BUILD = 261,                   /* BUILD  */
    COMPILE_WITH = 262,            /* COMPILE_WITH  */
    CONFIG = 263,                  /* CONFIG  */
    DEFINE = 264,                  /* DEFINE  */
    DEFOPT = 265,                  /* DEFOPT  */
    DEVICE = 266,                  /* DEVICE  */
    DISABLE = 267,                 /* DISABLE  */
    DUMPS = 268,                   /* DUMPS  */
    ENDFILE = 269,                 /* ENDFILE  */
    XFILE = 270,                   /* XFILE  */
    XOBJECT = 271,                 /* XOBJECT  */
    FLAGS = 272,                   /* FLAGS  */
    INCLUDE = 273,                 /* INCLUDE  */
    XMACHINE = 274,                /* XMACHINE  */
    MAJOR = 275,                   /* MAJOR  */
    MAKEOPTIONS = 276,             /* MAKEOPTIONS  */
    MINOR = 277,                   /* MINOR  */
    ON = 278,                      /* ON  */
    OPTIONS = 279,                 /* OPTIONS  */
    SELECT = 280,                  /* SELECT  */
    UNSELECT = 281,                /* UNSELECT  */
    PSEUDO_DEVICE = 282,           /* PSEUDO_DEVICE  */
    ROOT = 283,                    /* ROOT  */
    SOURCE = 284,                  /* SOURCE  */
    WITH = 285,                    /* WITH  */
    NEEDS_COUNT = 286,             /* NEEDS_COUNT  */
    NEEDS_FLAG = 287,              /* NEEDS_FLAG  */
    RMOPTIONS = 288,               /* RMOPTIONS  */
    ENABLE = 289,                  /* ENABLE  */
    NUMBER = 290,                  /* NUMBER  */
    PATHNAME = 291,                /* PATHNAME  */
    WORD = 292,                    /* WORD  */
    EMPTY = 293                    /* EMPTY  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define AND 258
#define AT 259
#define ATTACH 260
#define BUILD 261
#define COMPILE_WITH 262
#define CONFIG 263
#define DEFINE 264
#define DEFOPT 265
#define DEVICE 266
#define DISABLE 267
#define DUMPS 268
#define ENDFILE 269
#define XFILE 270
#define XOBJECT 271
#define FLAGS 272
#define INCLUDE 273
#define XMACHINE 274
#define MAJOR 275
#define MAKEOPTIONS 276
#define MINOR 277
#define ON 278
#define OPTIONS 279
#define SELECT 280
#define UNSELECT 281
#define PSEUDO_DEVICE 282
#define ROOT 283
#define SOURCE 284
#define WITH 285
#define NEEDS_COUNT 286
#define NEEDS_FLAG 287
#define RMOPTIONS 288
#define ENABLE 289
#define NUMBER 290
#define PATHNAME 291
#define WORD 292
#define EMPTY 293

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 94 "gram.y"

	struct	attr *attr;
	struct	devbase *devb;
	struct	deva *deva;
	struct	nvlist *list;
	const char *str;
	int	val;

#line 152 "gram.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_GRAM_H_INCLUDED  */
