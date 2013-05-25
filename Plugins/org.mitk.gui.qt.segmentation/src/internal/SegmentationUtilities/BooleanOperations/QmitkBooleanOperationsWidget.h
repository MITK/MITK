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

#ifndef QmitkBooleanOperationsWidget_h
#define QmitkBooleanOperationsWidget_h

#include <ui_QmitkBooleanOperationsWidgetControls.h>

class QmitkBooleanOperationsWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkBooleanOperationsWidget(QWidget* parent = NULL);
  ~QmitkBooleanOperationsWidget();

private slots:
  void OnSelectionChanged(unsigned int id, const mitk::DataNode* selection);

private:
  Ui::QmitkBooleanOperationsWidgetControls m_Controls;

  unsigned int m_1stID;
  unsigned int m_2ndID;
};

#endif
