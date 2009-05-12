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

#include "@FUNCTIONALITY_NAME@.h"
#include "@FUNCTIONALITY_NAME@Controls.h"
#include <qaction.h>
#include "icon.xpm"
#include "QmitkTreeNodeSelector.h"
#include "QmitkStdMultiWidget.h"
#include "mitkStatusBar.h"
#include "mitkProgressBar.h"


@FUNCTIONALITY_NAME@::@FUNCTIONALITY_NAME@(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
    : QmitkFunctionality(parent, name, it), m_MultiWidget(mitkStdMultiWidget), m_Controls(NULL)
{
  SetAvailability(true);
}


@FUNCTIONALITY_NAME@::~@FUNCTIONALITY_NAME@()
{}


QWidget * @FUNCTIONALITY_NAME@::CreateMainWidget(QWidget *parent)
{
  if ( m_MultiWidget == NULL )
  {
    m_MultiWidget = new QmitkStdMultiWidget( parent );
  }
  return m_MultiWidget;
}


QWidget * @FUNCTIONALITY_NAME@::CreateControlWidget(QWidget *parent)
{
  if (m_Controls == NULL)
  {
    m_Controls = new @FUNCTIONALITY_NAME@Controls(parent);
  }
  return m_Controls;
}


void @FUNCTIONALITY_NAME@::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_TreeNodeSelector), SIGNAL(Activated(mitk::DataTreeIteratorClone)),(QObject*) this, SLOT(ImageSelected(mitk::DataTreeIteratorClone)) );
    connect( (QObject*)(m_Controls->m_StartButton), SIGNAL(clicked()),(QObject*) this, SLOT(StartButtonClicked()));
  }
}


QAction * @FUNCTIONALITY_NAME@::CreateAction(QActionGroup *parent)
{
  QAction* action;
  action = new QAction( tr( "@FUNCTIONALITY_NAME@ description" ), QPixmap((const char**)icon_xpm), tr( "@FUNCTIONALITY_NAME@ menu" ), 0, parent, "@FUNCTIONALITY_NAME@" );
  return action;
}


void @FUNCTIONALITY_NAME@::TreeChanged()
{
  m_Controls->m_TreeNodeSelector->SetDataTreeNodeIterator(this->GetDataTreeIterator());
}


void @FUNCTIONALITY_NAME@::Activated()
{
  QmitkFunctionality::Activated();
}


void @FUNCTIONALITY_NAME@::ImageSelected(mitk::DataTreeIteratorClone imageIt)
{
  assert( imageIt.IsNotNull() ); // should never fail, the selection widget cares for that
    
  mitk::DataTreeNode* node = imageIt->Get();
  if ( node )
  {
    // here we have a valid mitk::DataTreeNode
    std::string name;
    if (node->GetName(name))
    {
      // a property called "name" was found for this DataTreeNode
      std::cout << "Tree node selected with name '" << name << "'" << std::endl;
    }

    // a node itself is not very useful, we need its data item
    mitk::BaseData* data = node->GetData();
    if (data)
    {
      // test if this data item is an image or not (could also be a surface or something totally different)
      mitk::Image* image = dynamic_cast<mitk::Image*>( data );
      if (image)
      {
        std::cout << "Surprise: this node contains a real image dataset." << std::endl;
      }
    }
  }
}


void @FUNCTIONALITY_NAME@::StartButtonClicked() 
{
  std::cout << "Start Button clicked!" << std::endl;
  WaitCursorOn(); // always good to show the user that the application is processing and will not react to user input for a while
  mitk::StatusBar::GetInstance()->DisplayText("@FUNCTIONALITY_NAME@ is doing something...", 4000);  // tell the user what you are doing
  mitk::ProgressBar::GetInstance()->AddStepsToDo(2);  // use progress bar to show that the application is doing something
  
  // Do something here, add progress while you do it:
  
  mitk::ProgressBar::GetInstance()->Progress();
  mitk::ProgressBar::GetInstance()->Progress(); // last step, we're finished!
  mitk::StatusBar::GetInstance()->DisplayText("@FUNCTIONALITY_NAME@ has finished...", 2000);
  WaitCursorOff();  // restore normal mouse cursor after you finished
}
