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

#include "mitkPickingTool.h"

#include "mitkToolManager.h"
#include "mitkProperties.h"
#include <mitkInteractionConst.h>
#include "mitkGlobalInteraction.h"

// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>


#include "mitkImageCast.h"
#include "mitkImageTimeSelector.h"
#include "mitkPointSetInteractor.h"
#include "mitkITKImageImport.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageTimeSelector.h"

#include <itkConnectedThresholdImageFilter.h>


namespace mitk {
MITK_TOOL_MACRO(MitkSegmentation_EXPORT, PickingTool, "PickingTool");
}

mitk::PickingTool::PickingTool()
{
  m_PointSetNode = mitk::DataNode::New();
  m_PointSetNode->GetPropertyList()->SetProperty("name", mitk::StringProperty::New("Picking_Seedpoint"));
  m_PointSetNode->GetPropertyList()->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_PointSet = mitk::PointSet::New();
  m_PointSetNode->SetData(m_PointSet);
  m_SeedPointInteractor = mitk::PointSetInteractor::New("singlepointinteractor", m_PointSetNode);

  //Watch for point added or modified
  itk::SimpleMemberCommand<PickingTool>::Pointer pointAddedCommand = itk::SimpleMemberCommand<PickingTool>::New();
  pointAddedCommand->SetCallbackFunction(this, &mitk::PickingTool::OnPointAdded);
  m_PointSetAddObserverTag = m_PointSet->AddObserver( mitk::PointSetAddEvent(), pointAddedCommand);

  //create new node for picked region
  m_ResultNode = mitk::DataNode::New();
  // set some properties
  m_ResultNode->SetProperty("name", mitk::StringProperty::New("result"));
  m_ResultNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_ResultNode->SetProperty("color", mitk::ColorProperty::New(0.0,1.0,0.0));
  m_ResultNode->SetProperty("layer", mitk::IntProperty::New(1));
  m_ResultNode->SetProperty("opacity", mitk::FloatProperty::New(0.7));


}

mitk::PickingTool::~PickingTool()
{
  m_PointSet->RemoveObserver(m_PointSetAddObserverTag);
}

const char** mitk::PickingTool::GetXPM() const
{
  return NULL;
}

const char* mitk::PickingTool::GetName() const
{
  return "Picking";
}

us::ModuleResource mitk::PickingTool::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Pick_48x48.png");
  return resource;
}


void mitk::PickingTool::Activated()
{
  //add to datastorage and enable interaction
  if (!GetDataStorage()->Exists(m_PointSetNode))
    GetDataStorage()->Add(m_PointSetNode, GetWorkingData());
  mitk::GlobalInteraction::GetInstance()->AddInteractor(m_SeedPointInteractor);

  // now add result to data tree
  GetDataStorage()->Add( m_ResultNode, this->GetWorkingData() );
}

void mitk::PickingTool::Deactivated()
{
  if (m_PointSet->GetPointSet()->GetNumberOfPoints() != 0)
  {
    mitk::Point3D point = m_PointSet->GetPoint(0);
    mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpREMOVE, point, 0);
    m_PointSet->ExecuteOperation(doOp);
  }
  //remove from data storage and disable interaction
  mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_SeedPointInteractor);
  GetDataStorage()->Remove(m_PointSetNode);
  GetDataStorage()->Remove( m_ResultNode);
}

mitk::DataNode* mitk::PickingTool::GetReferenceData(){
  return this->m_ToolManager->GetReferenceData(0);
}

mitk::DataStorage* mitk::PickingTool::GetDataStorage(){
  return this->m_ToolManager->GetDataStorage();
}

mitk::DataNode* mitk::PickingTool::GetWorkingData(){
  return this->m_ToolManager->GetWorkingData(0);
}

mitk::DataNode::Pointer mitk::PickingTool::GetPointSetNode()
{
  return m_PointSetNode;
}

void mitk::PickingTool::OnPointAdded()
{
  //Perform region growing/picking

  int timeStep = mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1") )->GetTimeStep();

  mitk::PointSet::PointType seedPoint = m_PointSet->GetPointSet(timeStep)->GetPoints()->Begin().Value();

  //as we want to pick a region from our segmentation image use the working data from ToolManager
  mitk::Image::Pointer orgImage = dynamic_cast<mitk::Image*> (m_ToolManager->GetWorkingData(0)->GetData());

  if (orgImage.IsNotNull())
  {
    if (orgImage->GetDimension() == 4)
    { //there may be 4D segmentation data even though we currently don't support that
      mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
      timeSelector->SetInput(orgImage);
      timeSelector->SetTimeNr( timeStep );
      timeSelector->UpdateLargestPossibleRegion();
      mitk::Image* timedImage = timeSelector->GetOutput();

      AccessByItk_2( timedImage , StartRegionGrowing, timedImage->GetGeometry(), seedPoint);
    }
    else if (orgImage->GetDimension() == 3)
    {
      AccessByItk_2(orgImage, StartRegionGrowing, orgImage->GetGeometry(), seedPoint);
    }
    this->m_PointSet->Clear();
  }
}



  template<typename TPixel, unsigned int VImageDimension>
  void mitk::PickingTool::StartRegionGrowing(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Geometry3D* imageGeometry, mitk::PointSet::PointType seedPoint)
  {
    typedef itk::Image<TPixel, VImageDimension> InputImageType;
    typedef typename InputImageType::IndexType IndexType;
    typedef itk::ConnectedThresholdImageFilter<InputImageType, InputImageType> RegionGrowingFilterType;
    typename RegionGrowingFilterType::Pointer regionGrower = RegionGrowingFilterType::New();

    // convert world coordinates to image indices
    IndexType seedIndex;
    imageGeometry->WorldToIndex( seedPoint, seedIndex);

    //perform region growing in desired segmented region
    regionGrower->SetInput( itkImage );
    regionGrower->AddSeed( seedIndex );

    regionGrower->SetLower( 1 );
    regionGrower->SetUpper( 255 );

    try
    {
      regionGrower->Update();
    }
    catch(itk::ExceptionObject &exc)
    {

      return; // can't work
    }
    catch( ... )
    {
      return;
    }

    //Store result and preview
    mitk::Image::Pointer resultImage = mitk::ImportItkImage(regionGrower->GetOutput(),imageGeometry)->Clone();

    m_ResultNode->SetData( resultImage );


    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }


  void mitk::PickingTool::ConfirmSegmentation()
  {
     //create a new node and store the image from the result node
     mitk::DataNode::Pointer newNode = mitk::DataNode::New();
     newNode->SetProperty("name", mitk::StringProperty::New("Picking_result"));
     newNode->SetProperty("helper object", mitk::BoolProperty::New(false));
     newNode->SetProperty("color", mitk::ColorProperty::New(1.0,0.0,0.0));
     newNode->SetProperty("opacity", mitk::FloatProperty::New(1.0));
     newNode->SetData(m_ResultNode->GetData());

     GetDataStorage()->Add(newNode);

     //reset result node
     m_ResultNode->SetData(NULL);

     mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
