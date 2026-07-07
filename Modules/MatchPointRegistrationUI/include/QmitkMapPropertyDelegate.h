/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMapPropertyDelegate_h
#define QmitkMapPropertyDelegate_h

#include <mitkBaseProperty.h>
#include <QStyledItemDelegate>

// MITK
#include <MitkMatchPointRegistrationUIExports.h>

/**
 * \class QmitkMapPropertyDelegate
 * \brief Item delegate for rendering and editing MatchPoint algorithm meta-properties in a QTableView.
 *
 * This delegate creates appropriate editor widgets (QSpinBox, QDoubleSpinBox, QComboBox, or
 * default QStyledItemDelegate editors) based on the Qt type of the property value. It handles
 * data transfer between the editors and the underlying model (typically a QmitkMAPAlgorithmModel)
 * and commits changes on relevant user interaction events (key release, mouse release, focus, etc.).
 *
 * Supported property types and their editors:
 * - \c int: QSpinBox
 * - \c float: QDoubleSpinBox (5 decimal places)
 * - \c QStringList: QComboBox (non-editable)
 * - Other types: default QStyledItemDelegate editor
 *
 * \sa QmitkMAPAlgorithmModel, QmitkAlgorithmSettingsConfig
 */
class MITKMATCHPOINTREGISTRATIONUI_EXPORT QmitkMapPropertyDelegate /** \cond */ : public QStyledItemDelegate /** \endcond */
{
  Q_OBJECT

public:
  /**
   * \brief Constructs the property delegate.
   * \param[in] parent Optional parent QObject.
   */
  QmitkMapPropertyDelegate(QObject *parent = nullptr);

  /**
   * \brief Paints the delegate for the given model index.
   * \param[in] painter The painter to use.
   * \param[in] option Style options for the item.
   * \param[in] index The model index to paint.
   */
  void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  /**
   * \brief Creates an appropriate editor widget for the property at the given index.
   *
   * The editor type is determined by the Qt meta type of the data at the index:
   * QSpinBox for int, QDoubleSpinBox for float, QComboBox for QStringList, or
   * the default delegate editor for other types.
   *
   * \param[in] parent The parent widget for the editor.
   * \param[in] option Style options for the item.
   * \param[in] index The model index identifying the property to edit.
   * \return The created editor widget, or a QLabel if the data is not editable.
   */
  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  /**
   * \brief Populates the editor widget with data from the model.
   * \param[in] editor The editor widget to populate.
   * \param[in] index The model index providing the data.
   */
  void setEditorData(QWidget *editor, const QModelIndex &index) const override;

  /**
   * \brief Commits data from the editor widget back to the model.
   * \param[in] editor The editor widget containing the user's input.
   * \param[in,out] model The model to receive the data.
   * \param[in] index The model index identifying the property being edited.
   */
  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

  /**
   * \brief Updates the editor geometry to fit the item's bounding rectangle.
   * \param[in] editor The editor widget to resize.
   * \param[in] option Style options containing the geometry.
   * \param[in] index The model index (unused).
   */
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
