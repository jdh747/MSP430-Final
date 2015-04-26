/* ============================================================================
 * Copyright (c) 2011, Peter A. Bigot (pabigot@users.sourceforge.net)
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  *  Neither the name of the copyright owner nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ============================================================================
 */

/* Glue to convert TI-provided headers to GCC-compatible declarations */

#if !defined(__MSP430_IOMACROS_H_)
#define __MSP430_IOMACROS_H_

#if defined(__ASSEMBLER__)

#define sfrb(x,x_) x=x_
#define sfrw(x,x_) x=x_
#define sfra(x,x_) x=x_

#define const_sfrb(x,x_) sfrb(x,x_)
#define const_sfrw(x,x_) sfrw(x,x_)
#define const_sfra(x,x_) sfra(x,x_)

#else /* __ASSEMBLER__ */

/* __d16__ is a type attribute that tells the compiler that the
 * address of an object with that type requires 16 bits.  It is paired
 * with __d20__, which indicates that the object may have an address
 * that requires 20 bits.
 *
 * These type attributes are semi-independent of the __near__ and
 * __far__ declaration attributes, which affect placement of an
 * object.  These attributes would be required when providing the
 * *definition* of an object, to ensure the object was given an
 * appropriate address.  ("semi-independent" because use of __near__
 * (__far__) on a declaration implicitly adds __d16__ (__d20__) to the
 * type so that pointers to the object will be correct.  The reverse
 * is not true.)
 *
 * The declaration attributes are not necessary here, because the
 * placement of the peripheral is specified through the linker
 * peripheral map: there is no section associated with the symbol.  We
 * are only informing the compiler that addresses of the peripherals
 * will always be 16 bits so it can generate better code.  Should
 * it be necessary to introduce your own declarations of peripheral
 * registers, you may use the __MSPGCC_PERIPHERAL__ macro in the same
 * manner.
 *
 * When writing code that involves objects that are pointers to
 * peripheral registers, the __d16__ attribute is not appropriate,
 * because it would apply to the pointer type, not to the type being
 * pointed to.  This can be avoided by using an intermediate typedef
 * to bind the attribute at the intended level of the type hierarchy,
 * as with:
 * 
 *   typedef __MSPGCC__PERIPHERAL__ unsigned char periph8_t;
 *   volatile periph8_t * pxout = &P1OUT;
 *
 * However, it is simpler to use the following in a standard pointer
 * declaration to note that the value of the pointer requires only 16
 * bits:
 *
 *   volatile unsigned char * __MSPGCC_POINTER_TO_PERIPHERAL__ pxout = &P1OUT;
 *   
 * Attribute __d16__ and the __MSPGCC_PERIPHERAL__ and
 * __MSPGCC_POINTER_TO_PERIPHERAL__ macros are present in public
 * releases of mspgcc strictly after release 20120627.
 */

#if __MSP430_CPU__ & __MSP430_CPU_MSP430X__
#define __MSPGCC_PERIPHERAL__ __attribute__((__d16__))
#define __MSPGCC_POINTER_TO_PERIPHERAL__ __attribute__((__a16__))
#else /* 430X available */
#define __MSPGCC_PERIPHERAL__
#define __MSPGCC_POINTER_TO_PERIPHERAL__
#endif /* 430X available */

#define sfrb_(x,x_) volatile __MSPGCC_PERIPHERAL__ unsigned char x __asm__("__" #x)
#define sfrb(x,x_) extern sfrb_(x,x_)

#define sfrw_(x,x_) volatile __MSPGCC_PERIPHERAL__ unsigned int x __asm__("__" #x)
#define sfrw(x,x_) extern sfrw_(x,x_)

#define const_sfrb(x,x_) extern const sfrb_(x,x_)
#define const_sfrw(x,x_) extern const sfrw_(x,x_)

#if __MSP430_CPU__ & __MSP430_CPU_MSP430X__
#define __MSPGCC_ATTRIBUTE_A20__ __attribute__((__a20__))
#else /* 430X available */
#define __MSPGCC_ATTRIBUTE_A20__
#endif /* 430X available */
#define sfra_(x,x_) volatile __MSPGCC_PERIPHERAL__ unsigned long int __MSPGCC_ATTRIBUTE_A20__ x __asm__("__" #x)
#define sfra(x,x_) extern sfra_(x,x_)
#define const_sfra(x,x_) extern const sfra_(x,x_)

#ifdef __GNUC__
#define __interrupt __attribute__((__interrupt__))
#endif /* __GNUC__ */

#endif /* __ASSEMBLER__ */

#endif /* __MSP430_IOMACROS_H_ */
