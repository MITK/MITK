/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkInitialValuesDelegate_h
#define QmitkInitialValuesDelegate_h


/// Toolkit includes.
#include <mitkDataStorage.h>
#include <mitkNodePredicateBase.h>

#include <QStyledItemDelegate>

#include <MitkModelFitUIExports.h>


/**
 * \class QmitkInitialValuesDelegate
 * \brief Item delegate for rendering and editing the initial value source of a model parameter.
 *
 * This delegate supports two value types determined by the Qt::UserRole data of the index:
 * - Type 0 (scalar): The edit role data is a double, and a standard spin box editor is used.
 * - Type 1 (image): The edit role data is a void pointer to a mitk::DataNode. A
 *   QmitkDataStorageComboBox is used to select an image node from the configured data storage.
 *
 * \sa QmitkInitialValuesModel
 * \sa QmitkInitialValuesTypeDelegate
 * \sa QmitkInitialValuesManagerWidget
 */
class MITKMODELFITUI_EXPORT QmitkInitialValuesDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:

  /**
   * \brief Constructs a QmitkInitialValuesDelegate.
   * \param[in] parent Optional parent QObject.
   */
  QmitkInitialValuesDelegate(QObject* parent = nullptr);

  /**
   * \brief Creates an appropriate editor widget based on the value type.
   *
   * For scalar types (type 0), delegates to QStyledItemDelegate::createEditor().
   * For image types (type 1), creates a QmitkDataStorageComboBox filtered by the
   * configured node predicate.
   *
   * \param[in] parent The parent widget for the editor.
   * \param[in] option Style options for the item.
   * \param[in] index The model index of the item being edited.
   * \return The newly created editor widget.
   */
  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option
                        , const QModelIndex& index) const override;

  /**
   * \brief Populates the editor with data from the model.
   *
   * For scalar types, delegates to QStyledItemDelegate. For image types, finds
   * and selects the corresponding data node in the combo box.
   *
   * \param[in] editor The editor widget to populate.
   * \param[in] index The model index of the item.
   */
  void setEditorData(QWidget* editor, const QModelIndex& index) const override;

  /**
   * \brief Transfers data from the editor back to the model.
   *
   * For image types, stores the selected data node pointer. For scalar types,
   * delegates to QStyledItemDelegate.
   *
   * \param[in] editor The editor widget containing the new value.
   * \param[in,out] model The abstract item model to update.
   * \param[in] index The model index of the item.
   */
  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

  /**
   * \brief Sets the data storage used to populate the image selection combo box.
   * \param[in] storage Pointer to the DataStorage instance.
   */
  void setDataStorage(mitk::DataStorage* storage);

  /**
   * \brief Sets the node predicate used to filter images in the data storage.
   * \param[in] predicate Pointer to the predicate for filtering valid initial value images.
   */
  void setNodePredicate(mitk::NodePredicateBase* predicate);

protected:
  /**
   * \brief Determines the value type for the given index.
   * \param[in] index The model index to query (uses Qt::UserRole data).
   * \return 0 for scalar type, 1 for image type.
   */
  int valueType(const QModelIndex& index) const;

  /** \brief Data storage used as source for potential initial value images. */
  mitk::DataStorage::Pointer m_Storage;
  /** \brief Predicate used to filter for valid initial value images in the data storage. */
  mitk::NodePredicateBase::Pointer m_Predicate;

};

#endif
