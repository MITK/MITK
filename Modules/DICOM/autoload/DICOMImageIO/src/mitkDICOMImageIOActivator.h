/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKDICOMIMAGEIOACTIVATOR_H
#define MITKDICOMIMAGEIOACTIVATOR_H

#include <usModuleActivator.h>
#include <usModuleEvent.h>

#include <memory>
#include <mutex>

namespace mitk {

struct IFileReader;
class IDICOMTagsOfInterest;

class DICOMImageIOActivator : public us::ModuleActivator
{
public:

  void Load(us::ModuleContext* context) override;
  void Unload(us::ModuleContext* context) override;

private:
  void EnsureManualSelectingDICOMSeriesReader(const us::ModuleEvent event);

  std::unique_ptr<IFileReader> m_AutoSelectingDICOMReader;
  std::unique_ptr<IFileReader> m_ManualSelectingDICOMSeriesReader;
  std::unique_ptr<IFileReader> m_SimpleVolumeDICOMSeriesReader;
  std::unique_ptr<IDICOMTagsOfInterest> m_DICOMTagsOfInterestService;

  us::ModuleContext* m_Context;

  /**mutex to guard the module listening */
  std::mutex m_Mutex;
};

}

#endif // MITKDICOMIMAGEIOACTIVATOR_H
