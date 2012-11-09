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

#ifndef QMITKPYTHONCOMMANDHISTORYTREEWIDGET_H_
#define QMITKPYTHONCOMMANDHISTORYTREEWIDGET_H_

#include <QTreeWidget>

class QmitkPythonCommandHistoryTreeWidget : public QTreeWidget
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  QmitkPythonCommandHistoryTreeWidget(QWidget *parent = 0);
  virtual ~QmitkPythonCommandHistoryTreeWidget();

signals:

protected slots:

protected:
  virtual void mouseMoveEvent(QMouseEvent *event);

private:
};

#endif
