/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkNodeSelectionPreferenceHelper.h"

#include <QmitkNodeSelectionConstants.h>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

#include <QmitkDataStorageSelectionHistoryInspector.h>
#include <QmitkDataStorageFavoriteNodesInspector.h>

#include "mitkExceptionMacro.h"
#include "mitkDataStorageInspectorGenerator.h"

void mitk::PutVisibleDataStorageInspectors(const VisibleDataStorageInspectorMapType &inspectors)
{
  auto* prefService = mitk::CoreServices::GetPreferencesService();
  auto* prefNode = prefService->GetSystemPreferences()->Node(mitk::NodeSelectionConstants::ROOT_PREFERENCE_NODE_ID);
  auto* visNode = prefNode->Node(mitk::NodeSelectionConstants::VISIBLE_INSPECTORS_NODE_ID);

  visNode->Clear();

  // store map in new node
  for (const auto &inspector : inspectors)
  {
    std::ostringstream sstr;
    sstr << inspector.first;
    auto* aNode = visNode->Node(sstr.str());

    aNode->Put(mitk::NodeSelectionConstants::VISIBLE_INSPECTOR_ID, inspector.second);
  }

  visNode->Flush();
}

mitk::VisibleDataStorageInspectorMapType mitk::GetVisibleDataStorageInspectors()
{
  auto* prefService = mitk::CoreServices::GetPreferencesService();

  auto* prefNode = prefService->GetSystemPreferences()->Node(mitk::NodeSelectionConstants::ROOT_PREFERENCE_NODE_ID);
  auto* visNode = prefNode->Node(mitk::NodeSelectionConstants::VISIBLE_INSPECTORS_NODE_ID);

  using NamesType = std::vector<std::string>;
  NamesType names = visNode->ChildrenNames();

  VisibleDataStorageInspectorMapType visMap;

  if (!names.empty())
  {
    for (const auto& name : names)
    {
      auto* aNode = visNode->Node(name);

      if (aNode == nullptr)
        mitkThrow() << "Error in preference interface. Cannot find preset node under given name. Name: " << name;

      std::istringstream isstr(name);
      unsigned int order = 0;
      isstr >> order;

      auto id = aNode->Get(mitk::NodeSelectionConstants::VISIBLE_INSPECTOR_ID, "");

      if (id.empty())
        mitkThrow() << "Error in preference interface. ID of visible inspector is not set. Inspector position: " << order;

      visMap.insert(std::make_pair(order, id));
    }
  }

  if (visMap.empty())
  { //no visibility preferences set. Generate default
    auto allProviders = mitk::DataStorageInspectorGenerator::GetProviders();

    //fill inspector list
    unsigned int pos = 0;
    for (const auto& iter : allProviders)
    {
      if (iter.first != QmitkDataStorageFavoriteNodesInspector::INSPECTOR_ID() && iter.first != QmitkDataStorageSelectionHistoryInspector::INSPECTOR_ID())
      {
        visMap.insert(std::make_pair(pos, iter.first));
        ++pos;
      }
    }
  }

  return visMap;
}

mitk::DataStorageInspectorIDType mitk::GetPreferredDataStorageInspector()
{
  auto* prefService = mitk::CoreServices::GetPreferencesService();
  auto* prefNode = prefService->GetSystemPreferences()->Node(mitk::NodeSelectionConstants::ROOT_PREFERENCE_NODE_ID);
  auto id = prefNode->Get(mitk::NodeSelectionConstants::PREFERRED_INSPECTOR_ID, "");

  if (id.empty())
  { //nothing set, deduce default preferred inspector
    auto visibleInspectors = GetVisibleDataStorageInspectors();
    if (!visibleInspectors.empty())
      id = visibleInspectors.begin()->second;
  }

  return id;
}

void mitk::PutPreferredDataStorageInspector(const DataStorageInspectorIDType &id)
{
  auto* prefService = mitk::CoreServices::GetPreferencesService();
  auto* prefNode = prefService->GetSystemPreferences()->Node(mitk::NodeSelectionConstants::ROOT_PREFERENCE_NODE_ID);

  prefNode->Put(mitk::NodeSelectionConstants::PREFERRED_INSPECTOR_ID, id);
  prefNode->Flush();
}

void mitk::PutShowFavoritesInspector(bool show)
{
  auto* prefService = mitk::CoreServices::GetPreferencesService();
  auto* prefNode = prefService->GetSystemPreferences()->Node(mitk::NodeSelectionConstants::ROOT_PREFERENCE_NODE_ID);

  prefNode->PutBool(mitk::NodeSelectionConstants::SHOW_FAVORITE_INSPECTOR, show);
  prefNode->Flush();
}

bool mitk::GetShowFavoritesInspector()
{
  auto* prefService = mitk::CoreServices::GetPreferencesService();
  auto* prefNode = prefService->GetSystemPreferences()->Node(mitk::NodeSelectionConstants::ROOT_PREFERENCE_NODE_ID);

  return prefNode->GetBool(mitk::NodeSelectionConstants::SHOW_FAVORITE_INSPECTOR, true);
}

void mitk::PutShowHistoryInspector(bool show)
{
  auto* prefService = mitk::CoreServices::GetPreferencesService();
  auto* prefNode = prefService->GetSystemPreferences()->Node(mitk::NodeSelectionConstants::ROOT_PREFERENCE_NODE_ID);

  prefNode->PutBool(mitk::NodeSelectionConstants::SHOW_HISTORY_INSPECTOR, show);
  prefNode->Flush();
}

bool mitk::GetShowHistoryInspector()
{
  auto* prefService = mitk::CoreServices::GetPreferencesService();
  auto* prefNode = prefService->GetSystemPreferences()->Node(mitk::NodeSelectionConstants::ROOT_PREFERENCE_NODE_ID);

  return prefNode->GetBool(mitk::NodeSelectionConstants::SHOW_HISTORY_INSPECTOR, true);
}
