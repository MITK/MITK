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

#ifndef QMITKSELECTIONWIDGET_H
#define QMITKSELECTIONWIDGET_H

// semantic relations UI module
#include "MitkSemanticRelationsUIExports.h"

// semantic relations module
#include <mitkSemanticRelations.h>

// mitk core
#include <mitkDataNode.h>
#include <mitkDataStorage.h>

// qt
#include <QWidget>

/*
* @brief The QmitkSelectionWidget is an abstract widget that provides subclasses with some base-functions and offers a Q_SIGNAL.
*
*   The QmitkSelectionWidget is typically used in a way where a specific derived widget should be used / injected. Several subclasses can be created
*   and used, wherever the QmitkSelectionWidget is needed.
*
*   This abstract widget provides a DataStorage instance, a SemanticRelations instance and the possibility to update the data storage for all its derived widgets.
*
*   The "SelectionChanged"-signal can be emitted from each subclass. This is needed in order to inform other widgets about a changed selection in the derived widget.
*   The 'QmitkPatientTableWidget' and the 'QmitkSimpleDatamanagerWidget' are currently derived from this abstract widget.
*/
class MITKSEMANTICRELATIONSUI_EXPORT QmitkSelectionWidget : public QWidget
{
  Q_OBJECT

public:

  virtual ~QmitkSelectionWidget() = 0;

  /*
  * @brief Sets the data storage and updates the semantic relations pointer at once
  *
  * @par dataStorage    A pointer to the data storage to set
  */
  virtual void SetDataStorage(mitk::DataStorage* dataStorage);

Q_SIGNALS:

  /*
  * @brief A base signal that can be emitted in any subclass of this widget
  *
  *         Has to be emitted in order for a QmitkSelectNodeDialog to receive the currently selected data node.
  *
  * @par    A pointer to the data node that is newly selected
  */
  void SelectionChanged(mitk::DataNode*);

protected:

  QmitkSelectionWidget(mitk::DataStorage* dataStorage, QWidget* parent = nullptr);
  mitk::DataStorage* m_DataStorage;
  std::shared_ptr<mitk::SemanticRelations> m_SemanticRelations;

};

#endif // QMITKSELECTIONWIDGET_H
