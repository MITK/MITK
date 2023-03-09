/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkInteractionSchemeToolBar_h
#define QmitkInteractionSchemeToolBar_h

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

protected Q_SLOTS:

  void AddButton(InteractionScheme id, const QString& toolName, const QIcon& icon, bool on = false);
  void OnInteractionSchemeChanged();

private:

  QActionGroup* m_ActionGroup;

  mitk::InteractionEventHandler::Pointer m_InteractionEventHandler;

};

#endif
