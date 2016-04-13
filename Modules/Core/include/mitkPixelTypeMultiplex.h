/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#ifndef MITKPIXELTYPEMULTIPLEX_H
#define MITKPIXELTYPEMULTIPLEX_H

#define mitkPixelTypeMultiplex0( function, ptype )    \
{                                                 \
    if ( ptype.GetComponentType() == itk::ImageIOBase::CHAR )\
      function<char>( ptype );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::UCHAR)\
      function<unsigned char>( ptype );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::SHORT )\
      function<short>( ptype );      \
    else if ( ptype.GetComponentType() == itk::ImageIOBase::USHORT )\
      function<unsigned short>( ptype  );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::INT )\
      function<int>( ptype );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::UINT )\
      function<unsigned int>( ptype );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::LONG )\
      function<long int>( ptype );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::ULONG )\
      function<unsigned long int>( ptype );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::FLOAT  )\
      function<float>( ptype );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::DOUBLE  )\
      function<double>( ptype );\
}\

#define mitkPixelTypeMultiplex1( function, ptype, param1 )    \
{                                                 \
    if ( ptype.GetComponentType() == itk::ImageIOBase::CHAR )\
      function<char>( ptype,  param1 );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::UCHAR)\
      function<unsigned char>( ptype,  param1 );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::SHORT )\
      function<short>( ptype,  param1 );      \
    else if ( ptype.GetComponentType() == itk::ImageIOBase::USHORT )\
      function<unsigned short>( ptype,  param1 );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::INT )\
      function<int>( ptype,  param1 );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::UINT )\
      function<unsigned int>( ptype,  param1 );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::LONG )\
      function<long int>( ptype,  param1 );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::ULONG )\
      function<unsigned long int>( ptype,  param1 );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::FLOAT  )\
      function<float>( ptype,  param1 );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::DOUBLE  )\
      function<double>( ptype,  param1 );\
}\

#define mitkPixelTypeMultiplex2( function, ptype, param1, param2 )    \
{                                                 \
    if ( ptype.GetComponentType() == itk::ImageIOBase::CHAR )\
      function<char>( ptype,   param1, param2 );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::UCHAR)\
      function<unsigned char>( ptype,   param1, param2 );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::SHORT )\
      function<short>( ptype,   param1, param2 );      \
    else if ( ptype.GetComponentType() == itk::ImageIOBase::USHORT )\
      function<unsigned short>( ptype,  param1, param2 );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::INT )\
      function<int>( ptype,   param1, param2 );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::UINT )\
      function<unsigned int>( ptype,  param1, param2 );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::LONG )\
      function<long int>( ptype,  param1, param2 );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::ULONG )\
      function<unsigned long int>( ptype,   param1, param2 );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::FLOAT  )\
      function<float>( ptype,   param1, param2 );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::DOUBLE  )\
      function<double>( ptype,  param1, param2 );\
}\

#define mitkPixelTypeMultiplex3( function, ptype, param1, param2, param3 )    \
{                                                 \
    if ( ptype.GetComponentType() == itk::ImageIOBase::CHAR )\
      function<char>( ptype,   param1, param2, param3 );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::UCHAR)\
      function<unsigned char>( ptype,   param1, param2, param3 );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::SHORT )\
      function<short>( ptype,   param1, param2, param3 );      \
    else if ( ptype.GetComponentType() == itk::ImageIOBase::USHORT )\
      function<unsigned short>( ptype,  param1, param2, param3 );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::INT )\
      function<int>( ptype,   param1, param2, param3 );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::UINT )\
      function<unsigned int>( ptype,  param1, param2, param3 );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::LONG )\
      function<long int>( ptype,  param1, param2, param3 );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::ULONG )\
      function<unsigned long int>( ptype,   param1, param2, param3 );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::FLOAT  )\
      function<float>( ptype,   param1, param2, param3 );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::DOUBLE  )\
      function<double>( ptype,  param1, param2, param3 );\
}\


// we have to have a default for, else Clang 3.6.1 complains about problems if 'if evaluates to false'
// therefore if type does not match double is assumed
#define mitkPixelTypeMultiplex4( function, ptype, param1, param2, param3, param4 )    \
{                                                 \
    if ( ptype.GetComponentType() == itk::ImageIOBase::CHAR )\
      function<char>( ptype,  param1, param2, param3, param4 );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::UCHAR)\
      function<unsigned char>( ptype,   param1, param2, param3, param4  );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::SHORT )\
      function<short>( ptype,   param1, param2, param3, param4  );      \
    else if ( ptype.GetComponentType() == itk::ImageIOBase::USHORT )\
      function<unsigned short>( ptype,  param1, param2, param3, param4  );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::INT )\
      function<int>( ptype,   param1, param2, param3, param4  );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::UINT )\
      function<unsigned int>( ptype,  param1, param2, param3, param4  );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::LONG )\
      function<long int>( ptype,  param1, param2, param3, param4  );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::ULONG )\
      function<unsigned long int>( ptype,   param1, param2, param3, param4  );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::FLOAT  )\
      function<float>( ptype,   param1, param2, param3, param4  );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::DOUBLE  )\
      function<double>( ptype,  param1, param2, param3, param4 );\
    else if (true)\
      function<double>( ptype,  param1, param2, param3, param4 );\
}\

// we have to have a default for, else Clang 3.6.1 complains about problems if 'if evaluates to false'
// therefore if type does not match double is assumed
#define mitkPixelTypeMultiplex5( function, ptype, param1, param2, param3, param4, param5 )    \
{                                                 \
    if ( ptype.GetComponentType() == itk::ImageIOBase::CHAR )\
      function<char>( ptype,  param1, param2, param3, param4, param5 );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::UCHAR)\
      function<unsigned char>( ptype,   param1, param2, param3, param4, param5  );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::SHORT )\
      function<short>( ptype,   param1, param2, param3, param4, param5  );      \
    else if ( ptype.GetComponentType() == itk::ImageIOBase::USHORT )\
      function<unsigned short>( ptype,  param1, param2, param3, param4, param5  );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::INT )\
      function<int>( ptype,   param1, param2, param3, param4, param5  );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::UINT )\
      function<unsigned int>( ptype,  param1, param2, param3, param4, param5  );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::LONG )\
      function<long int>( ptype,  param1, param2, param3, param4, param5  );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::ULONG )\
      function<unsigned long int>( ptype,   param1, param2, param3, param4, param5  );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::FLOAT  )\
      function<float>( ptype,   param1, param2, param3, param4, param5  );\
    else if ( ptype.GetComponentType() == itk::ImageIOBase::DOUBLE  )\
      function<double>( ptype,  param1, param2, param3, param4, param5 );\
    else \
      function<double>( ptype,  param1, param2, param3, param4, param5 );\
}\

#endif // MITKPIXELTYPEMULTIPLEX_H



