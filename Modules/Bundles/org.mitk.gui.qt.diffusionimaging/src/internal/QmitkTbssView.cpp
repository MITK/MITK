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

#include <iostream>
#include <fstream>


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

      else if(m_View->m_CurrentSelection->Size() == 1)
      {
              
        mitk::DataNode::Pointer skeletonNode;
        
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
              }

              
            }
          }
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
    connect( (QObject*)(m_Controls->m_OutputPoints), SIGNAL(clicked()), this, SLOT(OutputUnconnectedPointValues()) );
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

void QmitkTbssView::OutputUnconnectedPointValues()
{
  if(m_SkeletonNode.IsNull())
  {
    return;
  }

  //read roi
  /*mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( m_RoiNode->GetData() ); 
  RoiImageType::Pointer roi = RoiImageType::New();
  mitk::CastToItkImage(image, roi);*/
  
   //read the 4D skeleton
  mitk::Image::Pointer image2 = dynamic_cast<mitk::Image*>( m_SkeletonNode->GetData() ); 
  AllSkeletonType::Pointer skeleton = AllSkeletonType::New();  
  mitk::CastToItkImage(image2, skeleton);

 // RoiImageType::SizeType roiSize = roi->GetLargestPossibleRegion().GetSize();
  AllSkeletonType::SizeType skeletonSize = skeleton->GetLargestPossibleRegion().GetSize();



  AllSkeletonType::IndexType indices[3];
  indices[2][0] = 98; indices[2][1] = 126; indices[2][2] = 105;
  indices[3][0] = 101; indices[3][1] = 98; indices[3][2] = 104;
  indices[4][0] = 101; indices[4][1] = 76; indices[4][2] = 92;

  // output file
  std::string filename = "G://home//vanbrugg//spie2011//CingulumHealthy.csv";
  std::ofstream f;
  f.open(filename.c_str());

  for (int s=0; s<skeletonSize[3]; s++)
  {
    
    for(int t=0; t<3; t++)
    {
      indices[t][3] = s;     
      f << skeleton->GetPixel(indices[t]) << ",";
    }

    f << std::endl;
  }
  f.close();

  //for(int x=0; x<roiSize[0]; x++)
  //{
  //  for(int y=0; y<roiSize[1]; y++)
  //  {
  //    for(int z=0; z<roiSize[2]; z++)
  //    {
  //      RoiImageType::IndexType index;
  //      index[0] = x;
  //      index[1] = y;
  //      index[2] = z;
  //      if (roi->GetPixel(index) != 0)
  //      {
  //        // Output the values of this point for all subjects
  //        AllSkeletonType::IndexType skeletonIndex;
  //        skeletonIndex[0] = index[0];
  //        skeletonIndex[1] = index[1];
  //        skeletonIndex[2] = index[2];

  //       

  //        for(int t=0; t<skeletonSize[3]; t++)
  //        {
  //          if(index[0] == 90 && index[1] == 98 && index[2] == 95)
  //          {
  //            std::cout << "the empty voxel" << std::endl;
  //          }
  //          skeletonIndex[3] = t;
  //          std::cout << skeleton->GetPixel(skeletonIndex) << std::endl;
  //        }

  //        std::cout << " ";
  //      }
  //    }
  //  }
  //}

}

void QmitkTbssView::OutputValues()
{

  if(m_SkeletonNode.IsNull() || m_RoiNode.IsNull())
  {
    return;
  }


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

  std::vector<RoiImageType::IndexType> indices = SortPoints(roi);


  //For every subject (4th axis of the 4D volume) output the tract of interest and write results to file
  std::string filename = "G://home//vanbrugg//spie2011//paraHipLeftHealthy.csv";
  std::ofstream f;
  f.open(filename.c_str());
  

  for (int s=0; s<skeletonSize[3]; s++)
  {
    std::vector<RoiImageType::IndexType>::iterator it = indices.begin();    
    while(it != indices.end())
    {
      RoiImageType::IndexType roiIndex = *it;
      AllSkeletonType::IndexType skeletonIndex;
      skeletonIndex[0] = roiIndex[0];
      skeletonIndex[1] = roiIndex[1];
      skeletonIndex[2] = roiIndex[2];
      skeletonIndex[3] = s;

      //std::cout << skeleton->GetPixel(skeletonIndex) << ",";

      f << skeleton->GetPixel(skeletonIndex) << ",";
     

      ++it;
    }
    f << std::endl;
  }

  f.close();

  //for(int x=0; x<roiSize[0]; x++)
  //{
  //  for(int y=0; y<roiSize[1]; y++)
  //  {
  //    for(int z=0; z<roiSize[2]; z++)
  //    {
  //      RoiImageType::IndexType index;
  //      index[0] = x;
  //      index[1] = y;
  //      index[2] = z;
  //      if (roi->GetPixel(index) != 0)
  //      {
  //        // Output the values of this point for all subjects
  //        AllSkeletonType::IndexType skeletonIndex;
  //        skeletonIndex[0] = index[0];
  //        skeletonIndex[1] = index[1];
  //        skeletonIndex[2] = index[2];

  //       

  //        for(int t=0; t<skeletonSize[3]; t++)
  //        {
  //          if(index[0] == 90 && index[1] == 98 && index[2] == 95)
  //          {
  //            std::cout << "the empty voxel" << std::endl;
  //          }
  //          skeletonIndex[3] = t;
  //          std::cout << skeleton->GetPixel(skeletonIndex) << std::endl;
  //        }

  //        std::cout << " ";
  //      }
  //    }
  //  }
  //}
  
   
  
}

std::vector<RoiImageType::IndexType> QmitkTbssView::SortPoints(RoiImageType::Pointer roi)
{
  std::vector<RoiImageType::IndexType> indices;
  
  //set current point to be the start point. Hard coded for now, should be changed to function in general
  RoiImageType::IndexType currentPoint;
  //currentPoint[0] = 90; // For Corpus Callosum the constant index;
  //currentPoint[1] = 88;
  //currentPoint[2] = 82;

  //currentPoint[0] = 65; //standard for right parahippocampal white matter
  //currentPoint[1] = 108;
  //currentPoint[2] = 45;

  currentPoint[0] = 95; 
  currentPoint[1] = 166;
  currentPoint[2] = 78;

  bool ready = false;
  while(!ready)
  {
    indices.push_back(currentPoint);
    currentPoint = FindNextPoint(indices, currentPoint, roi, ready);
  }

  return indices;
}

bool QmitkTbssView::PointVisited(std::vector<RoiImageType::IndexType> points, RoiImageType::IndexType point)
{
  bool ret = false;
  std::vector<RoiImageType::IndexType>::iterator it = points.begin();
  while(it != points.end())
  {
    RoiImageType::IndexType p = *it;
    if(p[0] == point[0] && p[1] == point[1] && p[2] == point[2])
    {
      ret = true;
    }
    ++it;
  }

  return ret;
}

RoiImageType::IndexType QmitkTbssView::FindNextPoint(std::vector<RoiImageType::IndexType> pointsVisited, 
                                                     RoiImageType::IndexType currentPoint, RoiImageType::Pointer roi, bool &ready)
{  

  RoiImageType::IndexValueType dx[26] = {0, 0, 1, 1, 1, 0,-1,-1,-1,0,1,1, 1, 0,-1,-1,-1, 0, 1, 1, 1, 0,-1,-1,-1, 0};
  RoiImageType::IndexValueType dy[26] = {0, 1, 1, 0,-1,-1,-1, 0, 1,1,1,0,-1,-1,-1, 0, 1, 1, 1, 0,-1,-1,-1, 0, 1, 0};
  RoiImageType::IndexValueType dz[26] = {0, 0, 0, 0, 0, 0, 0, 0, 0,1,1,1, 1, 1, 1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1, 1};

  RoiImageType::IndexType newPoint;   

  for (int step=0; step<26; step++)
  {
    newPoint[0] = currentPoint[0] + dx[step];
    newPoint[1] = currentPoint[1] + dy[step];
    newPoint[2] = currentPoint[2] + dz[step];

    // Check if point is not yet visited
    if(!PointVisited(pointsVisited, newPoint))
    {
      if(roi->GetPixel(newPoint) == 1)
        return newPoint;
    }
  }

  // No points yet to be visited found so we are done searching
  ready = true;
  return currentPoint;

}


