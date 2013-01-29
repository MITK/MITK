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

// QMitk includes
#include <QmitkAbstractView.h>

// berry includes
#include <berryIWorkbenchWindow.h>

// Qt includes
#include <QString>

// ui includes
#include "ui_ListenerViewMitkControls.h"

/**
 * \ingroup org_mitk_example_gui_selectionservicemitk
 *
 * \brief This BlueBerry view is part of the BlueBerry example
 * "Selection service MITK". It creates two radio buttons that listen
 * for selection events of the Qlistwidget (SelectionProvider) and
 * change the radio button state accordingly.
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

protected:

  void CreateQtPartControl(QWidget *parent);

  void SetFocus();

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

private Q_SLOTS:

  /** @brief Simple slot function that changes the selection of the radio buttons according to the passed string.
   * @param selectStr QString that contains the name of the slected list element
   *
   */
  void ToggleRadioMethod(QString selectStr);

private:

  Ui::ListenerViewMitkControls m_Controls;

  QWidget* m_Parent;

};

#endif /*LISTENERVIEWMITK_H_*/
