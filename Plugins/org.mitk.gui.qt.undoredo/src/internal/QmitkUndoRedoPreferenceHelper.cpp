/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkUndoRedoPreferenceHelper.h"

#include <mitkCoreServices.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>
#include <mitkUndoController.h>

#include <string>

namespace
{
  const std::string NODE_PATH = "/General/UndoRedo";
  const std::string UNDO_LIMIT_KEY = "UndoLimit";
  const std::string LAST_UNDO_LIMIT_KEY = "LastUndoLimit";
}

mitk::IPreferences* QmitkUndoRedoPreferences::GetPreferences()
{
  auto* preferencesService = mitk::CoreServices::GetPreferencesService();
  auto* systemPref = preferencesService->GetSystemPreferences();
  return nullptr != systemPref ? systemPref->Node(NODE_PATH) : nullptr;
}

int QmitkUndoRedoPreferences::GetActiveLimit()
{
  auto* prefs = GetPreferences();
  return prefs != nullptr
    ? prefs->GetInt(UNDO_LIMIT_KEY, static_cast<int>(mitk::DEFAULT_UNDO_REDO_LIMIT))
    : static_cast<int>(mitk::DEFAULT_UNDO_REDO_LIMIT);
}

int QmitkUndoRedoPreferences::GetLastPositiveLimit()
{
  auto* prefs = GetPreferences();
  return prefs != nullptr
    ? prefs->GetInt(LAST_UNDO_LIMIT_KEY, static_cast<int>(mitk::DEFAULT_UNDO_REDO_LIMIT))
    : static_cast<int>(mitk::DEFAULT_UNDO_REDO_LIMIT);
}

void QmitkUndoRedoPreferences::StoreLimit(int limit)
{
  auto* prefs = GetPreferences();
  if (prefs == nullptr)
    return;

  prefs->PutInt(UNDO_LIMIT_KEY, limit);

  if (limit > 0)
    prefs->PutInt(LAST_UNDO_LIMIT_KEY, limit);
}
