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


#ifndef TESTVIEW_H_
#define TESTVIEW_H_

//#include <QmitkAbstractView.h>
#include <berryQtViewPart.h>

#include "ui_QmitkTestViewControls.h"

#include <berryISelectionListener.h>
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryIStructuredSelection.h>

class TestView : public berry::QtViewPart
{

  Q_OBJECT

public:

  static const std::string VIEW_ID;

  TestView();

  virtual ~TestView();

  virtual void CreateQtPartControl(QWidget *parent);

//private:
    //void SelectionChanged(berry::IWorkbenchPart::Pointer sourcepart,
    //                 berry::ISelection::ConstPointer selection);

    //berry::ISelectionListener::Pointer m_SelectionListener;
    //friend struct berry::SelectionChangedAdapter<MinimalView>;

  //private slots:
  //   void ToggleRadioMethod(); //Debugging only!

protected:

  void SetFocus();

  Ui::QmitkTestViewControls m_Controls;

  QWidget* m_Parent;

};

#endif /*TESTVIEW_H_*/
