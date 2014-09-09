/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#include "mitkDoseVisPreferenceHelper.h"
#include "mitkRTUIConstants.h"
#include "mitkIsoLevelsGenerator.h"


#include <berryIPreferencesService.h>

#include <berryIPreferences.h>

#include <ctkPluginContext.h>
#include <service/event/ctkEventAdmin.h>

void mitk::rt::StorePresetsMap(const PresetMapType& presetMap)
{
  berry::IPreferencesService::Pointer prefService =
    berry::Platform::GetServiceRegistry().GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  berry::IPreferences::Pointer doseVisNode = prefService->GetSystemPreferences()->Node(mitk::rt::UIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID);
  berry::IPreferences::Pointer presetsNode = doseVisNode->Node(mitk::rt::UIConstants::ROOT_ISO_PRESETS_PREFERENCE_NODE_ID);

  presetsNode->RemoveNode();
  doseVisNode->Flush();

  //new empty preset node
  presetsNode = doseVisNode->Node(mitk::rt::UIConstants::ROOT_ISO_PRESETS_PREFERENCE_NODE_ID);

  //store map in new node
  for (PresetMapType::const_iterator pos = presetMap.begin(); pos != presetMap.end(); ++pos)
  {
    berry::IPreferences::Pointer aPresetNode = presetsNode->Node(pos->first);

    unsigned int id = 0;

    for (mitk::IsoDoseLevelSet::ConstIterator levelPos = pos->second->Begin(); levelPos != pos->second->End(); ++levelPos, ++id )
    {
      std::ostringstream stream;
      stream << id;

      berry::IPreferences::Pointer levelNode = aPresetNode->Node(stream.str());

      levelNode->PutDouble(mitk::rt::UIConstants::ISO_LEVEL_DOSE_VALUE_ID,levelPos->GetDoseValue());
      levelNode->PutFloat(mitk::rt::UIConstants::ISO_LEVEL_COLOR_RED_ID,levelPos->GetColor().GetRed());
      levelNode->PutFloat(mitk::rt::UIConstants::ISO_LEVEL_COLOR_GREEN_ID,levelPos->GetColor().GetGreen());
      levelNode->PutFloat(mitk::rt::UIConstants::ISO_LEVEL_COLOR_BLUE_ID,levelPos->GetColor().GetBlue());
      levelNode->PutBool(mitk::rt::UIConstants::ISO_LEVEL_VISIBILITY_ISOLINES_ID,levelPos->GetVisibleIsoLine());
      levelNode->PutBool(mitk::rt::UIConstants::ISO_LEVEL_VISIBILITY_COLORWASH_ID,levelPos->GetVisibleColorWash());
      levelNode->Flush();
    }

    aPresetNode->Flush();
  }

  presetsNode->Flush();
}

mitk::rt::PresetMapType mitk::rt::LoadPresetsMap()
{
  berry::IPreferencesService::Pointer prefService =
    berry::Platform::GetServiceRegistry().GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  berry::IPreferences::Pointer presetsNode = prefService->GetSystemPreferences()->Node(mitk::rt::UIConstants::ROOT_ISO_PRESETS_PREFERENCE_NODE_ID);

  typedef std::vector<std::string> NamesType;
  NamesType names = presetsNode->ChildrenNames();

  PresetMapType presetMap;

  for (NamesType::const_iterator pos = names.begin(); pos != names.end(); ++pos)
  {
    berry::IPreferences::Pointer aPresetNode = presetsNode->Node(*pos);

    if (aPresetNode.IsNull())
    {
      mitkThrow()<< "Error in preference interface. Cannot find preset node under given name. Name: "<<*pos;
    }

    mitk::IsoDoseLevelSet::Pointer levelSet = mitk::IsoDoseLevelSet::New();

    NamesType levelNames =  aPresetNode->ChildrenNames();
    for (NamesType::const_iterator levelName = levelNames.begin(); levelName != levelNames.end(); ++levelName)
    {
      berry::IPreferences::Pointer levelNode = aPresetNode->Node(*levelName);
      if (aPresetNode.IsNull())
      {
        mitkThrow()<< "Error in preference interface. Cannot find level node under given preset name. Name: "<<*pos<<"; Level id: "<<*levelName;
      }

      mitk::IsoDoseLevel::Pointer isoLevel = mitk::IsoDoseLevel::New();

      isoLevel->SetDoseValue(levelNode->GetDouble(mitk::rt::UIConstants::ISO_LEVEL_DOSE_VALUE_ID,0.0));
      mitk::IsoDoseLevel::ColorType color;
      color.SetRed(levelNode->GetFloat(mitk::rt::UIConstants::ISO_LEVEL_COLOR_RED_ID,1.0));
      color.SetGreen(levelNode->GetFloat(mitk::rt::UIConstants::ISO_LEVEL_COLOR_GREEN_ID,1.0));
      color.SetBlue(levelNode->GetFloat(mitk::rt::UIConstants::ISO_LEVEL_COLOR_BLUE_ID,1.0));
      isoLevel->SetColor(color);
      isoLevel->SetVisibleIsoLine(levelNode->GetBool(mitk::rt::UIConstants::ISO_LEVEL_VISIBILITY_ISOLINES_ID,true));
      isoLevel->SetVisibleColorWash(levelNode->GetBool(mitk::rt::UIConstants::ISO_LEVEL_VISIBILITY_COLORWASH_ID,true));

      levelSet->SetIsoDoseLevel(isoLevel);
    }

    presetMap.insert(std::make_pair(*pos,levelSet));
  }

  if (presetMap.size() == 0)
  { //if there are no presets use fallback and store it.
    presetMap = mitk::rt::GenerateDefaultPresetsMap();
    StorePresetsMap(presetMap);
  }

  return presetMap;
}

mitk::rt::PresetMapType mitk::rt::GenerateDefaultPresetsMap()
{
  mitk::rt::PresetMapType result;

  result.insert(std::make_pair(std::string("Virtuos"), mitk::rt::GeneratIsoLevels_Virtuos()));
  return result;
}

std::string mitk::rt::GetSelectedPresetName()
{
  berry::IPreferencesService::Pointer prefService =
    berry::Platform::GetServiceRegistry().GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  berry::IPreferences::Pointer prefNode = prefService->GetSystemPreferences()->Node(mitk::rt::UIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID);

  std::string result = prefNode->Get(mitk::rt::UIConstants::SELECTED_ISO_PRESET_ID, "");

  return result;
}

void mitk::rt::SetSelectedPresetName(const std::string& presetName)
{
  berry::IPreferencesService::Pointer prefService =
    berry::Platform::GetServiceRegistry().GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  berry::IPreferences::Pointer prefNode = prefService->GetSystemPreferences()->Node(mitk::rt::UIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID);
  berry::IPreferences::Pointer presetsNode = prefService->GetSystemPreferences()->Node(mitk::rt::UIConstants::ROOT_ISO_PRESETS_PREFERENCE_NODE_ID);

  typedef std::vector<std::string> NamesType;
  NamesType presetNames = presetsNode->ChildrenNames();

  NamesType::iterator finding = std::find(presetNames.begin(),presetNames.end(),presetName);

  if (finding == presetNames.end())
  {
    mitkThrow()<< "Error. Tried to set invalid selected preset name. Preset name does not exist in the defined presets. Preset name: "<<presetName;
  }

  prefNode->Put(mitk::rt::UIConstants::SELECTED_ISO_PRESET_ID,presetName);
}

bool mitk::rt::GetReferenceDoseValue(mitk::DoseValueAbs& value)
{
  berry::IPreferencesService::Pointer prefService =
    berry::Platform::GetServiceRegistry().GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  berry::IPreferences::Pointer prefNode = prefService->GetSystemPreferences()->Node(mitk::rt::UIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID);

  bool result = prefNode->GetBool(mitk::rt::UIConstants::GLOBAL_REFERENCE_DOSE_SYNC_ID, true);
  value = prefNode->GetDouble(mitk::rt::UIConstants::REFERENCE_DOSE_ID, mitk::rt::UIConstants::DEFAULT_REFERENCE_DOSE_VALUE);

  return result;
}

void mitk::rt::SetReferenceDoseValue(bool globalSync, mitk::DoseValueAbs value)
{
  berry::IPreferencesService::Pointer prefService =
    berry::Platform::GetServiceRegistry().GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  berry::IPreferences::Pointer prefNode = prefService->GetSystemPreferences()->Node(mitk::rt::UIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID);

  prefNode->PutBool(mitk::rt::UIConstants::GLOBAL_REFERENCE_DOSE_SYNC_ID, globalSync);
  if (value >= 0)
  {
    prefNode->PutDouble(mitk::rt::UIConstants::REFERENCE_DOSE_ID, value);
  }
}


bool mitk::rt::GetDoseDisplayAbsolute()
{
  berry::IPreferencesService::Pointer prefService =
    berry::Platform::GetServiceRegistry().GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  berry::IPreferences::Pointer prefNode = prefService->GetSystemPreferences()->Node(mitk::rt::UIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID);

  bool result = prefNode->GetBool(mitk::rt::UIConstants::DOSE_DISPLAY_ABSOLUTE_ID, false);

  return result;
}

void mitk::rt::SetDoseDisplayAbsolute(bool isAbsolute)
{
  berry::IPreferencesService::Pointer prefService =
    berry::Platform::GetServiceRegistry().GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  berry::IPreferences::Pointer prefNode = prefService->GetSystemPreferences()->Node(mitk::rt::UIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID);

  prefNode->PutBool(mitk::rt::UIConstants::DOSE_DISPLAY_ABSOLUTE_ID, isAbsolute);
}

void mitk::rt::SignalReferenceDoseChange(bool globalSync, mitk::DoseValueAbs value, ctkPluginContext* context)
{
  ctkServiceReference ref = context->getServiceReference<ctkEventAdmin>();
  if (ref)
  {
    ctkEventAdmin* eventAdmin = context->getService<ctkEventAdmin>(ref);
    ctkProperties props;
    props["value"] = value;
    props["globalSync"] = globalSync;
    ctkEvent presetMapChangedEvent(mitk::rt::CTKEventConstants::TOPIC_REFERENCE_DOSE_CHANGED.c_str());
    eventAdmin->sendEvent(presetMapChangedEvent);
  }
}

void mitk::rt::SignalPresetMapChange(ctkPluginContext* context)
{
  ctkServiceReference ref = context->getServiceReference<ctkEventAdmin>();
  if (ref)
  {
    ctkEventAdmin* eventAdmin = context->getService<ctkEventAdmin>(ref);
    ctkEvent presetMapChangedEvent(mitk::rt::CTKEventConstants::TOPIC_ISO_DOSE_LEVEL_PRESETS_CHANGED.c_str());
    eventAdmin->sendEvent(presetMapChangedEvent);
  }
}
