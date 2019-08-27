/******************************************************************************
 *                                                                            *
 * test.c - Test libvaxdata conversion routines using data generated by the   *
 *          endian.f VAX Fortran program.                                     *
 *                                                                            *
 *          Notes                                                             *
 *                                                                            *
 *          . On machines with multiple floating-point formats, i.e., DEC     *
 *            Alpha, be sure to compile with the option that selects IEEE     *
 *            floating-point format.  For example, on OpenVMS/Alpha:          *
 *                                                                            *
 *               $ CC /Float=IEEE_Float test.c                                *
 *                                                                            *
 *          . The VAX H16 conversion tests are only run on an Alpha, since    *
 *            the IEEE X floating point format is Alpha-specific.  long       *
 *            doubles must be 128 bits (the default).  For example, on        *
 *            OpenVMS/Alpha:                                                  *
 *                                                                            *
 *               $ CC /Float=IEEE_Float /L_Double_Size=128 test.c             *
 *                                                                            *
 * Author:      Lawrence M. Baker                                             *
 *              U.S. Geological Survey                                        *
 *              345 Middlefield Road  MS977                                   *
 *              Menlo Park, CA  94025                                         *
 *              baker@usgs.gov                                                *
 *                                                                            *
 *                                 Disclaimer                                 *
 *                                                                            *
 * Although this program has been used by the USGS, no warranty, expressed or *
 * implied, is made by the USGS or the United States  Government  as  to  the *
 * accuracy  and functioning of the program and related program material, nor *
 * shall the fact of  distribution  constitute  any  such  warranty,  and  no *
 * responsibility is assumed by the USGS in connection therewith.             *
 *                                                                            *
 *                                                                            *
 * Modification History:                                                      *
 *                                                                            *
 * 28-Jan-2010  L. M. Baker      Original version.                            *
 *                                                                            *
 ******************************************************************************/

/*
 * Output from endian.f
 *

I2
      1  0001
     -1  FFFF
    256  0100
   -256  FF00
  12345  3039
 -12345  CFC7

I4
           1  00000001
          -1  FFFFFFFF
         256  00000100
        -256  FFFFFF00
       65536  00010000
      -65536  FFFF0000
    16777216  01000000
   -16777216  FF000000
   123456789  075BCD15
  -123456789  F8A432EB

F4
   1.000000      00004080
  -1.000000      0000C080
   3.500000      00004160
  -3.500000      0000C160
   3.141590      0FD04149
  -3.141590      0FD0C149
  9.9999999E+36  BDC27DF0
 -9.9999999E+36  BDC2FDF0
  9.9999999E-38  1CEA0308
 -9.9999999E-38  1CEA8308
   1.234568      0652409E
  -1.234568      0652C09E

D8
    1.000000000000000      0000000000004080
   -1.000000000000000      000000000000C080
    3.500000000000000      0000000000004160
   -3.500000000000000      000000000000C160
    3.141592653589793      68BEA2210FDA4149
   -3.141592653589793      68BEA2210FDAC149
   1.0000000000000000E+37  48DB1ABBBDC27DF0
  -1.0000000000000000E+37  48DB1ABBBDC2FDF0
   9.9999999999999999E-38  5C7514541CEA0308
  -9.9999999999999999E-38  5C7514541CEA8308
    1.234567890123450      CEE714620652409E
   -1.234567890123450      CEE714620652C09E

G8
    1.000000000000000      0000000000004010
   -1.000000000000000      000000000000C010
    3.500000000000000      000000000000402C
   -3.500000000000000      000000000000C02C
    3.141592653589793      2D18544421FB4029
   -3.141592653589793      2D18544421FBC029
   1.0000000000000000E+37  691B435717B847BE
  -1.0000000000000000E+37  691B435717B8C7BE
   9.9999999999999999E-38  8B8F428A039D3861
  -9.9999999999999999E-38  8B8F428A039DB861
    1.234567890123450      59DD428CC0CA4013
   -1.234567890123450      59DD428CC0CAC013

H16
   1.00000000000000000000000000000000       00000000000000000000000000004001
  -1.00000000000000000000000000000000       0000000000000000000000000000C001
   3.50000000000000000000000000000000       000000000000000000000000C0004002
  -3.50000000000000000000000000000000       000000000000000000000000C000C002
   3.14159265358979323846264338327900       FC9FC516898C846942D1B544921F4002
  -3.14159265358979323846264338327900       FC9FC516898C846942D1B544921FC002
  1.000000000000000000000000000000000E+037  0000A8003D0DB64076918435E17B407B
 -1.000000000000000000000000000000000E+037  0000A8003D0DB64076918435E17BC07B
  9.999999999999999999999999999999999E-038  EFB4AC82FCA1EAEAA8B8D42810393F86
 -9.999999999999999999999999999999999E-038  EFB4AC82FCA1EAEAA8B8D4281039BF86
   1.23456789012345678901234567890000       54516B6B7BE1B71AC59FA4283C0C4001
  -1.23456789012345678901234567890000       54516B6B7BE1B71AC59FA4283C0CC001

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "convert_vax_data.h"

#ifdef __alpha
#if !__IEEE_FLOAT
#error /Float=IEEE_Float is required
#endif
#if !__X_FLOAT
#error /L_Double_Size=128 is required
#endif
#define DO_H16
#endif

static void print_hex( const unsigned char *vax,
                       const unsigned char *buf, int n );

int main( int argc, char *argv[] ) {

   int i, n;
   unsigned char *buf, *vax_copy;

   short i2[6];
   static unsigned char vax_i2[] = {
      0x01, 0x00,
      0xFF, 0xFF,
      0x00, 0x01,
      0x00, 0xFF,
      0x39, 0x30,
      0xC7, 0xCF
   };

   int   i4[10];
   static unsigned char vax_i4[] = {
      0x01, 0x00, 0x00, 0x00,
      0xFF, 0xFF, 0xFF, 0xFF,
      0x00, 0x01, 0x00, 0x00,
      0x00, 0xFF, 0xFF, 0xFF,
      0x00, 0x00, 0x01, 0x00,
      0x00, 0x00, 0xFF, 0xFF,
      0x00, 0x00, 0x00, 0x01,
      0x00, 0x00, 0x00, 0xFF,
      0x15, 0xCD, 0x5B, 0x07,
      0xEB, 0x32, 0xA4, 0xF8
   };

   float f4[12];
   static unsigned char vax_f4[] = {
      0x80, 0x40, 0x00, 0x00,
      0x80, 0xC0, 0x00, 0x00,
      0x60, 0x41, 0x00, 0x00,
      0x60, 0xC1, 0x00, 0x00,
      0x49, 0x41, 0xD0, 0x0F,
      0x49, 0xC1, 0xD0, 0x0F,
      0xF0, 0x7D, 0xC2, 0xBD,
      0xF0, 0xFD, 0xC2, 0xBD,
      0x08, 0x03, 0xEA, 0x1C,
      0x08, 0x83, 0xEA, 0x1C,
      0x9E, 0x40, 0x52, 0x06,
      0x9E, 0xC0, 0x52, 0x06
   };

   double d8[12];
   static unsigned char vax_d8[] = {
      0x80, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x80, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x60, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x60, 0xC1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x49, 0x41, 0xDA, 0x0F, 0x21, 0xA2, 0xBE, 0x68,
      0x49, 0xC1, 0xDA, 0x0F, 0x21, 0xA2, 0xBE, 0x68,
      0xF0, 0x7D, 0xC2, 0xBD, 0xBB, 0x1A, 0xDB, 0x48,
      0xF0, 0xFD, 0xC2, 0xBD, 0xBB, 0x1A, 0xDB, 0x48,
      0x08, 0x03, 0xEA, 0x1C, 0x54, 0x14, 0x75, 0x5C,
      0x08, 0x83, 0xEA, 0x1C, 0x54, 0x14, 0x75, 0x5C,
      0x9E, 0x40, 0x52, 0x06, 0x62, 0x14, 0xE7, 0xCE,
      0x9E, 0xC0, 0x52, 0x06, 0x62, 0x14, 0xE7, 0xCE
   };

   double g8[12];
   static unsigned char vax_g8[] = {
      0x10, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x10, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x2C, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x2C, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x29, 0x40, 0xFB, 0x21, 0x44, 0x54, 0x18, 0x2D,
      0x29, 0xC0, 0xFB, 0x21, 0x44, 0x54, 0x18, 0x2D,
      0xBE, 0x47, 0xB8, 0x17, 0x57, 0x43, 0x1B, 0x69,
      0xBE, 0xC7, 0xB8, 0x17, 0x57, 0x43, 0x1B, 0x69,
      0x61, 0x38, 0x9D, 0x03, 0x8A, 0x42, 0x8F, 0x8B,
      0x61, 0xB8, 0x9D, 0x03, 0x8A, 0x42, 0x8F, 0x8B,
      0x13, 0x40, 0xCA, 0xC0, 0x8C, 0x42, 0xDD, 0x59,
      0x13, 0xC0, 0xCA, 0xC0, 0x8C, 0x42, 0xDD, 0x59
   };

#ifdef DO_H16
   long double h16[12];
   static unsigned char vax_h16[] = {
0x01,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x01,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x40,0x1F,0x92,0x44,0xB5,0xD1,0x42,0x69,0x84,0x8C,0x89,0x16,0xC5,0x9F,0xFC,
0x02,0xC0,0x1F,0x92,0x44,0xB5,0xD1,0x42,0x69,0x84,0x8C,0x89,0x16,0xC5,0x9F,0xFC,
0x7B,0x40,0x7B,0xE1,0x35,0x84,0x91,0x76,0x40,0xB6,0x0D,0x3D,0x00,0xA8,0x00,0x00,
0x7B,0xC0,0x7B,0xE1,0x35,0x84,0x91,0x76,0x40,0xB6,0x0D,0x3D,0x00,0xA8,0x00,0x00,
0x86,0x3F,0x39,0x10,0x28,0xD4,0xB8,0xA8,0xEA,0xEA,0xA1,0xFC,0x82,0xAC,0xB4,0xEF,
0x86,0xBF,0x39,0x10,0x28,0xD4,0xB8,0xA8,0xEA,0xEA,0xA1,0xFC,0x82,0xAC,0xB4,0xEF,
0x01,0x40,0x0C,0x3C,0x28,0xA4,0x9F,0xC5,0x1A,0xB7,0xE1,0x7B,0x6B,0x6B,0x51,0x54,
0x01,0xC0,0x0C,0x3C,0x28,0xA4,0x9F,0xC5,0x1A,0xB7,0xE1,0x7B,0x6B,0x6B,0x51,0x54
   };
#endif


   n = sizeof( i2 ) / sizeof( i2[0] );
   from_vax_i2( vax_i2, i2, &n );
   printf( "\nI%u\n", (unsigned int) sizeof( i2[0] ) );
   for ( i = 0; i < n; i++ ) {
      printf( "%7hd\n", i2[i] );
   }
   buf = (unsigned char *) malloc( sizeof( i2 ) );
   to_vax_i2( i2, buf, &n );
   if ( memcmp( buf, vax_i2, sizeof( i2 ) ) != 0 ) {
      print_hex( vax_i2, buf, n );
   }
   free( buf );

   n = sizeof( i4 ) / sizeof( i4[0] );
   from_vax_i4( vax_i4, i4, &n );
   printf( "\nI%u\n", (unsigned int) sizeof( i4[0] ) );
   for ( i = 0; i < n; i++ ) {
      printf( "%12d\n", i4[i] );
   }
   buf = (unsigned char *) malloc( sizeof( i4 ) );
   to_vax_i4( i4, buf, &n );
   if ( memcmp( buf, vax_i4, sizeof( i4 ) ) != 0 ) {
      print_hex( vax_i4, buf, n );
   }
   free( buf );

   n = sizeof( f4 ) / sizeof( f4[0] );
   from_vax_r4( vax_f4, f4, &n );
   printf( "\nF%u\n", (unsigned int) sizeof( f4[0] ) );
   for ( i = 0; i < n; i++ ) {
      printf( "%15.6g\n", f4[i] );
   }
   buf = (unsigned char *) malloc( sizeof( f4 ) );
   to_vax_r4( f4, buf, &n );
   if ( memcmp( buf, vax_f4, sizeof( f4 ) ) != 0 ) {
      print_hex( vax_f4, buf, n );
   }
   free( buf );

   n = sizeof( d8 ) / sizeof( d8[0] );
   from_vax_d8( vax_d8, d8, &n );
   printf( "\nD%u\n", (unsigned int) sizeof( d8[0] ) );
   for ( i = 0; i < n; i++ ) {
      printf( "%25.15g\n", d8[i] );
   }
   /* IEEE T format has 3 fewer bits in the mantissa. */
   /* Before comparing the reverse conversion, make a */
   /* copy of the original and mask off the low-order */
   /* bits in the mantissa.                           */
   vax_copy = (unsigned char *) malloc( sizeof( vax_d8 ) );
   memcpy( vax_copy, vax_d8, sizeof( vax_d8 ) );
   for ( i = 0; i < n; i++ ) {
      vax_copy[8*i+6] &= ~0x07;
   }
   buf = (unsigned char *) malloc( sizeof( d8 ) );
   to_vax_d8( d8, buf, &n );
   if ( memcmp( buf, vax_copy, sizeof( d8 ) ) != 0 ) {
      print_hex( vax_copy, buf, n );
   }
   free( buf );
   free( vax_copy );

   n = sizeof( g8 ) / sizeof( g8[0] );
   from_vax_g8( vax_g8, g8, &n );
   printf( "\nG%u\n", (unsigned int) sizeof( g8[0] ) );
   for ( i = 0; i < n; i++ ) {
      printf( "%25.15g\n", g8[i] );
   }
   buf = (unsigned char *) malloc( sizeof( g8 ) );
   to_vax_g8( g8, buf, &n );
   if ( memcmp( buf, vax_g8, sizeof( g8 ) ) != 0 ) {
      print_hex( vax_g8, buf, n );
   }
   free( buf );

#ifdef DO_H16
   n = sizeof( h16 ) / sizeof( h16[0] );
   from_vax_h16( vax_h16, h16, &n );
   printf( "\nH%u\n", (unsigned int) sizeof( h16[0] ) );
   for ( i = 0; i < n; i++ ) {
      printf( "%42.32Lg\n", h16[i] );
   }
   buf = (unsigned char *) malloc( sizeof( h16 ) );
   to_vax_h16( h16, buf, &n );
   if ( memcmp( buf, vax_h16, sizeof( h16 ) ) != 0 ) {
      print_hex( vax_h16, buf, n );
   }
   free( buf );
#endif

}

static void print_hex( const unsigned char *vax,
                       const unsigned char *buf, int n ) {

   int i, j;


   printf( "\n*** Reverse operation failed equality test. ***\n\n" );
   for ( i = 0; i < n; i++ ) {
      for ( j = 7; j >= 0; j-- ) {
         printf( "%02.2X", vax[8*i+j] );
      }
      printf( "  " );
      for ( j = 7; j >= 0; j-- ) {
         printf( "%02.2X", buf[8*i+j] );
      }
      printf( "\n" );
   }
}