/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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



#include "QmitkFctMediator.h"
#include "QmitkControlsRightFctLayoutTemplate.h"
#include "QmitkControlsLeftFctLayoutTemplate.h"

#include "mitkProperties.h"

#include <qwidgetstack.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qscrollview.h>
#include <qtoolbar.h>
#include <qaction.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qlabel.h>

#include <stdio.h>

#include <qapplication.h>

const QSizePolicy ignored( QSizePolicy::Ignored, QSizePolicy::Ignored );
const QSizePolicy preferred( QSizePolicy::Preferred, QSizePolicy::Preferred );


QmitkFctMediator::QmitkFctMediator( QObject *parent, const char *name )
: QObject( parent, name ), 
  m_LayoutTemplate( NULL ),
  m_MainStack( NULL ), 
  m_ControlStack( NULL ), 
  m_DialogBarsFrame( NULL ),
  m_ToolBar( NULL ), 
  m_DefaultMain( NULL ), 
  m_NumberOfFunctionalities( 0 ), 
  m_NumberOfDialogBars( 0 ), 
  m_FunctionalityActionGroup( NULL ), 
  m_CurrentFunctionality( -1 )
{
}

QmitkFctMediator::~QmitkFctMediator()
{
  // Delete all functionalities
  QmitkFunctionality *functionality;
  for ( functionality = m_Functionalities.first(); 
        functionality != 0; 
        functionality = m_Functionalities.next() )
  {
    delete functionality;
  }
  m_Functionalities.clear();

  // Delete all dialog bars
  QmitkDialogBar *dialogBar;
  for ( dialogBar = m_DialogBars.first(); 
        dialogBar != 0; 
        dialogBar = m_DialogBars.next() )
  {
    delete dialogBar;
  }
  m_DialogBars.clear();
}

void QmitkFctMediator::Initialize( QWidget *aLayoutTemplate )
{
  if (aLayoutTemplate==NULL)
  {
    return;
  }

  if (m_NumberOfFunctionalities>0)
  {
    itkGenericOutputMacro(<<"Number of functionalities >0 ("
      <<m_NumberOfFunctionalities<<") at initialize().");
  }

  if (m_NumberOfDialogBars>0)
  {
    itkGenericOutputMacro(<<"Number of dialog bars >0 ("
      <<m_NumberOfDialogBars<<") at initialize().");
  }

  // why is this cast done ? I remember we wanted an option for playing controls right...
  m_LayoutTemplate = static_cast<QWidget*>(aLayoutTemplate->child(
    "LayoutTemplate", "QmitkControlsRightFctLayoutTemplate"));

  if ( m_LayoutTemplate == NULL) {
    m_LayoutTemplate = static_cast<QWidget*>(aLayoutTemplate->child(
      "LayoutTemplate", "QmitkControlsLeftFctLayoutTemplate"));
  }

  assert (m_LayoutTemplate);

  QWidget *w;

  if ( ( w = static_cast<QWidget*>(
    aLayoutTemplate->child("MainParent", "QWidget"))) != NULL )
  {
    QHBoxLayout *hlayout=new QHBoxLayout(w);
    hlayout->setAutoAdd(true);
    m_MainStack = new QWidgetStack(
      w, "QmitkFctMediator::mainStack");

    m_DefaultMain = new QWidget(m_MainStack,
      "QmitkFctMediator::m_DefaultMain");
    m_MainStack->addWidget(m_DefaultMain, 0);
  }

  if ( ( w = static_cast< QWidget * >( 
    aLayoutTemplate->child( "ControlParent", "QWidget"))) != NULL )
  {
    QVBoxLayout *vlayout = new QVBoxLayout( w );
    vlayout->setAutoAdd( true );

    m_ControlStack = new QWidgetStack(
      w, "QmitkFctMediator::controlStack");

    m_DialogBarsFrame = new QVBox(
      w, "QmitkFctMediator::dialogBarsFrame");
    m_DialogBarsFrame->setSpacing( 10 );

    m_DialogBarsFrame->setSizePolicy( 
      QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum) );
  }

  QToolBar *toolBar;
  if ( (toolBar = static_cast< QToolBar * >( aLayoutTemplate->child(
    "ToolBar", "QWidget"))) != NULL )
  {
    m_ToolBar = toolBar;

    // Group functionality button actions exclusively and create connection
    m_FunctionalityActionGroup = new QActionGroup( this );
    m_FunctionalityActionGroup->setExclusive( true );

    connect( m_FunctionalityActionGroup, SIGNAL( selected( QAction * ) ), 
      this, SLOT( RaiseFunctionality( QAction * ) ) );
  }
}

bool QmitkFctMediator::AddFunctionality(QmitkFunctionality *functionality)
{
  // Append functionality at end of list
  m_Functionalities.append( functionality );

  QAction *action = NULL;
  if ( m_FunctionalityActionGroup != NULL )
  {
    action = functionality->CreateAction( m_FunctionalityActionGroup );
    if (action != NULL)
    {
      action->setToggleAction( true );
      if ( m_ToolBar != NULL )
      {
        action->addTo( m_ToolBar );
      }
    }
  }

  // Append action at end of list (same position as in functionality in
  // m_Functionalities)
  m_FunctionalityActions.append( action );

  if ( m_MainStack!=NULL )
  {
    QWidget *mainWidget = functionality->CreateMainWidget(m_MainStack);
    if ( (mainWidget!=NULL) && (mainWidget->parent()!=m_DefaultMain) )
    {
      QScrollView *scrollView = new QScrollView();
      scrollView->addChild(mainWidget);
      scrollView->setResizePolicy(QScrollView::AutoOneFit);
      scrollView->setSizePolicy(ignored);

      m_MainStack->addWidget(scrollView, m_NumberOfFunctionalities+1);
    }
    else
    {
      m_MainStack->addWidget(new QWidget(m_MainStack,
        "QmitkFctMediator::dummyMain"), m_NumberOfFunctionalities+1);
    }
  }
  QWidget *controlWidget;
  if (m_ControlStack!=NULL)
  {
    controlWidget = functionality->CreateControlWidget(m_ControlStack);
    if ( controlWidget != NULL)
    {
      QScrollView *scrollView = new QScrollView();
      scrollView->addChild(controlWidget);
      scrollView->setResizePolicy(QScrollView::AutoOneFit);
      scrollView->setSizePolicy(ignored);
      m_ControlStack->addWidget(scrollView, m_NumberOfFunctionalities);
    }
    else
    {
      m_ControlStack->addWidget(new QWidget(m_ControlStack,
        "QmitkFctMediator::dummyControl"), m_NumberOfFunctionalities);
    }
  }

  // Append widget at end of list (same position as in functionality in
  // m_Functionalities)
  m_DialogBarControls.append( controlWidget );

  if (m_NumberOfFunctionalities==0)
  {
    if (action!=NULL)
    {
      action->setOn(true);
    }
    if (m_ControlStack!=NULL)
    {
      m_ControlStack->raiseWidget(0);
    }
    if ((m_MainStack!=NULL) && (strcmp(
      m_MainStack->widget(0+1)->name(),"QmitkFctMediator::dummyMain")!=0))
    {
      m_MainStack->raiseWidget(0+1);
    }
  }

  functionality->CreateConnections();

  ++m_NumberOfFunctionalities;

  return true;
}

bool QmitkFctMediator::AddDialogBar( QmitkDialogBar *dialogBar )
{
  if ( dialogBar == NULL )
  {
    return true;
  }

  // Add dialog bar to the control pane and its icon to the toolbar

  m_DialogBars.append( dialogBar );

  QAction *action = dialogBar->CreateAction( this );
  if ( action != NULL )
  {
    action->setToggleAction( true );
    if ( m_ToolBar != NULL )
    {
      action->addTo( m_ToolBar );
    }
    connect( action, SIGNAL( toggled( bool ) ) , dialogBar, SLOT( ToggleVisible( bool ) ) );
  }
  m_DialogBarActions.append( action );

  if ( m_DialogBarsFrame != NULL )
  {
    QWidget *controlWidget = dialogBar->CreateControlWidget( m_DialogBarsFrame );
  }

  dialogBar->CreateConnections();


  ++m_NumberOfDialogBars;

  return true;
}

void QmitkFctMediator::AddSeparator()
{
  m_ToolBar->addSeparator();
}

QWidget *QmitkFctMediator::GetControlParent()
{
  return m_ControlStack;
}

QWidget *QmitkFctMediator::GetMainParent()
{
  return m_MainStack;
}

QWidget *QmitkFctMediator::GetToolBar()
{
  return m_ToolBar;
}

QWidget *QmitkFctMediator::GetDefaultMain()
{
  return m_DefaultMain;
}

QmitkFunctionality *QmitkFctMediator::GetFunctionalityByName(const char *name)
{
  QmitkFunctionality *functionality;
  for ( functionality = m_Functionalities.first(); 
        functionality != 0; 
        functionality = m_Functionalities.next() )
  {
    if (strcmp(functionality->GetFunctionalityName().ascii(),name)==0)
    {
      return functionality;
    }
  }
  return NULL;
}

int QmitkFctMediator::GetFunctionalityIdByName( const char *name ) 
{
  QmitkFunctionality *functionality;
  int id = 0;

  for ( functionality = m_Functionalities.first();
        functionality != NULL;
      functionality = m_Functionalities.next(), id++ ) 
  {
    if (strcmp(functionality->GetFunctionalityName().ascii(),name)==0)
    {
      return id;
    }
  }

  return -1;
}

void QmitkFctMediator::RaiseFunctionality( int id )
{
  QAction *action;
  action = m_FunctionalityActions.at( id );
  if ( (action!=NULL) && (action->isOn()==false) )
  {
    action->setOn( true);
    return; // we will come into this method again ...
  }

  // If a new functionality has been selected, deactivate the currently
  // active functionality
  if (id != m_CurrentFunctionality)
  {    
    if ( m_Functionalities.at( m_CurrentFunctionality ) ) 
    {
      m_Functionalities.at( m_CurrentFunctionality )->Deactivated();
      if ( m_Functionalities.at( m_CurrentFunctionality )->m_Activated )
      {
        // Output error message if deactivation was not successful
        itkGenericOutputMacro(<<"Method Deactivated() of functionality '"
          <<m_Functionalities.at(m_CurrentFunctionality)->GetFunctionalityName().latin1()
          << "' did not call QmitkFunctionality::Deactivated().");
        m_Functionalities.at( m_CurrentFunctionality )->m_Activated = false;
      }
    }
  }

  QWidget *newVisibleWidget, *oldVisibleWidget;
  oldVisibleWidget = m_ControlStack->visibleWidget();
  newVisibleWidget = m_ControlStack->widget(id);
  if((oldVisibleWidget!=NULL) && (oldVisibleWidget!=newVisibleWidget))
  {
    oldVisibleWidget->setSizePolicy(ignored);
  }
  newVisibleWidget->setSizePolicy(ignored);
  m_ControlStack->raiseWidget(newVisibleWidget);
  m_ControlStack->updateGeometry();
  
  newVisibleWidget = m_MainStack->widget(id+1);
  if(strcmp(newVisibleWidget->name(),"QmitkFctMediator::dummyMain")==0)
    newVisibleWidget = m_MainStack->widget(0);
  
  m_MainStack->raiseWidget(newVisibleWidget);
  
  // Set new functionality as active
  m_CurrentFunctionality = id;
  m_Functionalities.at( m_CurrentFunctionality )->Activated();
  if ( m_Functionalities.at( m_CurrentFunctionality )->m_Activated == false )
  {
    // Output error message if activation was not successful
    itkGenericOutputMacro(<<"Method Activated() of functionality '"
      <<m_Functionalities.at(m_CurrentFunctionality)->GetFunctionalityName().latin1()
      << "' did not call QmitkFunctionality::Activated(). TreeChanged() will not work!!");
    m_Functionalities.at(m_CurrentFunctionality)->m_Activated = true;
  }
}

void QmitkFctMediator::RaiseFunctionality( QAction *action )
{
  int id = m_FunctionalityActions.find(action);
  if( id >= 0 )
  {
    this->RaiseFunctionality( id );
  }
}

void QmitkFctMediator::RaiseFunctionality( QmitkFunctionality *aFunctionality )
{
  QmitkFunctionality *functionality;
  int id = 0;
  for ( functionality = m_Functionalities.first(); 
        functionality != 0; 
        functionality = m_Functionalities.next(), ++id )
  {
    if ( functionality == aFunctionality )
    {
      this->RaiseFunctionality( id );
      return;
    }
  }
}

void QmitkFctMediator::EnableDialogBar( int id, bool enable )
{
  QAction *action = m_DialogBarActions.at( id );

  if ( (action != NULL) && (action->isOn() == false) )
  {
    action->setOn( true );
    return; // we will come into this method again ...
  }

  QWidget *dialogBarControl = m_DialogBarControls.at( id );
  dialogBarControl->setHidden( !enable );
}

void QmitkFctMediator::EnableDialogBar( QmitkDialogBar *aDialogBar, bool enable )
{
  QmitkDialogBar *dialogBar;
  int id = 0;
  for ( dialogBar = m_DialogBars.first(); 
        dialogBar != 0; 
        dialogBar = m_DialogBars.next(), ++id )
  {
    if ( dialogBar == aDialogBar )
    {
      this->EnableDialogBar( id, enable );
      return;
    }
  }
}

void QmitkFctMediator::ToggleDialogBar( QAction *action )
{
  int id = m_DialogBarActions.find( action );
  if( id >= 0 )
  {
    this->EnableDialogBar( id, false );
  }
}

QmitkDialogBar *
QmitkFctMediator
::GetDialogBarByName( const char *name )
{
  QmitkDialogBar *dialogBar;
  for ( dialogBar = m_DialogBars.first(); 
        dialogBar != 0; 
        dialogBar = m_DialogBars.next() )
  {
    if (strcmp(dialogBar->GetFunctionalityName().ascii(),name)==0)
    {
      return dialogBar;
    }
  }
  return NULL;
}

QmitkDialogBar *
QmitkFctMediator
::GetDialogBarById( int id )
{
  return m_DialogBars.at( id );
}

int 
QmitkFctMediator
::GetDialogBarIdByName( const char *name )
{
  QmitkDialogBar *dialogBar;
  int id = 0;

  for ( dialogBar = m_DialogBars.first();
        dialogBar != NULL;
      dialogBar = m_DialogBars.next(), id++ ) 
  {
    if (strcmp(dialogBar->GetFunctionalityName().ascii(),name)==0)
    {
      return id;
    }
  }
  return -1;
}


void QmitkFctMediator::HideControls(bool hide)
{
  if(m_ControlStack==NULL) return;

  QWidget *controlStackParent = 
    dynamic_cast<QWidget*>(m_ControlStack->parent());

  if(controlStackParent==NULL) return;

  if(hide)
    controlStackParent->hide();
  else
    controlStackParent->show();
}

int QmitkFctMediator::GetActiveFunctionalityId() const
{
  return m_CurrentFunctionality;
}

QmitkFunctionality *QmitkFctMediator::GetActiveFunctionality()
{
  return m_Functionalities.at(m_CurrentFunctionality);
}

QmitkFunctionality *QmitkFctMediator::GetFunctionalityById(int id)
{
  return m_Functionalities.at(id);
}

unsigned int QmitkFctMediator::GetFunctionalityCount()
{  
  return m_NumberOfFunctionalities;
}

unsigned int QmitkFctMediator::GetDialogBarCount()
{  
  return m_NumberOfDialogBars;
}


void 
QmitkFctMediator
::ApplyOptionsToDialogBars( mitk::PropertyList::Pointer options )
{
  m_Options = options;

  if ( m_Options.IsNotNull() )
  {

    int i;
    for ( i = 0; i < m_NumberOfDialogBars; ++i )
    {
      QmitkDialogBar *dialogBar = m_DialogBars.at( i );
      QAction *action = m_DialogBarActions.at( i );
      
      // Pass global preferences to DialogBar so that it can manage the
      // persistancy of its state
      dialogBar->SetGlobalOptions( m_Options );

      // Read (or create) application property for this dialog bar; enable or
      // disable it accordingly; default is disabled
      QString dialogBarStateName = 
        "DialogBar " + dialogBar->GetCaption() + " active";

      mitk::BoolProperty *dialogBarState = 
        dynamic_cast< mitk::BoolProperty * >( 
          m_Options->GetProperty( dialogBarStateName.ascii() ).GetPointer() );          

      bool enableDialogBar;
      if ( dialogBarState != NULL )
      {
        enableDialogBar = dialogBarState->GetValue();
      }
      else
      {
        m_Options->SetProperty( dialogBarStateName.ascii(), new mitk::BoolProperty( false ) );
        enableDialogBar = false;
      }

      if ( action != NULL )
      {
        dialogBar->ToggleVisible( enableDialogBar );
        action->setOn( enableDialogBar );
      }
    }
  }
}
