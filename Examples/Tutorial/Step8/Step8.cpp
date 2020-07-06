/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "Step8.h"

#include "QmitkLevelWindowWidget.h"
#include "QmitkRenderWindow.h"
#include "QmitkStdMultiWidget.h"

#include "mitkRenderingManager.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

//##Documentation
//## @brief As Step6, but with QmitkStdMultiWidget as widget
Step8::Step8(int argc, char *argv[], QWidget *parent) : Step6(argc, argv, parent)
{
}

void Step8::SetupWidgets()
{
  //*************************************************************************
  // Part I: Create windows and pass the tree to it
  //*************************************************************************

  // Create toplevel widget with vertical layout
  QVBoxLayout *vlayout = new QVBoxLayout(this);
  vlayout->setMargin(0);
  vlayout->setSpacing(2);

  // Create viewParent widget with horizontal layout
  QWidget *viewParent = new QWidget(this);
  vlayout->addWidget(viewParent);
  QHBoxLayout *hlayout = new QHBoxLayout(viewParent);
  hlayout->setMargin(0);

  //*************************************************************************
  // Part Ia: create and initialize QmitkStdMultiWidget
  //*************************************************************************
  QmitkStdMultiWidget *multiWidget = new QmitkStdMultiWidget(viewParent);

  hlayout->addWidget(multiWidget);

  // Tell the multiWidget which DataStorage to render
  multiWidget->SetDataStorage(m_DataStorage);

  // Initialize the multiWidget with the render windows
  multiWidget->InitializeMultiWidget();

  // Add the displayed views to the DataStorage to see their positions in 2D and 3D
  multiWidget->AddPlanesToDataStorage();

  //*************************************************************************
  // Part Ib: create and initialize LevelWindowWidget
  //*************************************************************************
  QmitkLevelWindowWidget *levelWindowWidget = new QmitkLevelWindowWidget(viewParent);

  hlayout->addWidget(levelWindowWidget);

  // Tell the levelWindowWidget which DataStorage to access
  levelWindowWidget->SetDataStorage(m_DataStorage);

}
/**
 \example Step8.cpp
 */
