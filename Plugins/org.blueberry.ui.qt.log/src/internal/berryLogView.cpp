/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryLogView.h"

#include "berryQtLogView.h"

#include <QHBoxLayout>

namespace berry {

LogView::LogView()
{

}

void LogView::CreateQtPartControl(QWidget* parent)
{
  auto   layout = new QHBoxLayout(parent);
  layout->setContentsMargins(0,0,0,0);
  auto   logView = new QtLogView(parent);
  layout->addWidget(logView);
}

void LogView::SetFocus()
{

}

}
