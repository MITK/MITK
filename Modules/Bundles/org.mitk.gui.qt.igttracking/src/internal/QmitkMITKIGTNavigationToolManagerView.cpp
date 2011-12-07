/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkMITKIGTNavigationToolManagerView.h"
#include "QmitkStdMultiWidget.h"

// Qt
#include <QMessageBox>



const std::string QmitkMITKIGTNavigationToolManagerView::VIEW_ID = "org.mitk.views.mitkigtnavigationtoolmanager";

QmitkMITKIGTNavigationToolManagerView::QmitkMITKIGTNavigationToolManagerView()
: QmitkFunctionality()
, m_Controls( 0 )
, m_MultiWidget( NULL )
{
}

QmitkMITKIGTNavigationToolManagerView::~QmitkMITKIGTNavigationToolManagerView()
{
}


void QmitkMITKIGTNavigationToolManagerView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkMITKIGTNavigationToolManagerViewControls;
    m_Controls->setupUi( parent );
  }
  m_Controls->m_toolManagerWidget->Initialize(this->GetDataStorage());
}


void QmitkMITKIGTNavigationToolManagerView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}


void QmitkMITKIGTNavigationToolManagerView::StdMultiWidgetNotAvailable()
  {
  m_MultiWidget = NULL;
  }



