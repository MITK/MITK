/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkRegionGrowingToolGUI.h>

#include <QmitkInfoCard.h>

#include <QVBoxLayout>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkRegionGrowingToolGUI, "")

QmitkRegionGrowingToolGUI::QmitkRegionGrowingToolGUI()
{
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);

  // Qt reports the Command key as Ctrl on macOS, so that is the key to hold there.
#ifdef Q_OS_MACOS
  const QString cutModifier = QStringLiteral("Cmd");
#else
  const QString cutModifier = QStringLiteral("Ctrl");
#endif

  auto* infoCard = new QmitkInfoCard(this);
  infoCard->SetMessage(QStringLiteral(
    "<p><b>Segment:</b> Click into a structure, drag to its edge, release.</p>"
    "<p><b>Cut off a leak:</b> Hold %1 and draw a cutline across it.</p>"
    "<p><b>Cancel:</b> Press Esc or right-click before releasing the left mouse button.</p>").arg(cutModifier));
  layout->addWidget(infoCard);
}

QmitkRegionGrowingToolGUI::~QmitkRegionGrowingToolGUI()
{
}
