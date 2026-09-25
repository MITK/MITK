/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkWelcomeText.h"

namespace
{
  // A single paragraph fills the label, so Qt's default paragraph margins
  // would only add space around it.
  constexpr auto LINE_HEIGHT_STYLE = "style='line-height: 1.25; margin: 0'";
}

QString QmitkWelcomeTextWithLineHeight(QString html)
{
  // Qt's rich text does not inherit the line height, so list items need it, too.
  html.replace("<li>", QString("<li %1>").arg(LINE_HEIGHT_STYLE));
  return QString("<p %1>%2</p>").arg(LINE_HEIGHT_STYLE).arg(html);
}
