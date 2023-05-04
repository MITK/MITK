/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMultiWidgetConfigurationToolBar_h
#define QmitkMultiWidgetConfigurationToolBar_h

#include "MitkQtWidgetsExports.h"

#include <mitkInteractionSchemeSwitcher.h>

#include <nlohmann/json.hpp>

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
  void SaveLayout(std::ostream* outStream);
  void LoadLayout(const nlohmann::json* jsonData);
  void Synchronized(bool synchronized);
  void InteractionSchemeChanged(mitk::InteractionSchemeSwitcher::InteractionScheme scheme);

protected Q_SLOTS:

  void OnSetLayout();
  void OnSynchronize();
  void OnInteractionSchemeChanged();

private:

  void InitializeToolBar();;
  void AddButtons();

  QmitkAbstractMultiWidget* m_MultiWidget;

  QAction* m_SynchronizeAction;
  QAction* m_InteractionSchemeChangeAction;

  QmitkMultiWidgetLayoutSelectionWidget* m_LayoutSelectionPopup;

};

#endif
