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


#ifndef SELECTIONVIEW_H_
#define SELECTIONVIEW_H_

/// Qmitk
#include <QmitkAbstractView.h>
#include <QmitkDataNodeSelectionProvider.h>

#include "ui_SelectionViewControls.h"


class SelectionView : public QmitkAbstractView
{

  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkDataNodeSelectionProvider::Pointer m_SelectionProvider;

  SelectionView();

  virtual ~SelectionView();

  virtual void CreateQtPartControl(QWidget *parent);

private:

  
  private slots:
     void TestMethod(); //Debugging only!

protected:

  void SetFocus();

  Ui::SelectionViewControls m_Controls;

  QWidget* m_Parent;

};

#endif /*SELECTIONVIEW_H_*/
