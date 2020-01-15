/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkEndoDebugFromXmlFile_h
#define mitkEndoDebugFromXmlFile_h

#include "mitkEndoDebug.h"
#include "mitkAlgorithm.h"
#include <MitkCameraCalibrationExports.h>

namespace mitk
{

  ///
  /// d pointer forward declaration
  ///
  struct EndoDebugFromXmlFileData;

  ///
  /// read EndoDebug params from xml file
  ///
  class MITKCAMERACALIBRATION_EXPORT EndoDebugFromXmlFile:
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
    void Update() override;

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
