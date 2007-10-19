#include "QmitkToolSelectionBox.h" 
#include <qtoolbutton.h>
#include <qobjectlist.h>

#include <itkCommand.h>

QmitkToolSelectionBox::QmitkToolSelectionBox(QWidget* parent, const char* name)
:QButtonGroup("Tools", parent, name),
 m_SelfCall(false),
 m_Enabled(true),
 m_AlwaysEnabledOverride(false)
{
  QFont currentFont = QButtonGroup::font();
  currentFont.setBold(true);
  QButtonGroup::setFont( currentFont );

  m_ToolManager = mitk::ToolManager::New();

  // some features of QButtonGroup
  QButtonGroup::setExclusive( true ); // mutually exclusive toggle buttons
  
  RecreateButtons();
  // reactions to signals

  connect( this, SIGNAL(clicked(int)), this, SLOT(toolButtonClicked(int)) );

  // reactions to ToolManager events
  {
  itk::ReceptorMemberCommand<QmitkToolSelectionBox>::Pointer command = itk::ReceptorMemberCommand<QmitkToolSelectionBox>::New();
  command->SetCallbackFunction( this, &QmitkToolSelectionBox::OnToolManagerToolModified );
  m_ToolSelectedObserverTag = m_ToolManager->AddObserver( mitk::ToolSelectedEvent(), command );
  }

  {
  itk::ReceptorMemberCommand<QmitkToolSelectionBox>::Pointer command = itk::ReceptorMemberCommand<QmitkToolSelectionBox>::New();
  command->SetCallbackFunction( this, &QmitkToolSelectionBox::OnToolManagerReferenceDataModified );
  m_ToolReferenceDataChangedObserverTag = m_ToolManager->AddObserver( mitk::ToolReferenceDataChangedEvent(), command );
  }

  {
  itk::ReceptorMemberCommand<QmitkToolSelectionBox>::Pointer command = itk::ReceptorMemberCommand<QmitkToolSelectionBox>::New();
  command->SetCallbackFunction( this, &QmitkToolSelectionBox::OnToolManagerWorkingDataModified );
  m_ToolWorkingDataChangedObserverTag = m_ToolManager->AddObserver( mitk::ToolWorkingDataChangedEvent(), command );
  }

  // show active tool
  int id = m_ToolManager->GetActiveToolID();
  QToolButton* firstButton = dynamic_cast<QToolButton*>( QButtonGroup::find(id) );
  if (firstButton)
  {
    firstButton->setOn(true);
  }
  
  QButtonGroup::setEnabled( false ); 
}

QmitkToolSelectionBox::~QmitkToolSelectionBox()
{
}

void QmitkToolSelectionBox::SetAlwaysEnabled(bool alwaysEnabled)
{
  m_AlwaysEnabledOverride = alwaysEnabled;
  SetGUIEnabledAccordingToToolManagerState();
}

mitk::ToolManager* QmitkToolSelectionBox::GetToolManager()
{
  return m_ToolManager;
}

void QmitkToolSelectionBox::SetToolManager(mitk::ToolManager& newManager) // no NULL pointer allowed here, a manager is required
{
  // say bye to the old manager
  m_ToolManager->RemoveObserver( m_ToolSelectedObserverTag );
  m_ToolManager->RemoveObserver( m_ToolReferenceDataChangedObserverTag );
  m_ToolManager->RemoveObserver( m_ToolWorkingDataChangedObserverTag );

  if ( QGroupBox::isEnabled() ) 
  {
    m_ToolManager->UnregisterClient();
  }

  m_ToolManager = &newManager;
  RecreateButtons();

  // greet the new one
  {
  itk::ReceptorMemberCommand<QmitkToolSelectionBox>::Pointer command = itk::ReceptorMemberCommand<QmitkToolSelectionBox>::New();
  command->SetCallbackFunction( this, &QmitkToolSelectionBox::OnToolManagerToolModified );
  m_ToolManager->AddObserver( mitk::ToolSelectedEvent(), command );
  }

  {
  itk::ReceptorMemberCommand<QmitkToolSelectionBox>::Pointer command = itk::ReceptorMemberCommand<QmitkToolSelectionBox>::New();
  command->SetCallbackFunction( this, &QmitkToolSelectionBox::OnToolManagerReferenceDataModified );
  m_ToolManager->AddObserver( mitk::ToolReferenceDataChangedEvent(), command );
  }

  {
  itk::ReceptorMemberCommand<QmitkToolSelectionBox>::Pointer command = itk::ReceptorMemberCommand<QmitkToolSelectionBox>::New();
  command->SetCallbackFunction( this, &QmitkToolSelectionBox::OnToolManagerWorkingDataModified );
  m_ToolWorkingDataChangedObserverTag = m_ToolManager->AddObserver( mitk::ToolWorkingDataChangedEvent(), command );
  }

  if ( QGroupBox::isEnabled() ) 
  {
    m_ToolManager->RegisterClient();
  }

  // ask the new one what the situation is like
  int id = m_ToolManager->GetActiveToolID();
  QToolButton* firstButton = dynamic_cast<QToolButton*>( QButtonGroup::find(id) );
  if (firstButton)
  {
    firstButton->setOn(true);
  }
}
  
void QmitkToolSelectionBox::toolButtonClicked(int id)
{
  if ( !QGroupBox::isEnabled() ) return; // this method could be triggered from the constructor, when we are still disabled

  QToolButton* toolButton = dynamic_cast<QToolButton*>( QButtonGroup::find(id) );
if (toolButton)
{
  if ( m_ToolManager->GetActiveToolID() == id ) // the tool corresponding to this button is already active
  {
    // disable this button, disable all tools
    toolButton->setOn(false);
    m_ToolManager->ActivateTool(-1); // disable everything
  }
  else
  {
    // enable the corresponding tool
    m_SelfCall = true;

    m_ToolManager->ActivateTool( id );
    
    m_SelfCall = false;
  }
}

}

void QmitkToolSelectionBox::OnToolManagerToolModified(const itk::EventObject&)
{
  // we want to emit a signal in any case, whether we selected ourselves or somebody else changes "our" tool manager. --> emit before check on m_SelfCall
  int id = m_ToolManager->GetActiveToolID();
  emit ToolSelected(id);

  if (m_SelfCall) return;

  if (id != -1)
  {
    // enable the correct button
    QToolButton* toolButton = dynamic_cast<QToolButton*>( QButtonGroup::find(id) );
    if (toolButton)
    {
      toolButton->setOn(true);
    }
  }
  else
  {
    // disable all buttons
    QToolButton* toolButton = dynamic_cast<QToolButton*>( QButtonGroup::find( QButtonGroup::selectedId() ) );
    if (toolButton)
    {
      toolButton->setOn(false);
    }
  }
}

void QmitkToolSelectionBox::OnToolManagerReferenceDataModified(const itk::EventObject&)
{
  if (m_SelfCall) return;

  SetGUIEnabledAccordingToToolManagerState();
}

void QmitkToolSelectionBox::OnToolManagerWorkingDataModified(const itk::EventObject&)
{
  if (m_SelfCall) return;

  SetGUIEnabledAccordingToToolManagerState();
}

/**
 Implementes the logic, which decides, when tools are activated/deactivated.

 Tools are activated when
   - a reference and a working image is available
   - some GUI element called setEnabled(true)
  or
   - some GUI element called SetAlwaysEnabled(true), which might
     be done when some tools do not need e.g. a working image
*/
void QmitkToolSelectionBox::SetGUIEnabledAccordingToToolManagerState()
{
  mitk::DataTreeNode* referenceNode = m_ToolManager->GetReferenceData(0);
  mitk::DataTreeNode* workingNode = m_ToolManager->GetWorkingData(0);

  bool enabled = referenceNode && workingNode && m_Enabled || m_AlwaysEnabledOverride;
  
  if ( QGroupBox::isEnabled() == enabled ) return; // nothing to change
  
  QGroupBox::setEnabled( enabled );
  if (enabled)
  {
    m_ToolManager->RegisterClient();

    int id = m_ToolManager->GetActiveToolID();
    emit ToolSelected(id);
  }
  else
  {
    m_ToolManager->UnregisterClient();
    
    emit ToolSelected(-1);
  }

}
   
/**
 External enableization...
*/
void QmitkToolSelectionBox::setEnabled( bool enable )
{
  m_Enabled = enable;

  SetGUIEnabledAccordingToToolManagerState();
}
    

void QmitkToolSelectionBox::RecreateButtons()
{
  if (m_ToolManager.IsNull()) return;

  // remove all buttons that are there
  QObjectList *l = QButtonGroup::queryList( "QButton" );
  QObjectListIt it( *l ); // iterate over all buttons
  QObject *obj;

  while ( (obj = it.current()) != 0 ) 
  {
    ++it;
    QButton* button = dynamic_cast<QButton*>(obj);
    if (button)
    {
      QButtonGroup::remove(button);
      delete button;
    }
  }
  delete l; // delete the list, not the objects

  const mitk::ToolManager::ToolVectorTypeConst allTools = m_ToolManager->GetTools();

  // try to change layout... bad?
  if (allTools.size() == 1)
  {
    QGroupBox::setColumnLayout ( 1, Qt::Horizontal );
  }
  else
  {
    QGroupBox::setColumnLayout ( 2, Qt::Horizontal );
  }

  // fill group box with buttons
  for ( mitk::ToolManager::ToolVectorTypeConst::const_iterator iter = allTools.begin();
        iter != allTools.end();
        ++iter )
  {
    const mitk::Tool* tool = *iter;

    QToolButton* button = new QToolButton(this);
    button->setUsesTextLabel(true);
    button->setTextPosition( QToolButton::BelowIcon );
    button->setToggleButton( true );
    QString label( "&" );
    label += tool->GetName();
    button->setTextLabel( label );              // a label
    button->setPixmap( QPixmap( tool->GetXPM() ) );       // an icon
    QString firstLetter = QString( tool->GetName() );
    firstLetter.truncate( 1 );
    button->setAccel( firstLetter );                      // a keyboard shortcut (just the first letter of the given name w/o any CTRL or something)

    QFont currentFont = button->font();
    currentFont.setBold(false);
    button->setFont( currentFont );
  }
}

