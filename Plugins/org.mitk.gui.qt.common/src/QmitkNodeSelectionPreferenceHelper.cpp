/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkNodeSelectionPreferenceHelper.h"

#include <internal/QmitkNodeSelectionConstants.h>

#include <berryIPreferencesService.h>

#include <berryIPreferences.h>
#include <berryIPreferencesService.h>
#include <berryPlatform.h>


#include "mitkExceptionMacro.h"

void mitk::PutVisibleDataStorageInspectors(const VisibleDataStorageInspectorMapType &inspectors)
{
  berry::IPreferencesService *prefService = berry::Platform::GetPreferencesService();
  berry::IPreferences::Pointer prefNode =
    prefService->GetSystemPreferences()->Node(mitk::NodeSelectionConstants::ROOT_PREFERENCE_NODE_ID.c_str());
  berry::IPreferences::Pointer visNode =
    prefNode->Node(mitk::NodeSelectionConstants::VISIBLE_INSPECTORS_NODE_ID.c_str());

  visNode->RemoveNode();
  prefNode->Flush();

  // new empty preset node
  visNode = prefNode->Node(mitk::NodeSelectionConstants::VISIBLE_INSPECTORS_NODE_ID.c_str());

  // store map in new node
  for (const auto &inspector : inspectors)
  {
    std::ostringstream sstr;
    sstr << inspector.first;
    berry::IPreferences::Pointer aNode = visNode->Node(QString::fromStdString(sstr.str()));

    aNode->Put(mitk::NodeSelectionConstants::VISIBLE_INSPECTOR_ID.c_str(), inspector.second.c_str());
    aNode->Flush();
  }

  visNode->Flush();
}

mitk::VisibleDataStorageInspectorMapType mitk::GetVisibleDataStorageInspectors()
{
  berry::IPreferencesService *prefService = berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer prefNode =
    prefService->GetSystemPreferences()->Node(mitk::NodeSelectionConstants::ROOT_PREFERENCE_NODE_ID.c_str());
  berry::IPreferences::Pointer visNode =
    prefNode->Node(mitk::NodeSelectionConstants::VISIBLE_INSPECTORS_NODE_ID.c_str());

  typedef QStringList NamesType;
  NamesType names = visNode->ChildrenNames();

  VisibleDataStorageInspectorMapType visMap;

  if (!names.empty())
  {
    for (NamesType::const_iterator pos = names.begin(); pos != names.end(); ++pos)
    {
      berry::IPreferences::Pointer aNode = visNode->Node(*pos);

      if (aNode.IsNull())
      {
        mitkThrow() << "Error in preference interface. Cannot find preset node under given name. Name: "
                    << (*pos).toStdString();
      }

      std::istringstream isstr(pos->toStdString());
      unsigned int order = 0;
      isstr >> order;

      auto id = aNode->Get(mitk::NodeSelectionConstants::VISIBLE_INSPECTOR_ID.c_str(), "");
      if (id.isEmpty())
      {
        mitkThrow() << "Error in preference interface. ID of visible inspector is not set. Inspector position: "
                    << order;
      }

      visMap.insert(std::make_pair(order, id.toStdString()));
    }
  }
  return visMap;
}

mitk::DataStorageInspectorIDType mitk::GetPreferredDataStorageInspector()
{
  berry::IPreferencesService *prefService = berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer prefNode =
    prefService->GetSystemPreferences()->Node(mitk::NodeSelectionConstants::ROOT_PREFERENCE_NODE_ID.c_str());

  auto id = prefNode->Get(mitk::NodeSelectionConstants::PREFERRED_INSPECTOR_ID.c_str(), "");

  mitk::DataStorageInspectorIDType result = id.toStdString();
  return result;
}

void mitk::PutPreferredDataStorageInspector(const DataStorageInspectorIDType &id)
{
  berry::IPreferencesService *prefService = berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer prefNode =
    prefService->GetSystemPreferences()->Node(mitk::NodeSelectionConstants::ROOT_PREFERENCE_NODE_ID.c_str());

  prefNode->Put(mitk::NodeSelectionConstants::PREFERRED_INSPECTOR_ID.c_str(), id.c_str());
  prefNode->Flush();
}

void mitk::PutShowFavoritesInspector(bool show)
{
  berry::IPreferencesService *prefService = berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer prefNode =
    prefService->GetSystemPreferences()->Node(mitk::NodeSelectionConstants::ROOT_PREFERENCE_NODE_ID.c_str());

  prefNode->PutBool(mitk::NodeSelectionConstants::SHOW_FAVORITE_INSPECTOR.c_str(), show);
  prefNode->Flush();
}

bool mitk::GetShowFavoritesInspector()
{
  berry::IPreferencesService *prefService = berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer prefNode =
    prefService->GetSystemPreferences()->Node(mitk::NodeSelectionConstants::ROOT_PREFERENCE_NODE_ID.c_str());

  return prefNode->GetBool(mitk::NodeSelectionConstants::SHOW_FAVORITE_INSPECTOR.c_str(), true);
}

void mitk::PutShowHistoryInspector(bool show)
{
  berry::IPreferencesService *prefService = berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer prefNode =
    prefService->GetSystemPreferences()->Node(mitk::NodeSelectionConstants::ROOT_PREFERENCE_NODE_ID.c_str());

  prefNode->PutBool(mitk::NodeSelectionConstants::SHOW_HISTORY_INSPECTOR.c_str(), show);
  prefNode->Flush();
}

bool mitk::GetShowHistoryInspector()
{
  berry::IPreferencesService *prefService = berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer prefNode =
    prefService->GetSystemPreferences()->Node(mitk::NodeSelectionConstants::ROOT_PREFERENCE_NODE_ID.c_str());

  return prefNode->GetBool(mitk::NodeSelectionConstants::SHOW_HISTORY_INSPECTOR.c_str(), true);
}
