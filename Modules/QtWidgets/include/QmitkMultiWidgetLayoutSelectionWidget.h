/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKMULTIWIDGETLAYOUTSELECTIONWIDGET_H
#define QMITKMULTIWIDGETLAYOUTSELECTIONWIDGET_H

#include "MitkQtWidgetsExports.h"

#include "ui_QmitkMultiWidgetLayoutSelectionWidget.h"

// qt
#include "QWidget"

/**
* @brief 
*
*
*/
class MITKQTWIDGETS_EXPORT QmitkMultiWidgetLayoutSelectionWidget : public QWidget
{
  Q_OBJECT

public:

  QmitkMultiWidgetLayoutSelectionWidget(QWidget* parent = 0);

Q_SIGNALS:

  void LayoutSet(int row, int column);

private Q_SLOTS:

  void OnTableItemSelectionChanged();
  void OnSetLayoutButtonClicked();

private:

  void Init();


  Ui::QmitkMultiWidgetLayoutSelectionWidget ui;

};

#endif // QMITKMULTIWIDGETLAYOUTSELECTIONWIDGET_H
