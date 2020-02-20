/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryPolicy.h"

#include <berryPlatform.h>
#include <berryPlatformUI.h>

#include <QString>

namespace berry {

const bool Policy::DEFAULT = false;

bool Policy::DEBUG_UI_GLOBAL()
{
  static bool b = GetDebugOption("/debug");
  return b;
}

bool Policy::DEBUG_DRAG_DROP()
{
  static bool b = DEBUG_UI_GLOBAL() ? GetDebugOption("/trace/dragDrop") : DEFAULT;
  return b;
}

bool Policy::DEBUG_PERSPECTIVES()
{
  static bool b = DEBUG_UI_GLOBAL() ? GetDebugOption("/trace/perspectives") : DEFAULT;
  return b;
}

bool Policy::DEBUG_STALE_JOBS()
{
  static bool b = DEBUG_UI_GLOBAL() ? GetDebugOption("/debug/job.stale") : DEFAULT;
  return b;
}

bool Policy::DEBUG_SOURCES()
{
  static bool b = DEBUG_UI_GLOBAL() ? GetDebugOption("/trace/sources") : DEFAULT;
  return b;
}

bool Policy::DEBUG_KEY_BINDINGS()
{
  static bool b = DEBUG_UI_GLOBAL() ? GetDebugOption("/trace/keyBindings") : DEFAULT;
  return b;
}

bool Policy::DEBUG_KEY_BINDINGS_VERBOSE()
{
  static bool b = DEBUG_UI_GLOBAL() ? GetDebugOption("/trace/keyBindings.verbose") : DEFAULT;
  return  b;
}

bool Policy::DEBUG_COMMANDS()
{
  static bool b = DEBUG_UI_GLOBAL() ? GetDebugOption("/trace/commands") : DEFAULT;
  return b;
}

bool Policy::DEBUG_CONTEXTS()
{
  static bool b = DEBUG_UI_GLOBAL() ? GetDebugOption("/trace/contexts") : DEFAULT;
  return b;
}

bool Policy::DEBUG_CONTEXTS_PERFORMANCE()
{
  static bool b = DEBUG_UI_GLOBAL() ? GetDebugOption("/trace/contexts.performance") : DEFAULT;
  return b;
}

bool Policy::DEBUG_CONTEXTS_VERBOSE()
{
  static bool b = DEBUG_UI_GLOBAL() ? GetDebugOption("/trace/contexts.verbose") : DEFAULT;
  return b;
}

bool Policy::DEBUG_HANDLERS()
{
  static bool b = DEBUG_UI_GLOBAL() ? GetDebugOption("/trace/handlers") : DEFAULT;
  return b;
}

bool Policy::DEBUG_HANDLERS_PERFORMANCE()
{
  static bool b = DEBUG_UI_GLOBAL() ? GetDebugOption("/trace/handlers.performance") : DEFAULT;
  return b;
}

bool Policy::DEBUG_HANDLERS_VERBOSE()
{
  static bool b = DEBUG_UI_GLOBAL() ? GetDebugOption("/trace/handlers.verbose") : DEFAULT;
  return b;
}

bool Policy::DEBUG_OPERATIONS()
{
  static bool b = DEBUG_UI_GLOBAL() ? GetDebugOption("/trace/operations") : DEFAULT;
  return b;
}

bool Policy::DEBUG_OPERATIONS_VERBOSE()
{
  static bool b = DEBUG_UI_GLOBAL() ? GetDebugOption("/trace/operations.verbose") : DEFAULT;
  return b;
}

bool Policy::DEBUG_SHOW_ALL_JOBS()
{
  static bool b = DEBUG_UI_GLOBAL() ? GetDebugOption("/debug/showAllJobs") : DEFAULT;
  return b;
}

bool Policy::DEBUG_CONTRIBUTIONS()
{
  static bool b = DEBUG_UI_GLOBAL() ? GetDebugOption("/debug/contributions") : DEFAULT;
  return b;
}

QString Policy::DEBUG_HANDLERS_VERBOSE_COMMAND_ID()
{
  static QString s = Platform::GetDebugOption("/trace/handlers.verbose.commandId").toString();
  return s;
}

bool Policy::DEBUG_WORKING_SETS()
{
  static bool b = DEBUG_UI_GLOBAL() ? GetDebugOption("/debug/workingSets") : DEFAULT;
  return b;
}

bool Policy::GetDebugOption(const QString& option)
{
  return Platform::GetDebugOption(PlatformUI::PLUGIN_ID() + option).toBool();
}

}
