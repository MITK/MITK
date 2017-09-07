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

#ifndef QMITKSELECTNODEDIALOG_H
#define QMITKSELECTNODEDIALOG_H

#include <MitkSemanticRelationsUIExports.h>

// semantic relations UI module
#include "QmitkSelectionWidget.h"

// mitk
#include <mitkDataNode.h>

// qt
#include <QDialog>
#include <QLayout>

/*
* @brief The QmitkSelectNodeDialog is a QDialog to select nodes in a user defined way. The Widget to use for data node selection can be injected.
*
*   The QmitkSelectNodeDialog provides a 'SetSelectionWidget'-function, that uses the 'QmitkSelectionWidget'-parameter to receive a specific widget.
*   This specific widget can be implemented in any way, as a subclass of 'QmitkSelectionWidget'.
*
*   The QmitkSelectNodeDialog holds the currently selected data node, that can be received by using the 'GetSelectedDataNode'-function. However, in order to
*   have the dialog return the currently selected data node, the specific subclass of the 'QmitkSelectionWidget' has to emit the 'SelectionChanged(mitk::DataNode*)'-signal
*   with the currently selected data node. This data node will be stored and returned on request.
*/
class MITKSEMANTICRELATIONSUI_EXPORT QmitkSelectNodeDialog : public QDialog
{
  Q_OBJECT

public:

  QmitkSelectNodeDialog(QWidget* parent = nullptr);
  virtual ~QmitkSelectNodeDialog();

  void SetSelectionWidget(QmitkSelectionWidget* selectionWidget);
  mitk::DataNode* GetSelectedDataNode() const;

private Q_SLOTS:

  /*
  * @brief A slot that is used on the 'SelectionChanged(mitk::DataNode*)'-signal of the injected 'QmitkSelectionWidget'.
  *
  * @par    A pointer to the data node that is newly selected
  */
  void OnSelectionWidgetSelectionChanged(mitk::DataNode*);

private:

  QBoxLayout* m_VerticalLayout;
  QmitkSelectionWidget* m_SelectionWidget;
  mitk::DataNode* m_SelectedDataNode;

};

#endif // QMITKSELECTNODEDIALOG_H
