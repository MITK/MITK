/** @file
 *	DKFZ - Heidelberg
 *
 *	Module Name:	ipPicTypeMultiplex
 * 
 *	Module Klasse: 	ADT
 *
 *	Beschreibung:	Multiplext einen Methodenaufruf entsprechend dem Bilddatentype.
 *					Das Type bestimmende Bild muss als erster Parameter übergeben werden.
 *					Bei der Methode muss es sich um eine template-Methode handeln.
 *					Die Anzahl der Parameter ist auf 16 beschränkt. 
 *					
 *					Aufruf einer Methode mit zwei Parametern ohne Rückgabewert:
 *					ipPicTypeMultiplex2( funktionsname, parameter1, Parameter2 );
 *
 *					Aufruf einer Methode mit einem Parameter und mit Rückgabewert:
 *					ipPicTypeMultiplexR1( funktionsname, rückgabewert, parameter );
 *
 *					R gibt an, dass die Methode einen Rückgabewert liefert.
 *					Die Zahl bestimmt die Anzahl der Parameter
 *
 *	Exportierte Klassen und Funktionen:
 *					keine
 *
 *	Importierte Klassen und Funktionen:
 *					std::complex
 *
 *	@warning		Dieses Modul wurde noch nicht abgenommen!
 *
 *
 *	@version		1.0
 *
 *  @date			07.09.2000
 *
 *  @author			Marcus Vetter
 *
 */

#ifndef IP_PIC_TYPE_MULTIPLEX_H
#define IP_PIC_TYPE_MULTIPLEX_H

#include <ipPic.h>
//#include <complex>

#define ipPicTypeMultiplex1( function, pic, param1 )										\
{																					\
  	if ( ( pic->type == ipPicInt || pic->type == ipPicUInt ) && pic->bpe == 1 ) {	\
	} else if ( pic->type == ipPicInt && pic->bpe == 8 ) {							\
		function<char>( pic, param1 );													\
	} else if ( pic->type == ipPicUInt && pic->bpe == 8 ) {							\
		function<unsigned char>( pic, param1 );											\
	} else if ( pic->type == ipPicInt && pic->bpe == 16 ) {							\
		function<short>( pic, param1 );													\
	} else if ( pic->type == ipPicUInt && pic->bpe == 16 ) {						\
		function<unsigned short>( pic, param1 );											\
	} else if ( pic->type == ipPicInt && pic->bpe == 32 ) {							\
		function<int>( pic, param1 );													\
	} else if ( pic->type == ipPicUInt && pic->bpe == 32 ) {						\
		function<unsigned int>( pic, param1 );											\
	} else if ( pic->type == ipPicInt && pic->bpe == 64 ) {							\
		function<long>( pic, param1 );													\
	} else if ( pic->type == ipPicUInt && pic->bpe == 64 ) {						\
		function<unsigned long>( pic, param1 );											\
	} else if ( pic->type == ipPicFloat && pic->bpe == 32 ) {						\
		function<float>( pic, param1 );													\
	} else if ( pic->type == ipPicFloat && pic->bpe == 64 ) {						\
		function<double>( pic, param1 );													\
	}																				\
}
/*
 else if ( pic->type == ipPicComplex && pic->bpe == 64 ) {						\
		function<std::complex<float> >( pic, param1 );									\
	} else if ( pic->type == ipPicComplex && pic->bpe == 128 ) {					\
		function<std::complex<double> >( pic, param1 );									\
	}
*/

#define ipPicTypeMultiplex2( function, pic, param1, param2 )								\
{																					\
	if ( pic->type == ipPicInt && pic->bpe == 8 ) {									\
		function<char>( pic, param1, param2 );											\
	} else if ( pic->type == ipPicUInt && pic->bpe == 8 ) {							\
		function<unsigned char>( pic, param1, param2 );									\
	} else if ( pic->type == ipPicInt && pic->bpe == 16 ) {							\
		function<short>( pic, param1, param2 );											\
	} else if ( pic->type == ipPicUInt && pic->bpe == 16 ) {						\
		function<unsigned short>( pic, param1, param2 );									\
	} else if ( pic->type == ipPicInt && pic->bpe == 32 ) {							\
		function<int>( pic, param1, param2 );											\
	} else if ( pic->type == ipPicUInt && pic->bpe == 32 ) {						\
		function<unsigned int>( pic, param1, param2 );									\
	} else if ( pic->type == ipPicInt && pic->bpe == 64 ) {							\
		function<long>( pic, param1, param2 );											\
	} else if ( pic->type == ipPicUInt && pic->bpe == 64 ) {						\
		function<unsigned long>( pic, param1, param2 );									\
	} else if ( pic->type == ipPicFloat && pic->bpe == 32 ) {						\
		function<float>( pic, param1, param2 );											\
	} else if ( pic->type == ipPicFloat && pic->bpe == 64 ) {						\
		function<double>( pic, param1, param2 );											\
	}																				\
}
/*
 else if ( pic->type == ipPicComplex && pic->bpe == 64 ) {						\
		function<std::complex<float> >( pic, param1, param2 );							\
	} else if ( pic->type == ipPicComplex && pic->bpe == 128 ) {					\
		function<std::complex<double> >( pic, param1, param2 );							\
	}																				
*/
#define ipPicTypeMultiplex3( function, pic, param1, param2, param3 )						\
{																					\
	if ( pic->type == ipPicInt && pic->bpe == 8 ) {									\
		function<char>( pic, param1, param2, param3 );									\
	} else if ( pic->type == ipPicUInt && pic->bpe == 8 ) {							\
		function<unsigned char>( pic, param1, param2, param3 );							\
	} else if ( pic->type == ipPicInt && pic->bpe == 16 ) {							\
		function<short>( pic, param1, param2, param3 );									\
	} else if ( pic->type == ipPicUInt && pic->bpe == 16 ) {						\
		function<unsigned short>( pic, param1, param2, param3 );							\
	} else if ( pic->type == ipPicInt && pic->bpe == 32 ) {							\
		function<int>( pic, param1, param2, param3 );									\
	} else if ( pic->type == ipPicUInt && pic->bpe == 32 ) {						\
		function<unsigned int>( pic, param1, param2, param3 );							\
	} else if ( pic->type == ipPicInt && pic->bpe == 64 ) {							\
		function<long>( pic, param1, param2, param3 );									\
	} else if ( pic->type == ipPicUInt && pic->bpe == 64 ) {						\
		function<unsigned long>( pic, param1, param2, param3 );							\
	} else if ( pic->type == ipPicFloat && pic->bpe == 32 ) {						\
		function<float>( pic, param1, param2, param3 );									\
	} else if ( pic->type == ipPicFloat && pic->bpe == 64 ) {						\
		function<double>( pic, param1, param2, param3 );									\
	} 																				\
}
/*else if ( pic->type == ipPicComplex && pic->bpe == 64 ) {						\
		function<std::complex<float> >( pic, param1, param2, param3 );					\
	} else if ( pic->type == ipPicComplex && pic->bpe == 128 ) {					\
		function<std::complex<double> >( pic, param1, param2, param3 );					\
	}																				*/

#define ipPicTypeMultiplex4( function, pic, param1, param2, param3, param4 )				\
{																					\
	if ( pic->type == ipPicInt && pic->bpe == 8 ) {									\
		function<char>( pic, param1, param2, param3, param4 );							\
	} else if ( pic->type == ipPicUInt && pic->bpe == 8 ) {							\
		function<unsigned char>( pic, param1, param2, param3, param4 );					\
	} else if ( pic->type == ipPicInt && pic->bpe == 16 ) {							\
		function<short>( pic, param1, param2, param3, param4 );							\
	} else if ( pic->type == ipPicUInt && pic->bpe == 16 ) {						\
		function<unsigned short>( pic, param1, param2, param3, param4 );					\
	} else if ( pic->type == ipPicInt && pic->bpe == 32 ) {							\
		function<int>( pic, param1, param2, param3, param4 );							\
	} else if ( pic->type == ipPicUInt && pic->bpe == 32 ) {						\
		function<unsigned int>( pic, param1, param2, param3, param4 );					\
	} else if ( pic->type == ipPicInt && pic->bpe == 64 ) {							\
		function<long>( pic, param1, param2, param3, param4 );							\
	} else if ( pic->type == ipPicUInt && pic->bpe == 64 ) {						\
		function<unsigned long>( pic, param1, param2, param3, param4 );					\
	} else if ( pic->type == ipPicFloat && pic->bpe == 32 ) {						\
		function<float>( pic, param1, param2, param3, param4 );							\
	} else if ( pic->type == ipPicFloat && pic->bpe == 64 ) {						\
		function<double>( pic, param1, param2, param3, param4 );							\
	} 																				\
}
/*
else if ( pic->type == ipPicComplex && pic->bpe == 64 ) {						\
		function<std::complex<float> >( pic, param1, param2, param3, param4 );			\
	} else if ( pic->type == ipPicComplex && pic->bpe == 128 ) {					\
		function<std::complex<double> >( pic, param1, param2, param3, param4 );			\
	}*/

#define ipPicTypeMultiplex5( function, pic, param1, param2, param3, param4, param5 )		\
{																					\
	if ( pic->type == ipPicInt && pic->bpe == 8 ) {									\
		function<char>( pic, param1, param2, param3, param4, param5 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 8 ) {							\
		function<unsigned char>( pic, param1, param2, param3, param4, param5 );			\
	} else if ( pic->type == ipPicInt && pic->bpe == 16 ) {							\
		function<short>( pic, param1, param2, param3, param4, param5 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 16 ) {						\
		function<unsigned short>( pic, param1, param2, param3, param4, param5 );			\
	} else if ( pic->type == ipPicInt && pic->bpe == 32 ) {							\
		function<int>( pic, param1, param2, param3, param4, param5 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 32 ) {						\
		function<unsigned int>( pic, param1, param2, param3, param4, param5 );			\
	} else if ( pic->type == ipPicInt && pic->bpe == 64 ) {							\
		function<long>( pic, param1, param2, param3, param4, param5 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 64 ) {						\
		function<unsigned long>( pic, param1, param2, param3, param4, param5 );			\
	} else if ( pic->type == ipPicFloat && pic->bpe == 32 ) {						\
		function<float>( pic, param1, param2, param3, param4, param5 );					\
	} else if ( pic->type == ipPicFloat && pic->bpe == 64 ) {						\
		function<double>( pic, param1, param2, param3, param4, param5 );					\
	}																				\
}
/* else if ( pic->type == ipPicComplex && pic->bpe == 64 ) {						\
		function<std::complex<float> >( pic, param1, param2, param3, param4, param5 );	\
	} else if ( pic->type == ipPicComplex && pic->bpe == 128 ) {					\
		function<std::complex<double> >( pic, param1, param2, param3, param4, param5 );	\
	}																				
*/																				
#define ipPicTypeMultiplex6( function, pic, param1, param2, param3, param4, param5, param6 )		\
{																					\
	if ( pic->type == ipPicInt && pic->bpe == 8 ) {											\
		function<char>( pic, param1, param2, param3, param4, param5, param6 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 8 ) {									\
		function<unsigned char>( pic, param1, param2, param3, param4, param5, param6 );			\
	} else if ( pic->type == ipPicInt && pic->bpe == 16 ) {									\
		function<short>( pic, param1, param2, param3, param4, param5, param6 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 16 ) {								\
		function<unsigned short>( pic, param1, param2, param3, param4, param5, param6 );			\
	} else if ( pic->type == ipPicInt && pic->bpe == 32 ) {									\
		function<int>( pic, param1, param2, param3, param4, param5, param6 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 32 ) {								\
		function<unsigned int>( pic, param1, param2, param3, param4, param5, param6 );			\
	} else if ( pic->type == ipPicInt && pic->bpe == 64 ) {									\
		function<long>( pic, param1, param2, param3, param4, param5, param6 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 64 ) {								\
		function<unsigned long>( pic, param1, param2, param3, param4, param5, param6 );			\
	} else if ( pic->type == ipPicFloat && pic->bpe == 32 ) {								\
		function<float>( pic, param1, param2, param3, param4, param5, param6 );					\
	} else if ( pic->type == ipPicFloat && pic->bpe == 64 ) {								\
		function<double>( pic, param1, param2, param3, param4, param5, param6 );					\
	} 																				\
}
/*else if ( pic->type == ipPicComplex && pic->bpe == 64 ) {								\
		function<std::complex<float> >( pic, param1, param2, param3, param4, param5, param6 );	\
	} else if ( pic->type == ipPicComplex && pic->bpe == 128 ) {							\
		function<std::complex<double> >( pic, param1, param2, param3, param4, param5, param6 );	\
	}				*/

#define ipPicTypeMultiplex7( function, pic, param1, param2, param3, param4, param5, param6, param7 )		\
{																					\
	if ( pic->type == ipPicInt && pic->bpe == 8 ) {											\
		function<char>( pic, param1, param2, param3, param4, param5, param6, param7 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 8 ) {									\
		function<unsigned char>( pic, param1, param2, param3, param4, param5, param6, param7 );			\
	} else if ( pic->type == ipPicInt && pic->bpe == 16 ) {									\
		function<short>( pic, param1, param2, param3, param4, param5, param6, param7 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 16 ) {								\
		function<unsigned short>( pic, param1, param2, param3, param4, param5, param6, param7 );			\
	} else if ( pic->type == ipPicInt && pic->bpe == 32 ) {									\
		function<int>( pic, param1, param2, param3, param4, param5, param6, param7 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 32 ) {								\
		function<unsigned int>( pic, param1, param2, param3, param4, param5, param6, param7 );			\
	} else if ( pic->type == ipPicInt && pic->bpe == 64 ) {									\
		function<long>( pic, param1, param2, param3, param4, param5, param6, param7 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 64 ) {								\
		function<unsigned long>( pic, param1, param2, param3, param4, param5, param6, param7 );			\
	} else if ( pic->type == ipPicFloat && pic->bpe == 32 ) {								\
		function<float>( pic, param1, param2, param3, param4, param5, param6, param7 );					\
	} else if ( pic->type == ipPicFloat && pic->bpe == 64 ) {								\
		function<double>( pic, param1, param2, param3, param4, param5, param6, param7 );					\
	} 																				\
}
/*else if ( pic->type == ipPicComplex && pic->bpe == 64 ) {								\
		function<std::complex<float> >( pic, param1, param2, param3, param4, param5, param6 );	\
	} else if ( pic->type == ipPicComplex && pic->bpe == 128 ) {							\
		function<std::complex<double> >( pic, param1, param2, param3, param4, param5, param6 );	\
	}				*/

#define ipPicTypeMultiplex7( function, pic, param1, param2, param3, param4, param5, param6, param7 )		\
{																					\
	if ( pic->type == ipPicInt && pic->bpe == 8 ) {											\
		function<char>( pic, param1, param2, param3, param4, param5, param6, param7 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 8 ) {									\
		function<unsigned char>( pic, param1, param2, param3, param4, param5, param6, param7 );			\
	} else if ( pic->type == ipPicInt && pic->bpe == 16 ) {									\
		function<short>( pic, param1, param2, param3, param4, param5, param6, param7 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 16 ) {								\
		function<unsigned short>( pic, param1, param2, param3, param4, param5, param6, param7 );			\
	} else if ( pic->type == ipPicInt && pic->bpe == 32 ) {									\
		function<int>( pic, param1, param2, param3, param4, param5, param6, param7 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 32 ) {								\
		function<unsigned int>( pic, param1, param2, param3, param4, param5, param6, param7 );			\
	} else if ( pic->type == ipPicInt && pic->bpe == 64 ) {									\
		function<long>( pic, param1, param2, param3, param4, param5, param6, param7 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 64 ) {								\
		function<unsigned long>( pic, param1, param2, param3, param4, param5, param6, param7 );			\
	} else if ( pic->type == ipPicFloat && pic->bpe == 32 ) {								\
		function<float>( pic, param1, param2, param3, param4, param5, param6, param7 );					\
	} else if ( pic->type == ipPicFloat && pic->bpe == 64 ) {								\
		function<double>( pic, param1, param2, param3, param4, param5, param6, param7 );					\
	} 																				\
}
/*else if ( pic->type == ipPicComplex && pic->bpe == 64 ) {								\
		function<std::complex<float> >( pic, param1, param2, param3, param4, param5, param6 );	\
	} else if ( pic->type == ipPicComplex && pic->bpe == 128 ) {							\
		function<std::complex<double> >( pic, param1, param2, param3, param4, param5, param6 );	\
	}				*/

#define ipPicTypeMultiplex8( function, pic, param1, param2, param3, param4, param5, param6, param7, param8)		\
{																					\
	if ( pic->type == ipPicInt && pic->bpe == 8 ) {											\
		function<char>( pic, param1, param2, param3, param4, param5, param6, param7, param8);					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 8 ) {									\
		function<unsigned char>( pic, param1, param2, param3, param4, param5, param6, param7, param8);			\
	} else if ( pic->type == ipPicInt && pic->bpe == 16 ) {									\
		function<short>( pic, param1, param2, param3, param4, param5, param6, param7, param8);					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 16 ) {								\
		function<unsigned short>( pic, param1, param2, param3, param4, param5, param6, param7, param8);			\
	} else if ( pic->type == ipPicInt && pic->bpe == 32 ) {									\
		function<int>( pic, param1, param2, param3, param4, param5, param6, param7, param8);					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 32 ) {								\
		function<unsigned int>( pic, param1, param2, param3, param4, param5, param6, param7, param8);			\
	} else if ( pic->type == ipPicInt && pic->bpe == 64 ) {									\
		function<long>( pic, param1, param2, param3, param4, param5, param6, param7, param8);					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 64 ) {								\
		function<unsigned long>( pic, param1, param2, param3, param4, param5, param6, param7, param8);			\
	} else if ( pic->type == ipPicFloat && pic->bpe == 32 ) {								\
		function<float>( pic, param1, param2, param3, param4, param5, param6, param7, param8);					\
	} else if ( pic->type == ipPicFloat && pic->bpe == 64 ) {								\
		function<double>( pic, param1, param2, param3, param4, param5, param6, param7, param8);					\
	} 																				\
}
/*else if ( pic->type == ipPicComplex && pic->bpe == 64 ) {								\
		function<std::complex<float> >( pic, param1, param2, param3, param4, param5, param6 );	\
	} else if ( pic->type == ipPicComplex && pic->bpe == 128 ) {							\
		function<std::complex<double> >( pic, param1, param2, param3, param4, param5, param6 );	\
	}				*/

#define ipPicTypeMultiplex9( function, pic, param1, param2, param3, param4, param5, param6, param7, param8, param9)		\
{																					\
	if ( pic->type == ipPicInt && pic->bpe == 8 ) {											\
		function<char>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9);					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 8 ) {									\
		function<unsigned char>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9);			\
	} else if ( pic->type == ipPicInt && pic->bpe == 16 ) {									\
		function<short>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9);					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 16 ) {								\
		function<unsigned short>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9);			\
	} else if ( pic->type == ipPicInt && pic->bpe == 32 ) {									\
		function<int>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9);					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 32 ) {								\
		function<unsigned int>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9);			\
	} else if ( pic->type == ipPicInt && pic->bpe == 64 ) {									\
		function<long>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9);					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 64 ) {								\
		function<unsigned long>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9);			\
	} else if ( pic->type == ipPicFloat && pic->bpe == 32 ) {								\
		function<float>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9);					\
	} else if ( pic->type == ipPicFloat && pic->bpe == 64 ) {								\
		function<double>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9);					\
	} 																				\
}
/*else if ( pic->type == ipPicComplex && pic->bpe == 64 ) {								\
		function<std::complex<float> >( pic, param1, param2, param3, param4, param5, param6 );	\
	} else if ( pic->type == ipPicComplex && pic->bpe == 128 ) {							\
		function<std::complex<double> >( pic, param1, param2, param3, param4, param5, param6 );	\
	}				*/

#define ipPicTypeMultiplex10( function, pic, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10)		\
{																					\
	if ( pic->type == ipPicInt && pic->bpe == 8 ) {											\
		function<char>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 8 ) {									\
		function<unsigned char>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);			\
	} else if ( pic->type == ipPicInt && pic->bpe == 16 ) {									\
		function<short>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 16 ) {								\
		function<unsigned short>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);			\
	} else if ( pic->type == ipPicInt && pic->bpe == 32 ) {									\
		function<int>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 32 ) {								\
		function<unsigned int>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);			\
	} else if ( pic->type == ipPicInt && pic->bpe == 64 ) {									\
		function<long>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 64 ) {								\
		function<unsigned long>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);			\
	} else if ( pic->type == ipPicFloat && pic->bpe == 32 ) {								\
		function<float>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);					\
	} else if ( pic->type == ipPicFloat && pic->bpe == 64 ) {								\
		function<double>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);					\
	} 																				\
}
/*else if ( pic->type == ipPicComplex && pic->bpe == 64 ) {								\
		function<std::complex<float> >( pic, param1, param2, param3, param4, param5, param6 );	\
	} else if ( pic->type == ipPicComplex && pic->bpe == 128 ) {							\
		function<std::complex<double> >( pic, param1, param2, param3, param4, param5, param6 );	\
	}				*/

#define ipPicTypeMultiplex16( function, pic, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10, param11, param12, param13, param14, param15, param16 )		\
{																					\
	if ( pic->type == ipPicInt && pic->bpe == 8 ) {											\
		function<char>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10, param11, param12, param13, param14, param15, param16 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 8 ) {									\
		function<unsigned char>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10, param11, param12, param13, param14, param15, param16 );			\
	} else if ( pic->type == ipPicInt && pic->bpe == 16 ) {									\
		function<short>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10, param11, param12, param13, param14, param15, param16 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 16 ) {								\
		function<unsigned short>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10, param11, param12, param13, param14, param15, param16 );			\
	} else if ( pic->type == ipPicInt && pic->bpe == 32 ) {									\
		function<int>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10, param11, param12, param13, param14, param15, param16 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 32 ) {								\
		function<unsigned int>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10, param11, param12, param13, param14, param15, param16 );			\
	} else if ( pic->type == ipPicInt && pic->bpe == 64 ) {									\
		function<long>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10, param11, param12, param13, param14, param15, param16 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 64 ) {								\
		function<unsigned long>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10, param11, param12, param13, param14, param15, param16 );			\
	} else if ( pic->type == ipPicFloat && pic->bpe == 32 ) {								\
		function<float>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10, param11, param12, param13, param14, param15, param16 );					\
	} else if ( pic->type == ipPicFloat && pic->bpe == 64 ) {								\
		function<double>( pic, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10, param11, param12, param13, param14, param15, param16 );					\
	} 																				\
}
/*else if ( pic->type == ipPicComplex && pic->bpe == 64 ) {								\
		function<std::complex<float> >( pic, param1, param2, param3, param4, param5, param6 );	\
	} else if ( pic->type == ipPicComplex && pic->bpe == 128 ) {							\
		function<std::complex<double> >( pic, param1, param2, param3, param4, param5, param6 );	\
	}				*/
/**
 * mit Rückgabewert
 */

#define ipPicTypeMultiplexR1( function, pic, returnValue, param1 )								\
{																					\
  	if ( ( pic->type == ipPicInt || pic->type == ipPicUInt ) && pic->bpe == 1 ) {			\
	} else if ( pic->type == ipPicInt && pic->bpe == 8 ) {									\
		returnValue = function<char>( pic, param1 );												\
	} else if ( pic->type == ipPicUInt && pic->bpe == 8 ) {									\
		returnValue = function<unsigned char>( pic, param1 );									\
	} else if ( pic->type == ipPicInt && pic->bpe == 16 ) {									\
		returnValue = function<short>( pic, param1 );											\
	} else if ( pic->type == ipPicUInt && pic->bpe == 16 ) {								\
		returnValue = function<unsigned short>( pic, param1 );									\
	} else if ( pic->type == ipPicInt && pic->bpe == 32 ) {									\
		returnValue = function<int>( pic, param1 );												\
	} else if ( pic->type == ipPicUInt && pic->bpe == 32 ) {								\
		returnValue = function<unsigned int>( pic, param1 );										\
	} else if ( pic->type == ipPicInt && pic->bpe == 64 ) {									\
		returnValue = function<long>( pic, param1 );												\
	} else if ( pic->type == ipPicUInt && pic->bpe == 64 ) {								\
		returnValue = function<unsigned long>( pic, param1 );									\
	} else if ( pic->type == ipPicFloat && pic->bpe == 32 ) {								\
		returnValue = function<float>( pic, param1 );											\
	} else if ( pic->type == ipPicFloat && pic->bpe == 64 ) {								\
		returnValue = function<double>( pic, param1 );											\
	}																				\
}

#define ipPicTypeMultiplexR2( function, pic, returnValue, param1, param2 )						\
{																					\
	if ( pic->type == ipPicInt && pic->bpe == 8 ) {											\
		returnValue = function<char>( pic, param1, param2 );										\
	} else if ( pic->type == ipPicUInt && pic->bpe == 8 ) {									\
		returnValue = function<unsigned char>( pic, param1, param2 );							\
	} else if ( pic->type == ipPicInt && pic->bpe == 16 ) {									\
		returnValue = function<short>( pic, param1, param2 );									\
	} else if ( pic->type == ipPicUInt && pic->bpe == 16 ) {								\
		returnValue = function<unsigned short>( pic, param1, param2 );							\
	} else if ( pic->type == ipPicInt && pic->bpe == 32 ) {									\
		returnValue = function<int>( pic, param1, param2 );										\
	} else if ( pic->type == ipPicUInt && pic->bpe == 32 ) {								\
		returnValue = function<unsigned int>( pic, param1, param2 );								\
	} else if ( pic->type == ipPicInt && pic->bpe == 64 ) {									\
		returnValue = function<long>( pic, param1, param2 );										\
	} else if ( pic->type == ipPicUInt && pic->bpe == 64 ) {								\
		returnValue = function<unsigned long>( pic, param1, param2 );							\
	} else if ( pic->type == ipPicFloat && pic->bpe == 32 ) {								\
		returnValue = function<float>( pic, param1, param2 );									\
	} else if ( pic->type == ipPicFloat && pic->bpe == 64 ) {								\
		returnValue = function<double>( pic, param1, param2 );									\
	}																				\
}

#define ipPicTypeMultiplexR3( function, pic, returnValue, param1, param2, param3 )				\
{																					\
	if ( pic->type == ipPicInt && pic->bpe == 8 ) {											\
		returnValue = function<char>( pic, param1, param2, param3 );								\
	} else if ( pic->type == ipPicUInt && pic->bpe == 8 ) {									\
		returnValue = function<unsigned char>( pic, param1, param2, param3 );					\
	} else if ( pic->type == ipPicInt && pic->bpe == 16 ) {									\
		returnValue = function<short>( pic, param1, param2, param3 );							\
	} else if ( pic->type == ipPicUInt && pic->bpe == 16 ) {								\
		returnValue = function<unsigned short>( pic, param1, param2, param3 );					\
	} else if ( pic->type == ipPicInt && pic->bpe == 32 ) {									\
		returnValue = function<int>( pic, param1, param2, param3 );								\
	} else if ( pic->type == ipPicUInt && pic->bpe == 32 ) {								\
		returnValue = function<unsigned int>( pic, param1, param2, param3 );						\
	} else if ( pic->type == ipPicInt && pic->bpe == 64 ) {									\
		returnValue = function<long>( pic, param1, param2, param3 );								\
	} else if ( pic->type == ipPicUInt && pic->bpe == 64 ) {								\
		returnValue = function<unsigned long>( pic, param1, param2, param3 );					\
	} else if ( pic->type == ipPicFloat && pic->bpe == 32 ) {								\
		returnValue = function<float>( pic, param1, param2, param3 );							\
	} else if ( pic->type == ipPicFloat && pic->bpe == 64 ) {								\
		returnValue = function<double>( pic, param1, param2, param3 );							\
	}																				\
}

#define ipPicTypeMultiplexR4( function, pic, returnValue, param1, param2, param3, param4 )		\
{																					\
	if ( pic->type == ipPicInt && pic->bpe == 8 ) {											\
		returnValue = function<char>( pic, param1, param2, param3, param4 );						\
	} else if ( pic->type == ipPicUInt && pic->bpe == 8 ) {									\
		returnValue = function<unsigned char>( pic, param1, param2, param3, param4 );			\
	} else if ( pic->type == ipPicInt && pic->bpe == 16 ) {									\
		returnValue = function<short>( pic, param1, param2, param3, param4 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 16 ) {								\
		returnValue = function<unsigned short>( pic, param1, param2, param3, param4 );			\
	} else if ( pic->type == ipPicInt && pic->bpe == 32 ) {									\
		returnValue = function<int>( pic, param1, param2, param3, param4 );						\
	} else if ( pic->type == ipPicUInt && pic->bpe == 32 ) {								\
		returnValue = function<unsigned int>( pic, param1, param2, param3, param4 );				\
	} else if ( pic->type == ipPicInt && pic->bpe == 64 ) {									\
		returnValue = function<long>( pic, param1, param2, param3, param4 );						\
	} else if ( pic->type == ipPicUInt && pic->bpe == 64 ) {								\
		returnValue = function<unsigned long>( pic, param1, param2, param3, param4 );			\
	} else if ( pic->type == ipPicFloat && pic->bpe == 32 ) {								\
		returnValue = function<float>( pic, param1, param2, param3, param4 );					\
	} else if ( pic->type == ipPicFloat && pic->bpe == 64 ) {								\
		returnValue = function<double>( pic, param1, param2, param3, param4 );					\
	}																				\
}

#define ipPicTypeMultiplexR5( function, pic, returnValue, param1, param2, param3, param4, param5 )	\
{																					\
	if ( pic->type == ipPicInt && pic->bpe == 8 ) {												\
		returnValue = function<char>( pic, param1, param2, param3, param4, param5 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 8 ) {										\
		returnValue = function<unsigned char>( pic, param1, param2, param3, param4, param5 );		\
	} else if ( pic->type == ipPicInt && pic->bpe == 16 ) {										\
		returnValue = function<short>( pic, param1, param2, param3, param4, param5 );				\
	} else if ( pic->type == ipPicUInt && pic->bpe == 16 ) {									\
		returnValue = function<unsigned short>( pic, param1, param2, param3, param4, param5 );		\
	} else if ( pic->type == ipPicInt && pic->bpe == 32 ) {										\
		returnValue = function<int>( pic, param1, param2, param3, param4, param5 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 32 ) {									\
		returnValue = function<unsigned int>( pic, param1, param2, param3, param4, param5 );			\
	} else if ( pic->type == ipPicInt && pic->bpe == 64 ) {										\
		returnValue = function<long>( pic, param1, param2, param3, param4, param5 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 64 ) {									\
		returnValue = function<unsigned long>( pic, param1, param2, param3, param4, param5 );		\
	} else if ( pic->type == ipPicFloat && pic->bpe == 32 ) {									\
		returnValue = function<float>( pic, param1, param2, param3, param4, param5 );				\
	} else if ( pic->type == ipPicFloat && pic->bpe == 64 ) {									\
		returnValue = function<double>( pic, param1, param2, param3, param4, param5 );				\
	} 																				\
}
																				
#define ipPicTypeMultiplexR6( function, pic, returnValue, param1, param2, param3, param4, param5, param6 )	\
{																					\
	if ( pic->type == ipPicInt && pic->bpe == 8 ) {														\
		returnValue = function<char>( pic, param1, param2, param3, param4, param5, param6 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 8 ) {												\
		returnValue = function<unsigned char>( pic, param1, param2, param3, param4, param5, param6 );		\
	} else if ( pic->type == ipPicInt && pic->bpe == 16 ) {												\
		returnValue = function<short>( pic, param1, param2, param3, param4, param5, param6 );				\
	} else if ( pic->type == ipPicUInt && pic->bpe == 16 ) {											\
		returnValue = function<unsigned short>( pic, param1, param2, param3, param4, param5, param6 );		\
	} else if ( pic->type == ipPicInt && pic->bpe == 32 ) {												\
		returnValue = function<int>( pic, param1, param2, param3, param4, param5, param6 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 32 ) {											\
		returnValue = function<unsigned int>( pic, param1, param2, param3, param4, param5, param6 );			\
	} else if ( pic->type == ipPicInt && pic->bpe == 64 ) {												\
		returnValue = function<long>( pic, param1, param2, param3, param4, param5, param6 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 64 ) {											\
		returnValue = function<unsigned long>( pic, param1, param2, param3, param4, param5, param6 );		\
	} else if ( pic->type == ipPicFloat && pic->bpe == 32 ) {											\
		returnValue = function<float>( pic, param1, param2, param3, param4, param5, param6 );				\
	} else if ( pic->type == ipPicFloat && pic->bpe == 64 ) {											\
		returnValue = function<double>( pic, param1, param2, param3, param4, param5, param6 );				\
	} 																				\
}

#define ipPicTypeMultiplexR7( function, pic, returnValue, param1, param2, param3, param4, param5, param6, param7 )	\
{																					\
	if ( pic->type == ipPicInt && pic->bpe == 8 ) {														\
		returnValue = function<char>( pic, param1, param2, param3, param4, param5, param6, param7 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 8 ) {												\
		returnValue = function<unsigned char>( pic, param1, param2, param3, param4, param5, param6, param7 );		\
	} else if ( pic->type == ipPicInt && pic->bpe == 16 ) {												\
		returnValue = function<short>( pic, param1, param2, param3, param4, param5, param6, param7 );				\
	} else if ( pic->type == ipPicUInt && pic->bpe == 16 ) {											\
		returnValue = function<unsigned short>( pic, param1, param2, param3, param4, param5, param6, param7 );		\
	} else if ( pic->type == ipPicInt && pic->bpe == 32 ) {												\
		returnValue = function<int>( pic, param1, param2, param3, param4, param5, param6, param7 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 32 ) {											\
		returnValue = function<unsigned int>( pic, param1, param2, param3, param4, param5, param6, param7 );			\
	} else if ( pic->type == ipPicInt && pic->bpe == 64 ) {												\
		returnValue = function<long>( pic, param1, param2, param3, param4, param5, param6, param7 );					\
	} else if ( pic->type == ipPicUInt && pic->bpe == 64 ) {											\
		returnValue = function<unsigned long>( pic, param1, param2, param3, param4, param5, param6, param7 );		\
	} else if ( pic->type == ipPicFloat && pic->bpe == 32 ) {											\
		returnValue = function<float>( pic, param1, param2, param3, param4, param5, param6, param7 );				\
	} else if ( pic->type == ipPicFloat && pic->bpe == 64 ) {											\
		returnValue = function<double>( pic, param1, param2, param3, param4, param5, param6, param7 );				\
	}																				\
}

#endif // IP_PIC_TYPE_MULTIPLEX_H