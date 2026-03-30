/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkInitialValuesTypeDelegate_h
#define QmitkInitialValuesTypeDelegate_h


#include <QStyledItemDelegate>

#include <MitkModelFitUIExports.h>


/**
 * \class QmitkInitialValuesTypeDelegate
 * \brief Item delegate for rendering and editing the type of an initial parameter value.
 *
 * This delegate provides a QComboBox editor allowing the user to switch between "scalar"
 * (type 0) and "image" (type 1) as the source type for a model parameter's initial value.
 * The type is encoded as an integer in the model data via the Qt::EditRole.
 *
 * \sa QmitkInitialValuesModel
 * \sa QmitkInitialValuesDelegate
 * \sa QmitkInitialValuesManagerWidget
 */
class MITKMODELFITUI_EXPORT QmitkInitialValuesTypeDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:

  /**
   * \brief Constructs a QmitkInitialValuesTypeDelegate.
   * \param[in] parent Optional parent QObject.
   */
  QmitkInitialValuesTypeDelegate(QObject* parent = nullptr);

  /**
   * \brief Creates a QComboBox editor with "scalar" and "image" options.
   *
   * If the model data for the index is invalid, returns a read-only QLabel instead.
   *
   * \param[in] parent The parent widget for the editor.
   * \param[in] option Style options for the item.
   * \param[in] index The model index of the item being edited.
   * \return The newly created editor widget.
   */
  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option
                        , const QModelIndex& index) const override;

  /**
   * \brief Sets the combo box selection to match the current model data.
   * \param[in] editor The editor widget (expected to be a QComboBox).
   * \param[in] index The model index of the item.
   */
  void setEditorData(QWidget* editor, const QModelIndex& index) const override;

  /**
   * \brief Transfers the selected type index from the combo box back to the model.
   * \param[in] editor The editor widget containing the selection.
   * \param[in,out] model The abstract item model to update.
   * \param[in] index The model index of the item.
   */
  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
};

#endif
