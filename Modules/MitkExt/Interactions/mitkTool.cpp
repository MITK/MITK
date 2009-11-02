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
#include "mitkProperties.h"
#include "mitkLevelWindowProperty.h"
#include "mitkVtkResliceInterpolationProperty.h"

#include <itkObjectFactory.h>

mitk::Tool::Tool(const char* type)
: StateMachine(type),
  // for working image
  m_IsSegmentationPredicate(NodePredicateProperty::New("segmentation", BoolProperty::New(true))),
  // for reference images
  m_PredicateImages(NodePredicateDataType::New("Image")),
  m_PredicateDim3(NodePredicateDimension::New(3, 1)),
  m_PredicateDim4(NodePredicateDimension::New(4, 1)),
  m_PredicateDimension( mitk::NodePredicateOR::New(m_PredicateDim3, m_PredicateDim4) ),
  m_PredicateImage3D( NodePredicateAND::New(m_PredicateImages, m_PredicateDimension) ),

  m_PredicateBinary(NodePredicateProperty::New("binary", BoolProperty::New(true))),
  m_PredicateNotBinary( NodePredicateNOT::New(m_PredicateBinary) ),

  m_PredicateSegmentation(NodePredicateProperty::New("segmentation", BoolProperty::New(true))),
  m_PredicateNotSegmentation( NodePredicateNOT::New(m_PredicateSegmentation) ),
  
  m_PredicateHelper(NodePredicateProperty::New("helper object", BoolProperty::New(true))),
  m_PredicateNotHelper( NodePredicateNOT::New(m_PredicateHelper) ),
  
  m_PredicateImageColorful( NodePredicateAND::New(m_PredicateNotBinary, m_PredicateNotSegmentation) ),

  m_PredicateImageColorfulNotHelper( NodePredicateAND::New(m_PredicateImageColorful, m_PredicateNotHelper) ),
  
  m_PredicateReference( NodePredicateAND::New(m_PredicateImage3D, m_PredicateImageColorfulNotHelper) )
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
      LOG_ERROR << "There is more than one GUI for " << classname << " (several factories claim ability to produce a " << guiClassname << " ) " << std::endl;
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
  return m_IsSegmentationPredicate.GetPointer();
}

mitk::DataTreeNode::Pointer mitk::Tool::CreateEmptySegmentationNode( Image* original, const std::string& organName, const mitk::Color& color )
{
  // we NEED a reference image for size etc.
  if (!original) return NULL;

  // actually create a new empty segmentation
  PixelType pixelType( typeid(DefaultSegmentationDataType) );
  Image::Pointer segmentation = Image::New();
 
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

  return CreateSegmentationNode( segmentation, organName, color );
}

mitk::DataTreeNode::Pointer mitk::Tool::CreateSegmentationNode( Image* image, const std::string& organName, const mitk::Color& color )
{
  if (!image) return NULL;

  // decorate the datatreenode with some properties
  DataTreeNode::Pointer segmentationNode = DataTreeNode::New();
  segmentationNode->SetData( image );

  // name
  segmentationNode->SetProperty( "name", StringProperty::New( organName ) );

  // visualization properties
  segmentationNode->SetProperty( "binary", BoolProperty::New(true) );
  segmentationNode->SetProperty( "color", ColorProperty::New(color) );
  segmentationNode->SetProperty( "texture interpolation", BoolProperty::New(false) );
  segmentationNode->SetProperty( "layer", IntProperty::New(10) );
  segmentationNode->SetProperty( "levelwindow", LevelWindowProperty::New( LevelWindow(0.5, 1) ) );
  segmentationNode->SetProperty( "opacity", FloatProperty::New(0.3) );
  segmentationNode->SetProperty( "segmentation", BoolProperty::New(true) );
  segmentationNode->SetProperty( "reslice interpolation", VtkResliceInterpolationProperty::New() ); // otherwise -> segmentation appears in 2 slices sometimes (only visual effect, not different data)
  segmentationNode->SetProperty( "showVolume", BoolProperty::New( false ) );
  
  return segmentationNode;
}

