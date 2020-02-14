/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDirectoryListWidget.h"

#include <ctkPathListWidget.h>
#include <ctkPathListButtonsWidget.h>

//-----------------------------------------------------------------------------
QmitkDirectoryListWidget::QmitkDirectoryListWidget(QWidget*)
{
  this->setupUi(this);
  this->m_PathListWidget->setMode(ctkPathListWidget::DirectoriesOnly);
  this->m_PathListWidget->setDirectoryOptions(ctkPathListWidget::Exists | ctkPathListWidget::Readable | ctkPathListWidget::Executable);
  this->m_PathListButtonsWidget->init(this->m_PathListWidget);
  this->m_PathListButtonsWidget->setOrientation(Qt::Vertical);
  connect(this->m_PathListWidget, SIGNAL(pathsChanged(QStringList,QStringList)), this, SLOT(OnPathsChanged(QStringList, QStringList)));
}


//-----------------------------------------------------------------------------
void QmitkDirectoryListWidget::OnPathsChanged(const QStringList& before, const QStringList& after)
{
  emit pathsChanged(before, after);
}


//-----------------------------------------------------------------------------
QStringList QmitkDirectoryListWidget::directories(bool absolutePath) const
{
  return this->m_PathListWidget->directories(absolutePath);
}


//-----------------------------------------------------------------------------
void QmitkDirectoryListWidget::setDirectories(const QStringList& paths)
{
  this->m_PathListWidget->setPaths(paths);
}
