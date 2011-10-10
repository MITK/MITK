#ifndef MITKPIXELTYPEMULTIPLEX_H
#define MITKPIXELTYPEMULTIPLEX_H

#define mitkPixelTypeMultiplex0( function, ptype )    \
{                                                 \
    if ( ptype.GetTypeId() == typeid(char) )\
      function<char>( );\
    else if ( ptype.GetTypeId() == typeid(unsigned char))\
      function<unsigned char>( );\
    else if ( ptype.GetTypeId() == typeid(short) )\
      function<short>( );      \
    else if ( ptype.GetTypeId() == typeid(unsigned short) )\
      function<unsigned short>( );\
    else if ( ptype.GetTypeId() == typeid(int) )\
      function<int>( );\
    else if ( ptype.GetTypeId() == typeid(unsigned int) )\
      function<unsigned int>( );\
    else if ( ptype.GetTypeId() == typeid(long int) )\
      function<long int>( );\
    else if ( ptype.GetTypeId() == typeid(unsigned long int) )\
      function<unsigned long int>( );\
    else if ( ptype.GetTypeId() == typeid(float)  )\
      function<float>( );\
    else if ( ptype.GetTypeId() == typeid(double)  )\
      function<double>( );\
}\

#define mitkPixelTypeMultiplex1( function, ptype, param1 )    \
{                                                 \
    if ( ptype.GetTypeId() == typeid(char) )\
      function<char>( param1 );\
    else if ( ptype.GetTypeId() == typeid(unsigned char))\
      function<unsigned char>( param1 );\
    else if ( ptype.GetTypeId() == typeid(short) )\
      function<short>( param1 );      \
    else if ( ptype.GetTypeId() == typeid(unsigned short) )\
      function<unsigned short>( param1 );\
    else if ( ptype.GetTypeId() == typeid(int) )\
      function<int>( param1 );\
    else if ( ptype.GetTypeId() == typeid(unsigned int) )\
      function<unsigned int>( param1 );\
    else if ( ptype.GetTypeId() == typeid(long int) )\
      function<long int>( param1 );\
    else if ( ptype.GetTypeId() == typeid(unsigned long int) )\
      function<unsigned long int>( param1 );\
    else if ( ptype.GetTypeId() == typeid(float)  )\
      function<float>( param1 );\
    else if ( ptype.GetTypeId() == typeid(double)  )\
      function<double>( param1 );\
}\

#define mitkPixelTypeMultiplex2( function, ptype, param1, param2 )    \
{                                                 \
    if ( ptype.GetTypeId() == typeid(char) )\
      function<char>(  param1, param2 );\
    else if ( ptype.GetTypeId() == typeid(unsigned char))\
      function<unsigned char>(  param1, param2 );\
    else if ( ptype.GetTypeId() == typeid(short) )\
      function<short>(  param1, param2 );      \
    else if ( ptype.GetTypeId() == typeid(unsigned short) )\
      function<unsigned short>( param1, param2 );\
    else if ( ptype.GetTypeId() == typeid(int) )\
      function<int>(  param1, param2 );\
    else if ( ptype.GetTypeId() == typeid(unsigned int) )\
      function<unsigned int>( param1, param2 );\
    else if ( ptype.GetTypeId() == typeid(long int) )\
      function<long int>( param1, param2 );\
    else if ( ptype.GetTypeId() == typeid(unsigned long int) )\
      function<unsigned long int>(  param1, param2 );\
    else if ( ptype.GetTypeId() == typeid(float)  )\
      function<float>(  param1, param2 );\
    else if ( ptype.GetTypeId() == typeid(double)  )\
      function<double>( param1, param2 );\
}\

#define mitkPixelTypeMultiplex3( function, ptype, param1, param2, param3 )    \
{                                                 \
    if ( ptype.GetTypeId() == typeid(char) )\
      function<char>(  param1, param2, param3 );\
    else if ( ptype.GetTypeId() == typeid(unsigned char))\
      function<unsigned char>(  param1, param2, param3 );\
    else if ( ptype.GetTypeId() == typeid(short) )\
      function<short>(  param1, param2, param3 );      \
    else if ( ptype.GetTypeId() == typeid(unsigned short) )\
      function<unsigned short>( param1, param2, param3 );\
    else if ( ptype.GetTypeId() == typeid(int) )\
      function<int>(  param1, param2, param3 );\
    else if ( ptype.GetTypeId() == typeid(unsigned int) )\
      function<unsigned int>( param1, param2, param3 );\
    else if ( ptype.GetTypeId() == typeid(long int) )\
      function<long int>( param1, param2, param3 );\
    else if ( ptype.GetTypeId() == typeid(unsigned long int) )\
      function<unsigned long int>(  param1, param2, param3 );\
    else if ( ptype.GetTypeId() == typeid(float)  )\
      function<float>(  param1, param2, param3 );\
    else if ( ptype.GetTypeId() == typeid(double)  )\
      function<double>( param1, param2, param3 );\
}\

#define mitkPixelTypeMultiplex4( function, ptype, param1, param2, param3, param4 )    \
{                                                 \
    if ( ptype.GetTypeId() == typeid(char) )\
      function<char>( param1, param2, param3, param4 );\
    else if ( ptype.GetTypeId() == typeid(unsigned char))\
      function<unsigned char>(  param1, param2, param3, param4  );\
    else if ( ptype.GetTypeId() == typeid(short) )\
      function<short>(  param1, param2, param3, param4  );      \
    else if ( ptype.GetTypeId() == typeid(unsigned short) )\
      function<unsigned short>( param1, param2, param3, param4  );\
    else if ( ptype.GetTypeId() == typeid(int) )\
      function<int>(  param1, param2, param3, param4  );\
    else if ( ptype.GetTypeId() == typeid(unsigned int) )\
      function<unsigned int>( param1, param2, param3, param4  );\
    else if ( ptype.GetTypeId() == typeid(long int) )\
      function<long int>( param1, param2, param3, param4  );\
    else if ( ptype.GetTypeId() == typeid(unsigned long int) )\
      function<unsigned long int>(  param1, param2, param3, param4  );\
    else if ( ptype.GetTypeId() == typeid(float)  )\
      function<float>(  param1, param2, param3, param4  );\
    else if ( ptype.GetTypeId() == typeid(double)  )\
      function<double>( param1, param2, param3, param4 );\
}\

#endif // MITKPIXELTYPEMULTIPLEX_H


