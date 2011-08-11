/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
 Version:   $Revision: 21975 $

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
#include "QmitkFiberBundleDeveloperView.h"
#include <QmitkStdMultiWidget.h>

// Qt
#include <QString>

const std::string QmitkFiberBundleDeveloperView::VIEW_ID = "org.mitk.views.fiberbundledeveloper";
const std::string id_DataManager = "org.mitk.views.datamanager";
using namespace berry;


QmitkFiberBundleDeveloperView::QmitkFiberBundleDeveloperView()
: QmitkFunctionality()
, m_Controls( 0 )
, m_MultiWidget( NULL )
{

}

// Destructor
QmitkFiberBundleDeveloperView::~QmitkFiberBundleDeveloperView()
{

}


void QmitkFiberBundleDeveloperView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done in QtDesigner, etc.
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkFiberBundleDeveloperViewControls;
    m_Controls->setupUi( parent );

    connect( m_Controls->buttonGenerateFibers, SIGNAL(clicked()), this, SLOT(DoGenerateFibers()) );

    
  }
  
  if ( m_DirectionRadios.empty() )
  {
   
//    feed the vector with directionRadioButtons
    m_DirectionRadios.insert(0, m_Controls->radioButton_directionRandom);
    m_DirectionRadios.insert(1, m_Controls->radioButton_directionX);
    m_DirectionRadios.insert(2, m_Controls->radioButton_directionY);
    m_DirectionRadios.insert(3, m_Controls->radioButton_directionZ);
    
  }
}

void QmitkFiberBundleDeveloperView::DoGenerateFibers()
{
  
// GET SELECTED FIBER DIRECTION
  QString fibDirection;
  QVector<QRadioButton*>::const_iterator i;
  for (i = m_DirectionRadios.begin(); i != m_DirectionRadios.end(); ++i) {
    
    QRadioButton* rdbtn = *i;
    if (rdbtn->isChecked()) {
      fibDirection = rdbtn->objectName();
      MITK_INFO << fibDirection.toStdString() << " is checked, safe it :-)" ;
    } else {
      MITK_INFO << fibDirection.toStdString() << " is NOT checked" ;
    }
  }
  

  
}





void QmitkFiberBundleDeveloperView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}


void QmitkFiberBundleDeveloperView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

/* OnSelectionChanged is registered to SelectionService, therefore no need to
 implement SelectionService Listener explicitly */

void QmitkFiberBundleDeveloperView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{

  
}

void QmitkFiberBundleDeveloperView::Activated()
{

  MITK_INFO << "FB OPerations ACTIVATED()";


}



