/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDICOMImageIOActivator.h"

#include "mitkAutoSelectingDICOMReaderService.h"
#include "mitkManualSelectingDICOMReaderService.h"
#include "mitkDICOMTagsOfInterestService.h"
#include "mitkSimpleVolumeDICOMSeriesReaderService.h"
#include "mitkCoreServices.h"
#include "mitkPropertyPersistenceInfo.h"
#include "mitkDICOMIOMetaInformationPropertyConstants.h"
#include "mitkIPropertyPersistence.h"
#include "mitkTemporoSpatialStringProperty.h"

#include <usModuleContext.h>
#include <usModuleRegistry.h>

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

  void DICOMImageIOActivator::Load(us::ModuleContext* context)
  {
    m_Context = context;

    m_AutoSelectingDICOMReader = std::make_unique<AutoSelectingDICOMReaderService>();
    m_SimpleVolumeDICOMSeriesReader = std::make_unique<SimpleVolumeDICOMSeriesReaderService>();

    m_DICOMTagsOfInterestService = std::make_unique<DICOMTagsOfInterestService>();
    context->RegisterService<mitk::IDICOMTagsOfInterest>(m_DICOMTagsOfInterestService.get());

    DICOMTagPathMapType tagmap = GetDefaultDICOMTagsOfInterest();
    for (const auto &tag : tagmap)
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

    //We have to handle ManualSelectingDICOMSeriesReader different then the other
    //readers. Reason: The reader uses DICOMFileReaderSelector in its constructor.
    //this class needs to access resources of MitkDICOM module, which might
    //not be initialized yet (that would lead to a crash, see i.a. T27553). Thus check if the module
    //is alreade loaded. If not, register a listener and create the reader as soon
    //as the module is available.
    auto dicomModule = us::ModuleRegistry::GetModule("MitkDICOM");
    if (nullptr == dicomModule)
    {
      std::lock_guard<std::mutex> lock(m_Mutex);
      // Listen for events of module life cycle.
      m_Context->AddModuleListener(this, &DICOMImageIOActivator::EnsureManualSelectingDICOMSeriesReader);
    }
    else
    {
      m_ManualSelectingDICOMSeriesReader = std::make_unique<ManualSelectingDICOMReaderService>();
    }
  }

  void DICOMImageIOActivator::Unload(us::ModuleContext*)
  {
  }

  void DICOMImageIOActivator::EnsureManualSelectingDICOMSeriesReader(const us::ModuleEvent event)
  {
    //We have to handle ManualSelectingDICOMSeriesReader different then the other
    //readers. For more details see the explanations in the constructor.
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (nullptr == m_ManualSelectingDICOMSeriesReader && event.GetModule()->GetName()=="MitkDICOM" && event.GetType() == us::ModuleEvent::LOADED)
    {
      m_ManualSelectingDICOMSeriesReader = std::make_unique<ManualSelectingDICOMReaderService>();
    }
  }
}

US_EXPORT_MODULE_ACTIVATOR(mitk::DICOMImageIOActivator)
