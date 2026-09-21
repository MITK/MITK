/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkInteractionSchemeToolBar_h
#define QmitkInteractionSchemeToolBar_h

#include <MitkQtWidgetsExports.h>

// mitk core
#include <mitkInteractionSchemeSwitcher.h>

#include <QToolBar>

class QActionGroup;

/**
* \brief Toolbar widget to select the action of the left mouse button in PACS mode.
*
* The toolbar only requests a scheme via InteractionSchemeChanged(); applying
* it is up to the owner of the interaction event handler, which reports the
* result back through SetInteractionScheme(). This keeps the button states in
* sync with the scheme that is actually active, including when it is changed
* somewhere else.
*/
class MITKQTWIDGETS_EXPORT QmitkInteractionSchemeToolBar : public QToolBar
{
  Q_OBJECT

public:

  using InteractionScheme = mitk::InteractionSchemeSwitcher::InteractionScheme;

  QmitkInteractionSchemeToolBar(QWidget* parent = nullptr);
  ~QmitkInteractionSchemeToolBar() override;

public Q_SLOTS:

  /**
  * \brief Checks the button that belongs to the given scheme, without emitting
  *        InteractionSchemeChanged(). A scheme that no button represents,
  *        including any MITK scheme, leaves all buttons unchecked.
  */
  void SetInteractionScheme(mitk::InteractionSchemeSwitcher::InteractionScheme interactionScheme);

Q_SIGNALS:

  void InteractionSchemeChanged(mitk::InteractionSchemeSwitcher::InteractionScheme interactionScheme);

private:

  void AddButton(InteractionScheme interactionScheme, const QString& toolName, const QString& iconResource);
  void OnActionTriggered();

  QActionGroup* m_ActionGroup;

};

#endif
