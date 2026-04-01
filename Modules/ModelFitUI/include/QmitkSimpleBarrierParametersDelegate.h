/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSimpleBarrierParametersDelegate_h
#define QmitkSimpleBarrierParametersDelegate_h


/// Toolkit includes.
#include <QStyledItemDelegate>

#include <MitkModelFitUIExports.h>


/**
 * \class QmitkSimpleBarrierParametersDelegate
 * \brief Item delegate for rendering and editing the parameters of a barrier constraint.
 *
 * This delegate handles the display and selection of parameters relevant to a barrier
 * constraint. It assumes the following data conventions:
 * - Qt::EditRole data is a QStringList of all available parameter names.
 * - Qt::DisplayRole data is a QStringList of the currently selected parameter names.
 *
 * The editor is a QListWidget with checkboxes, allowing multi-selection of parameters.
 * When data is transferred back to the model, a QStringList of all checked parameter
 * names is written.
 *
 * \sa QmitkSimpleBarrierModel
 * \sa QmitkSimpleBarrierManagerWidget
 */
class MITKMODELFITUI_EXPORT QmitkSimpleBarrierParametersDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:

  /**
   * \brief Constructs a QmitkSimpleBarrierParametersDelegate.
   * \param[in] parent Optional parent QObject.
   */
  QmitkSimpleBarrierParametersDelegate(QObject* parent = nullptr);

  /**
   * \brief Paints the selected parameter names as a comma-separated string.
   *
   * If the display data is invalid, renders "Invalid data".
   *
   * \param[in] painter The QPainter to use for rendering.
   * \param[in] option Style options for the item.
   * \param[in] index The model index to paint.
   */
  void paint(QPainter* painter, const QStyleOptionViewItem& option
             , const QModelIndex& index) const override;

  /**
   * \brief Creates a QListWidget editor with checkable parameter name items.
   *
   * If the edit role data is invalid, returns a read-only QLabel instead.
   *
   * \param[in] parent The parent widget for the editor.
   * \param[in] option Style options for the item.
   * \param[in] index The model index of the item being edited.
   * \return The newly created editor widget.
   */
  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option
                        , const QModelIndex& index) const override;

  /**
   * \brief Populates the list widget with parameter names, checking currently selected ones.
   *
   * Uses the edit role data for the full list and the display role data for checked items.
   *
   * \param[in] editor The editor widget (expected to be a QListWidget).
   * \param[in] index The model index of the item.
   */
  void setEditorData(QWidget* editor, const QModelIndex& index) const override;

  /**
   * \brief Transfers the checked parameter names from the list widget back to the model.
   * \param[in] editor The editor widget containing the selections.
   * \param[in,out] model The abstract item model to update.
   * \param[in] index The model index of the item.
   */
  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

};

#endif
