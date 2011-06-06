#ifndef mitkEndoDebug_h
#define mitkEndoDebug_h

#include <set>
#include <string>
#include <iostream>
#include <mitkCameraCalibrationExports.h>

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
  struct mitkCameraCalibration_EXPORT EndoDebug
  {
    ///
    /// singleton class
    ///
    static EndoDebug& GetInstance();

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
    bool GetShowImagesTimeOut();

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
    void AddFileToDebug(const std::string& fileToDebug);

    ///
    /// same as files to debug, but the user can provide
    /// any symbol string. if one or more symbols
    /// are set only for these symbols Debug() will return true
    ///
    void AddSymbolToDebug(const std::string& symbolToDebug);

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

///
/// macro for debugging purposes
///
#define endodebugmarker\
  if( mitk::EndoDebug::GetInstance().Debug(__FILE__) ) \
  { \
    std::cout << mitk::EndoDebug::GetInstance().GetFilenameWithoutExtension(__FILE__) << ", " << __LINE__\
    << ": " << __FUNCTION__ << std::endl;\
  }

///
/// macro for debugging purposes
///
#define endodebug(msg)\
  if( mitk::EndoDebug::GetInstance().Debug(__FILE__) ) \
  { \
    std::cout << mitk::EndoDebug::GetInstance().GetFilenameWithoutExtension(__FILE__) << ", " << __LINE__\
      << ": " << msg << std::endl;\
  }

///
/// macro for debugging variables
///
#define endodebugvar(var)\
  if( mitk::EndoDebug::GetInstance().Debug(__FILE__) ) \
  { \
    std::cout << mitk::EndoDebug::GetInstance().GetFilenameWithoutExtension(__FILE__) << ", " << __LINE__\
      << ": " #var " = " << var << std::endl;\
  }

///
/// macro for debugging a variable as symbol
///
#define endodebugsymbol(var, mSymbol)\
  if( mitk::EndoDebug::GetInstance().Debug(__FILE__, mSymbol) ) \
  { \
    std::cout << mitk::EndoDebug::GetInstance().GetFilenameWithoutExtension(__FILE__) << ", " << __LINE__\
      << ": " #var " = " << var << std::endl;\
  }

///
/// macro for showing cv image if in debug mode
/// highgui.h must be included before
///
#define endodebugimg(imgVariableName)\
  if( mitk::EndoDebug::GetInstance().Debug(__FILE__) \
      && mitk::EndoDebug::GetInstance().GetShowImagesInDebug()) \
  { \
    std::cout << mitk::EndoDebug::GetInstance().GetFilenameWithoutExtension(__FILE__) << ", " << __LINE__\
      << ": Showing " #imgVariableName << std::endl; \
    cv::imshow( "Debug", imgVariableName ); \
    cv::waitKey( mitk::EndoDebug::GetInstance().GetShowImagesTimeOut() ); \
  }

#endif // mitkEndoDebug_h


