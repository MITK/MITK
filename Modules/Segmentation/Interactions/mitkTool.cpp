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

#include "mitkTool.h"
#include "mitkToolManager.h"
#include "mitkDataNodeFactory.h"
#include "mitkProperties.h"
#include "mitkImageWriteAccessor.h"
#include "mitkLevelWindowProperty.h"
#include "mitkVtkResliceInterpolationProperty.h"
#include "mitkImageReadAccessor.h"

// us
#include <usModuleResource.h>
#include <usGetModuleContext.h>

#include <itkObjectFactory.h>

mitk::Tool::Tool(const char* type) :
m_InteractorType( type ),
m_PaintingPixelValue(0),
m_CurrentTimeStep(0),
m_PredicateWorking(mitk::NodePredicateAnd::New()),
m_PredicateReference(mitk::NodePredicateAnd::New())
{
  m_PredicateWorking->AddPredicate(NodePredicateDimension::New(3, 1));
  m_PredicateWorking->AddPredicate(mitk::NodePredicateDataType::New("LabelSetImage"));
  m_PredicateWorking->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));

  mitk::NodePredicateDataType::Pointer isDwi = mitk::NodePredicateDataType::New("DiffusionImage");
  mitk::NodePredicateDataType::Pointer isDti = mitk::NodePredicateDataType::New("TensorImage");
  mitk::NodePredicateDataType::Pointer isQbi = mitk::NodePredicateDataType::New("QBallImage");

  mitk::NodePredicateOr::Pointer validImages = mitk::NodePredicateOr::New();
  validImages->AddPredicate( mitk::TNodePredicateDataType<mitk::Image>::New() );
  validImages->AddPredicate(isDwi);
  validImages->AddPredicate(isDti);
  validImages->AddPredicate(isQbi);

  m_PredicateReference->AddPredicate(validImages);
  m_PredicateReference->AddPredicate(mitk::NodePredicateNot::New(m_PredicateWorking));
  m_PredicateReference->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
}

mitk::Tool::~Tool()
{
}

void mitk::Tool::InitializeStateMachine()
{
  if (m_InteractorType.empty())
    return;

  m_InteractorType += ".xml";

  try
  {
    LoadStateMachine( m_InteractorType, us::GetModuleContext()->GetModule() );
    SetEventConfig( "SegmentationToolsConfig.xml", us::GetModuleContext()->GetModule() );
  }
  catch( const std::exception& e )
  {
    MITK_ERROR << "Could not load statemachine pattern " << m_InteractorType << " with exception: " << e.what();
  }
}

void mitk::Tool::Notify( InteractionEvent* interactionEvent, bool isHandled )
{
  // to use the state machine pattern,
  // the event is passed to the state machine interface to be handled
  if ( !isHandled )
  {
    this->HandleEvent(interactionEvent, NULL);
  }
}

void mitk::Tool::ConnectActionsAndFunctions()
{
}


bool mitk::Tool::FilterEvents(InteractionEvent* , DataNode* )
{
  return true;
}



const char* mitk::Tool::GetGroup() const
{
  return "default";
}

void mitk::Tool::SetToolManager(ToolManager* manager)
{
  m_ToolManager = manager;
}

void mitk::Tool::Activated()
{
  m_WorkingNode = m_ToolManager->GetWorkingData(0);
  assert(m_WorkingNode);
}

void mitk::Tool::Deactivated()
{
  // ToDo: reactivate this feature!
  //StateMachine::ResetStatemachineToStartState(); // forget about the past
}

itk::Object::Pointer mitk::Tool::GetGUI(const std::string& toolkitPrefix, const std::string& toolkitPostfix)
{
  itk::Object::Pointer object;

  std::string classname = this->GetNameOfClass();
  std::string guiClassname = toolkitPrefix + classname + toolkitPostfix;

  std::list<itk::LightObject::Pointer> allGUIs = itk::ObjectFactoryBase::CreateAllInstance(guiClassname.c_str());
  for( std::list<itk::LightObject::Pointer>::iterator iter = allGUIs.begin();
       iter != allGUIs.end();
       ++iter )
  {
    if (object.IsNull())
    {
      object = dynamic_cast<itk::Object*>( iter->GetPointer() );
    }
    else
    {
      MITK_ERROR << "There is more than one GUI for " << classname << " (several factories claim ability to produce a " << guiClassname << " ) " << std::endl;
      return NULL; // people should see and fix this error
    }
  }

  return object;
}

mitk::NodePredicateBase::ConstPointer mitk::Tool::GetReferenceDataPreference() const
{
  return m_PredicateReference.GetPointer();
}


mitk::NodePredicateBase::ConstPointer mitk::Tool::GetWorkingDataPreference() const
{
  return m_PredicateWorking.GetPointer();
}

mitk::DataNode::Pointer mitk::Tool::CreateEmptySegmentationNode( Image* original, const std::string& organName, const mitk::Color& color )
{
  // we NEED a reference image for size etc.
  if (!original) return NULL;

  // actually create a new empty segmentation
  PixelType pixelType(mitk::MakeScalarPixelType<DefaultSegmentationDataType>() );
  Image::Pointer segmentation = Image::New();

  if (original->GetDimension() == 2)
  {
    const unsigned int dimensions[] = { original->GetDimension(0), original->GetDimension(1), 1 };
    segmentation->Initialize(pixelType, 3, dimensions);
  }
  else
  {
    segmentation->Initialize(pixelType, original->GetDimension(), original->GetDimensions());
  }

  unsigned int byteSize = sizeof(DefaultSegmentationDataType);

  if(segmentation->GetDimension() < 4)
  {
    for (unsigned int dim = 0; dim < segmentation->GetDimension(); ++dim)
    {
      byteSize *= segmentation->GetDimension(dim);
    }

    mitk::ImageWriteAccessor writeAccess(segmentation, segmentation->GetVolumeData(0));

    memset( writeAccess.GetData(), 0, byteSize );
  }
  else
  {//if we have a time-resolved image we need to set memory to 0 for each time step
    for (unsigned int dim = 0; dim < 3; ++dim)
    {
      byteSize *= segmentation->GetDimension(dim);
    }

    for( unsigned int volumeNumber = 0; volumeNumber < segmentation->GetDimension(3); volumeNumber++)
    {
      mitk::ImageWriteAccessor writeAccess(segmentation, segmentation->GetVolumeData(volumeNumber));

      memset( writeAccess.GetData(), 0, byteSize );
    }
  }

  if (original->GetTimeGeometry() )
  {
    TimeGeometry::Pointer originalGeometry = original->GetTimeGeometry()->Clone();
    segmentation->SetTimeGeometry( originalGeometry );
  }
  else
  {
    Tool::ErrorMessage("Original image does not have a 'Time sliced geometry'! Cannot create a segmentation.");
    return NULL;
  }

  return CreateSegmentationNode( segmentation, organName, color );
}

mitk::DataNode::Pointer mitk::Tool::CreateSegmentationNode( Image* image, const std::string& organName, const mitk::Color& color )
{
  if (!image) return NULL;

  // decorate the datatreenode with some properties
  DataNode::Pointer segmentationNode = DataNode::New();
  segmentationNode->SetData( image );

  // name
  segmentationNode->SetProperty( "name", StringProperty::New( organName ) );

  // visualization properties
  //segmentationNode->SetProperty( "binary", BoolProperty::New(true) );
  segmentationNode->SetProperty( "color", ColorProperty::New(color) );
  segmentationNode->SetProperty( "texture interpolation", BoolProperty::New(false) );
  segmentationNode->SetProperty( "layer", IntProperty::New(10) );
  segmentationNode->SetProperty( "levelwindow", LevelWindowProperty::New( LevelWindow(0.5, 1) ) );
  segmentationNode->SetProperty( "opacity", FloatProperty::New(0.3) );
  segmentationNode->SetProperty( "segmentation", BoolProperty::New(true) );
  segmentationNode->SetProperty( "reslice interpolation", VtkResliceInterpolationProperty::New() ); // otherwise -> segmentation appears in 2 slices sometimes (only visual effect, not different data)
  // For MITK-3M3 release, the volume of all segmentations should be shown
  segmentationNode->SetProperty( "showVolume", BoolProperty::New( true ) );

  return segmentationNode;
}

us::ModuleResource mitk::Tool::GetIconResource() const
{
  // Each specific tool should load its own resource. This one will be invalid
  return us::ModuleResource();
}

us::ModuleResource mitk::Tool::GetCursorIconResource() const
{
  // Each specific tool should load its own resource. This one will be invalid
  return us::ModuleResource();
}
