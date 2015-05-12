/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkCTKWidgetsWidget.h"
#include <ctkDoubleSlider.h>
#include <ctkDoubleRangeSlider.h>
#include <ctkRangeWidget.h>
#include <ctkMenuButton.h>
#include <QLabel>
#include <QMenu>
#include <QVBoxLayout>

QmitkCTKWidgetsWidget::QmitkCTKWidgetsWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent)
  : QmitkSegmentationUtilityWidget(timeNavigationController, parent)
{
  auto   layout = new QVBoxLayout(this);
  this->setLayout(layout);

  layout->addWidget(new QLabel("ctkDoubleSlider", this));
  layout->addWidget(new ctkDoubleSlider(Qt::Horizontal, this));

  layout->addWidget(new QLabel("ctkDoubleRangeSlider", this));
  layout->addWidget(new ctkDoubleRangeSlider(Qt::Horizontal, this));

  layout->addWidget(new QLabel("ctkRangeWidget", this));
  layout->addWidget(new ctkRangeWidget(this));

  QIcon icon(":/SegmentationUtilities/CTKWidgets_48x48.png");

  auto   menu = new QMenu(this);
  menu->addAction(icon, "Axial");
  menu->addAction(icon, "Sagittal");
  menu->addAction(icon, "Coronal");

  ctkMenuButton* menuButton = new ctkMenuButton("Apply", this);
  menuButton->setMenu(menu);
  layout->addWidget(new QLabel("ctkMenuButton", this));
  layout->addWidget(menuButton);

  layout->addStretch();
}

QmitkCTKWidgetsWidget::~QmitkCTKWidgetsWidget()
{
}
