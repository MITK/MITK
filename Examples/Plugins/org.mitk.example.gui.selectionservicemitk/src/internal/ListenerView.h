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


#ifndef LISTENERVIEW_H_
#define LISTENERVIEW_H_

/// Qmitk
#include <QmitkAbstractView.h>

/// Berry
#include <berryISelectionListener.h>
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryIStructuredSelection.h>

#include "ui_ListenerViewControls.h"

class ListenerView : public QmitkAbstractView
{

  Q_OBJECT

public:

  static const std::string VIEW_ID;

  ListenerView();

  virtual ~ListenerView();

  virtual void CreateQtPartControl(QWidget *parent);

private:
    void SelectionChanged(berry::IWorkbenchPart::Pointer sourcepart,
                     berry::ISelection::ConstPointer selection);

    berry::ISelectionListener::Pointer m_SelectionListener;
    friend struct berry::SelectionChangedAdapter<ListenerView>;

  private slots:
     void ToggleRadioMethod(); //Debugging only!

protected:

  void SetFocus();

  Ui::ListenerViewControls m_Controls;

  QWidget* m_Parent;

};

#endif /*LISTENERVIEW_H_*/
