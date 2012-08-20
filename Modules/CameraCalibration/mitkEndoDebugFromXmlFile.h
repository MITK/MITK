#ifndef mitkEndoDebugFromXmlFile_h
#define mitkEndoDebugFromXmlFile_h

#include "mitkEndoDebug.h"
#include "mitkAlgorithm.h"
#include <mitkCameraCalibrationExports.h>

namespace mitk
{

  ///
  /// d pointer forward declaration
  ///
  struct EndoDebugFromXmlFileData;

  ///
  /// read EndoDebug params from xml file
  ///
  class mitkCameraCalibration_EXPORT EndoDebugFromXmlFile:
    public Algorithm
  {
  public:
    ///
    /// init default values
    ///
    EndoDebugFromXmlFile(
      const std::string* _FileName,
      EndoDebug* _EndoDebug);

    ///
    /// Actually Updates the algorithm
    ///
    void Update();

    ///
    /// nothing to do here yet
    ///
    virtual ~EndoDebugFromXmlFile();
  protected:
  private:
    ///
    /// d pointer implementation
    ///
    EndoDebugFromXmlFileData* d;
  };

} // namespace mitk

#endif // mitkEndoDebugFromXmlFile_h
