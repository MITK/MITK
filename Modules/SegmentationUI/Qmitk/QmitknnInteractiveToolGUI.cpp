/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitknnInteractiveToolGUI.h"
#include <ui_QmitknnInteractiveToolGUI.h>

#include <QBoxLayout>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitknnInteractiveToolGUI, "")

QmitknnInteractiveToolGUI::QmitknnInteractiveToolGUI()
  : QmitkSegWithPreviewToolGUIBase(false),
    m_Ui(new Ui::QmitknnInteractiveToolGUI)
{
}

QmitknnInteractiveToolGUI::~QmitknnInteractiveToolGUI()
{
}

void QmitknnInteractiveToolGUI::InitializeUI(QBoxLayout* mainLayout)
{
  auto wrapperWidget = new QWidget(this);
  mainLayout->addWidget(wrapperWidget);
  m_Ui->setupUi(wrapperWidget);

  Superclass::InitializeUI(mainLayout);
}
