/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkWelcomePalette.h"

#include "QmitkMitkWorkbenchIntroPlugin.h"

#include <berryIQtStyleManager.h>

namespace
{
  // The accents match iconAccentColor of the respective theme style sheet.
  const QmitkWelcomePalette DARK_PALETTE = {
    QColor("#202021"), QColor("#3f3f46"), QColor("#2d2d30"),
    QColor("#f1f1f1"), QColor("#a8a8b0"), QColor("#d4821e")
  };

  const QmitkWelcomePalette LIGHT_PALETTE = {
    QColor("#f3f3f3"), QColor("#ffffff"), QColor("#ebebeb"),
    QColor("#1e1e1e"), QColor("#5f5f66"), QColor("#2067bd")
  };
}

QmitkWelcomePalette QmitkWelcomePalette::GetCurrent()
{
  const auto* styleManager = QmitkMitkWorkbenchIntroPlugin::GetDefault()->GetStyleManager();

  return styleManager != nullptr && styleManager->GetStyle().name == "Dark"
    ? DARK_PALETTE
    : LIGHT_PALETTE;
}
