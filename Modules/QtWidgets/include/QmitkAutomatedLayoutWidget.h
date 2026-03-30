/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

/**
 * \file QmitkAutomatedLayoutWidget.h
 * \brief Declares QmitkAutomatedLayoutWidget, a widget for selecting data nodes and
 *        triggering an automated render window layout based on the selection.
 *
 * \ingroup QmitkModule
 */

#ifndef QmitkAutomatedLayoutWidget_h
#define QmitkAutomatedLayoutWidget_h

#include <MitkQtWidgetsExports.h>

#include <mitkDataStorage.h>

#include <QWidget>
#include <memory>

namespace Ui
{
  class QmitkAutomatedLayoutWidget;
}

/**
 * \brief Widget that lets users select image nodes and request an automated render window layout.
 *
 * Provides a node selection dialog (filtered to visible, non-helper image nodes) and a
 * "Set Layout" button. When the user confirms the selection, the SetDataBasedLayout
 * signal is emitted with the chosen nodes so that the application can arrange render
 * windows accordingly.
 *
 * \ingroup QmitkModule
 */
class MITKQTWIDGETS_EXPORT QmitkAutomatedLayoutWidget : public QWidget
{
  Q_OBJECT

private Q_SLOTS:
  /** \brief Handle the "Set Layout" button click: emit the signal and hide the widget. */
  void OnSetLayoutClicked();

  /** \brief Handle the selection dialog being closed: enable/disable the layout button. */
  void OnSelectionDialogClosed();

Q_SIGNALS:
  /**
   * \brief Emitted when the user confirms a node selection for automated layout.
   *
   * \param nodes The list of selected data nodes to arrange in render windows.
   */
  void SetDataBasedLayout(const QList<mitk::DataNode::Pointer>& nodes);

public:
  /**
   * \brief Construct the automated layout widget.
   *
   * \param parent The parent widget (optional).
   */
  explicit QmitkAutomatedLayoutWidget(QWidget* parent = nullptr);

  /** \brief Destructor. */
  ~QmitkAutomatedLayoutWidget() override;

  /**
   * \brief Set the data storage from which image nodes can be selected.
   *
   * \param dataStorage Pointer to the data storage to use.
   */
  void SetDataStorage(mitk::DataStorage::Pointer dataStorage);

private:

  std::unique_ptr<Ui::QmitkAutomatedLayoutWidget> m_Controls; ///< UI controls created from the .ui file.
  mitk::DataStorage::Pointer m_DataStorage; ///< Holds a reference to the data storage.
};


#endif
