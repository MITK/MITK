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

void mitk::StorePresetsMap(const PresetMapType& presetMap)
{
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
  berry::IPreferences::Pointer doseVisNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID.c_str());

//  berry::IPreferences::Pointer presetsNode = doseVisNode->Node(mitk::RTUIConstants::ROOT_ISO_PRESETS_PREFERENCE_NODE_ID.c_str());
  berry::IPreferences::Pointer presetsNode = doseVisNode->Node("isoPresets");
  presetsNode->RemoveNode();
  doseVisNode->Flush();

  //new empty preset node
//  presetsNode = doseVisNode->Node(mitk::RTUIConstants::ROOT_ISO_PRESETS_PREFERENCE_NODE_ID.c_str());
  presetsNode = doseVisNode->Node("isoPresets");

  //store map in new node
  for (PresetMapType::const_iterator pos = presetMap.begin(); pos != presetMap.end(); ++pos)
  {
    berry::IPreferences::Pointer aPresetNode = presetsNode->Node(pos->first.c_str());

    unsigned int id = 0;

    for (mitk::IsoDoseLevelSet::ConstIterator levelPos = pos->second->Begin(); levelPos != pos->second->End(); ++levelPos, ++id )
    {
      std::ostringstream stream;
      stream << id;

      berry::IPreferences::Pointer levelNode = aPresetNode->Node(QString::fromStdString(stream.str()));

      levelNode->PutDouble(mitk::RTUIConstants::ISO_LEVEL_DOSE_VALUE_ID.c_str(),levelPos->GetDoseValue());
      levelNode->PutFloat(mitk::RTUIConstants::ISO_LEVEL_COLOR_RED_ID.c_str(),levelPos->GetColor().GetRed());
      levelNode->PutFloat(mitk::RTUIConstants::ISO_LEVEL_COLOR_GREEN_ID.c_str(),levelPos->GetColor().GetGreen());
      levelNode->PutFloat(mitk::RTUIConstants::ISO_LEVEL_COLOR_BLUE_ID.c_str(),levelPos->GetColor().GetBlue());
      levelNode->PutBool(mitk::RTUIConstants::ISO_LEVEL_VISIBILITY_ISOLINES_ID.c_str(),levelPos->GetVisibleIsoLine());
      levelNode->PutBool(mitk::RTUIConstants::ISO_LEVEL_VISIBILITY_COLORWASH_ID.c_str(),levelPos->GetVisibleColorWash());
      levelNode->Flush();
    }

    aPresetNode->Flush();
  }

  presetsNode->Flush();
}

bool mitk::GetGlobalIsolineVis()
{
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer glIsolineVisNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID.c_str());

  bool vis = glIsolineVisNode->GetBool(mitk::RTUIConstants::GLOBAL_VISIBILITY_ISOLINES_ID.c_str(),false);

  return vis;
}

bool mitk::GetGlobalColorwashVis()
{
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer glDoseVisNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID.c_str());

  bool vis = glDoseVisNode->GetBool(mitk::RTUIConstants::GLOBAL_VISIBILITY_COLORWASH_ID.c_str(),false);

  return vis;
}

mitk::PresetMapType mitk::LoadPresetsMap()
{
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer presetsNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_ISO_PRESETS_PREFERENCE_NODE_ID.c_str());

  typedef QStringList NamesType;
  NamesType names = presetsNode->ChildrenNames();

  PresetMapType presetMap;

  if(!names.empty()){
  for (NamesType::const_iterator pos = names.begin(); pos != names.end(); ++pos)
  {
    berry::IPreferences::Pointer aPresetNode = presetsNode->Node(*pos);

    if (aPresetNode.IsNull())
    {
      mitkThrow()<< "Error in preference interface. Cannot find preset node under given name. Name: "<<(*pos).toStdString();
    }

    mitk::IsoDoseLevelSet::Pointer levelSet = mitk::IsoDoseLevelSet::New();

    NamesType levelNames =  aPresetNode->ChildrenNames();
    for (NamesType::const_iterator levelName = levelNames.begin(); levelName != levelNames.end(); ++levelName)
    {
      berry::IPreferences::Pointer levelNode = aPresetNode->Node(*levelName);
      if (aPresetNode.IsNull())
      {
        mitkThrow()<< "Error in preference interface. Cannot find level node under given preset name. Name: "<<(*pos).toStdString()<<"; Level id: "<<(*levelName).toStdString();
      }

      mitk::IsoDoseLevel::Pointer isoLevel = mitk::IsoDoseLevel::New();

      isoLevel->SetDoseValue(levelNode->GetDouble(mitk::RTUIConstants::ISO_LEVEL_DOSE_VALUE_ID.c_str(),0.0));
      mitk::IsoDoseLevel::ColorType color;
      color.SetRed(levelNode->GetFloat(mitk::RTUIConstants::ISO_LEVEL_COLOR_RED_ID.c_str(),1.0));
      color.SetGreen(levelNode->GetFloat(mitk::RTUIConstants::ISO_LEVEL_COLOR_GREEN_ID.c_str(),1.0));
      color.SetBlue(levelNode->GetFloat(mitk::RTUIConstants::ISO_LEVEL_COLOR_BLUE_ID.c_str(),1.0));
      isoLevel->SetColor(color);
      isoLevel->SetVisibleIsoLine(levelNode->GetBool(mitk::RTUIConstants::ISO_LEVEL_VISIBILITY_ISOLINES_ID.c_str(),true));
      isoLevel->SetVisibleColorWash(levelNode->GetBool(mitk::RTUIConstants::ISO_LEVEL_VISIBILITY_COLORWASH_ID.c_str(),true));

      levelSet->SetIsoDoseLevel(isoLevel);
    }

    presetMap.insert(std::make_pair((*pos).toStdString(),levelSet));
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
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer prefNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID.c_str());

  QString result = prefNode->Get(mitk::RTUIConstants::SELECTED_ISO_PRESET_ID.c_str(), "");

  return result.toStdString();
}

void mitk::SetSelectedPresetName(const std::string& presetName)
{
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer prefNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID.c_str());
  berry::IPreferences::Pointer presetsNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_ISO_PRESETS_PREFERENCE_NODE_ID.c_str());

  typedef QStringList NamesType;
  typedef std::vector<std::string> StdNamesType;
  NamesType presetNames = presetsNode->ChildrenNames();

  StdNamesType stdPresetNames;

  for (NamesType::const_iterator pos = presetNames.begin(); pos != presetNames.end(); ++pos)
  {
    stdPresetNames.push_back((*pos).toStdString());
  }

  StdNamesType::iterator finding = std::find(stdPresetNames.begin(),stdPresetNames.end(),presetName);

  if (finding == stdPresetNames.end())
  {
    mitkThrow()<< "Error. Tried to set invalid selected preset name. Preset name does not exist in the defined presets. Preset name: "<<presetName;
  }

  prefNode->Put(mitk::RTUIConstants::SELECTED_ISO_PRESET_ID.c_str(),presetName.c_str());
}

bool mitk::GetReferenceDoseValue(mitk::DoseValueAbs& value)
{
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer prefNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID.c_str());

  bool result = prefNode->GetBool(mitk::RTUIConstants::GLOBAL_REFERENCE_DOSE_SYNC_ID.c_str(), true);
  value = prefNode->GetDouble(mitk::RTUIConstants::REFERENCE_DOSE_ID.c_str(), mitk::RTUIConstants::DEFAULT_REFERENCE_DOSE_VALUE);

  return result;
}

void mitk::SetReferenceDoseValue(bool globalSync, mitk::DoseValueAbs value)
{
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer prefNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID.c_str());

  prefNode->PutBool(mitk::RTUIConstants::GLOBAL_REFERENCE_DOSE_SYNC_ID.c_str(), globalSync);
  if (value >= 0)
  {
    prefNode->PutDouble(mitk::RTUIConstants::REFERENCE_DOSE_ID.c_str(), value);
  }
}


bool mitk::GetDoseDisplayAbsolute()
{
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer prefNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID.c_str());

  bool result = prefNode->GetBool(mitk::RTUIConstants::DOSE_DISPLAY_ABSOLUTE_ID.c_str(), false);

  return result;
}

void mitk::SetDoseDisplayAbsolute(bool isAbsolute)
{
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer prefNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID.c_str());

  prefNode->PutBool(mitk::RTUIConstants::DOSE_DISPLAY_ABSOLUTE_ID.c_str(), isAbsolute);
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
