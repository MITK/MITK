/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkThemedNodeDescriptor.h>
#include <QmitkStyleManager.h>

#include <QApplication>

struct QmitkThemedNodeDescriptor::Impl
{
  QString PathToIcon;
  QString StyleSheet;
  QIcon Icon;
  bool Themed = false;
};

QmitkThemedNodeDescriptor::QmitkThemedNodeDescriptor(const QString &className, const QString &pathToIcon, mitk::NodePredicateBase *predicate, QObject *parent)
  : QmitkNodeDescriptor(className, QIcon(), predicate, parent),
    m_Impl(new Impl)
{
  m_Impl->PathToIcon = pathToIcon;
}

QmitkThemedNodeDescriptor::~QmitkThemedNodeDescriptor()
{
  delete m_Impl;
}

QIcon QmitkThemedNodeDescriptor::GetIcon(const mitk::DataNode *) const
{
  // An empty style sheet is a valid state to theme for, hence the extra flag
  // instead of comparing against a default constructed style sheet.
  const auto styleSheet = qApp->styleSheet();

  if (!m_Impl->Themed || styleSheet != m_Impl->StyleSheet)
  {
    m_Impl->Icon = QmitkStyleManager::ThemeIcon(m_Impl->PathToIcon);
    m_Impl->StyleSheet = styleSheet;
    m_Impl->Themed = true;
  }

  return m_Impl->Icon;
}
