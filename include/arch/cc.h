/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
n * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __CC_H__
#define __CC_H__

#include <stdlib.h>
#include <stellaris/hw_types.h>
#include <stellaris/utils/uartstdio.h>

#define uipprintf(...) UARTprintf(__VA_ARGS__)

#ifndef __GNUC__
	#include <limits.h>
	#pragma warning (disable: 4244) /* disable conversion warning (implicit integer promotion!) */
	#pragma warning (disable: 4127) /* conditional expression is constant */
	#pragma warning (disable: 4996) /* 'strncpy' was declared deprecated */
	#pragma warning (disable: 4103) /* structure packing changed by including file */
#endif

#define LWIP_PROVIDE_ERRNO

typedef unsigned    char    u8_t;
typedef signed      char    s8_t;
typedef unsigned    short   u16_t;
typedef signed      short   s16_t;
typedef unsigned    long    u32_t;
typedef signed      long    s32_t;
typedef u32_t               mem_ptr_t;
typedef u32_t sys_prot_t;

#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif

#if defined(__arm__) && defined(__ARMCC_VERSION)
    //
    // Setup PACKing macros for KEIL/RVMDK Tools
    //
    #define PACK_STRUCT_BEGIN __packed
    #define PACK_STRUCT_STRUCT 
    #define PACK_STRUCT_END
    #define PACK_STRUCT_FIELD(x) x
#elif defined (__IAR_SYSTEMS_ICC__)
    //
    // Setup PACKing macros for IAR Tools
    //
    #define PACK_STRUCT_BEGIN
    #define PACK_STRUCT_STRUCT
    #define PACK_STRUCT_END
    #define PACK_STRUCT_FIELD(x) x
    #define PACK_STRUCT_USE_INCLUDES
#else
    //
    // Setup PACKing macros for GCC Tools
    //
    #define PACK_STRUCT_BEGIN
    #define PACK_STRUCT_STRUCT __attribute__ ((__packed__))
    #define PACK_STRUCT_END
    #define PACK_STRUCT_FIELD(x) x
#endif

/* Define (sn)printf formatters for these lwIP types */
#define X8_F  "x"
#define U16_F "u"
#define S16_F "d"
#define X16_F "x"
#define U32_F "u"
#define S32_F "d"
#define X32_F "x"
#define SZT_F U32_F

/* Plaform specific diagnostic output */
#define LWIP_PLATFORM_DIAG(x)   do { UARTprintf x; } while(0)

#define LWIP_PLATFORM_ASSERT(x) do { UARTprintf("Assertion \"%s\" failed at line %d in %s\n", \
												x, __LINE__, __FILE__); for(;;); } while(0)

#define LWIP_ERROR(message, expression, handler) do { if (!(expression)) { \
  printf("Assertion \"%s\" failed at line %d in %s\n", message, __LINE__, __FILE__); \
  ;handler;} } while(0)

/* C runtime functions redefined */
u32_t dns_lookup_external_hosts_file(const char *name);

#define LWIP_RAND() ((u32_t)rand())

#endif
