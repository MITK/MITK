/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


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
