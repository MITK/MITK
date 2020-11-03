/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "berryMenuUtil.h"

#include <QString>

namespace berry {

QString MenuUtil::WORKBENCH_MENU = "menu:org.blueberry.ui.workbench.menu";
QString MenuUtil::MAIN_MENU = "menu:org.blueberry.ui.main.menu";
QString MenuUtil::MAIN_TOOLBAR = "toolbar:org.blueberry.ui.main.toolbar";

QString MenuUtil::ANY_POPUP = "popup:org.blueberry.ui.popup.any";

QString MenuUtil::QUERY_BEFORE = "before";
QString MenuUtil::QUERY_AFTER = "after";
QString MenuUtil::QUERY_ENDOF = "endof";

QString MenuUtil::SHOW_IN_MENU_ID = "popup:org.blueberry.ui.menus.showInMenu";

QString MenuUtil::MenuUri(const QString& id)
{
  return QString("menu:") + id;
}

QString MenuUtil::MenuAddition(const QString& id, const QString& location,
                               const QString& refId)
{
  return MenuUri(id) + '?' + location + '=' + refId;
}

QString MenuUtil::MenuAddition(const QString& id)
{
  return MenuAddition(id, "after", "additions");
}

QString MenuUtil::ToolbarUri(const QString& id)
{
  return QString("toolbar:") + id;
}

QString MenuUtil::ToolbarAddition(const QString& id, const QString& location,
                                  const QString& refId)
{
  return ToolbarUri(id) + '?' + location + '=' + refId;
}

QString MenuUtil::ToolbarAddition(const QString& id)
{
  return ToolbarAddition(id, "after", "additions");
}

}
