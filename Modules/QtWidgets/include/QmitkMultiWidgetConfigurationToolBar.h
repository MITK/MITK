/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMultiWidgetConfigurationToolBar_h
#define QmitkMultiWidgetConfigurationToolBar_h

#include <MitkQtWidgetsExports.h>

#include <mitkInteractionSchemeSwitcher.h>

// qt
#include <QToolBar>

class QmitkAbstractMultiWidget;

/**
 * \brief Toolbar for configuring a QmitkAbstractMultiWidget layout and interaction scheme.
 *
 * Provides a button for switching the interaction scheme (MITK default vs.
 * PACS mode). Opening the layout editor and managing synchronization now live
 * on the per-cell viewport furniture (the sync barcode) and the layout editor
 * view, so this toolbar no longer carries those buttons.
 *
 * \sa QmitkAbstractMultiWidget
 * \sa QmitkInteractionSchemeToolBar
 */
class MITKQTWIDGETS_EXPORT QmitkMultiWidgetConfigurationToolBar : public QToolBar
{
  Q_OBJECT

public:

  /**
   * \brief Constructs the toolbar for the given multi-widget.
   * \param[in] multiWidget The multi-widget to configure.
   */
  QmitkMultiWidgetConfigurationToolBar(QmitkAbstractMultiWidget* multiWidget);
  ~QmitkMultiWidgetConfigurationToolBar() override;

public Q_SLOTS:

  /**
   * \brief Updates the interaction mode button to the given scheme, without
   *        emitting InteractionSchemeChanged().
   * \param[in] scheme The interaction scheme that is currently active.
   */
  void SetInteractionScheme(mitk::InteractionSchemeSwitcher::InteractionScheme scheme);

Q_SIGNALS:

  /**
   * \brief Emitted when the interaction scheme changes.
   * \param[in] scheme The new interaction scheme.
   */
  void InteractionSchemeChanged(mitk::InteractionSchemeSwitcher::InteractionScheme scheme);

protected Q_SLOTS:

  void OnInteractionSchemeChanged();

private:

  void AddButtons();
  void UpdateInteractionSchemeAction(bool pacs);

  QmitkAbstractMultiWidget* m_MultiWidget;

  QAction* m_InteractionSchemeChangeAction;

};

#endif
