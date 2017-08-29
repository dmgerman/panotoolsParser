/*
 *  scanner.h
 *
 *  Copyright  Daniel M. German
 *  
 *  April 2007
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this software; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 *  Author: Daniel M German dmgerman at uvic doooot ca
 * 
 *  This lex input file implements the panotools and hugin input scripts
 *
 *  It ignores 'v' and 'c' lines:
 * 
 *  in 'i' lines it ignores 'K' and 'V' variables which are unique to hugin
 *
 */

%option noyywrap

%{
/* need this for the call to atof() below */
#include <math.h>
#include <string.h>
#include "tparserdebug.h"
#include "tparser.h"
#include "tparserprivate.h"
#include "parser.h"

char strBuffer[PT_TOKEN_MAX_LEN+1];
char *strBuffer_ptr;

double ptTokenInt;
double ptTokenFloat;
char ptTokenLineType;
int inVariable = 0;

#define YY_INPUT(buf,result,max_size)  {\
    result = panoScriptScannerGetNextChar(buf, max_size); \
    if (  result <= 0  ) \
      result = YY_NULL; \
    }

%}

DIGIT    [0-9]
LINETYPE [a-zA-Z]
ID       [a-zA-Z]

%x str
%x parm
%x hugin


%%

  /* Simple rules for all purpose */

^[ \t]*\n    { /* Ignore empty lines */
              DEBUG_1("Empty line\n");
              inVariable = 0;
            }

    /* Simple rules to read numbers */

 
[-+]?{DIGIT}+    { /* Simple integer */
                DEBUG_3( "An integer: %s (%d)\n", yytext, atoi( yytext ) );
                panoScriptScannerTokenBegin(yytext); 
                yylval.fVal = atof( yytext );
                return PT_TOKEN_NUMBER;
               }

[-+]?{DIGIT}+"."{DIGIT}* { /* float point with period */
               /*printf( "A float: %s (%g)\n", yytext, atof( yytext ) );*/
                panoScriptScannerTokenBegin(yytext); 
                yylval.fVal = atof( yytext );
                return PT_TOKEN_NUMBER;
            }

[ \t]*\n    { /* END OF LINE is an important marker when processing a rule */
            DEBUG_2( "End of line", yytext );
            panoScriptScannerTokenBegin(yytext); 
            return PT_TOKEN_EOL;
            }

<<EOF>>     { /* We are done */
               DEBUG_1("End of file\n");
               panoScriptScannerTokenBegin(yytext); 
               return PT_TOKEN_EOF;
            }

[ \t]+      {  /* token separator */
               panoScriptScannerTokenBegin(yytext); 
               return PT_TOKEN_SEP;
            }


   /* Tokens to ignore  for the time being */

^v.*\n   { ; } /* Ignore v lines TODO XXXXXXXXXXXX*/

  /* Hugin specific variables. When they are supported it would
  be easier to generate a new type of token for them. For the
  time being just ignore them */


[K][0-9A-Za-z.=-]+[ \t]* {  /* ignore K parameters */
               panoScriptScannerTokenBegin(yytext);
             }


  /****************************************/
  /* These are the types of lines supported */
   
^o           {   /* Output line */ 
                panoScriptScannerTokenBegin(yytext); 
                return PT_TOKEN_OUTPUT_LINE;
             }

^i           {  /* input line */
                panoScriptScannerTokenBegin(yytext); 
                return PT_TOKEN_INPUT_LINE;
             }

^p           {  /* panorama line */
                panoScriptScannerTokenBegin(yytext); 
                return PT_TOKEN_PANO_LINE;
             }

^m           {  /* Optimization options line */
                panoScriptScannerTokenBegin(yytext); 
                return PT_TOKEN_OPTIMIZE_OPT_LINE;
             }

^c  {
             panoScriptScannerTokenBegin(yytext); 
             return PT_TOKEN_CONTROL_PT_LINE;
             }

^C  {
             panoScriptScannerTokenBegin(yytext); 
             return PT_TOKEN_MORPH_PT_LINE;
             }

^#" hugin project file, version" { /* hugin header file for PTO files */
              DEBUG_1("In PTo version line\n");
              panoScriptScannerTokenBegin(yytext); 
              return PT_TOKEN_HUGIN_VERSION;
            }

^"#hugin_options" { /* #hugin uses #hugin for options */
             panoScriptScannerTokenBegin(yytext); 
             return PT_TOKEN_HUGIN_OPTIONS_LINE;
            }

^"#-hugin"  {     /* and #-hugin for options */
               panoScriptScannerTokenBegin(yytext); 
               BEGIN(hugin);
               return PT_TOKEN_HUGIN_LINE;
            }

          /********************************/
          /* Main tokens in different libpano lines */
","           { 
              panoScriptScannerTokenBegin(yytext); 
              return PT_TOKEN_COMMA;  
              }


[CS]        {
             /* These are the C and S parameters that contain multiple values */
               strncpy(yylval.strVal, yytext, PT_TOKEN_MAX_LEN);
               return PT_TOKEN_CROPPING;
            } 

{ID}        {
                DEBUG_2( "An identifier for a variable: %s\n", yytext );
                if (!inVariable) {
                   inVariable = 1;
                   /* move into parameter mode */
                   BEGIN(parm);
                   panoScriptScannerTokenBegin(yytext); 
                   strncpy(yylval.strVal, yytext, PT_TOKEN_MAX_LEN);
                   return PT_TOKEN_KEYWORD;
                } else {
                   fprintf(stderr, "Parser error: variable within variable \n");
                   exit(1);
                } 
             }

<parm>{DIGIT}+   { /* Simple integer */
                DEBUG_3( "An integer: %s (%d)\n", yytext, atoi( yytext ) );
                /* move back to default mode */
                inVariable = 0;
                BEGIN(INITIAL);
                panoScriptScannerTokenBegin(yytext); 
                yylval.fVal = atof( yytext );
                return PT_TOKEN_NUMBER;
               }

<parm>{DIGIT}+"."{DIGIT}*   { /* a float */
                DEBUG_3( "An float: %s (%f)\n", yytext, atof( yytext ) );
                /* move back to default mode */
                inVariable = 0;
                BEGIN(INITIAL);
                panoScriptScannerTokenBegin(yytext); 
                yylval.fVal = atof( yytext );
                return PT_TOKEN_NUMBER;
               }



<parm>a-zA-Z[0-9a-zA-Z]+ {
                  /* Read the rest of the parameter as a string by default */
                   /* move back to default mode */
                   BEGIN(INITIAL);
                   inVariable = 0;
                   panoScriptScannerTokenBegin(yytext); 
                   strncat(strBuffer, yytext, PT_TOKEN_MAX_LEN -1);
                   strncpy(yylval.strVal, strBuffer, PT_TOKEN_MAX_LEN);
                   return PT_TOKEN_STRING;
                }


  /**********************************/
  /* In hugin mode */

E[erb]v?    { /* Hugin defines these variables in the i line */
                  panoScriptScannerTokenBegin(yytext); 
                  strncpy(yylval.strVal, yytext, PT_TOKEN_MAX_LEN);
                  return PT_TOKEN_KEYWORD;
            }

V[mabcdxy]    { /* Hugin defines these variables in the i line */
                  panoScriptScannerTokenBegin(yytext); 
                  strncpy(yylval.strVal, yytext, PT_TOKEN_MAX_LEN);
                  return PT_TOKEN_KEYWORD;
            }



R[abcde]    { /* Hugin defines these variables in the i line */
                  panoScriptScannerTokenBegin(yytext); 
                  strncpy(yylval.strVal, yytext, PT_TOKEN_MAX_LEN);
                  return PT_TOKEN_KEYWORD;
            }

  /********************************/
   /* From this on process strings */
<parm>\"       { /* Begin of string */
                 panoScriptScannerTokenBegin(yytext); 
                 strBuffer_ptr = strBuffer; 
                 inVariable = 0;
                 BEGIN(str);
                 //printf("Beginning of string\n");
               }

<str>\"        { /* saw closing quote - all done */
                   BEGIN(INITIAL);
                   *strBuffer_ptr = '\0';
                   /* return string constant token type and
                    * value to parser
                    */
              //     printf("A string %s\n", strBuffer);
                   strncpy(yylval.strVal, strBuffer, PT_TOKEN_MAX_LEN);
                   return PT_TOKEN_STRING;
                }

<str>\n        {
                 /* error - unterminated string constant */
                 /* generate error message */
                 panoScriptParserError(1,"non-terminated string: %s\n", yytext ); 
                 return -0;
               }

<str>\\n  *strBuffer_ptr++ = '\n';
<str>\\t  *strBuffer_ptr++ = '\t';
<str>\\r  *strBuffer_ptr++ = '\r';
<str>\\b  *strBuffer_ptr++ = '\b';
<str>\\f  *strBuffer_ptr++ = '\f';

<str>\\(.|\n)  *strBuffer_ptr++ = yytext[1];

<str>[^\\\n\"]+    {
                     int currentLen = strlen(strBuffer);
                     char *yptr = yytext;
                     while ( *yptr && ++currentLen < PT_TOKEN_MAX_LEN)
                           *strBuffer_ptr++ = *yptr++;
                     panoScriptScannerTokenBegin(yytext); 
                   }

  /* Ignore comments */

^#.*\n         {
                  DEBUG_1("Comment\n");
                }



  /********************************/
  /* Catch errors */
.           { 
               panoScriptParserError(1,"Unrecognized character: [%s]\n", yytext );
               return PT_TOKEN_ERROR;
            }



%%

