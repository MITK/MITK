/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKINTERACTIONSCHEMETOOLBAR_H
#define QMITKINTERACTIONSCHEMETOOLBAR_H

#include "MitkQtWidgetsExports.h"

// mitk core
#include "mitkInteractionSchemeSwitcher.h"

#include <QActionGroup>
#include <QToolBar>

/**
* @brief
*
*
*/
class MITKQTWIDGETS_EXPORT QmitkInteractionSchemeToolBar : public QToolBar
{
  Q_OBJECT

public:

  using InteractionScheme = mitk::InteractionSchemeSwitcher::InteractionScheme;

  QmitkInteractionSchemeToolBar(QWidget* parent = nullptr);
  ~QmitkInteractionSchemeToolBar() override;

  void SetInteractionEventHandler(mitk::InteractionEventHandler::Pointer interactionEventHandler);

protected slots:

  void OnInteractionSchemeChanged();
  void AddButton(InteractionScheme id, const QString& toolName, const QIcon& icon, bool on = false);

private:

  QActionGroup* m_ActionGroup;

  mitk::InteractionSchemeSwitcher::Pointer m_InteractionSchemeSwitcher;
  mitk::InteractionEventHandler::Pointer m_InteractionEventHandler;

};

#endif // QMITKINTERACTIONSCHEMETOOLBAR_H
