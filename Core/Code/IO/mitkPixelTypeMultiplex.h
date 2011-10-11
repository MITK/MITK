#ifndef MITKPIXELTYPEMULTIPLEX_H
#define MITKPIXELTYPEMULTIPLEX_H

#define mitkPixelTypeMultiplex0( function, ptype )    \
{                                                 \
    if ( ptype.GetTypeId() == typeid(char) )\
      function<char>( ptype,  );\
    else if ( ptype.GetTypeId() == typeid(unsigned char))\
      function<unsigned char>( ptype,  );\
    else if ( ptype.GetTypeId() == typeid(short) )\
      function<short>( ptype,  );      \
    else if ( ptype.GetTypeId() == typeid(unsigned short) )\
      function<unsigned short>( ptype,  );\
    else if ( ptype.GetTypeId() == typeid(int) )\
      function<int>( ptype,  );\
    else if ( ptype.GetTypeId() == typeid(unsigned int) )\
      function<unsigned int>( ptype,  );\
    else if ( ptype.GetTypeId() == typeid(long int) )\
      function<long int>( ptype,  );\
    else if ( ptype.GetTypeId() == typeid(unsigned long int) )\
      function<unsigned long int>( ptype,  );\
    else if ( ptype.GetTypeId() == typeid(float)  )\
      function<float>( ptype,  );\
    else if ( ptype.GetTypeId() == typeid(double)  )\
      function<double>( ptype,  );\
}\

#define mitkPixelTypeMultiplex1( function, ptype, param1 )    \
{                                                 \
    if ( ptype.GetTypeId() == typeid(char) )\
      function<char>( ptype,  param1 );\
    else if ( ptype.GetTypeId() == typeid(unsigned char))\
      function<unsigned char>( ptype,  param1 );\
    else if ( ptype.GetTypeId() == typeid(short) )\
      function<short>( ptype,  param1 );      \
    else if ( ptype.GetTypeId() == typeid(unsigned short) )\
      function<unsigned short>( ptype,  param1 );\
    else if ( ptype.GetTypeId() == typeid(int) )\
      function<int>( ptype,  param1 );\
    else if ( ptype.GetTypeId() == typeid(unsigned int) )\
      function<unsigned int>( ptype,  param1 );\
    else if ( ptype.GetTypeId() == typeid(long int) )\
      function<long int>( ptype,  param1 );\
    else if ( ptype.GetTypeId() == typeid(unsigned long int) )\
      function<unsigned long int>( ptype,  param1 );\
    else if ( ptype.GetTypeId() == typeid(float)  )\
      function<float>( ptype,  param1 );\
    else if ( ptype.GetTypeId() == typeid(double)  )\
      function<double>( ptype,  param1 );\
}\

#define mitkPixelTypeMultiplex2( function, ptype, param1, param2 )    \
{                                                 \
    if ( ptype.GetTypeId() == typeid(char) )\
      function<char>( ptype,   param1, param2 );\
    else if ( ptype.GetTypeId() == typeid(unsigned char))\
      function<unsigned char>( ptype,   param1, param2 );\
    else if ( ptype.GetTypeId() == typeid(short) )\
      function<short>( ptype,   param1, param2 );      \
    else if ( ptype.GetTypeId() == typeid(unsigned short) )\
      function<unsigned short>( ptype,  param1, param2 );\
    else if ( ptype.GetTypeId() == typeid(int) )\
      function<int>( ptype,   param1, param2 );\
    else if ( ptype.GetTypeId() == typeid(unsigned int) )\
      function<unsigned int>( ptype,  param1, param2 );\
    else if ( ptype.GetTypeId() == typeid(long int) )\
      function<long int>( ptype,  param1, param2 );\
    else if ( ptype.GetTypeId() == typeid(unsigned long int) )\
      function<unsigned long int>( ptype,   param1, param2 );\
    else if ( ptype.GetTypeId() == typeid(float)  )\
      function<float>( ptype,   param1, param2 );\
    else if ( ptype.GetTypeId() == typeid(double)  )\
      function<double>( ptype,  param1, param2 );\
}\

#define mitkPixelTypeMultiplex3( function, ptype, param1, param2, param3 )    \
{                                                 \
    if ( ptype.GetTypeId() == typeid(char) )\
      function<char>( ptype,   param1, param2, param3 );\
    else if ( ptype.GetTypeId() == typeid(unsigned char))\
      function<unsigned char>( ptype,   param1, param2, param3 );\
    else if ( ptype.GetTypeId() == typeid(short) )\
      function<short>( ptype,   param1, param2, param3 );      \
    else if ( ptype.GetTypeId() == typeid(unsigned short) )\
      function<unsigned short>( ptype,  param1, param2, param3 );\
    else if ( ptype.GetTypeId() == typeid(int) )\
      function<int>( ptype,   param1, param2, param3 );\
    else if ( ptype.GetTypeId() == typeid(unsigned int) )\
      function<unsigned int>( ptype,  param1, param2, param3 );\
    else if ( ptype.GetTypeId() == typeid(long int) )\
      function<long int>( ptype,  param1, param2, param3 );\
    else if ( ptype.GetTypeId() == typeid(unsigned long int) )\
      function<unsigned long int>( ptype,   param1, param2, param3 );\
    else if ( ptype.GetTypeId() == typeid(float)  )\
      function<float>( ptype,   param1, param2, param3 );\
    else if ( ptype.GetTypeId() == typeid(double)  )\
      function<double>( ptype,  param1, param2, param3 );\
}\

#define mitkPixelTypeMultiplex4( function, ptype, param1, param2, param3, param4 )    \
{                                                 \
    if ( ptype.GetTypeId() == typeid(char) )\
      function<char>( ptype,  param1, param2, param3, param4 );\
    else if ( ptype.GetTypeId() == typeid(unsigned char))\
      function<unsigned char>( ptype,   param1, param2, param3, param4  );\
    else if ( ptype.GetTypeId() == typeid(short) )\
      function<short>( ptype,   param1, param2, param3, param4  );      \
    else if ( ptype.GetTypeId() == typeid(unsigned short) )\
      function<unsigned short>( ptype,  param1, param2, param3, param4  );\
    else if ( ptype.GetTypeId() == typeid(int) )\
      function<int>( ptype,   param1, param2, param3, param4  );\
    else if ( ptype.GetTypeId() == typeid(unsigned int) )\
      function<unsigned int>( ptype,  param1, param2, param3, param4  );\
    else if ( ptype.GetTypeId() == typeid(long int) )\
      function<long int>( ptype,  param1, param2, param3, param4  );\
    else if ( ptype.GetTypeId() == typeid(unsigned long int) )\
      function<unsigned long int>( ptype,   param1, param2, param3, param4  );\
    else if ( ptype.GetTypeId() == typeid(float)  )\
      function<float>( ptype,   param1, param2, param3, param4  );\
    else if ( ptype.GetTypeId() == typeid(double)  )\
      function<double>( ptype,  param1, param2, param3, param4 );\
}\

#endif // MITKPIXELTYPEMULTIPLEX_H


