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

#include "QmitkTbssView.h"
#include "mitkDiffusionImagingConfigure.h"

#include "mitkDataNodeObject.h"

#include "berryIWorkbenchWindow.h"
#include "berryIWorkbenchPage.h"
#include "berryISelectionService.h"
#include "berryConstants.h"
#include "berryPlatformUI.h"

#include <itkNiftiImageIO.h>
#include <itkImageFileReader.h>
#include <itkImage.h>

const std::string QmitkTbssView::VIEW_ID = 
  "org.mitk.views.tbss";

using namespace berry;




struct TbSelListener : ISelectionListener
{

  berryObjectMacro(TrSelListener);

  TbSelListener(QmitkTbssView* view)
  {
    m_View = view;
  }

  void DoSelectionChanged(ISelection::ConstPointer selection)
  {
//    if(!m_View->IsVisible())
//      return;
    // save current selection in member variable
    m_View->m_CurrentSelection = selection.Cast<const IStructuredSelection>();
    
    // Disable the m_OutputValues button
    m_View->m_Controls->m_OutputValues->setEnabled(false);

    // do something with the selected items
    if(m_View->m_CurrentSelection)
    {

      if(m_View->m_CurrentSelection->Size() == 2)
      {
        m_View->m_Controls->m_StatusLabel->hide();
        bool foundSkeleton = false;
        bool foundRoi = false;
        
        mitk::DataNode::Pointer skeletonNode;
        mitk::DataNode::Pointer roiNode;



        for (IStructuredSelection::iterator i = m_View->m_CurrentSelection->Begin(); 
          i != m_View->m_CurrentSelection->End(); ++i)
        {
          // extract datatree node
          if (mitk::DataNodeObject::Pointer nodeObj = i->Cast<mitk::DataNodeObject>())
          {
            mitk::DataNode::Pointer node = nodeObj->GetDataNode();
            
            if(QString("Image").compare(node->GetData()->GetNameOfClass())== 0)
            {
              // Find 4D image (3d skeleton + user ID)
              if (dynamic_cast<mitk::Image*>(node->GetData())->GetDimension() == 4)
              {
                m_View->SkeletonSelected(node);
                foundSkeleton = true;
              }

              // The ROI
              else if (dynamic_cast<mitk::Image*>(node->GetData())->GetDimension() == 3)
              {
                m_View->RoiSelected(node);
                foundRoi = true;
              }
            }
          }
        }


        if(foundSkeleton && foundRoi)
        {
          m_View->m_Controls->m_OutputValues->setEnabled(true);
        }
        else
        {          
           m_View->m_Controls->m_StatusLabel->show();
          m_View->m_Controls->m_RoiNameLabel->setText(QString::fromStdString(""));
          m_View->m_Controls->m_SkeletonNameLabel->setText(QString::fromStdString(""));
        }


      }

      else{
        m_View->m_Controls->m_StatusLabel->show();
        m_View->m_Controls->m_RoiNameLabel->setText(QString::fromStdString(""));
        m_View->m_Controls->m_SkeletonNameLabel->setText(QString::fromStdString(""));

      }


      
      
      
    }
  }

  void SelectionChanged(IWorkbenchPart::Pointer part, ISelection::ConstPointer selection)
  {
    // check, if selection comes from datamanager
    if (part)
    {
      QString partname(part->GetPartName().c_str());
      if(partname.compare("Datamanager")==0)
      {

        // apply selection
        DoSelectionChanged(selection);

      }
    }
  }

  QmitkTbssView* m_View;
};

QmitkTbssView::QmitkTbssView()
: QmitkFunctionality(),
  m_Controls(NULL),
  m_MultiWidget(NULL)
{
}

QmitkTbssView::~QmitkTbssView()
{
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->RemovePostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelListener);
}

void QmitkTbssView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkTbssViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections(); 
    
  }

  m_IsInitialized = false;
  m_SelListener = berry::ISelectionListener::Pointer(new TbSelListener(this));
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelListener);
  berry::ISelection::ConstPointer sel(
    this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
  m_CurrentSelection = sel.Cast<const IStructuredSelection>();
  m_SelListener.Cast<TbSelListener>()->DoSelectionChanged(sel);
}

void QmitkTbssView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  berry::ISelection::ConstPointer sel(
    this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
  m_CurrentSelection = sel.Cast<const IStructuredSelection>();
  m_SelListener.Cast<TbSelListener>()->DoSelectionChanged(sel);
  m_MultiWidget = &stdMultiWidget;
}

void QmitkTbssView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkTbssView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_OutputValues), SIGNAL(clicked()), this, SLOT(OutputValues()) );
  }
}

void QmitkTbssView::Activated()
{
  QmitkFunctionality::Activated();
}

void QmitkTbssView::Deactivated()
{
  QmitkFunctionality::Deactivated();
}

void QmitkTbssView::SkeletonSelected(mitk::DataNode::Pointer skeletonNode)
{
  m_SkeletonNode = skeletonNode; 
  m_SkeletonNode->SetVisibility(true);
  m_Controls->m_SkeletonNameLabel->setText(QString::fromStdString(m_SkeletonNode->GetName()));  
}

void QmitkTbssView::RoiSelected(mitk::DataNode::Pointer roiNode)
{
  m_RoiNode = roiNode; 
  m_RoiNode->SetVisibility(true);
  m_Controls->m_RoiNameLabel->setText(QString::fromStdString(m_RoiNode->GetName()));
}

void QmitkTbssView::OutputValues()
{

  if(m_SkeletonNode.IsNull() || m_RoiNode.IsNull())
  {
    return;
  }

  typedef itk::Image<char, 3> RoiImageType;
  typedef itk::Image<float, 4> AllSkeletonType;

  //read roi
  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( m_RoiNode->GetData() ); 
  RoiImageType::Pointer roi = RoiImageType::New();
  mitk::CastToItkImage(image, roi);

  //read the 4D skeleton
  mitk::Image::Pointer image2 = dynamic_cast<mitk::Image*>( m_SkeletonNode->GetData() ); 
  AllSkeletonType::Pointer skeleton = AllSkeletonType::New();  
  mitk::CastToItkImage(image2, skeleton);

  RoiImageType::SizeType roiSize = roi->GetLargestPossibleRegion().GetSize();
  AllSkeletonType::SizeType skeletonSize = skeleton->GetLargestPossibleRegion().GetSize();


  for(int x=0; x<roiSize[0]; x++)
  {
    for(int y=0; y<roiSize[1]; y++)
    {
      for(int z=0; z<roiSize[2]; z++)
      {
        RoiImageType::IndexType index;
        index[0] = x;
        index[1] = y;
        index[2] = z;
        if (roi->GetPixel(index) == 1)
        {
          std::cout << roi->GetPixel(index);
        }
      }
    }
  }
  
  
  
}


