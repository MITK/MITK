/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkEndoDebugToXmlFile_h
#define mitkEndoDebugToXmlFile_h

#include "mitkEndoDebug.h"
#include "mitkAlgorithm.h"

#include <MitkCameraCalibrationExports.h>

namespace mitk
{

  ///
  /// d pointer forward declaration
  ///
  struct EndoDebugToXmlFileData;

  ///
  /// prints the EndoDebug to xml
  ///
  class MITKCAMERACALIBRATION_EXPORT EndoDebugToXmlFile:
    virtual public mitk::Algorithm
  {
  public:
    ///
    /// init default values
    ///
    EndoDebugToXmlFile(
      EndoDebug* _EndoDebug=nullptr,
      const std::string* _FileName=nullptr);


    ///
    /// Actually executes the algorithm.
    ///
    void Update() override;

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

#endif
