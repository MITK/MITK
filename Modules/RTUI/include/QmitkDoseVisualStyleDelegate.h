/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDoseVisualStyleDelegate_h
#define QmitkDoseVisualStyleDelegate_h


/// Toolkit includes.
#include <QStyledItemDelegate>

#include <MitkRTUIExports.h>


/**
 * \class QmitkDoseVisualStyleDelegate
 * \brief Item delegate for rendering and toggling boolean dose visualization options.
 *
 * This delegate renders a checkbox for boolean visualization properties of iso dose levels
 * (e.g. iso line visibility, color wash visibility). A left mouse click toggles the value.
 *
 * \sa QmitkIsoDoseLevelSetModel
 * \sa QmitkDoseColorDelegate
 * \sa QmitkDoseValueDelegate
 * \ingroup MitkRTUIModule
 */
class MITKRTUI_EXPORT QmitkDoseVisualStyleDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  /**
   * \brief Constructor.
   * \param[in] parent Optional parent QObject.
   */
  explicit QmitkDoseVisualStyleDelegate(QObject *parent = nullptr);

  /**
   * \brief Paints a centered checkbox representing the boolean value.
   * \param[in] painter The QPainter to use for drawing.
   * \param[in] option Style options for the item.
   * \param[in] index The model index of the item to paint.
   */
  void paint(QPainter *painter, const QStyleOptionViewItem &option
    , const QModelIndex &index) const override;

  /**
   * \brief Toggles the boolean value on left mouse button release.
   * \param[in] event The event to handle.
   * \param[in,out] model The model to read from and write to.
   * \param[in] option Style options for the item.
   * \param[in] index The model index of the item.
   * \return True if the value was successfully toggled.
   */
  bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
    const QModelIndex &index) override;

};

#endif
