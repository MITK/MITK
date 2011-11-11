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
#include "MITKDICOM.h"
#include "QmitkStdMultiWidget.h"

// Qt
#include <QMessageBox>


const std::string MITKDICOM::VIEW_ID = "org.mitk.views.mitkdicom";

MITKDICOM::MITKDICOM()
: QmitkFunctionality()
{
}

MITKDICOM::~MITKDICOM()
{
}

void MITKDICOM::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  m_Controls.m_ctkDICOMAppWidget->cl
  connect( m_Controls.buttonPerformImageProcessing, SIGNAL(clicked()), this, SLOT(DoImageProcessing()) );
}

void MITKDICOM::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
  // iterate all selected objects, adjust warning visibility
  for( std::vector<mitk::DataNode*>::iterator it = nodes.begin();
       it != nodes.end();
       ++it )
  {
    mitk::DataNode::Pointer node = *it;

    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
    {
      //m_Controls.labelWarning->setVisible( false );
      //m_Controls.buttonPerformImageProcessing->setEnabled( true );
      return;
    }
  }

  //m_Controls.labelWarning->setVisible( true );
  //m_Controls.buttonPerformImageProcessing->setEnabled( false );
}


void MITKDICOM::DoImageProcessing()
{
  std::vector<mitk::DataNode*> nodes = this->GetDataManagerSelection();
  if (nodes.empty()) return;

  mitk::DataNode* node = nodes.front();

  if (!node)
  {
    // Nothing selected. Inform the user and return
    QMessageBox::information( NULL, "Template", "Please load and select an image before starting image processing.");
    return;
  }

  // here we have a valid mitk::DataNode

  // a node itself is not very useful, we need its data item (the image)
  mitk::BaseData* data = node->GetData();
  if (data)
  {
    // test if this data item is an image or not (could also be a surface or something totally different)
    mitk::Image* image = dynamic_cast<mitk::Image*>( data );
    if (image)
    {
      std::stringstream message;
      std::string name;
      message << "Performing image processing for image ";
      if (node->GetName(name))
      {
        // a property called "name" was found for this DataNode
        message << "'" << name << "'";
      }
      message << ".";
      MITK_INFO << message.str();

      // actually do something here...

    }
  }
}
