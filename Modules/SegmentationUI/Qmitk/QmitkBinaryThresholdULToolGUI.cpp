/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkBinaryThresholdULToolGUI.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <QApplication>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkBinaryThresholdULToolGUI, "")

QmitkBinaryThresholdULToolGUI::QmitkBinaryThresholdULToolGUI() : QmitkBinaryThresholdToolGUIBase(true)
{
}

QmitkBinaryThresholdULToolGUI::~QmitkBinaryThresholdULToolGUI()
{
}

