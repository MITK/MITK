/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Do, 28 Mai 2009) $
Version:   $Revision: 17495 $ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkColourImageProcessingView.h"
#include "mitkColourImageProcessor.h"

#include "mitkNodePredicateDataType.h"
#include "QmitkDataStorageComboBox.h"
#include "QmitkStdMultiWidget.h"
#include <QMessageBox>
#include <QComboBox>

#include <berryISelectionProvider.h>
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <mitkDataTreeNodeObject.h>

#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>
#include "QmitkPiecewiseFunctionCanvas.h"
#include "QmitkColorTransferFunctionCanvas.h"

#include <QColor>
#include <QColorDialog>


const std::string QmitkColourImageProcessingView::VIEW_ID = "org.mitk.views.colourimageprocessing";

QmitkColourImageProcessingView::QmitkColourImageProcessingView()
: QmitkFunctionality(),
  m_MultiWidget(NULL)
{
  m_Controls=NULL;
  
  m_Color[0]= 255;
  m_Color[1]= 0;
  m_Color[2]= 0;
}

QmitkColourImageProcessingView::~QmitkColourImageProcessingView()
{
 berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  if(s)
    s->RemoveSelectionListener(m_SelectionListener);
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
  
   m_SelectionListener = berry::ISelectionListener::Pointer(new berry::SelectionChangedAdapter<QmitkColourImageProcessingView>
    (this, &QmitkColourImageProcessingView::SelectionChanged));
  berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  if(s)
    s->AddSelectionListener(m_SelectionListener);

}


void QmitkColourImageProcessingView::SelectionChanged( berry::IWorkbenchPart::Pointer, berry::ISelection::ConstPointer selection )
{ 

  mitk::DataTreeNodeSelection::ConstPointer _DataTreeNodeSelection 
    = selection.Cast<const mitk::DataTreeNodeSelection>();

  if(_DataTreeNodeSelection.IsNotNull())
  {
    std::vector<mitk::DataTreeNode*> selectedNodes;
    mitk::DataTreeNodeObject* _DataTreeNodeObject = 0;

    for(mitk::DataTreeNodeSelection::iterator it = _DataTreeNodeSelection->Begin();it != _DataTreeNodeSelection->End(); ++it)
    {
      _DataTreeNodeObject = dynamic_cast<mitk::DataTreeNodeObject*>((*it).GetPointer());
      if(_DataTreeNodeObject)
      {
        mitk::DataTreeNode::Pointer node = _DataTreeNodeObject->GetDataTreeNode();
      
        if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData())&&dynamic_cast<mitk::Image*>(node->GetData())->GetDimension()>=3   )
          selectedNodes.push_back( node );
      }
    }
  
    mitk::DataTreeNode::Pointer node;
    
    if(selectedNodes.size() > 0)
      node=selectedNodes[0];
    
    if( node.IsNotNull() )
    {
      m_SelectedNode = node;
      
      m_Controls->m_NoImageSelectedLabel->hide();
      m_Controls->m_ImageSelectedLabel->show();

      std::string  infoText = std::string("Selected Image: ") + node->GetName();

      if(selectedNodes.size() > 1)
      {
        mitk::DataTreeNode::Pointer node2;
        node2=selectedNodes[1];
        m_SelectedNode2=node2;
        infoText = infoText + " and " + node2->GetName();
      }
      else
      {
        m_SelectedNode2= 0;
      }
      
      m_Controls->m_ImageSelectedLabel->setText( QString( infoText.c_str() ) );
      
    }
    else
    {
      m_Controls->m_ImageSelectedLabel->hide();
      m_Controls->m_NoImageSelectedLabel->show();
      
      m_SelectedNode = 0;
      m_SelectedNode2= 0;
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
  
  mitk::DataTreeNode::Pointer dtn = mitk::DataTreeNode::New();
  
  dtn->SetData( RGBAImageResult );
  
  this->GetDefaultDataStorage()->Add( dtn ); // add as a child, because the segmentation "derives" from the original

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
  
  mitk::DataTreeNode::Pointer dtn = mitk::DataTreeNode::New();
  
  dtn->SetData( RGBAImageResult );
  
  this->GetDefaultDataStorage()->Add( dtn ); // add as a child, because the segmentation "derives" from the original

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
  mitk::DataTreeNode::Pointer dtn = mitk::DataTreeNode::New();
  
  dtn->SetData( RGBAImageResult );
  
  this->GetDefaultDataStorage()->Add( dtn ); // add as a child, because the segmentation "derives" from the original

}