/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berrySourcePriorityNameMapping.h"

#include <Poco/Exception.h>


namespace berry
{

QHash<QString,int> SourcePriorityNameMapping::sourcePrioritiesByName;

SourcePriorityNameMapping::SourcePriorityNameMapping()
{
  // This class should not be instantiated.
}

const QString SourcePriorityNameMapping::LEGACY_LEGACY_NAME()
{
  static const QString val = "LEGACY";
  return val;
}

int SourcePriorityNameMapping::NO_SOURCE_PRIORITY()
{
  static int val = 0;
  return val;
}

SourcePriorityNameMapping::Initializer::Initializer()
{
  SourcePriorityNameMapping::AddMapping(ISources::ACTIVE_ACTION_SETS_NAME(),
      ISources::ACTIVE_ACTION_SETS());
  SourcePriorityNameMapping::AddMapping(ISources::ACTIVE_CONTEXT_NAME(), ISources::ACTIVE_CONTEXT());
  SourcePriorityNameMapping::AddMapping(ISources::ACTIVE_CURRENT_SELECTION_NAME(),
      ISources::ACTIVE_CURRENT_SELECTION());
  SourcePriorityNameMapping::AddMapping(ISources::ACTIVE_EDITOR_NAME(), ISources::ACTIVE_EDITOR());
  SourcePriorityNameMapping::AddMapping(ISources::ACTIVE_EDITOR_ID_NAME(), ISources::ACTIVE_EDITOR_ID());
  SourcePriorityNameMapping::AddMapping(ISources::ACTIVE_MENU_NAME(), ISources::ACTIVE_MENU());
  SourcePriorityNameMapping::AddMapping(ISources::ACTIVE_MENU_SELECTION_NAME(), ISources::ACTIVE_MENU());
  SourcePriorityNameMapping::AddMapping(ISources::ACTIVE_MENU_EDITOR_INPUT_NAME(),
      ISources::ACTIVE_MENU());
  SourcePriorityNameMapping::AddMapping(ISources::ACTIVE_FOCUS_CONTROL_ID_NAME(),
      ISources::ACTIVE_MENU());
  SourcePriorityNameMapping::AddMapping(ISources::ACTIVE_FOCUS_CONTROL_NAME(), ISources::ACTIVE_MENU());
  SourcePriorityNameMapping::AddMapping(ISources::ACTIVE_PART_NAME(), ISources::ACTIVE_PART());
  SourcePriorityNameMapping::AddMapping(ISources::ACTIVE_PART_ID_NAME(), ISources::ACTIVE_PART_ID());
  SourcePriorityNameMapping::AddMapping(ISources::ACTIVE_SHELL_NAME(), ISources::ACTIVE_SHELL());
  SourcePriorityNameMapping::AddMapping(ISources::ACTIVE_SITE_NAME(), ISources::ACTIVE_SITE());
  SourcePriorityNameMapping::AddMapping(ISources::ACTIVE_WORKBENCH_WINDOW_NAME(),
      ISources::ACTIVE_WORKBENCH_WINDOW());
  SourcePriorityNameMapping::AddMapping(ISources::ACTIVE_WORKBENCH_WINDOW_SHELL_NAME(),
      ISources::ACTIVE_WORKBENCH_WINDOW_SHELL());
  SourcePriorityNameMapping::AddMapping(
      ISources::ACTIVE_WORKBENCH_WINDOW_IS_TOOLBAR_VISIBLE_NAME(),
      ISources::ACTIVE_WORKBENCH_WINDOW_SUBORDINATE());
  SourcePriorityNameMapping::AddMapping(
      ISources::ACTIVE_WORKBENCH_WINDOW_ACTIVE_PERSPECTIVE_NAME(),
      ISources::ACTIVE_WORKBENCH_WINDOW_SUBORDINATE());
  SourcePriorityNameMapping::AddMapping(
      ISources::ACTIVE_WORKBENCH_WINDOW_IS_PERSPECTIVEBAR_VISIBLE_NAME(),
      ISources::ACTIVE_WORKBENCH_WINDOW_SUBORDINATE());
  SourcePriorityNameMapping::AddMapping("workbench", ISources::WORKBENCH());
}

SourcePriorityNameMapping::Initializer SourcePriorityNameMapping::initializer;

void SourcePriorityNameMapping::AddMapping(const QString& sourceName,
    int sourcePriority)
{
  if (sourceName == "")
  {
    throw Poco::InvalidArgumentException("The source name cannot be empty."); //$NON-NLS-1$
  }

  if (sourcePrioritiesByName.find(sourceName) == sourcePrioritiesByName.end())
  {
    sourcePrioritiesByName.insert(sourceName, sourcePriority);
  }
}

int SourcePriorityNameMapping::ComputeSourcePriority(
    Expression::ConstPointer expression)
{
  int sourcePriority = ISources::WORKBENCH();

  if (expression == 0)
  {
    return sourcePriority;
  }

  const ExpressionInfo* info = expression->ComputeExpressionInfo();

  // Add the default variable, if any.
  if (info->HasDefaultVariableAccess())
  {
    sourcePriority |= ISources::ACTIVE_CURRENT_SELECTION();
  }

  // Add all of the reference variables.
  QSet<QString> sourceNames = info->GetAccessedVariableNames();
  foreach (QString name, sourceNames)
  {
    sourcePriority |= GetMapping(name);
  }

  return sourcePriority;
}

int SourcePriorityNameMapping::GetMapping(const QString& sourceName)
{
  if (sourcePrioritiesByName.contains(sourceName))
  {
    return sourcePrioritiesByName[sourceName];
  }
  return NO_SOURCE_PRIORITY();
}

}
