/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMapPropertyDelegate_h
#define QmitkMapPropertyDelegate_h

/// Toolkit includes.
#include "mitkBaseProperty.h"
#include <QStyledItemDelegate>

// MITK
#include "MitkMatchPointRegistrationUIExports.h"

/// Forward declarations.

///
/// \class QmitkPropertyDelegate
/// \brief An item delegate for rendering and editing mitk::Properties in a QTableView.
///
/// \see QmitkPropertiesTableModel
class MITKMATCHPOINTREGISTRATIONUI_EXPORT QmitkMapPropertyDelegate /** \cond */ : public QStyledItemDelegate /** \endcond */
{
  Q_OBJECT

public:
  ///
  /// Creates a new PropertyDelegate.
  ///
  QmitkMapPropertyDelegate(QObject *parent = nullptr);

  ///
  /// Renders a specific property  (overwritten from QItemDelegate)
  ///
  void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  ///
  /// Create an editor for a specific property  (overwritten from QItemDelegate)
  ///
  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  ///
  /// Create an editor for a specific property  (overwritten from QItemDelegate)
  ///
  void setEditorData(QWidget *editor, const QModelIndex &index) const override;

  ///
  /// When the user accepts input this func commits the data to the model  (overwritten from QItemDelegate)
  ///
  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

  ///
  /// \brief Fit an editor to some geometry (overwritten from QItemDelegate)
  ///
  void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

protected:
  bool eventFilter(QObject *o, QEvent *e) override;

private slots:
  ///
  /// Invoked when the user accepts editor input, that is when he does not pushes ESC.
  ///
  void commitAndCloseEditor();
  void showColorDialog();
  void ComboBoxCurrentIndexChanged(int index);
  void SpinBoxValueChanged(const QString &value);
};

#endif
