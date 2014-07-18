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

#ifndef QmitkBaseTreeWidget_h
#define QmitkBaseTreeWidget_h

#include <QMap>
#include <QTreeWidget>

namespace sofa
{
  namespace core
  {
    namespace objectmodel
    {
      class Base;
    }
  }
}

class QmitkSceneTreeWidget;

class QmitkBaseTreeWidget : public QTreeWidget
{
  Q_OBJECT

public:
  typedef sofa::core::objectmodel::Base Base;

  explicit QmitkBaseTreeWidget(QWidget* parent = NULL);
  ~QmitkBaseTreeWidget();

  // QTreeWidget, QTreeView, and QAbstractItemView Interfaces /////////////////
  void clear();
  /////////////////////////////////////////////////////////////////////////////

public slots:
  void OnSelectedBaseChanged(Base* base);

private:
  void FillTreeWidget();

  Base* m_Base;
  QMap<QString, QTreeWidgetItem*> m_GroupItemMap;
};

#endif
