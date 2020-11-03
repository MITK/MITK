/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPropertiesTableEditor_h
#define QmitkPropertiesTableEditor_h

#include <MitkQtWidgetsExports.h>

/// Own includes.
#include "mitkDataNode.h"

/// Toolkit includes.
#include <QWidget>

/// Forward declarations.
class QmitkPropertiesTableModel;
class QTableView;
class QLineEdit;

/**
 * \ingroup QmitkModule
 * \brief Combines a QTableView along with a QmitkPropertiesTableModel to a reusable
 * Property Editor component.
 *
 * \see QmitkPropertyDelegate
 */
class MITKQTWIDGETS_EXPORT QmitkPropertiesTableEditor : public QWidget
{
  Q_OBJECT

public:
  ///
  /// Constructs a new QmitkDataStorageTableModel
  /// and sets the DataNode for this TableModel.
  QmitkPropertiesTableEditor(QWidget *parent = nullptr, Qt::WindowFlags f = nullptr, mitk::DataNode::Pointer _Node = nullptr);

  ///
  /// Standard dtor. Nothing to do here.
  ~QmitkPropertiesTableEditor() override;

  ///
  /// Convenience method. Sets the property list in the model.
  ///
  void SetPropertyList(mitk::PropertyList::Pointer _List);

  ///
  /// Get the model.
  ///
  QmitkPropertiesTableModel *getModel() const;

  QTableView *getTable() const;

protected slots:
  void PropertyFilterKeyWordTextChanged(const QString &text);

protected:
  ///
  /// Initialise empty GUI.
  ///
  virtual void init();
  ///
  /// The table view that renders the property list.
  ///
  QTableView *m_NodePropertiesTableView;
  ///
  /// A text field in which the user can enter a filter keyword for the properties. Only properties containing with this
  /// keyword
  /// will be selected.
  ///
  QLineEdit *m_TxtPropertyFilterKeyWord;
  ///
  /// The property list table model.
  ///
  QmitkPropertiesTableModel *m_Model;
};

#endif /* QMITKPROPERTIESTABLEMODEL_H_ */
