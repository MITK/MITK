/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "mitkDoseVisPreferenceHelper.h"
#include "mitkRTUIConstants.h"
#include "mitkIsoLevelsGenerator.h"

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

#include <ctkPluginContext.h>
#include <service/event/ctkEventAdmin.h>

void mitk::StorePresetsMap(const PresetMapType& presetMap)
{
  auto* prefService = mitk::CoreServices::GetPreferencesService();
  auto* doseVisNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID);
  auto* presetsNode = doseVisNode->Node("isoPresets");

  presetsNode->Clear();

  //store map in new node
  for (PresetMapType::const_iterator pos = presetMap.begin(); pos != presetMap.end(); ++pos)
  {
    auto* aPresetNode = presetsNode->Node(pos->first);

    unsigned int id = 0;

    for (mitk::IsoDoseLevelSet::ConstIterator levelPos = pos->second->Begin(); levelPos != pos->second->End(); ++levelPos, ++id )
    {
      std::ostringstream stream;
      stream << id;

      auto* levelNode = aPresetNode->Node(stream.str());

      levelNode->PutDouble(mitk::RTUIConstants::ISO_LEVEL_DOSE_VALUE_ID,levelPos->GetDoseValue());
      levelNode->PutFloat(mitk::RTUIConstants::ISO_LEVEL_COLOR_RED_ID,levelPos->GetColor().GetRed());
      levelNode->PutFloat(mitk::RTUIConstants::ISO_LEVEL_COLOR_GREEN_ID,levelPos->GetColor().GetGreen());
      levelNode->PutFloat(mitk::RTUIConstants::ISO_LEVEL_COLOR_BLUE_ID,levelPos->GetColor().GetBlue());
      levelNode->PutBool(mitk::RTUIConstants::ISO_LEVEL_VISIBILITY_ISOLINES_ID,levelPos->GetVisibleIsoLine());
      levelNode->PutBool(mitk::RTUIConstants::ISO_LEVEL_VISIBILITY_COLORWASH_ID,levelPos->GetVisibleColorWash());
    }
  }

  presetsNode->Flush();
}

bool mitk::GetGlobalIsolineVis()
{
  auto* prefService = mitk::CoreServices::GetPreferencesService();

  auto* glIsolineVisNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID);

  bool vis = glIsolineVisNode->GetBool(mitk::RTUIConstants::GLOBAL_VISIBILITY_ISOLINES_ID,false);

  return vis;
}

bool mitk::GetGlobalColorwashVis()
{
  auto* prefService = mitk::CoreServices::GetPreferencesService();

  auto* glDoseVisNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID);

  bool vis = glDoseVisNode->GetBool(mitk::RTUIConstants::GLOBAL_VISIBILITY_COLORWASH_ID,false);

  return vis;
}

mitk::PresetMapType mitk::LoadPresetsMap()
{
  auto* prefService = mitk::CoreServices::GetPreferencesService();

  auto* presetsNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_ISO_PRESETS_PREFERENCE_NODE_ID);

  auto names = presetsNode->ChildrenNames();

  PresetMapType presetMap;

  if(!names.empty()){
  for (const auto& name : names)
  {
    auto* aPresetNode = presetsNode->Node(name);

    if (aPresetNode == nullptr)
      mitkThrow()<< "Error in preference interface. Cannot find preset node under given name. Name: " << name;

    mitk::IsoDoseLevelSet::Pointer levelSet = mitk::IsoDoseLevelSet::New();

    auto levelNames = aPresetNode->ChildrenNames();

    for (const auto& levelName : levelNames)
    {
      auto* levelNode = aPresetNode->Node(levelName);

      if (aPresetNode == nullptr)
        mitkThrow()<< "Error in preference interface. Cannot find level node under given preset name. Name: " << name << "; Level id: " << levelName;

      mitk::IsoDoseLevel::Pointer isoLevel = mitk::IsoDoseLevel::New();

      isoLevel->SetDoseValue(levelNode->GetDouble(mitk::RTUIConstants::ISO_LEVEL_DOSE_VALUE_ID,0.0));
      mitk::IsoDoseLevel::ColorType color;
      color.SetRed(levelNode->GetFloat(mitk::RTUIConstants::ISO_LEVEL_COLOR_RED_ID,1.0));
      color.SetGreen(levelNode->GetFloat(mitk::RTUIConstants::ISO_LEVEL_COLOR_GREEN_ID,1.0));
      color.SetBlue(levelNode->GetFloat(mitk::RTUIConstants::ISO_LEVEL_COLOR_BLUE_ID,1.0));
      isoLevel->SetColor(color);
      isoLevel->SetVisibleIsoLine(levelNode->GetBool(mitk::RTUIConstants::ISO_LEVEL_VISIBILITY_ISOLINES_ID,true));
      isoLevel->SetVisibleColorWash(levelNode->GetBool(mitk::RTUIConstants::ISO_LEVEL_VISIBILITY_COLORWASH_ID,true));

      levelSet->SetIsoDoseLevel(isoLevel);
    }

    presetMap.insert(std::make_pair(name,levelSet));
  }
  }

  if (presetMap.size() == 0)
  { //if there are no presets use fallback and store it.
    presetMap = mitk::GenerateDefaultPresetsMap();
    StorePresetsMap(presetMap);
  }

  return presetMap;
}

mitk::PresetMapType mitk::GenerateDefaultPresetsMap()
{
  mitk::PresetMapType result;

  result.insert(std::make_pair(std::string("Virtuos"), mitk::GenerateIsoLevels_Virtuos()));
  return result;
}

std::string mitk::GetSelectedPresetName()
{
  auto* prefService = mitk::CoreServices::GetPreferencesService();
  auto* prefNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID);

  return prefNode->Get(mitk::RTUIConstants::SELECTED_ISO_PRESET_ID, "");
}

void mitk::SetSelectedPresetName(const std::string& presetName)
{
  auto* prefService = mitk::CoreServices::GetPreferencesService();
  auto* prefNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID);
  auto* presetsNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_ISO_PRESETS_PREFERENCE_NODE_ID);

  auto presetNames = presetsNode->ChildrenNames();

  auto finding = std::find(presetNames.begin(), presetNames.end(), presetName);

  if (finding == presetNames.end())
    mitkThrow() << "Error. Tried to set invalid selected preset name. Preset name does not exist in the defined presets. Preset name: " << presetName;

  prefNode->Put(mitk::RTUIConstants::SELECTED_ISO_PRESET_ID, presetName);
}

bool mitk::GetReferenceDoseValue(mitk::DoseValueAbs& value)
{
  auto* prefService = mitk::CoreServices::GetPreferencesService();
  auto* prefNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID);
  bool result = prefNode->GetBool(mitk::RTUIConstants::GLOBAL_REFERENCE_DOSE_SYNC_ID, true);

  value = prefNode->GetDouble(mitk::RTUIConstants::REFERENCE_DOSE_ID, mitk::RTUIConstants::DEFAULT_REFERENCE_DOSE_VALUE);

  return result;
}

void mitk::SetReferenceDoseValue(bool globalSync, mitk::DoseValueAbs value)
{
  auto* prefService = mitk::CoreServices::GetPreferencesService();
  auto* prefNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID);

  prefNode->PutBool(mitk::RTUIConstants::GLOBAL_REFERENCE_DOSE_SYNC_ID, globalSync);
  
  if (value >= 0)
    prefNode->PutDouble(mitk::RTUIConstants::REFERENCE_DOSE_ID, value);
}


bool mitk::GetDoseDisplayAbsolute()
{
  auto* prefService = mitk::CoreServices::GetPreferencesService();
  auto* prefNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID);

  return prefNode->GetBool(mitk::RTUIConstants::DOSE_DISPLAY_ABSOLUTE_ID, false);
}

void mitk::SetDoseDisplayAbsolute(bool isAbsolute)
{
  auto* prefService = mitk::CoreServices::GetPreferencesService();
  auto* prefNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID);

  prefNode->PutBool(mitk::RTUIConstants::DOSE_DISPLAY_ABSOLUTE_ID, isAbsolute);
}

void mitk::SignalReferenceDoseChange(bool globalSync, mitk::DoseValueAbs value, ctkPluginContext* context)
{
  ctkServiceReference ref = context->getServiceReference<ctkEventAdmin>();
  if (ref)
  {
    ctkEventAdmin* eventAdmin = context->getService<ctkEventAdmin>(ref);
    ctkProperties props;
    props["value"] = value;
    props["globalSync"] = globalSync;
    ctkEvent presetMapChangedEvent(mitk::RTCTKEventConstants::TOPIC_REFERENCE_DOSE_CHANGED.c_str());
    eventAdmin->sendEvent(presetMapChangedEvent);
  }
}

void mitk::SignalGlobalVisChange(bool globalSync, bool isolineVis, bool colorwashVis, ctkPluginContext *context)
{
  ctkServiceReference ref = context->getServiceReference<ctkEventAdmin>();
  if (ref)
  {
    ctkEventAdmin* eventAdmin = context->getService<ctkEventAdmin>(ref);
    ctkProperties props;
    props["globalIsolineVis"] = isolineVis;
    props["globalColorwashVis"] = colorwashVis;
    props["globalSync"] = globalSync;
    ctkEvent presetMapChangedEvent(mitk::RTCTKEventConstants::TOPIC_GLOBAL_VISIBILITY_CHANGED.c_str());
    eventAdmin->sendEvent(presetMapChangedEvent);
  }
}

void mitk::SignalPresetMapChange(ctkPluginContext* context)
{
  ctkServiceReference ref = context->getServiceReference<ctkEventAdmin>();
  if (ref)
  {
    ctkEventAdmin* eventAdmin = context->getService<ctkEventAdmin>(ref);
    ctkEvent presetMapChangedEvent(mitk::RTCTKEventConstants::TOPIC_ISO_DOSE_LEVEL_PRESETS_CHANGED.c_str());
    eventAdmin->sendEvent(presetMapChangedEvent);
  }
}
