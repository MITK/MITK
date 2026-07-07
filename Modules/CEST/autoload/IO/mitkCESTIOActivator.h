/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCESTIOActivator_h
#define mitkCESTIOActivator_h

#include <mitkCustomMimeType.h>
#include <mitkDICOMTagsOfInterestAddHelper.h>

#include <usModuleActivator.h>

#include <memory>

namespace mitk
{
  struct IFileReader;

  /**
   * \brief Module activator for the CEST IO module.
   *
   * Registers CEST-specific DICOM reader services and associated MIME types
   * when the module is loaded, and unregisters them on unload.
   */
  class CESTIOActivator : public us::ModuleActivator
  {
  public:
    /** \brief Register CEST DICOM reader services and MIME types. */
    void Load(us::ModuleContext *context) override;

    /** \brief Unregister all CEST IO services. */
    void Unload(us::ModuleContext *context) override;

  private:
    std::unique_ptr<IFileReader> m_CESTDICOMReader;
    std::unique_ptr<IFileReader> m_CESTDICOMManualWithMetaFileReader;
    std::unique_ptr<IFileReader> m_CESTDICOMManualWithOutMetaFileReader;
    std::vector<mitk::CustomMimeType *> m_MimeTypes;

    // Module context
    us::ModuleContext* m_Context;

    DICOMTagsOfInterestAddHelper m_TagHelper;
  };

}

#endif
