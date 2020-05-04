/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDICOMReaderServicesActivator.h"

#include "mitkAutoSelectingDICOMReaderService.h"
#include "mitkClassicDICOMSeriesReaderService.h"
#include "mitkDICOMTagsOfInterestService.h"
#include "mitkSimpleVolumeDICOMSeriesReaderService.h"
#include "mitkCoreServices.h"
#include "mitkPropertyPersistenceInfo.h"
#include "mitkDICOMIOMetaInformationPropertyConstants.h"
#include "mitkIPropertyPersistence.h"
#include "mitkTemporoSpatialStringProperty.h"

#include <usModuleContext.h>

void AddPropertyPersistence(const mitk::PropertyKeyPath& propPath, bool temporoSpatial = false)
{
  mitk::CoreServicePointer<mitk::IPropertyPersistence> persistenceService(mitk::CoreServices::GetPropertyPersistence());

  mitk::PropertyPersistenceInfo::Pointer info = mitk::PropertyPersistenceInfo::New();
  if (propPath.IsExplicit())
  {
    std::string name = mitk::PropertyKeyPathToPropertyName(propPath);
    std::string key = name;
    std::replace(key.begin(), key.end(), '.', '_');
    info->SetNameAndKey(name, key);
  }
  else
  {
    std::string key = mitk::PropertyKeyPathToPersistenceKeyRegEx(propPath);
    std::string keyTemplate = mitk::PropertyKeyPathToPersistenceKeyTemplate(propPath);
    std::string propRegEx = mitk::PropertyKeyPathToPropertyRegEx(propPath);
    std::string propTemplate = mitk::PropertyKeyPathToPersistenceNameTemplate(propPath);
    info->UseRegEx(propRegEx, propTemplate, key, keyTemplate);
  }

  if (temporoSpatial)
  {
    info->SetDeserializationFunction(mitk::PropertyPersistenceDeserialization::deserializeJSONToTemporoSpatialStringProperty);
    info->SetSerializationFunction(mitk::PropertyPersistenceSerialization::serializeTemporoSpatialStringPropertyToJSON);
  }

  persistenceService->AddInfo(info);
}

namespace mitk {

  void DICOMReaderServicesActivator::Load(us::ModuleContext* context)
  {
    m_AutoSelectingDICOMReader.reset(new AutoSelectingDICOMReaderService());
    m_SimpleVolumeDICOMSeriesReader.reset(new SimpleVolumeDICOMSeriesReaderService());

    m_DICOMTagsOfInterestService.reset(new DICOMTagsOfInterestService());
    context->RegisterService<mitk::IDICOMTagsOfInterest>(m_DICOMTagsOfInterestService.get());

    DICOMTagPathMapType tagmap = GetDefaultDICOMTagsOfInterest();
    for (auto tag : tagmap)
    {
      m_DICOMTagsOfInterestService->AddTagOfInterest(tag.first);
    }

    //add properties that should be persistent (if possible/supported by the writer)
    AddPropertyPersistence(mitk::DICOMIOMetaInformationPropertyConstants::READER_3D_plus_t());
    AddPropertyPersistence(mitk::DICOMIOMetaInformationPropertyConstants::READER_CONFIGURATION());
    AddPropertyPersistence(mitk::DICOMIOMetaInformationPropertyConstants::READER_DCMTK());
    AddPropertyPersistence(mitk::DICOMIOMetaInformationPropertyConstants::READER_FILES(), true);
    AddPropertyPersistence(mitk::DICOMIOMetaInformationPropertyConstants::READER_GANTRY_TILT_CORRECTED());
    AddPropertyPersistence(mitk::DICOMIOMetaInformationPropertyConstants::READER_GDCM());
    AddPropertyPersistence(mitk::DICOMIOMetaInformationPropertyConstants::READER_IMPLEMENTATION_LEVEL());
    AddPropertyPersistence(mitk::DICOMIOMetaInformationPropertyConstants::READER_IMPLEMENTATION_LEVEL_STRING());
    AddPropertyPersistence(mitk::DICOMIOMetaInformationPropertyConstants::READER_PIXEL_SPACING_INTERPRETATION());
    AddPropertyPersistence(mitk::DICOMIOMetaInformationPropertyConstants::READER_PIXEL_SPACING_INTERPRETATION_STRING());

  }

  void DICOMReaderServicesActivator::Unload(us::ModuleContext*)
  {
  }

}

US_EXPORT_MODULE_ACTIVATOR(mitk::DICOMReaderServicesActivator)
