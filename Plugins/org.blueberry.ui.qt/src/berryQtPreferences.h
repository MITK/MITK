/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYQTPREFERENCES_H_
#define BERRYQTPREFERENCES_H_

#include <org_blueberry_ui_qt_Export.h>

#include <string>

namespace berry {

struct BERRY_UI_QT QtPreferences
{
  static const QString QT_STYLES_NODE; // = "qtstyles";
  static const QString QT_STYLE_NAME; // = "stylename";
  static const QString QT_STYLE_SEARCHPATHS; // = "searchpaths";
  static const QString QT_FONT_NAME; // = "fontname";
  static const QString QT_FONT_SIZE; // = "fontsize";
  static const QString QT_SHOW_TOOLBAR_CATEGORY_NAMES; // = "show_toolbar_category_names";
};
}

#endif /* BERRYQTPREFERENCES_H_ */
