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


#ifndef BERRYQTPREFERENCES_H_
#define BERRYQTPREFERENCES_H_

#include <org_blueberry_ui_qt_Export.h>

#include <string>

namespace berry {

struct BERRY_UI_QT QtPreferences
{
  static const std::string QT_STYLES_NODE; // = "qtstyles";
  static const std::string QT_STYLE_NAME; // = "stylename";
  static const std::string QT_STYLE_SEARCHPATHS; // = "searchpaths";
  static const std::string QT_ICON_THEME; // = "icontheme";
};
}

#endif /* BERRYQTPREFERENCES_H_ */
