/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usModuleActivator.h>
#include <usModuleContext.h>

#include <mitkDicomRTMimeTypes.h>
#include <mitkRTDoseReaderService.h>
#include <mitkRTPlanReaderService.h>
#include <mitkRTStructureSetReaderService.h>

namespace mitk
{
  class DicomRTIOActivator : public us::ModuleActivator
  {
  public:
    DicomRTIOActivator()
    {
    }

    ~DicomRTIOActivator() = default;

    void Load(us::ModuleContext* context) override
    {
      us::ServiceProperties props;
      props[us::ServiceConstants::SERVICE_RANKING()] = 100;

      for (const auto& mimeType : DicomRTMimeTypes::Get())
        context->RegisterService(mimeType.get(), props);

      m_RTDoseReader.reset(new RTDoseReaderService);
      m_RTPlanReader.reset(new RTPlanReaderService);
      m_RTStructureSetReader.reset(new RTStructureSetReaderService);
    }

    void Unload(us::ModuleContext*) override
    {
    }

  private:
    std::unique_ptr<RTDoseReaderService> m_RTDoseReader;
    std::unique_ptr<RTPlanReaderService> m_RTPlanReader;
    std::unique_ptr<RTStructureSetReaderService> m_RTStructureSetReader;
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::DicomRTIOActivator)
