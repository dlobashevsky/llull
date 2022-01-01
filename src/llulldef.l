
%option noyywrap yylineno

%x STRING
%x CODE
%x COMMENT

X		[0-9a-fA-F]
CHAR3	\\x{X}{2}
CHAR4	\\[\\\"abfnrtv]
CHAR5	[^\\\"\n\r]
WS		[ \t\r\n]+

%{
#include "macros.h"
#include "llull.h"
#include "llulldef.tab.h"

static grammar_string_t* global_string=0;

%}

%%

"/*"		{ BEGIN(COMMENT); }
<COMMENT>"*/"	{ BEGIN(INITIAL); }
<COMMENT>.*

"//".*


\"		{ 
                  BEGIN(STRING);
                  if(global_string) grammar_string_free(global_string); 
                  global_string=grammar_string_init(0); 
                }
<STRING>\"	{
                  yylval.s=md_strdup(grammar_string_plain(global_string));
                  BEGIN(INITIAL);
                  return TOKEN_STRING;
                }

<STRING>\\\"		grammar_string_append(global_string,"\\\"");
<STRING>\\\\		grammar_string_append(global_string,"\\\\");
<STRING>.	{ grammar_string_append0(global_string,yytext[0]); }


%\{			{
			  BEGIN(CODE);
			  if(global_string) grammar_string_free(global_string);
                          global_string=grammar_string_init(0);
			}

<CODE>[^%$\n]+		{ grammar_string_append(global_string,yytext); }

<CODE>%+[^}$\n]+	{ grammar_string_append(global_string,yytext); }
<CODE>\n		{ grammar_string_append0(global_string,'\n');}


<CODE>\$\$		{ grammar_string_append(global_string,"_r"); }
<CODE>\$!\$		{ grammar_string_append(global_string,"_rv"); }
<CODE>\$[1-9][0-9]*\$	{
                          grammar_string_append(global_string,"_a");
                          char* t=md_strdup(yytext+1);
                          t[strlen(t)-1]=0;
                          grammar_string_append(global_string,t);
                          md_free(t);
                        }
<CODE>\$@\$		{  grammar_string_append(global_string,"_ctx"); }
<CODE>\$0\$		{  grammar_string_append(global_string,"_a0"); }
<CODE>%\}		{
			  yylval.s=md_strdup(grammar_string_plain(global_string));
			  grammar_string_free(global_string);global_string=0;
			  BEGIN(INITIAL);
			  return FREE_CODE;
			}


[1-9][0-9]*		{  yylval.n=strtoul(yytext,0,0); return TOKEN_NUMBER; }
0[0-7]*			{  yylval.n=strtoul(yytext,0,0);return TOKEN_NUMBER; }
0[xX]{X}+		{  yylval.n=strtoul(yytext,0,0); return TOKEN_NUMBER; }

^%init			{  yylval.s=yytext; return TOKEN_INIT; }
^%name			{  yylval.s=yytext;return TOKEN_NAME; }
^%codon			{  yylval.s=yytext;return TOKEN_CODON; }
^%var			{  yylval.s=yytext;return TOKEN_VAR; }
^%uuid			{  yylval.s=yytext;return TOKEN_UUID; }
^%context		{  yylval.s=yytext;return TOKEN_CONTEXT; }
^%fitness		{  yylval.s=yytext;return TOKEN_FITNESS; }

^%%			{  return TOKEN_BLOCK; }

"<"[a-zA-Z_0-9*\]\[\(\) ]+">"		{ yylval.s=md_strdup(yytext); return TOKEN_CTYPE; }

[a-zA-Z][_a-zA-Z0-9]*			{  yylval.s=md_strdup(yytext); return TOKEN_ID; }

;		{ return TOKEN_SEMICOLON; }
:		{ return TOKEN_RULE; }
\|		{ return TOKEN_VARIANT; }
@		{ return TOKEN_AT; }
\[		{ return TOKEN_SBRA; }
\]		{ return TOKEN_SKET; }


{WS}

.		{ grammar_string_free(global_string); global_string=0; return TOKEN_EOF; }
