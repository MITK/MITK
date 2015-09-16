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

#ifndef QmitkMITKIGTNavigationToolManagerView_h
#define QmitkMITKIGTNavigationToolManagerView_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>
#include <usServiceReference.h>

#include "ui_QmitkMITKIGTNavigationToolManagerViewControls.h"



/*!
  \brief QmitkMITKIGTNavigationToolManagerView

  \warning  This application module is not yet documented. Use "svn blame/praise/annotate" and ask the author to provide basic documentation.

*/
class QmitkMITKIGTNavigationToolManagerView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkMITKIGTNavigationToolManagerView();
    virtual ~QmitkMITKIGTNavigationToolManagerView();

    virtual void CreateQtPartControl(QWidget *parent) override;

    virtual void SetFocus() override;

  protected slots:

    void NewStorageByWidget(mitk::NavigationToolStorage::Pointer storage,std::string);
    void ToolStorageSelected(mitk::NavigationToolStorage::Pointer);

  protected:

    Ui::QmitkMITKIGTNavigationToolManagerViewControls* m_Controls;

    /** Someone needs to hold the smart pointers of new storages, otherwise the objects will be lost although they are listed as microservice. */
    std::vector<mitk::NavigationToolStorage::Pointer> m_AllStoragesHandledByThisWidget;
};



#endif // _QMITKMITKIGTNAVIGATIONTOOLMANAGERVIEW_H_INCLUDED
