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


#ifndef DICOMVIEW_H_
#define DICOMVIEW_H_

#include <QmitkAbstractView.h>
#include <berryQtViewPart.h>

#include "ui_QmitkDicomViewControls.h"

#include <berryISelectionListener.h>
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryIStructuredSelection.h>

class DicomView : public QmitkAbstractView
{

  Q_OBJECT

public:

  static const std::string VIEW_ID;

  DicomView();

  virtual ~DicomView();

  virtual void CreateQtPartControl(QWidget *parent);

//private:
    //void SelectionChanged(berry::IWorkbenchPart::Pointer sourcepart,
    //                 berry::ISelection::ConstPointer selection);

    //berry::ISelectionListener::Pointer m_SelectionListener;
    //friend struct berry::SelectionChangedAdapter<DicomView>;

  //private slots:
  //   void ToggleRadioMethod(); //Debugging only!

protected slots:
  void AddDataNodeFromDICOM(const QStringList& Properties);

protected:

  void SetFocus();

  Ui::QmitkDicomViewControls m_Controls;

  QWidget* m_Parent;

};

#endif /*DICOMVIEW_H_*/
