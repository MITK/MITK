/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 14147 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkPACS.h"

//Qt
#include <qaction.h>

//MITK
#include "mitkPACSPluginEvents.h"

#include "QmitkStdMultiWidget.h"
#include "QmitkPACSLoadDialog.h"

#include "icon.xpm"

QmitkPACS::QmitkPACS( QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it )
: QmitkFunctionality( parent, name, it ),
  m_Plugin( NULL ),
  m_MultiWidget( mitkStdMultiWidget ),
  m_Controls( NULL )
{
  SetAvailability( true );
  
  // register as observer to PACS plugin 
  m_Plugin = mitk::PACSPlugin::GetInstance();
  if (!m_Plugin) throw std::logic_error ("There should always be an instance of mitk::PACSPlugin.");

  itk::ReceptorMemberCommand<QmitkPACS>::Pointer command = itk::ReceptorMemberCommand<QmitkPACS>::New();
  command->SetCallbackFunction( this, &QmitkPACS::PluginEventNewStudySelected );
  m_ObserverTag = m_Plugin->AddObserver( mitk::PluginStudySelected(), command );
}

QmitkPACS::~QmitkPACS()
{
  /* functionality is deleted AFTER plugin, so do not access plugin anymore
  if( m_Plugin )
  {
    m_Plugin->RemoveObserver( m_ObserverTag );
  }
  */
}

QWidget * QmitkPACS::CreateMainWidget( QWidget *parent )
{
  if( m_MultiWidget == NULL )
  {
    m_MultiWidget = new QmitkStdMultiWidget( parent );
  }
  return m_MultiWidget;
}


QWidget * QmitkPACS::CreateControlWidget( QWidget *parent )
{
  if( !m_Controls )
  {
    m_Controls = new QmitkPACSLoadDialog( parent, true );
  }
  
  return m_Controls;
}

void QmitkPACS::AbortPACSImport()
{
  m_Plugin->AbortPACSImport();
}

QAction * QmitkPACS::CreateAction( QActionGroup *parent )
{
  QAction* action;
  action = new QAction( tr( "Load/Save functions for PACS" ), 
                        QPixmap( (const char**) icon_xpm ), 
                        tr( "Load/Save functions for &PACS" ), 
                        0, 
                        parent, 
                        "QmitkPACS" );
  return action;
}

void QmitkPACS::TreeChanged()
{
}

void QmitkPACS::PluginEventNewStudySelected( const itk::EventObject& )
{
}
  
