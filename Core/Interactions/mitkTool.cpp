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

#include "mitkTool.h"
#include "mitkDataTreeNodeFactory.h"
#include "mitkOrganTypeProperty.h"
#include "mitkProperties.h"
#include "mitkLevelWindowProperty.h"
#include "mitkVtkResliceInterpolationProperty.h"

#include <itkObjectFactory.h>

mitk::Tool::Tool(const char* type)
: StateMachine(type),
  // for working image
  m_IsSegmentationPredicate("segmentation", BoolProperty::New(true)),
  // for reference images
  m_PredicateImages("Image"),
  m_PredicateDim3(3, 1),
  m_PredicateDim4(4, 1),
  m_PredicateDimension( m_PredicateDim3, m_PredicateDim4 ),
  m_PredicateImage3D( m_PredicateImages, m_PredicateDimension ),

  m_PredicateBinary("binary", BoolProperty::New(true)),
  m_PredicateNotBinary( m_PredicateBinary ),

  m_PredicateSegmentation("segmentation", BoolProperty::New(true)),
  m_PredicateNotSegmentation( m_PredicateSegmentation ),
  
  m_PredicateHelper("helper object", BoolProperty::New(true)),
  m_PredicateNotHelper( m_PredicateHelper ),
  
  m_PredicateImageColorful( m_PredicateNotBinary, m_PredicateNotSegmentation ),

  m_PredicateImageColorfulNotHelper( m_PredicateImageColorful, m_PredicateNotHelper ),
  
  m_PredicateReference( m_PredicateImage3D, m_PredicateImageColorfulNotHelper )
{
}

mitk::Tool::~Tool()
{
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
}

void mitk::Tool::Deactivated()
{
  StateMachine::ResetStatemachineToStartState(); // forget about the past
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
      std::cerr << "ERROR: There is more than one GUI for " << classname << " (several factories claim ability to produce a " << guiClassname << " ) " << std::endl;
      return NULL; // people should see and fix this error
    }
  }

  return object;
}

const mitk::NodePredicateBase& mitk::Tool::GetReferenceDataPreference() const
{
  return m_PredicateReference;
}


const mitk::NodePredicateBase& mitk::Tool::GetWorkingDataPreference() const
{
  return m_IsSegmentationPredicate;
}

//--------------------------------------------------------------------------------
//----          ToolLogger
//--------------------------------------------------------------------------------

unsigned int mitk::Tool::ToolLogger::s_Verboseness = 0;
mitk::Tool::ToolLogger::NullStream mitk::Tool::ToolLogger::s_NullStream;

std::ostream& mitk::Tool::ToolLogger::Logger(unsigned int verboseness)
{
  if (verboseness <= s_Verboseness)
  {
    return std::cout;
  }
  else
  {
    return s_NullStream;
  }
}

unsigned int mitk::Tool::ToolLogger::GetVerboseness()
{
  return s_Verboseness;
}

void mitk::Tool::ToolLogger::SetVerboseness( unsigned int verboseness )
{
  s_Verboseness = verboseness;
}

mitk::DataTreeNode::Pointer mitk::Tool::CreateEmptySegmentationNode( Image* original, const std::string& organType, const std::string& organName )
{
  // we NEED a reference image for size etc.
  if (!original) return NULL;

  // actually create a new empty segmentation
  PixelType pixelType( typeid(DefaultSegmentationDataType) );
  Image::Pointer segmentation = Image::New();
  segmentation->SetProperty( "organ type", OrganTypeProperty::New( organType ) );
  segmentation->Initialize( pixelType, original->GetDimension(), original->GetDimensions() );

  unsigned int byteSize = sizeof(DefaultSegmentationDataType);
  for (unsigned int dim = 0; dim < segmentation->GetDimension(); ++dim) 
  {
    byteSize *= segmentation->GetDimension(dim);
  }
  memset( segmentation->GetData(), 0, byteSize );

  if (original->GetTimeSlicedGeometry() )
  {
    AffineGeometryFrame3D::Pointer originalGeometryAGF = original->GetTimeSlicedGeometry()->Clone();
    TimeSlicedGeometry::Pointer originalGeometry = dynamic_cast<TimeSlicedGeometry*>( originalGeometryAGF.GetPointer() );
    segmentation->SetGeometry( originalGeometry );
  }
  else
  {
    Tool::ErrorMessage("Original image does not have a 'Time sliced geometry'! Cannot create a segmentation.");
    return NULL;
  }

  return CreateSegmentationNode( segmentation, organType, organName );
}

mitk::DataTreeNode::Pointer mitk::Tool::CreateSegmentationNode( Image* image, const std::string& organType, const std::string& organName )
{
  if (!image) return NULL;

  // decorate the datatreenode with some properties
  DataTreeNode::Pointer segmentationNode = DataTreeNode::New();
  segmentationNode->SetData( image );
  DataTreeNodeFactory::SetDefaultImageProperties ( segmentationNode );

  // name
  segmentationNode->SetProperty( "name", StringProperty::New( organName ) );

  // organ type
  OrganTypeProperty::Pointer organTypeProperty = OrganTypeProperty::New( organType );
  if ( !organTypeProperty->IsValidEnumerationValue( organType ) )
  {
    organTypeProperty->AddEnum( organType, organTypeProperty->Size() ); // add a new organ type
    organTypeProperty->SetValue( organType );
  }

  // visualization properties
  segmentationNode->SetProperty( "binary", BoolProperty::New(true) );
  segmentationNode->SetProperty( "color", DataTreeNodeFactory::DefaultColorForOrgan( organType ) );
  segmentationNode->SetProperty( "texture interpolation", BoolProperty::New(false) );
  segmentationNode->SetProperty( "layer", IntProperty::New(10) );
  segmentationNode->SetProperty( "levelwindow", LevelWindowProperty::New( LevelWindow(0.5, 1) ) );
  segmentationNode->SetProperty( "opacity", FloatProperty::New(0.3) );
  segmentationNode->SetProperty( "segmentation", BoolProperty::New(true) );
  segmentationNode->SetProperty( "reslice interpolation", VtkResliceInterpolationProperty::New() ); // otherwise -> segmentation appears in 2 slices sometimes (only visual effect, not different data)

  return segmentationNode;
}

