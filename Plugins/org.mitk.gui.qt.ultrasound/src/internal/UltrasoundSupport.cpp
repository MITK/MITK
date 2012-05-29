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
#include "UltrasoundSupport.h"

// Qt
#include <QMessageBox>

// Ultrasound
#include "mitkUSDevice.h"

const std::string UltrasoundSupport::VIEW_ID = "org.mitk.views.ultrasoundsupport";


void UltrasoundSupport::SetFocus()
{
  m_Controls.buttonPerformImageProcessing->setFocus();
}

void UltrasoundSupport::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  connect( m_Controls.pushButton, SIGNAL(clicked()), this, SLOT(OnClickedAddNewDevice()) );
  connect( this, SIGNAL(DeviceServiceUpdated()),     m_Controls.m_DeviceManagerWidget, SLOT(OnDeviceServiceUpdated()) );

  // Initializations
  m_DeviceService = mitk::USDeviceService::New();
  m_Controls.m_DeviceManagerWidget->Initialize(m_DeviceService);
  MITK_INFO << "Initialized Plugin";
}


void UltrasoundSupport::OnClickedAddNewDevice(){
   
  // Debug: add fake Device
  mitk::USDevice::Pointer newDevice;
  newDevice = mitk::USDevice::New("Manufacturer", "Model", "Comment");
  m_DeviceService->ActivateDevice(newDevice);

  emit DeviceServiceUpdated();
}
















/** EXAMPLE CODE FOR WORKING WITH DATANODES ///

void UltrasoundSupport::DoImageProcessing()
{
  MITK_INFO << "DO SUPM";
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
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

**/
