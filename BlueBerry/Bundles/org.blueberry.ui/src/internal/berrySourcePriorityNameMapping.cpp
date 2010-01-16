/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "berrySourcePriorityNameMapping.h"

#include <Poco/Exception.h>


namespace berry
{

std::map<std::string,int> SourcePriorityNameMapping::sourcePrioritiesByName;

SourcePriorityNameMapping::SourcePriorityNameMapping()
{
  // This class should not be instantiated.
}

const std::string SourcePriorityNameMapping::LEGACY_LEGACY_NAME()
{
  static const std::string val = "LEGACY";
  return val;
}

const int SourcePriorityNameMapping::NO_SOURCE_PRIORITY()
{
  static const int val = 0;
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
      ISources::ACTIVE_WORKBENCH_WINDOW_IS_COOLBAR_VISIBLE_NAME(),
      ISources::ACTIVE_WORKBENCH_WINDOW_SUBORDINATE());
  SourcePriorityNameMapping::AddMapping(
      ISources::ACTIVE_WORKBENCH_WINDOW_ACTIVE_PERSPECTIVE_NAME(),
      ISources::ACTIVE_WORKBENCH_WINDOW_SUBORDINATE());
  SourcePriorityNameMapping::AddMapping(
      ISources::ACTIVE_WORKBENCH_WINDOW_IS_PERSPECTIVEBAR_VISIBLE_NAME(),
      ISources::ACTIVE_WORKBENCH_WINDOW_SUBORDINATE());
  SourcePriorityNameMapping::AddMapping(LEGACY_LEGACY_NAME(), LEGACY_LEGACY());
  SourcePriorityNameMapping::AddMapping("workbench", ISources::WORKBENCH());
}

SourcePriorityNameMapping::Initializer SourcePriorityNameMapping::initializer;

void SourcePriorityNameMapping::AddMapping(const std::string& sourceName,
    int sourcePriority)
{
  if (sourceName == "")
  {
    throw Poco::InvalidArgumentException("The source name cannot be empty."); //$NON-NLS-1$
  }

  if (sourcePrioritiesByName.find(sourceName) == sourcePrioritiesByName.end())
  {
    sourcePrioritiesByName.insert(std::make_pair(sourceName, sourcePriority));
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
  std::set<std::string> sourceNames = info->GetAccessedVariableNames();
  for (std::set<std::string>::iterator iter = sourceNames.begin();
       iter != sourceNames.end(); ++iter)
  {
    sourcePriority |= GetMapping(*iter);
  }

  return sourcePriority;
}

int SourcePriorityNameMapping::GetMapping(const std::string& sourceName)
{
  std::map<std::string, int>::iterator mapping = sourcePrioritiesByName.find(
      sourceName);
  if (mapping != sourcePrioritiesByName.end())
  {
    return mapping->second;
  }

  return NO_SOURCE_PRIORITY();
}

}
