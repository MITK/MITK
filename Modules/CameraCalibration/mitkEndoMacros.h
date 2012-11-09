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
#ifndef mitkEndoMacros_h
#define mitkEndoMacros_h

///
/// COLLECTION OF MACROS FOR THE ENDOTRACKING MODULE
///

///
/// multiplexing for cv mats
///
#define endoAccessCvMat(function, T, arg1, arg2) \
if( arg2.type() == cv::DataType<float>::type ) \
  function<float, T>( arg1, arg2 ); \
else if( arg2.type() == cv::DataType<double>::type ) \
  function<double, T>( arg1, arg2 ); \
else if( arg2.type() == cv::DataType<unsigned int>::type ) \
  function<unsigned int, T>( arg1, arg2 ); \
else if( arg2.type() == cv::DataType<unsigned short>::type ) \
  function<unsigned short, T>( arg1, arg2 ); \
else if( arg2.type() == cv::DataType<unsigned char>::type ) \
  function<unsigned char, T>( arg1, arg2 ); \
else if( arg2.type() == cv::DataType<int>::type ) \
  function<int, T>( arg1, arg2 ); \
else if( arg2.type() == cv::DataType<short>::type ) \
  function<short, T>( arg1, arg2 ); \
else if( arg2.type() == cv::DataType<char>::type ) \
  function<char, T>( arg1, arg2 ); \
else \
  throw std::invalid_argument("Unknown type for cv::Mat");

///
/// exec an algorithm with 1 output argument
///
#define endoExec(macroAlgName, macroOutputType, macroOutputVarName, ...)\
  macroOutputType macroOutputVarName;\
  { \
    macroAlgName _macroAlgName(__VA_ARGS__, &macroOutputVarName);\
    _macroAlgName.Update();\
  }

///
/// exec an algorithm with 2 output argument
///
#define endoExec2(macroAlgName, macroOutputType1, macroOutputVarName1, macroOutputType2, macroOutputVarName2, ...)\
  macroOutputType1 macroOutputVarName1;\
  macroOutputType1 macroOutputVarName1;\
  { \
    macroAlgName _macroAlgName(__VA_ARGS__, &macroOutputVarName1, &macroOutputVarName2);\
    _macroAlgName.Update();\
  }

///
/// an assert macro for throwing exceptions from an assert
///
#define endoAssert(a) if(!(a)) { \
std::ostringstream s; \
s << mitk::EndoDebug::GetInstance().GetFilenameWithoutExtension(__FILE__) << ", " \
  << __LINE__ << ", failed: " << #a; \
throw std::invalid_argument(s.str()); }

///
/// same as above but with an output error stream
/// use it like this: endoAssertMsg( file.read() == true, file << "could not be read" );
///
#define endoAssertMsg(a, msg) if(!(a)) { \
  std::ostringstream s; \
  s << mitk::EndoDebug::GetInstance().GetFilenameWithoutExtension(__FILE__) << ", " \
    << __LINE__ << ": " << msg; \
  throw std::invalid_argument(s.str()); \
  }

///
/// definition of the corresponding directory separator
///
#ifdef WIN32
  static const std::string DIR_SEPARATOR =  "\\";
#else
  static const std::string DIR_SEPARATOR = "/";
#endif

#endif // mitkEndoMacros_h
