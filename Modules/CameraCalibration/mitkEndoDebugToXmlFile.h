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
      EndoDebug* _EndoDebug=0,
      const std::string* _FileName=0);


    ///
    /// Actually executes the algorithm.
    ///
    void Update();

    ///
    /// setter EndoDebug
    ///
    void SetEndoDebug(EndoDebug* _EndoDebug);

    ///
    /// setter FileName
    ///
    void SetFileName(const std::string* _FileName);

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
