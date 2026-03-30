/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkNodeDetailsDialog_h
#define QmitkNodeDetailsDialog_h

#include <mitkDataNode.h>
#include <MitkQtWidgetsExports.h>

#include <QDialog>
#include <memory>

namespace Ui
{
  class QmitkNodeDetailsDialog;
}

/** \brief Dialog that displays detailed property information for data nodes.
 *
 * Shows a searchable tree view of all properties for one or more selected
 * data nodes, with the ability to filter properties by keyword.
 *
 * \sa mitk::DataNode, QmitkAbstractNodeSelectionWidget
 */
class MITKQTWIDGETS_EXPORT QmitkNodeDetailsDialog : public QDialog
{
  Q_OBJECT

public:
  /** \brief Construct with a list of const data nodes.
   * \param[in] nodes The data nodes to display details for.
   * \param[in] parent Parent widget.
   * \param[in] flags Window flags.
   */
  QmitkNodeDetailsDialog(const QList<mitk::DataNode::ConstPointer>& nodes, QWidget* parent = nullptr, Qt::WindowFlags flags = {});

  /** \brief Construct with a list of mutable data nodes.
   * \param[in] nodes The data nodes to display details for.
   * \param[in] parent Parent widget.
   * \param[in] flags Window flags.
   */
  QmitkNodeDetailsDialog(const QList<mitk::DataNode::Pointer>& nodes, QWidget* parent = nullptr, Qt::WindowFlags flags = {});
  ~QmitkNodeDetailsDialog() override;

  /** \brief Update the displayed properties when the selected node changes. */
  void OnSelectionChanged(const mitk::DataNode*);

  /** \brief Handle the search button click to filter properties. */
  void OnSearchButtonClicked();

  /** \brief Filter displayed properties by keyword text.
   * \param[in] text The search keyword.
   */
  void KeywordTextChanged(const QString& text);

private:
  bool eventFilter(QObject* obj, QEvent* event) override;
  void InitWidgets(const QList<mitk::DataNode::ConstPointer>& nodes);

  std::unique_ptr<Ui::QmitkNodeDetailsDialog> m_Ui;
};

#endif
