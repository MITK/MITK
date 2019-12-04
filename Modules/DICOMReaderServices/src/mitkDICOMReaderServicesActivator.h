/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKDICOMREADERSERVICESACTIVATOR_H
#define MITKDICOMREADERSERVICESACTIVATOR_H

#include <usModuleActivator.h>
#include <usServiceEvent.h>

#include <memory>

namespace mitk {

struct IFileReader;
class IDICOMTagsOfInterest;

class DICOMReaderServicesActivator : public us::ModuleActivator
{
public:

  void Load(us::ModuleContext* context) override;
  void Unload(us::ModuleContext* context) override;

  void AliasServiceChanged(const us::ServiceEvent event);

private:

  std::unique_ptr<IFileReader> m_AutoSelectingDICOMReader;
  std::unique_ptr<IFileReader> m_ClassicDICOMSeriesReader;
  std::unique_ptr<IFileReader> m_SimpleVolumeDICOMSeriesReader;
  std::unique_ptr<IDICOMTagsOfInterest> m_DICOMTagsOfInterestService;

  us::ModuleContext* mitkContext;

};

}

#endif // MITKDICOMREADERSERVICESACTIVATOR_H
