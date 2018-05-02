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

mitk::DataStorageInspectorIDType mitk::GetFavoriteDataStorageInspector()
{
  berry::IPreferencesService *prefService = berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer prefNode =
    prefService->GetSystemPreferences()->Node(mitk::NodeSelectionConstants::ROOT_PREFERENCE_NODE_ID.c_str());

  auto id = prefNode->Get(mitk::NodeSelectionConstants::FAVORITE_INSPECTOR_ID.c_str(), "");

  mitk::DataStorageInspectorIDType result = id.toStdString();
  return result;
}

void mitk::PutFavoriteDataStorageInspector(const DataStorageInspectorIDType &id)
{
  berry::IPreferencesService *prefService = berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer prefNode =
    prefService->GetSystemPreferences()->Node(mitk::NodeSelectionConstants::ROOT_PREFERENCE_NODE_ID.c_str());

  prefNode->Put(mitk::NodeSelectionConstants::FAVORITE_INSPECTOR_ID.c_str(), id.c_str());
  prefNode->Flush();
}
