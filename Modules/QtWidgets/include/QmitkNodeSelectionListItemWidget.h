/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkNodeSelectionListItemWidget_h
#define QmitkNodeSelectionListItemWidget_h

#include <MitkQtWidgetsExports.h>

#include <QWidget>

#include <mitkDataNode.h>
#include <memory>

namespace Ui { class QmitkNodeSelectionListItemWidget; }

/** \brief Widget representing a single data node selection item with a clear button.
 *
 * Displays the node's name and class icon, with an optional clear button
 * to remove the selection. Used as an item widget in node selection lists.
 *
 * \sa QmitkAbstractNodeSelectionWidget, QmitkSingleNodeSelectionWidget
 */
class MITKQTWIDGETS_EXPORT QmitkNodeSelectionListItemWidget : public QWidget
{
  Q_OBJECT

public:
  /** \brief Construct the list item widget.
   * \param[in] parent Parent widget.
   */
  explicit QmitkNodeSelectionListItemWidget(QWidget* parent = nullptr);
  ~QmitkNodeSelectionListItemWidget() override;

  /** \brief Get the currently displayed data node.
   * \return The selected data node, or \c nullptr if none.
   */
  const mitk::DataNode* GetSelectedNode() const;

public Q_SLOTS :
  /** \brief Set the data node to display.
   * \param[in] node The data node to show in this item.
   */
  virtual void SetSelectedNode(const mitk::DataNode* node);

  /** \brief Enable or disable the clear button.
   * \param[in] allowed If \c true, the clear button is shown.
   */
  virtual void SetClearAllowed(bool allowed);

signals:
  /** \brief Emitted when the user clicks the clear button.
   * \param[in] node The node being cleared.
   */
  void ClearSelection(const mitk::DataNode* node);

protected Q_SLOTS:
  void OnClearSelection();

protected:
  bool eventFilter(QObject *obj, QEvent *ev) override;

  std::unique_ptr<Ui::QmitkNodeSelectionListItemWidget> m_Controls;
};


#endif
