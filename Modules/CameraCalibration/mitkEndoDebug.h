#ifndef mitkEndoDebug_h
#define mitkEndoDebug_h

#include <set>
#include <string>
#include <iostream>
#include <sstream>
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
    /// helper function getting unique file name
    ///
    static std::string GetUniqueFileName( const std::string& dir, const std::string& ext="jpg" );

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
        outputFile =  mitk::EndoDebug::GetInstance().GetUniqueFileName(outputFile, "jpg");\
        cv::imwrite(outputFile, imgVariableName);\
      }\
      cv::imshow( "Debug", imgVariableName ); \
      cv::waitKey( mitk::EndoDebug::GetInstance().GetShowImagesTimeOut() ); \
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

#endif

#endif // mitkEndoDebug_h


