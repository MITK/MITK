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

#include "QmitkColourImageProcessingView.h"

#include "ui_QmitkColourImageProcessingViewControls.h"

#include "mitkColourImageProcessor.h"
#include "mitkDataNodeObject.h"
#include "mitkTransferFunction.h"
#include "mitkTransferFunctionProperty.h"

#include "QmitkPiecewiseFunctionCanvas.h"
#include "QmitkColorTransferFunctionCanvas.h"

#include <berryISelectionProvider.h>
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

#include <QMessageBox>
#include <QComboBox>
#include <QColor>
#include <QColorDialog>


const std::string QmitkColourImageProcessingView::VIEW_ID = "org.mitk.views.colourimageprocessing";

QmitkColourImageProcessingView::QmitkColourImageProcessingView()
  : m_Controls(nullptr)
{
  m_Color[0]= 255;
  m_Color[1]= 0;
  m_Color[2]= 0;
}

void QmitkColourImageProcessingView::SetFocus()
{
  m_Controls->m_ConvertImageToRGBA->setFocus();
}

void QmitkColourImageProcessingView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    m_Controls = new Ui::QmitkColourImageProcessingViewControls;
    m_Controls->setupUi(parent);

    connect( m_Controls->m_ConvertImageToRGBA, SIGNAL( clicked(bool) ),this, SLOT( OnConvertToRGBAImage() ));
    connect( m_Controls->m_ConvertImageMaskToRGBA, SIGNAL( clicked(bool) ),this, SLOT( OnConvertToRGBAImage() ));
    connect( m_Controls->m_ConvertImageMaskColorToRGBA, SIGNAL( clicked(bool) ),this, SLOT( OnConvertImageMaskColorToRGBAImage() ));
    connect( m_Controls->m_ColorButton, SIGNAL( clicked(bool) ),this, SLOT( OnChangeColor() ));
    connect( m_Controls->m_CombineRGBAButton, SIGNAL( clicked(bool) ),this, SLOT( OnCombineRGBA() ));

    m_Controls->m_ImageSelectedLabel->hide();
    m_Controls->m_NoImageSelectedLabel->show();
  }
}


void QmitkColourImageProcessingView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/, const QList<mitk::DataNode::Pointer> &nodes)
{
  if(!nodes.isEmpty())
  {
    QList<mitk::DataNode::Pointer> selectedNodes;

    foreach(const mitk::DataNode::Pointer node, nodes)
    {
      if( node.IsNotNull() )
      {
        mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData());
        if (image->GetDimension()>=3)
        {
          selectedNodes.push_back( node );
        }
      }
    }

    mitk::DataNode::Pointer node;

    if(selectedNodes.size() > 0)
    {
      node=selectedNodes[0];

      m_SelectedNode = node;

      m_Controls->m_NoImageSelectedLabel->hide();
      m_Controls->m_ImageSelectedLabel->show();

      std::string  infoText = std::string("Selected Image: ") + node->GetName();

      if(selectedNodes.size() > 1)
      {
        mitk::DataNode::Pointer node2;
        node2=selectedNodes[1];
        m_SelectedNode2=node2;
        infoText = infoText + " and " + node2->GetName();
      }
      else
      {
        m_SelectedNode2= nullptr;
      }
      m_Controls->m_ImageSelectedLabel->setText( QString( infoText.c_str() ) );
    }
    else
    {
      m_Controls->m_ImageSelectedLabel->hide();
      m_Controls->m_NoImageSelectedLabel->show();

      m_SelectedNode = nullptr;
      m_SelectedNode2= nullptr;
    }
  }
}

void QmitkColourImageProcessingView::OnConvertToRGBAImage()
{
  if(m_SelectedNode.IsNull())
    return;


  mitk::TransferFunctionProperty::Pointer transferFunctionProp = dynamic_cast<mitk::TransferFunctionProperty*>(m_SelectedNode->GetProperty("TransferFunction"));

  if(transferFunctionProp.IsNull())
    return;

  mitk::TransferFunction::Pointer tf = transferFunctionProp->GetValue();

  if(tf.IsNull())
    return;

  mitk::mitkColourImageProcessor CImageProcessor;

  mitk::Image::Pointer RGBAImageResult;

  if(m_SelectedNode2.IsNotNull())
  {
    RGBAImageResult = CImageProcessor.convertWithBinaryToRGBAImage(dynamic_cast<mitk::Image*>(m_SelectedNode->GetData()),dynamic_cast<mitk::Image*>(m_SelectedNode2->GetData()),tf);
  }
  else
  {
    RGBAImageResult = CImageProcessor.convertToRGBAImage(dynamic_cast<mitk::Image*>(m_SelectedNode->GetData()),tf);
  }

  if (!RGBAImageResult)
  {
    QMessageBox::warning(nullptr, "Warning", QString("Unsupported pixeltype"));
    return;
  }

  mitk::DataNode::Pointer dtn = mitk::DataNode::New();

  dtn->SetData( RGBAImageResult );
  dtn->SetName(m_SelectedNode->GetName() + "_RGBA");
  this->GetDataStorage()->Add( dtn ); // add as a child, because the segmentation "derives" from the original

  MITK_INFO << "convertToRGBAImage finish";
}

void QmitkColourImageProcessingView::OnConvertImageMaskColorToRGBAImage( )
{
   if(m_SelectedNode.IsNull())
    return;


  mitk::TransferFunctionProperty::Pointer transferFunctionProp = dynamic_cast<mitk::TransferFunctionProperty*>(m_SelectedNode->GetProperty("TransferFunction"));

  if(transferFunctionProp.IsNull())
    return;

  mitk::TransferFunction::Pointer tf = transferFunctionProp->GetValue();

  if(tf.IsNull())
    return;

  mitk::mitkColourImageProcessor CImageProcessor;

  mitk::Image::Pointer RGBAImageResult;

  if(m_SelectedNode2.IsNotNull())
  {
    RGBAImageResult = CImageProcessor.convertWithBinaryAndColorToRGBAImage(dynamic_cast<mitk::Image*>(m_SelectedNode->GetData()),dynamic_cast<mitk::Image*>(m_SelectedNode2->GetData()),tf, m_Color);
  }
  else
  {
    RGBAImageResult = CImageProcessor.convertToRGBAImage(dynamic_cast<mitk::Image*>(m_SelectedNode->GetData()),tf);
  }

  if (!RGBAImageResult)
  {
    QMessageBox::warning(nullptr, "Warning", QString("Unsupported pixeltype"));
    return;
  }

  mitk::DataNode::Pointer dtn = mitk::DataNode::New();

  dtn->SetData( RGBAImageResult );
  dtn->SetName(m_SelectedNode->GetName() + "_RGBA");

  this->GetDataStorage()->Add( dtn ); // add as a child, because the segmentation "derives" from the original
}


void QmitkColourImageProcessingView::OnChangeColor( )
{
  QColor color = QColorDialog::getColor();
  if (color.spec() == 0)
  {
    color.setRed(255);
    color.setGreen(0);
    color.setBlue(0);
  }

  m_Color[0]= color.red();
  m_Color[1]= color.green();
  m_Color[2]= color.blue();

  m_Controls->m_ColorButton->setStyleSheet(QString("background-color:rgb(%1,%2, %3)").arg(color.red()).arg(color.green()).arg(color.blue()));
}


void QmitkColourImageProcessingView::OnCombineRGBA( )
{
 if(m_SelectedNode.IsNull())
    return;

  if(m_SelectedNode2.IsNull())
    return;


  mitk::mitkColourImageProcessor CImageProcessor;

  mitk::Image::Pointer RGBAImageResult;

  RGBAImageResult = CImageProcessor.combineRGBAImage(dynamic_cast<mitk::Image*>(m_SelectedNode->GetData()),dynamic_cast<mitk::Image*>(m_SelectedNode2->GetData()));
  MITK_INFO <<"RGBAImage Result";
  mitk::DataNode::Pointer dtn = mitk::DataNode::New();

  dtn->SetData( RGBAImageResult );

  this->GetDataStorage()->Add( dtn ); // add as a child, because the segmentation "derives" from the original
}
