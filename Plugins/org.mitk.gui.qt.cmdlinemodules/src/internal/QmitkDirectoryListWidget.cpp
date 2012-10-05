/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
