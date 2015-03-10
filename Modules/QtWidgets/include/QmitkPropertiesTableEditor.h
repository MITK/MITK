/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
  QmitkPropertiesTableEditor(QWidget* parent = 0, Qt::WindowFlags f = 0,mitk::DataNode::Pointer _Node = 0);

  ///
  /// Standard dtor. Nothing to do here.
  virtual ~QmitkPropertiesTableEditor();



  ///
  /// Convenience method. Sets the property list in the model.
  ///
  void SetPropertyList(mitk::PropertyList::Pointer _List);

  ///
  /// Get the model.
  ///
  QmitkPropertiesTableModel* getModel() const;

  QTableView* getTable() const;

  protected slots:
    void PropertyFilterKeyWordTextChanged(const QString & text);
protected:
  ///
  /// Initialise empty GUI.
  ///
  virtual void init();
  ///
  /// The table view that renders the property list.
  ///
  QTableView* m_NodePropertiesTableView;
  ///
  /// A text field in which the user can enter a filter keyword for the properties. Only properties containing with this keyword
  /// will be selected.
  ///
  QLineEdit* m_TxtPropertyFilterKeyWord;
  ///
  /// The property list table model.
  ///
  QmitkPropertiesTableModel* m_Model;
};

#endif /* QMITKPROPERTIESTABLEMODEL_H_ */
