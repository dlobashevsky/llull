#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>

#include "macros.h"
#include "context.h"
#include "data.h"
#include "irisdt.h"
#include "irisdt__proto.h"


float irisdt__attr_pw(irisdt_ctx_t* ctx,int* err) {  *err=0;  return ctx->data->data[ctx->line].attrs[0]; }
float irisdt__attr_pl(irisdt_ctx_t* ctx,int* err) {  *err=0;  return ctx->data->data[ctx->line].attrs[1]; }
float irisdt__attr_sw(irisdt_ctx_t* ctx,int* err) {  *err=0;  return ctx->data->data[ctx->line].attrs[2]; }
float irisdt__attr_sl(irisdt_ctx_t* ctx,int* err) {  *err=0;  return ctx->data->data[ctx->line].attrs[3]; }

float irisdt__var(irisdt_ctx_t* ctx,irisdt__codon_t v,int* err) {  *err=0;  return v/128.; }

