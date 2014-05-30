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
#ifndef mitkEndoDebug_h
#define mitkEndoDebug_h

#include <set>
#include <string>
#include <iostream>
#include <sstream>
#include <MitkCameraCalibrationExports.h>

namespace mitk
{
  ///
  /// again d pointer impl
  ///
  struct EndoDebugData;

  ///
  /// class responsible for handling debug matters
  /// in endotracking
  ///
  struct MitkCameraCalibration_EXPORT EndoDebug
  {
    ///
    /// singleton class
    ///
    static EndoDebug& GetInstance();

    ///
    /// helper function getting unique file name
    ///
    static std::string GetUniqueFileName(const std::string& dir, const std::string& ext="jpg" , const std::string &prefix="");

    ///
    /// set if debug is enabled at all
    ///
    void SetDebugEnabled(bool _DebugEnabled);

    ///
    /// \return true if debug should be enabled
    ///
    bool GetDebugEnabled();

    ///
    /// set if debug is enabled at all
    ///
    void SetShowImagesInDebug(bool _ShowImagesInDebug);

    ///
    /// \return true if debug should be enabled
    ///
    bool GetShowImagesInDebug();

    ///
    /// set if debug is enabled at all
    ///
    void SetShowImagesTimeOut(size_t _ShowImagesTimeOut);

    ///
    /// \return true if debug should be enabled
    ///
    size_t GetShowImagesTimeOut();

    ///
    /// sets an output directory. if set all images that are shown are also written to that
    /// output dir
    ///
    void SetDebugImagesOutputDirectory(const std::string& _DebugImagesOutputDirectory);

    ///
    /// \return true if debug should be enabled
    ///
    std::string GetDebugImagesOutputDirectory() const;

    ///
    /// \return the basename of a file without path
    ///
    std::string GetFilenameWithoutExtension(const std::string& s);

    ///
    /// add a file to debug ( if one or more files are set )
    /// only those files will be debugged when using the macros
    /// below. e.g. call AddFileToDebug("MyClass.cpp"), then
    /// statements like endodebug(...) will be evaluated in
    /// MyClass.cpp and nowhere else
    ///
    bool AddFileToDebug(const std::string& fileToDebug);

    ///
    /// \see AddFileToDebug
    ///
    void SetFilesToDebug(const std::set<std::string>& filesToDebug);

    ///
    /// \return the files to be debugged
    ///
    std::set<std::string> GetFilesToDebug();

    ///
    /// same as files to debug, but the user can provide
    /// any symbol string. if one or more symbols
    /// are set only for these symbols Debug() will return true
    ///
    bool AddSymbolToDebug(const std::string& symbolToDebug);

    ///
    /// \see AddSymbolToDebug
    ///
    void SetSymbolsToDebug(const std::set<std::string>& symbolsToDebug);

    ///
    /// \return the symbols to be debugged
    ///
    std::set<std::string> GetSymbolsToDebug();

    ///
    /// \return true if file should be debugged
    ///
    bool DebugFile( const std::string& fileToDebug );

    ///
    /// \return true if symbol should be debugged
    ///
    bool DebugSymbol( const std::string& symbolToDebug );
    ///
    /// \return the all in all status if debug output
    /// should be generated
    ///
    bool Debug( const std::string& fileToDebug, const std::string& symbol="" );
    ///
    /// set a log file. if a log file is set and debug is activated all messages will be appended to that file
    ///
    void SetLogFile( const std::string& file );
    ///
    /// shows a message or writes it to a log file if a file is set (and is valid for writing)
    ///
    void ShowMessage( const std::string& message );
    ///
    /// init defaults
    ///
    EndoDebug();
    ///
    /// delete d pointer
    ///
    virtual ~EndoDebug();

   private:
    ///
    /// d pointer
    ///
    EndoDebugData* d;
  };
}

// DISABLE DEBUGGING FOR RELEASE MODE ON WINDOWS
#if (defined(WIN32) &&  !defined(_DEBUG)) || defined(NDEBUG)
    #define endodebugmarker
    #define endodebug(msg)
    #define endodebugvar(var)
    #define endodebugsymbol(var, mSymbol)
    #define endodebugimg(imgVariableName)
    #define endodebugbegin if( false ) {
    #define endodebugend }
    #define endoAssert(a) \
    if(!(a)) { \
        throw std::invalid_argument("FAILED: " #a); \
    }

    #define endoAssertMsg(a, msg) \
    if(!(a)) { \
    throw std::invalid_argument( "FAILED: " #a ); \
    }

    #define endodebugcode(code)
    #define endoAssertCode(assertCode)
#else
  ///
  /// macro for debugging purposes
  ///
  #define endodebugmarker\
    if( mitk::EndoDebug::GetInstance().Debug(__FILE__) ) \
    { \
      std::ostringstream ___ostringstream; \
      ___ostringstream << mitk::EndoDebug::GetInstance().GetFilenameWithoutExtension(__FILE__) << ", " << __LINE__\
      << ": " << __FUNCTION__ << std::endl;\
      mitk::EndoDebug::GetInstance().ShowMessage( ___ostringstream.str() ); \
    }

  ///
  /// macro for debugging purposes
  ///
  #define endodebug(msg)\
    if( mitk::EndoDebug::GetInstance().Debug(__FILE__) ) \
    { \
      std::ostringstream ___ostringstream; \
      ___ostringstream << mitk::EndoDebug::GetInstance().GetFilenameWithoutExtension(__FILE__) << ", " << __LINE__\
        << ": " << msg << std::endl;\
      mitk::EndoDebug::GetInstance().ShowMessage( ___ostringstream.str() ); \
    }

  ///
  /// macro for debugging variables
  ///
  #define endodebugvar(var)\
    if( mitk::EndoDebug::GetInstance().Debug(__FILE__) ) \
    { \
      std::ostringstream ___ostringstream; \
      ___ostringstream << mitk::EndoDebug::GetInstance().GetFilenameWithoutExtension(__FILE__) << ", " << __LINE__\
        << ": " #var " = " << var << std::endl;\
      mitk::EndoDebug::GetInstance().ShowMessage( ___ostringstream.str() ); \
    }

  ///
  /// macro for debugging a variable as symbol
  ///
  #define endodebugsymbol(var, mSymbol)\
    if( mitk::EndoDebug::GetInstance().Debug(__FILE__, mSymbol) ) \
    { \
      std::ostringstream ___ostringstream; \
      ___ostringstream << mitk::EndoDebug::GetInstance().GetFilenameWithoutExtension(__FILE__) << ", " << __LINE__\
        << ": " #var " = " << var << std::endl;\
      mitk::EndoDebug::GetInstance().ShowMessage( ___ostringstream.str() ); \
    }

  ///
  /// macro for showing cv image if in debug mode
  /// highgui.h must be included before
  ///
  #define endodebugimg(imgVariableName)\
    if( mitk::EndoDebug::GetInstance().Debug(__FILE__) \
        && mitk::EndoDebug::GetInstance().GetShowImagesInDebug() \
        && (imgVariableName).cols > 0 && (imgVariableName).rows > 0 && (imgVariableName).data) \
    { \
      std::ostringstream ___ostringstream; \
      ___ostringstream << mitk::EndoDebug::GetInstance().GetFilenameWithoutExtension(__FILE__) << ", " << __LINE__\
        << ": Showing " #imgVariableName << std::endl; \
      mitk::EndoDebug::GetInstance().ShowMessage( ___ostringstream.str() ); \
      std::string outputFile = mitk::EndoDebug::GetInstance().GetDebugImagesOutputDirectory(); \
      if( !outputFile.empty() ) \
      {\
        outputFile =  mitk::EndoDebug::GetInstance().GetUniqueFileName(outputFile, "jpg", std::string(#imgVariableName) );\
        cv::imwrite(outputFile, imgVariableName);\
      }\
      else\
      {\
        cv::imshow( "Debug", imgVariableName ); \
        cv::waitKey( mitk::EndoDebug::GetInstance().GetShowImagesTimeOut() ); \
      }\
    }

  ///
  /// macro for a section that should only be executed if debugging is enabled
  ///
  #define endodebugbegin \
    if( mitk::EndoDebug::GetInstance().Debug(__FILE__) ) \
    {

  ///
  /// macro for a section that should only be executed if debugging is enabled
  ///
  #define endodebugend \
    }

  #define endodebugcode(code) \
    endodebugbegin \
    code \
    endodebugend

///
/// an assert macro for throwing exceptions from an assert
///
#define endoAssert(a) \
if(!(a)) { \
std::ostringstream s; \
s << mitk::EndoDebug::GetInstance().GetFilenameWithoutExtension(__FILE__) << ", " \
  << __LINE__ << ", failed: " << #a; \
throw std::invalid_argument(s.str()); }

///
/// same as above but with an output error stream
/// use it like this: endoAssertMsg( file.read() == true, file << "could not be read" );
///
#define endoAssertMsg(a, msg) \
if(!(a)) { \
  std::ostringstream s; \
  s << mitk::EndoDebug::GetInstance().GetFilenameWithoutExtension(__FILE__) << ", " \
    << __LINE__ << ": " << msg; \
  throw std::invalid_argument(s.str()); \
  }

#define endoAssertCode(assertCode) \
    assertCode

#endif

#endif // mitkEndoDebug_h


