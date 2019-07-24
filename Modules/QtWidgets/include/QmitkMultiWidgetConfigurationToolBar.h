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

#ifndef QMITKMULTIWIDGETCONFIGURATIONTOOLBAR_H
#define QMITKMULTIWIDGETCONFIGURATIONTOOLBAR_H

#include "MitkQtWidgetsExports.h"

// qt
#include <QToolBar>

class QmitkAbstractMultiWidget;
class QmitkMultiWidgetLayoutSelectionWidget;

/**
* @brief 
*
*
*/
class MITKQTWIDGETS_EXPORT QmitkMultiWidgetConfigurationToolBar : public QToolBar
{
  Q_OBJECT

public:

  QmitkMultiWidgetConfigurationToolBar(QmitkAbstractMultiWidget* multiWidget);
  ~QmitkMultiWidgetConfigurationToolBar() override;

Q_SIGNALS:

  void LayoutSet(int row, int column);
  void Synchronized(bool synchronized);

protected Q_SLOTS:

  void OnSetLayout();
  void OnSynchronize();

private:

  void InitializeToolBar();;
  void AddButtons();

  QmitkAbstractMultiWidget* m_MultiWidget;

  QAction* m_SynchronizeAction;

  QmitkMultiWidgetLayoutSelectionWidget* m_LayoutSelectionPopup;

};

#endif // QMITKMULTIWIDGETCONFIGURATIONTOOLBAR_H
