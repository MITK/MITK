/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkFileListWidget.h"

#include <ctkPathListWidget.h>
#include <ctkPathListButtonsWidget.h>

#include <ui_QmitkPathListWidget.h>

QmitkFileListWidget::QmitkFileListWidget(QWidget*)
  : m_Controls(std::make_unique<Ui::QmitkPathListWidget>())
{
  m_Controls->setupUi(this);
  m_Controls->m_PathListWidget->setMode(ctkPathListWidget::FilesOnly);
  m_Controls->m_PathListWidget->setFileOptions(ctkPathListWidget::Exists | ctkPathListWidget::Readable | ctkPathListWidget::Executable);
  m_Controls->m_PathListButtonsWidget->init(m_Controls->m_PathListWidget);
  m_Controls->m_PathListButtonsWidget->setOrientation(Qt::Vertical);
  connect(m_Controls->m_PathListWidget, SIGNAL(pathsChanged(QStringList,QStringList)), this, SLOT(OnPathsChanged(QStringList, QStringList)));
}

QmitkFileListWidget::~QmitkFileListWidget()
{
}

void QmitkFileListWidget::OnPathsChanged(const QStringList& before, const QStringList& after)
{
  emit pathsChanged(before, after);
}

QStringList QmitkFileListWidget::files(bool absolutePath) const
{
  return m_Controls->m_PathListWidget->files(absolutePath);
}

void QmitkFileListWidget::setFiles(const QStringList& paths)
{
  m_Controls->m_PathListWidget->setPaths(paths);
}

void QmitkFileListWidget::setText(const QString& text)
{
  m_Controls->m_Label->setText(text);
}
