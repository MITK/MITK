/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDoseColorDelegate_h
#define QmitkDoseColorDelegate_h


#include <QStyledItemDelegate>

#include <MitkRTUIExports.h>

/**
 * \class QmitkDoseColorDelegate
 * \brief Item delegate for rendering and editing dose level colors in a QTableView.
 *
 * This delegate paints a filled rectangle with the dose color and opens a
 * QColorDialog on left-click to allow the user to change the color. It reads
 * and writes color data via the Qt::EditRole.
 *
 * \sa QmitkIsoDoseLevelSetModel
 * \sa QmitkDoseValueDelegate
 * \sa QmitkDoseVisualStyleDelegate
 * \ingroup MitkRTUIModule
 */
class MITKRTUI_EXPORT QmitkDoseColorDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  /**
   * \brief Constructor.
   * \param[in] parent Optional parent QObject.
   */
  explicit QmitkDoseColorDelegate(QObject *parent = nullptr);

  /**
   * \brief Handles mouse click events to open a color chooser dialog.
   *
   * On left mouse button release, opens a QColorDialog initialized with the
   * current color. If the user selects a valid color, it is written back to the model.
   *
   * \param[in] event The event to handle.
   * \param[in,out] model The model to read from and write to.
   * \param[in] option Style options for the item.
   * \param[in] index The model index of the item.
   * \return True if the color was successfully changed.
   */
  bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
    const QModelIndex &index) override;

  /**
   * \brief Paints the cell as a solid rectangle filled with the dose color.
   * \param[in] painter The QPainter to use for drawing.
   * \param[in] option Style options for the item.
   * \param[in] index The model index of the item to paint.
   */
  void paint(QPainter *painter, const QStyleOptionViewItem &option,
    const QModelIndex &index) const override;
};

#endif
