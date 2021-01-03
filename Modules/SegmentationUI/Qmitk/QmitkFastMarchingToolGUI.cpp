/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkFastMarchingToolGUI.h"

#include "QmitkNewSegmentationDialog.h"

#include "mitkBaseRenderer.h"
#include "mitkStepper.h"
#include <QApplication>
#include <QGroupBox>
#include <QMessageBox>
#include <ctkRangeWidget.h>
#include <ctkSliderWidget.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkFastMarchingToolGUI, "")

QmitkFastMarchingToolGUI::QmitkFastMarchingToolGUI() : QmitkFastMarchingToolGUIBase(true)
{}
