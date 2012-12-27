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


#ifndef SELECTIONVIEWMITK_H_
#define SELECTIONVIEWMITK_H_

// QMitk includes
#include <QmitkAbstractView.h>

// ui includes
#include "ui_SelectionViewMitkControls.h"

/**
 * \ingroup org_mitk_example_gui_selectionservicemitk
 *
 * \brief This BlueBerry view is part of the BlueBerry example
 * "Selection service MITK". It creates a QListWidget that provides
 * the selection events for the selection listener. The selection
 * provider is MITK-based.
 */
class SelectionViewMitk : public QmitkAbstractView
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  SelectionViewMitk();

protected:

  virtual void CreateQtPartControl(QWidget *parent);

  void SetFocus();

private:

  //! [MITK Selection Provider method]
  /** @brief Reimplementation of method from QmitkAbstractView that returns the data node
  *   selection model for the selection listener.
  */
  QItemSelectionModel* GetDataNodeSelectionModel() const;
  //! [MITK Selection Provider method]

  Ui::SelectionViewMitkControls m_Controls;

  QWidget* m_Parent;

};

#endif /*SELECTIONVIEWMITK_H_*/
