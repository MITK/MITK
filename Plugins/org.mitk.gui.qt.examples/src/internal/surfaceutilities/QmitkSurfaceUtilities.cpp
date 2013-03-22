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

#include "QmitkSurfaceUtilities.h"

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// MITK
#include <mitkNodePredicateDataType.h>




const std::string QmitkSurfaceUtilities::VIEW_ID = "org.mitk.views.qmitksurfaceutilies";

QmitkSurfaceUtilities::QmitkSurfaceUtilities()
    : QmitkAbstractView()
{

}

QmitkSurfaceUtilities::~QmitkSurfaceUtilities()
{

}

void QmitkSurfaceUtilities::CreateQtPartControl( QWidget *parent )
{
  // build up qt view
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls = Ui::QmitkSurfaceUtilitiesControls();
  m_Controls.setupUi( parent );
  //connect( m_Controls.buttonPerformImageProcessing, SIGNAL(clicked()), this, SLOT(DoImageProcessing()) );

  m_Controls.m_applySurfaceBox->SetDataStorage(this->GetDataStorage());
  m_Controls.m_applySurfaceBox->SetAutoSelectNewItems(true);
  m_Controls.m_applySurfaceBox->SetPredicate(mitk::NodePredicateDataType::New("Surface"));
}

void QmitkSurfaceUtilities::SetFocus()
{

}
