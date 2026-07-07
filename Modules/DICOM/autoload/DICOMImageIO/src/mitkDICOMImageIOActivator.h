/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMImageIOActivator_h
#define mitkDICOMImageIOActivator_h

#include <usModuleActivator.h>
#include <usModuleEvent.h>

#include <memory>
#include <mutex>

namespace mitk {

struct IFileReader;
class IDICOMTagsOfInterest;

/**
 * \brief Module activator for the DICOM Image IO module.
 *
 * Registers DICOM reader services (auto-selecting, manual-selecting, and
 * simple volume readers) and DICOM tags-of-interest service when the module
 * is loaded.
 */
class DICOMImageIOActivator : public us::ModuleActivator
{
public:
  /** \brief Register DICOM reader services and tags-of-interest service. */
  void Load(us::ModuleContext* context) override;

  /** \brief Unregister all DICOM IO services. */
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

#endif
