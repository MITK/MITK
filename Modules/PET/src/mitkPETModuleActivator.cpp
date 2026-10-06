/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usModuleActivator.h>
#include <usModuleContext.h>

#include <mitkCoreServices.h>
#include <mitkDICOMTagPath.h>
#include <mitkIPropertyDescriptions.h>
#include <mitkIPropertyPersistence.h>
#include <mitkPropertyPersistenceInfo.h>
#include <mitkSUVCalculationHelper.h>
#include <mitkTemporoSpatialStringProperty.h>

#include <algorithm>
#include <string>

namespace mitk
{
  /**
   * \brief Module activator for the PET module.
   *
   * Registers persistence for the two properties SUVImageFilter writes to
   * record which IBSI-SUV input adaptations it applied. Without a
   * registration ItkImageIO drops a property on save without an error, a
   * warning, or a log line (mitkItkImageIO.cpp, GetInfo / empty infoList),
   * so an unregistered audit trail would pass every in-memory test and
   * vanish on disk.
   */
  class PETModuleActivator : public us::ModuleActivator
  {
  public:
    void Load(us::ModuleContext*) override
    {
      CoreServicePointer<IPropertyDescriptions> descriptions(CoreServices::GetPropertyDescriptions());
      CoreServicePointer<IPropertyPersistence> persistence(CoreServices::GetPropertyPersistence());

      descriptions->AddDescription(
        SUV_ADAPTATIONS_PROPERTY_NAME,
        "JSON array of the IBSI-SUV input adaptations applied while computing "
        "this SUV image. Empty when the input needed none, which is the only "
        "possible outcome under the strict DICOM read policy.");

      auto adaptations = PropertyPersistenceInfo::New();
      adaptations->SetNameAndKey(SUV_ADAPTATIONS_PROPERTY_NAME, "mitk_pet_suv_adaptations");
      persistence->AddInfo(adaptations, true);

      // (0008,2111) Derivation Description is not among MITK's DICOM tags of
      // interest, so unlike the tags SUVImageFilter already writes it gets no
      // registration from DICOMTagsOfInterestService and needs its own. The
      // serialization pair and the '.' -> '_' key convention mirror that
      // service exactly; the filter writes the property as a
      // TemporoSpatialStringProperty, and a mismatched serializer here would
      // be the same silent data loss in a different place.
      const std::string name = DICOMTagPathToPropertyName(DICOMTagPath(0x0008, 0x2111));
      std::string key = name;
      std::replace(key.begin(), key.end(), '.', '_');

      descriptions->AddDescription(name, "DICOM tag: DerivationDescription");

      auto derivation = PropertyPersistenceInfo::New();
      derivation->SetNameAndKey(name, key);
      derivation->SetSerializationFunction(
        PropertyPersistenceSerialization::serializeTemporoSpatialStringPropertyToJSON);
      derivation->SetDeserializationFunction(
        PropertyPersistenceDeserialization::deserializeJSONToTemporoSpatialStringProperty);
      persistence->AddInfo(derivation, true);
    }

    void Unload(us::ModuleContext*) override {}
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::PETModuleActivator)
