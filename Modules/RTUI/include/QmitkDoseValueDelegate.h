/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDoseValueDelegate_h
#define QmitkDoseValueDelegate_h


#include <QStyledItemDelegate>

#include <MitkRTUIExports.h>

/**
 * \class QmitkDoseValueDelegate
 * \brief Item delegate for rendering and editing dose values in a QTableView.
 *
 * This delegate renders dose values right-aligned and creates a QDoubleSpinBox editor
 * for editing. The delegate inspects Qt::UserRole+1 on the model to determine whether
 * the dose is displayed as absolute (Gy) or relative (%). The spin box suffix and step
 * size are adjusted accordingly.
 *
 * \sa QmitkIsoDoseLevelSetModel
 * \sa QmitkDoseColorDelegate
 * \sa QmitkDoseVisualStyleDelegate
 */
class MITKRTUI_EXPORT QmitkDoseValueDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  /**
   * \brief Constructor.
   * \param[in] parent Optional parent QObject.
   */
  explicit QmitkDoseValueDelegate(QObject *parent = nullptr);

  /**
   * \brief Renders the dose value text right-aligned in the cell.
   * \param[in] painter The QPainter to use for drawing.
   * \param[in] option Style options for the item.
   * \param[in] index The model index of the item to paint.
   */
  void paint(QPainter *painter, const QStyleOptionViewItem &option
    , const QModelIndex &index) const override;

  /**
   * \brief Creates a QDoubleSpinBox editor for editing dose values.
   *
   * The spin box is configured with appropriate suffix ("Gy" or "%"),
   * step size, and range [0, 9999] based on whether the dose is absolute or relative.
   *
   * \param[in] parent The parent widget for the editor.
   * \param[in] option Style options for the item.
   * \param[in] index The model index of the item to edit.
   * \return A QDoubleSpinBox for valid data, or a QLabel for invalid data.
   */
  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option
    , const QModelIndex &index) const override;

  /**
   * \brief Populates the editor with the current dose value from the model.
   * \param[in,out] editor The editor widget (QDoubleSpinBox) to populate.
   * \param[in] index The model index to read data from.
   */
  void setEditorData(QWidget *editor, const QModelIndex &index) const override;

  /**
   * \brief Commits the edited dose value from the spin box back to the model.
   * \param[in] editor The editor widget (QDoubleSpinBox) containing the new value.
   * \param[in,out] model The model to write the new value to.
   * \param[in] index The model index of the item being edited.
   */
  void setModelData(QWidget *editor, QAbstractItemModel* model, const QModelIndex &index) const override;
};

#endif
