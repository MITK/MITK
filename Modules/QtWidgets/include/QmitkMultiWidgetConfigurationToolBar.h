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
#include <mitkDataStorage.h>

#include <nlohmann/json.hpp>

// qt
#include <QToolBar>

class QmitkAbstractMultiWidget;
class QmitkMultiWidgetLayoutSelectionWidget;

/**
 * \brief Toolbar for configuring a QmitkAbstractMultiWidget layout and interaction scheme.
 *
 * Provides buttons for setting the multi-widget layout (row/column grid),
 * toggling synchronized scrolling, and switching the interaction scheme
 * (e.g., MITK default vs. PACS mode). A popup widget allows custom layout
 * selection and data-based layout configuration.
 *
 * \sa QmitkAbstractMultiWidget
 * \sa QmitkMultiWidgetLayoutSelectionWidget
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

  /**
   * \brief Sets the data storage for data-based layout operations.
   * \param[in] dataStorage The data storage to use.
   */
  void SetDataStorage(mitk::DataStorage::Pointer dataStorage);

Q_SIGNALS:

  /**
   * \brief Emitted when a grid layout is selected.
   * \param[out] row    The number of rows.
   * \param[out] column The number of columns.
   */
  void LayoutSet(int row, int column);

  /**
   * \brief Emitted when the user requests to save the current layout.
   * \param[out] outStream The output stream to write the layout to.
   */
  void SaveLayout(std::ostream* outStream);

  /**
   * \brief Emitted when the user requests to load a layout from JSON.
   * \param[out] jsonData The JSON data to load from.
   */
  void LoadLayout(const nlohmann::json* jsonData);

  /**
   * \brief Emitted when a data-based layout is requested.
   * \param[out] nodes The selected data nodes for the layout.
   */
  void SetDataBasedLayout(const QList<mitk::DataNode::Pointer>& nodes);

  /**
   * \brief Emitted when the synchronization state changes.
   * \param[out] synchronized True if synchronized scrolling is enabled.
   */
  void Synchronized(bool synchronized);

  /**
   * \brief Emitted when the interaction scheme changes.
   * \param[out] scheme The new interaction scheme.
   */
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
