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
 * Provides buttons for opening the layout editor, toggling synchronized
 * scrolling, and switching the interaction scheme (e.g., MITK default vs.
 * PACS mode). The layout-shape controls themselves (grid, presets,
 * save/load) live in the layout editor view; the layout button only
 * requests it.
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

Q_SIGNALS:

  /**
   * \brief Emitted when the user presses the layout button; the hosting
   *        editor part shows/toggles the layout editor view.
   */
  void LayoutEditorRequested();

  /**
   * \brief Emitted when the synchronization state changes.
   * \param[in] synchronized True if synchronized scrolling is enabled.
   */
  void Synchronized(bool synchronized);

  /**
   * \brief Emitted when the interaction scheme changes.
   * \param[in] scheme The new interaction scheme.
   */
  void InteractionSchemeChanged(mitk::InteractionSchemeSwitcher::InteractionScheme scheme);

protected Q_SLOTS:

  void OnSynchronize();
  void OnInteractionSchemeChanged();

private:

  void AddButtons();

  QmitkAbstractMultiWidget* m_MultiWidget;

  QAction* m_SynchronizeAction;
  QAction* m_InteractionSchemeChangeAction;

};

#endif
