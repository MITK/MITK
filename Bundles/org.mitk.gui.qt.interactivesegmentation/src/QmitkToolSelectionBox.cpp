/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-10-30 12:33:31 +0100 (Do, 30 Okt 2008) $
Version:   $Revision: 15606 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkToolSelectionBox.h"
#include "QmitkToolGUI.h"

//!mm
//#added:
#include "cherryPlatform.h"
#include "cherryPlatformException.h"
#include "service/cherryIExtensionPointService.h"
//using namespace cherry;
//!

#include <qtoolbutton.h>
#include <QList>
#include <qtooltip.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <qapplication.h>

QmitkToolSelectionBox::QmitkToolSelectionBox(QWidget* parent)
:QGroupBox("Tools", parent),
 m_SelfCall(false),
 m_Enabled(true),
 m_DisplayedGroups("default"),
 m_LayoutColumns(2),
 m_ShowNames(true),
 m_GenerateAccelerators(false),
 m_ToolGUIWidget(NULL),
 m_LastToolGUI(NULL),
 m_ToolButtonGroup(0),
 m_ButtonLayout(0),
 m_EnabledMode(EnabledWithReferenceAndWorkingData)
{
  QFont currentFont = QGroupBox::font();
  currentFont.setBold(true);
  QGroupBox::setFont( currentFont );

  m_ToolManager = mitk::ToolManager::New();

  // muellerm
  // QButtonGroup
  m_ToolButtonGroup = new QButtonGroup;
  // some features of QButtonGroup
  m_ToolButtonGroup->setExclusive( false ); // mutually exclusive toggle buttons

  RecreateButtons();

  // reactions to signals
  connect( m_ToolButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(toolButtonClicked(int)) );

  // reactions to ToolManager events

  m_ToolManager->ActiveToolChanged += mitk::MessageDelegate<QmitkToolSelectionBox>( this, &QmitkToolSelectionBox::OnToolManagerToolModified );
  m_ToolManager->ReferenceDataChanged += mitk::MessageDelegate<QmitkToolSelectionBox>( this, &QmitkToolSelectionBox::OnToolManagerReferenceDataModified );
  m_ToolManager->WorkingDataChanged += mitk::MessageDelegate<QmitkToolSelectionBox>( this, &QmitkToolSelectionBox::OnToolManagerWorkingDataModified );

  // show active tool
  SetOrUnsetButtonForActiveTool();

  QGroupBox::setEnabled( false );
}

QmitkToolSelectionBox::~QmitkToolSelectionBox()
{
  m_ToolManager->ActiveToolChanged -= mitk::MessageDelegate<QmitkToolSelectionBox>( this, &QmitkToolSelectionBox::OnToolManagerToolModified );
  m_ToolManager->ReferenceDataChanged -= mitk::MessageDelegate<QmitkToolSelectionBox>( this, &QmitkToolSelectionBox::OnToolManagerReferenceDataModified );
  m_ToolManager->WorkingDataChanged -= mitk::MessageDelegate<QmitkToolSelectionBox>( this, &QmitkToolSelectionBox::OnToolManagerWorkingDataModified );

}

void QmitkToolSelectionBox::SetEnabledMode(EnabledMode mode)
{
  m_EnabledMode = mode;
  SetGUIEnabledAccordingToToolManagerState();
}

mitk::ToolManager* QmitkToolSelectionBox::GetToolManager()
{
  return m_ToolManager;
}

void QmitkToolSelectionBox::SetToolManager(mitk::ToolManager& newManager) // no NULL pointer allowed here, a manager is required
{
  // say bye to the old manager
  m_ToolManager->ActiveToolChanged -= mitk::MessageDelegate<QmitkToolSelectionBox>( this, &QmitkToolSelectionBox::OnToolManagerToolModified );
  m_ToolManager->ReferenceDataChanged -= mitk::MessageDelegate<QmitkToolSelectionBox>( this, &QmitkToolSelectionBox::OnToolManagerReferenceDataModified );
  m_ToolManager->WorkingDataChanged -= mitk::MessageDelegate<QmitkToolSelectionBox>( this, &QmitkToolSelectionBox::OnToolManagerWorkingDataModified );

  if ( QGroupBox::isEnabled() )
  {
    m_ToolManager->UnregisterClient();
  }

  m_ToolManager = &newManager;
  RecreateButtons();

  // greet the new one
  m_ToolManager->ActiveToolChanged += mitk::MessageDelegate<QmitkToolSelectionBox>( this, &QmitkToolSelectionBox::OnToolManagerToolModified );
  m_ToolManager->ReferenceDataChanged += mitk::MessageDelegate<QmitkToolSelectionBox>( this, &QmitkToolSelectionBox::OnToolManagerReferenceDataModified );
  m_ToolManager->WorkingDataChanged += mitk::MessageDelegate<QmitkToolSelectionBox>( this, &QmitkToolSelectionBox::OnToolManagerWorkingDataModified );

  if ( QGroupBox::isEnabled() )
  {
    m_ToolManager->RegisterClient();
  }

  // ask the new one what the situation is like
  SetOrUnsetButtonForActiveTool();
}

void QmitkToolSelectionBox::toolButtonClicked(int id)
{
  if ( !QGroupBox::isEnabled() ) return; // this method could be triggered from the constructor, when we are still disabled

  //QToolButton* toolButton = dynamic_cast<QToolButton*>( Q3ButtonGroup::find(id) );
  QToolButton* toolButton = dynamic_cast<QToolButton*>( m_ToolButtonGroup->buttons().at(id) );
  if (toolButton)
  {
    if (    (m_ButtonIDForToolID.find( m_ToolManager->GetActiveToolID() ) != m_ButtonIDForToolID.end()) // if we have this tool in our box
         && (m_ButtonIDForToolID[ m_ToolManager->GetActiveToolID() ] == id) ) // the tool corresponding to this button is already active
    {
      // disable this button, disable all tools
      // mmueller
      toolButton->setChecked(false);
      m_ToolManager->ActivateTool(-1); // disable everything
    }
    else
    {
      // enable the corresponding tool
      m_SelfCall = true;

      m_ToolManager->ActivateTool( m_ToolIDForButtonID[id] );

      m_SelfCall = false;
    }
  }

}

void QmitkToolSelectionBox::OnToolManagerToolModified()
{
  SetOrUnsetButtonForActiveTool();
}

void QmitkToolSelectionBox::SetOrUnsetButtonForActiveTool()
{
  // we want to emit a signal in any case, whether we selected ourselves or somebody else changes "our" tool manager. --> emit before check on m_SelfCall
  int id = m_ToolManager->GetActiveToolID();
  emit ToolSelected(id);

  // delete old GUI (if any)
  if ( m_LastToolGUI && m_ToolGUIWidget )
  {
    if (m_ToolGUIWidget->layout())
    {
      m_ToolGUIWidget->layout()->removeWidget(m_LastToolGUI);
    }


    //m_LastToolGUI->reparent(NULL, QPoint(0,0));
    // TODO: reparent <-> setParent, Daniel fragen
    m_LastToolGUI->setParent(0);
    delete m_LastToolGUI; // will hopefully notify parent and layouts
    m_LastToolGUI = NULL;

    QLayout* layout = m_ToolGUIWidget->layout();
    if (layout)
    {
      layout->activate();
    }
  }

  QToolButton* toolButton(NULL);
  mitk::Tool* tool = m_ToolManager->GetActiveTool();

  if (m_ButtonIDForToolID.find(id) != m_ButtonIDForToolID.end()) // if this tool is in our box
  {
    //toolButton = dynamic_cast<QToolButton*>( Q3ButtonGroup::find( m_ButtonIDForToolID[id] ) );
    toolButton = dynamic_cast<QToolButton*>( m_ToolButtonGroup->buttons().at( m_ButtonIDForToolID[id] ) );
  }

  if ( toolButton )
  {
    // mmueller
    // uncheck all other buttons
    QAbstractButton* tmpBtn = 0;
    QList<QAbstractButton*>::iterator it;
    for(unsigned int i=0; i<m_ToolButtonGroup->buttons().size();
      i++)
    {
      tmpBtn = m_ToolButtonGroup->buttons().at(i);
      if(tmpBtn != toolButton)
        dynamic_cast<QToolButton*>( tmpBtn )->setChecked(false);
    }

    toolButton->setChecked(true);
    
    if (m_ToolGUIWidget && tool)
    {
      // create and reparent new GUI (if any)
      //!mm
      //itk::Object::Pointer possibleGUI = tool->GetGUI("Qmitk", "GUI").GetPointer(); // prefix and postfix
      //QmitkToolGUI* gui = dynamic_cast<QmitkToolGUI*>( possibleGUI.GetPointer() );
      //#changed:
      cherry::IExtensionPointService* service = cherry::Platform::GetExtensionPointService();
      cherry::IConfigurationElement::vector ces(
        service->GetConfigurationElementsFor("org.mitk.gui.qt.interactivesegmentation.qmitktoolguis"));

      // name of gui class
      std::string nameOfPossibleGUI = "Qmitk";
      nameOfPossibleGUI.append(tool->GetNameOfClass());
      nameOfPossibleGUI.append("GUI");
      QmitkToolGUI* gui = 0;
      for (cherry::IConfigurationElement::vector::iterator i= ces.begin(); i != ces.end(); ++i)
      {
        std::string cid;
        if ((*i)->GetAttribute("id", cid))
        {
          if (cid == nameOfPossibleGUI)
          {
            gui = (*i)->CreateExecutableExtension<QmitkToolGUI>("class");
            break;
          }
        }
      }
      //!
      m_LastToolGUI = gui;
      if (gui)
      {
        gui->SetTool( tool );

        // mmueller
        //gui->reparent(m_ToolGUIWidget, gui->geometry().topLeft(), true );
        gui->setParent(m_ToolGUIWidget);
        gui->move(gui->geometry().topLeft());
        gui->show();

        QLayout* layout = m_ToolGUIWidget->layout();
        if (!layout)
        {
          layout = new QVBoxLayout( m_ToolGUIWidget );
        }
        if (layout)
        {
          // mmueller
          layout->addWidget( gui );
          //layout->add( gui );
          layout->activate();
        }
      }
    }
  }
  else
  {
    // disable all buttons
    QToolButton* selectedToolButton = dynamic_cast<QToolButton*>( m_ToolButtonGroup->checkedButton() );
    //QToolButton* selectedToolButton = dynamic_cast<QToolButton*>( Q3ButtonGroup::find( Q3ButtonGroup::selectedId() ) );
    if (selectedToolButton)
    {
      // mmueller
      selectedToolButton->setChecked(false);
      //selectedToolButton->setOn(false);
    }
  }
}

void QmitkToolSelectionBox::OnToolManagerReferenceDataModified()
{
  if (m_SelfCall) return;

  SetGUIEnabledAccordingToToolManagerState();
}

void QmitkToolSelectionBox::OnToolManagerWorkingDataModified()
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

  bool enabled = true;
  switch ( m_EnabledMode )
  {
    default:
    case EnabledWithReferenceAndWorkingData:
      enabled = referenceNode && workingNode && m_Enabled;
      break;
    case EnabledWithReferenceData:
      enabled = referenceNode && m_Enabled;
      break;
    case EnabledWithWorkingData:
      enabled = workingNode && m_Enabled;
      break;
    case AlwaysEnabled:
      enabled = m_Enabled;
      break;
  }

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

  /*
  // remove all buttons that are there
  QObjectList *l = Q3ButtonGroup::queryList( "QButton" );
  QObjectListIt it( *l ); // iterate over all buttons
  QObject *obj;

  while ( (obj = it.current()) != 0 )
  {
    ++it;
    QButton* button = dynamic_cast<QButton*>(obj);
    if (button)
    {
      Q3ButtonGroup::remove(button);
      delete button;
    }
  }
  delete l; // delete the list, not the objects
  */

  // mmueller Qt4 impl
  QList<QAbstractButton *> l = m_ToolButtonGroup->buttons();
  // remove all buttons that are there
  QList<QAbstractButton *>::iterator it;
  QAbstractButton * btn;

  for(it=l.begin(); it!=l.end();++it)
  {
    btn = *it;
    m_ToolButtonGroup->removeButton(btn);
    //this->removeChild(btn);
    delete btn;
  }
  // end mmueller Qt4 impl

  const mitk::ToolManager::ToolVectorTypeConst allTools = m_ToolManager->GetTools();

  // try to change layout... bad?
  //Q3GroupBox::setColumnLayout ( m_LayoutColumns, Qt::Horizontal );
  // mmueller using gridlayout instead of Q3GroupBox
  //this->setLayout(0);
  if(m_ButtonLayout == 0)
    m_ButtonLayout = new QGridLayout;
  /*else
    delete m_ButtonLayout;*/

  int row(0);
  int column(-1);

  int currentButtonID(0);
  int currentToolID(0);
  m_ButtonIDForToolID.clear();
  m_ToolIDForButtonID.clear();
  QToolButton* button = 0;

  // fill group box with buttons
  for ( mitk::ToolManager::ToolVectorTypeConst::const_iterator iter = allTools.begin();
        iter != allTools.end();
        ++iter)
  {
    const mitk::Tool* tool = *iter;

    if ( (m_DisplayedGroups.empty()) || ( m_DisplayedGroups.find( tool->GetGroup() ) != std::string::npos ) ||
                                        ( m_DisplayedGroups.find( tool->GetName() ) != std::string::npos )
       )
    {
      ++column;
      // new line if we are at the maximum columns
      if(column == m_LayoutColumns)
      {        
        ++row;
        column = 0;
      }

      button = new QToolButton;
      button->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
      // add new button to the group
      m_ToolButtonGroup->addButton(button, currentButtonID);
      // ... and to the layout
      m_ButtonLayout->addWidget(button, row, column);

      if (m_LayoutColumns == 1)
      {
        //button->setTextPosition( QToolButton::BesideIcon );
        // mmueller
        button->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
      }
      else
      {
        //button->setTextPosition( QToolButton::BelowIcon );
        // mmueller
        button->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
      }

      //button->setToggleButton( true );
      // mmueller
      button->setCheckable ( true );

      QString label;
      if (m_GenerateAccelerators)
      {
        label += "&";
      }
      label += tool->GetName();
      QString tooltip = tool->GetName();

      if ( m_ShowNames )
      {
        /*
        button->setUsesTextLabel(true);
        button->setTextLabel( label );              // a label
        QToolTip::add( button, tooltip );
        */
        // mmueller Qt4
        button->setText( label );              // a label
        button->setToolTip( tooltip );
        // mmueller

        QFont currentFont = button->font();
        currentFont.setBold(false);
        button->setFont( currentFont );
      }

      //button->setPixmap( QPixmap( tool->GetXPM() ) );       // an icon
      // mmueller
      button->setIcon( QIcon( QPixmap( tool->GetXPM() ) ) );       // an icon

      if (m_GenerateAccelerators)
      {
        QString firstLetter = QString( tool->GetName() );
        firstLetter.truncate( 1 );
        button->setShortcut( firstLetter );                      // a keyboard shortcut (just the first letter of the given name w/o any CTRL or something)
      }

      m_ButtonIDForToolID[currentToolID] = currentButtonID;
      m_ToolIDForButtonID[currentButtonID] = currentToolID;

      tool->GUIProcessEventsMessage += mitk::MessageDelegate<QmitkToolSelectionBox>( this, &QmitkToolSelectionBox::OnToolGUIProcessEventsMessage ); // will never add a listener twice, so we don't have to check here
      tool->ErrorMessage += mitk::MessageDelegate1<QmitkToolSelectionBox, std::string>( this, &QmitkToolSelectionBox::OnToolErrorMessage ); // will never add a listener twice, so we don't have to check here
      tool->GeneralMessage += mitk::MessageDelegate1<QmitkToolSelectionBox, std::string>( this, &QmitkToolSelectionBox::OnGeneralToolMessage );

      ++currentButtonID;
    }

    ++currentToolID;
  }
  // setting grid layout for this groupbox
  this->setLayout(m_ButtonLayout);

  //this->update();
}

void QmitkToolSelectionBox::OnToolGUIProcessEventsMessage()
{
  qApp->processEvents();
}

void QmitkToolSelectionBox::OnToolErrorMessage(std::string s)
{
  QMessageBox::critical(NULL, "MITK", QString( s.c_str() ), QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
}

void QmitkToolSelectionBox::OnGeneralToolMessage(std::string s)
{
  QMessageBox::information(NULL, "MITK", QString( s.c_str() ), QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
}


void QmitkToolSelectionBox::SetDisplayedToolGroups(const std::string& toolGroups)
{
  if (m_DisplayedGroups != toolGroups)
  {
    m_DisplayedGroups = toolGroups;
    RecreateButtons();
    SetOrUnsetButtonForActiveTool();
  }
}

void QmitkToolSelectionBox::SetLayoutColumns(int columns)
{
  if (columns > 0 && columns != m_LayoutColumns)
  {
    m_LayoutColumns = columns;
    RecreateButtons();
  }
}

void QmitkToolSelectionBox::SetShowNames(bool show)
{
  if (show != m_ShowNames)
  {
    m_ShowNames = show;
    RecreateButtons();
  }
}


void QmitkToolSelectionBox::SetGenerateAccelerators(bool accel)
{
  if (accel != m_GenerateAccelerators)
  {
    m_GenerateAccelerators = accel;
    RecreateButtons();
  }
}


void QmitkToolSelectionBox::SetToolGUIArea( QWidget* parentWidget )
{
  m_ToolGUIWidget = parentWidget;
}

