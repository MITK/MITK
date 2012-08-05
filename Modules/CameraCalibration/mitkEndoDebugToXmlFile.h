#ifndef mitkEndoDebugToXmlFile_h
#define mitkEndoDebugToXmlFile_h

#include "mitkEndoDebug.h"
#include "mitkAlgorithm.h"

#include <mitkCameraCalibrationExports.h>

namespace mitk
{

  ///
  /// d pointer forward declaration
  ///
  struct EndoDebugToXmlFileData;

  ///
  /// prints the EndoDebug to xml
  ///
  class mitkCameraCalibration_EXPORT EndoDebugToXmlFile:
    virtual public mitk::Algorithm
  {
  public:
    ///
    /// init default values
    ///
    EndoDebugToXmlFile(
      EndoDebug* _EndoDebug,
      const std::string* _FileName);

    ///
    /// Actually executes the algorithm.
    ///
    void Update();
    ///
    /// nothing to do here yet
    ///
    virtual ~EndoDebugToXmlFile();

  private:
    ///
    /// d pointer implementation
    ///
    EndoDebugToXmlFileData* d;
  };

} // namespace mitk

#endif // mitkEndoDebugToXmlFile_h
