/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-09-07 13:46:44 +0200 (Di, 07 Sep 2010) $
Version:   $Revision: 25948 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
