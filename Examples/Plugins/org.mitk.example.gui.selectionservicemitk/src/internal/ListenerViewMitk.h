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


#ifndef LISTENERVIEWMITK_H_
#define LISTENERVIEWMITK_H_

// qmitk Includes
#include <QmitkAbstractView.h>

// berry Includes
#include <berryIWorkbenchWindow.h>

// qt Includes
#include <QString.h>

// ui Includes
#include "ui_ListenerViewMitkControls.h"

 /** Documentation:
  *   \brief This BlueBerry view is part of the BlueBerry example 
  *   "Selection service MITK". It creates two radio buttons that listen
  *   for selection events of the Qlistwidget (SelectionProvider) and
  *   change the radio button state accordingly.
  *
  * @see SelectionViewMitk
  *
  */
class ListenerViewMitk : public QmitkAbstractView
{

  Q_OBJECT

public:

  static const std::string VIEW_ID;

  ListenerViewMitk();

  virtual ~ListenerViewMitk();

  virtual void CreateQtPartControl(QWidget *parent);

  //! [MITK Selection Listener method]
  /** @brief Reimplemention of method from QmitkAbstractView that implements the selection listener functionality.
    * @param part The workbench part responsible for the selection change.
    * @param nodes A list of selected nodes.
    *
    * @see QmitkAbstractView
    *
    */
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer> &nodes);
  //! [MITK Selection Listener method]

private:

  private slots:
    /** @brief Simple slot function that changes the selection of the radio buttons according to the passed string.
      * @param selectStr QString that contains the name of the slected list element
      *
      */    
    void ToggleRadioMethod(QString selectStr);

protected:

  void SetFocus();

  Ui::ListenerViewMitkControls m_Controls;

  QWidget* m_Parent;

};

#endif /*LISTENERVIEWMITK_H_*/
