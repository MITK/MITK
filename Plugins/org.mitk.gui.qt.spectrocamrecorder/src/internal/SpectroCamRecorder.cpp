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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "SpectroCamRecorder.h"

// Qt
#include <QMessageBox>

//mitk image
#include <mitkImage.h>

// other
#include <mitkSpectroCamController.h>

const std::string SpectroCamRecorder::VIEW_ID = "org.mitk.views.spectrocamrecorder";

void SpectroCamRecorder::SetFocus()
{
    m_Controls.buttonPerformImageProcessing->setFocus();
}

void SpectroCamRecorder::CreateQtPartControl( QWidget *parent )
{
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls.setupUi( parent );
    connect( m_Controls.buttonPerformImageProcessing, SIGNAL(clicked()), this, SLOT(DoImageProcessing()) );
    connect( m_Controls.buttonWhiteBalance, SIGNAL(clicked()), this, SLOT(SetWhiteBalance()) );
    connect( m_Controls.buttonSave, SIGNAL(clicked()), this, SLOT(SaveImage()) );

    // intialize the camera (!= start).
    m_Controller.Ini();
}

SpectroCamRecorder::~SpectroCamRecorder()
{
    if (m_Controller.isCameraRunning())
    {
        m_Controller.CloseCameraConnection();
    }
}

void SpectroCamRecorder::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                            const QList<mitk::DataNode::Pointer>& nodes )
{
    //// iterate all selected objects, adjust warning visibility
    //foreach( mitk::DataNode::Pointer node, nodes )
    //{
    //  if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
    //  {
    //    m_Controls.labelWarning->setVisible( false );
    //    m_Controls.buttonPerformImageProcessing->setEnabled( true );
    //    return;
    //  }
    //}

    //m_Controls.labelWarning->setVisible( true );
    //m_Controls.buttonPerformImageProcessing->setEnabled( false );
}


void SpectroCamRecorder::SaveImage()
{
  mitk::Image::Pointer imageToSave = m_Controller.GetCurrentImage();
  QString imageName = m_Controls.lineEdit->text();


  mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
  imageNode->SetData(imageToSave);
  imageNode->SetName(imageName.toStdString());


  GetDataStorage()->Add(imageNode);

}

void SpectroCamRecorder::SetWhiteBalance()
{
  m_Controller.SetCurrentImageAsWhiteBalance();
}

void SpectroCamRecorder::DoImageProcessing()
{
    //QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
    //if (nodes.empty()) return;

    //mitk::DataNode* node = nodes.front();

    //if (!node)
    //{
    //  // Nothing selected. Inform the user and return
    //  QMessageBox::information( NULL, "Template", "Please load and select an image before starting image processing.");
    //  return;
    //}

    //// here we have a valid mitk::DataNode

    //// a node itself is not very useful, we need its data item (the image)
    //mitk::BaseData* data = node->GetData();
    //if (data)
    //{
    //  // test if this data item is an image or not (could also be a surface or something totally different)
    //  mitk::Image* image = dynamic_cast<mitk::Image*>( data );
    //  if (image)
    //  {
    //    std::stringstream message;
    //    std::string name;
    //    message << "Performing image processing for image ";
    //    if (node->GetName(name))
    //    {
    //      // a property called "name" was found for this DataNode
    //      message << "'" << name << "'";
    //    }
    //    message << ".";
    //    MITK_INFO << message.str();

    //    // actually do something here...

    //  }
    //}

    if (m_Controller.isCameraRunning())
    {
        m_Controller.CloseCameraConnection();
    }
    else
    {
        m_Controller.OpenCameraConnection();
    }
}
