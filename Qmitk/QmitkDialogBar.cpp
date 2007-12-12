/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 11316 $ 

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "QmitkDialogBar.h"

#include "mitkProperties.h"

#include <qgroupbox.h>
#include <qlayout.h>
#include <qfont.h>
#include <qobjectlist.h>

QmitkDialogBar
::QmitkDialogBar( 
  const char *caption, QObject *parent, const char *name, QmitkStdMultiWidget *multiWidget,
  mitk::DataTreeIteratorBase *dataIt )
: QmitkBaseFunctionalityComponent( parent, name, dataIt ),
  m_GroupBox( NULL ),
  m_MultiWidget( multiWidget ),
  m_Caption( caption )
{
}

QmitkDialogBar
::~QmitkDialogBar()
{
}

QWidget *
QmitkDialogBar
::CreateControlWidget( QWidget *parent )
{
  m_GroupBox = new QGroupBox( parent );
  m_GroupBox->setHidden( true );

  m_GroupBox->setColumnLayout( 0, Qt::Vertical );
  m_GroupBox->layout()->setSpacing( 6 );
  m_GroupBox->setMargin( 11 );
  m_GroupBox->setTitle( m_Caption );

  QWidget *dialogBar = this->CreateDialogBar( m_GroupBox );

  QFont captionFont = m_GroupBox->font();
  captionFont.setBold( true );
  m_GroupBox->setFont( captionFont );

  QObjectList* childList = m_GroupBox->queryList( "QWidget" );
  QObjectListIt childIter( *childList ); 
  QObject* child;
  QWidget* widget;
  while ( (child = childIter.current()) )
  {
    if (child->inherits("QWidget"))
    {
      widget = static_cast<QWidget*>( child );
      widget->unsetFont();
      QFont normalFont = widget->font();
      normalFont.setBold( false );
      widget->setFont( normalFont );
    }

    ++childIter;
  }
 
  QGridLayout *grid = new QGridLayout( m_GroupBox->layout(), 1, 1, 2 );
  grid->setAlignment( Qt::AlignTop );
  grid->addWidget( dialogBar, 0, 0 );
  
  return m_GroupBox;
}

void 
QmitkDialogBar
::ToggleVisible( bool visible )
{
  if ( m_GroupBox )
  {
    m_GroupBox->setHidden( !visible );
  }

  // Reflect new visible state in global preferences
  if ( m_GlobalOptions )
  {
    QString dialogBarStateName = "DialogBar " + m_Caption + " active";

    m_GlobalOptions->SetProperty( 
      dialogBarStateName, new mitk::BoolProperty( visible ) );
  }
}

const QString &
QmitkDialogBar
::GetCaption() const
{
  return m_Caption;
}

QmitkStdMultiWidget *
QmitkDialogBar
::GetMultiWidget() const
{
  return m_MultiWidget;
}

void
QmitkDialogBar
::SetGlobalOptions( mitk::PropertyList::Pointer options )
{
  m_GlobalOptions = options;
}
