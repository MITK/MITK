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

//MITK
#include <mitkAbstractTransformGeometry.h>
#include <mitkDataNode.h>
#include <mitkImageSliceSelector.h>
#include <mitkLevelWindowProperty.h>
#include <mitkLookupTableProperty.h>
#include <mitkPlaneGeometry.h>
#include <mitkProperties.h>
#include <mitkResliceMethodProperty.h>
#include <mitkVtkResliceInterpolationProperty.h>
#include <mitkPixelType.h>
#include <mitkTransferFunctionProperty.h>
#include "mitkImageStatisticsHolder.h"
#include "mitkPlaneClipping.h"

#include "mitkRegVisPropertyTags.h"
#include "mitkRegVisHelper.h"
#include "mitkRegEvalStyleProperty.h"
#include "mitkRegEvalWipeStyleProperty.h"

//MITK Rendering
#include "mitkRegEvaluationMapper2D.h"
#include "vtkMitkThickSlicesFilter.h"
#include "vtkMitkLevelWindowFilter.h"
#include "vtkNeverTranslucentTexture.h"

//VTK
#include <vtkProperty.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkLookupTable.h>
#include <vtkImageData.h>
#include <vtkPoints.h>
#include <vtkGeneralTransform.h>
#include <vtkImageExtractComponents.h>
#include <vtkImageReslice.h>
#include <vtkImageChangeInformation.h>
#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkCellArray.h>
#include <vtkCamera.h>
#include <vtkColorTransferFunction.h>
#include <vtkImageCheckerboard.h>
#include <vtkImageWeightedSum.h>
#include <vtkImageMathematics.h>
#include <vtkImageRectilinearWipe.h>
#include <vtkImageGradientMagnitude.h>
#include <vtkImageAppendComponents.h>

//ITK
#include <itkRGBAPixel.h>
#include <mitkRenderingModeProperty.h>

//MatchPoint
#include <mitkRegEvaluationObject.h>
#include <mitkImageMappingHelper.h>

mitk::RegEvaluationMapper2D::RegEvaluationMapper2D()
{
}

mitk::RegEvaluationMapper2D::~RegEvaluationMapper2D()
{
}

//set the two points defining the textured plane according to the dimension and spacing
void mitk::RegEvaluationMapper2D::GeneratePlane(mitk::BaseRenderer* renderer, double planeBounds[6])
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  float depth = this->CalculateLayerDepth(renderer);
  //Set the origin to (xMin; yMin; depth) of the plane. This is necessary for obtaining the correct
  //plane size in crosshair rotation and swivel mode.
  localStorage->m_Plane->SetOrigin(planeBounds[0], planeBounds[2], depth);
  //These two points define the axes of the plane in combination with the origin.
  //Point 1 is the x-axis and point 2 the y-axis.
  //Each plane is transformed according to the view (axial, coronal and saggital) afterwards.
  localStorage->m_Plane->SetPoint1(planeBounds[1] , planeBounds[2], depth); //P1: (xMax, yMin, depth)
  localStorage->m_Plane->SetPoint2(planeBounds[0], planeBounds[3], depth); //P2: (xMin, yMax, depth)
}

float mitk::RegEvaluationMapper2D::CalculateLayerDepth(mitk::BaseRenderer* renderer)
{
  //get the clipping range to check how deep into z direction we can render images
  double maxRange = renderer->GetVtkRenderer()->GetActiveCamera()->GetClippingRange()[1];

  //Due to a VTK bug, we cannot use the whole clipping range. /100 is empirically determined
  float depth = -maxRange*0.01; // divide by 100
  int layer = 0;
  GetDataNode()->GetIntProperty( "layer", layer, renderer);
  //add the layer property for each image to render images with a higher layer on top of the others
  depth += layer*10; //*10: keep some room for each image (e.g. for QBalls in between)
  if(depth > 0.0f) {
    depth = 0.0f;
    MITK_WARN << "Layer value exceeds clipping range. Set to minimum instead.";
  }
  return depth;
}

const mitk::Image* mitk::RegEvaluationMapper2D::GetTargetImage( void )
{
  const mitk::RegEvaluationObject* evalObj = dynamic_cast< const mitk::RegEvaluationObject* >( GetDataNode()->GetData() );
  if (evalObj)
  {
    return evalObj->GetTargetImage();
  }

  return NULL;
}

const mitk::Image* mitk::RegEvaluationMapper2D::GetMovingImage( void )
{
  const mitk::RegEvaluationObject* evalObj = dynamic_cast< const mitk::RegEvaluationObject* >( GetDataNode()->GetData() );
  if (evalObj)
  {
    return evalObj->GetMovingImage();
  }

  return NULL;
}

const mitk::MAPRegistrationWrapper* mitk::RegEvaluationMapper2D::GetRegistration( void )
{
  const mitk::RegEvaluationObject* evalObj = dynamic_cast< const mitk::RegEvaluationObject* >( GetDataNode()->GetData() );
  if (evalObj)
  {
    return evalObj->GetRegistration();
  }

  return NULL;
}

vtkProp* mitk::RegEvaluationMapper2D::GetVtkProp(mitk::BaseRenderer* renderer)
{
  //return the actor corresponding to the renderer
  return m_LSH.GetLocalStorage(renderer)->m_Actors;
}

void mitk::RegEvaluationMapper2D::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
  bool updated = false;
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  mitk::Image::Pointer targetInput = const_cast< mitk::Image * >( this->GetTargetImage() );
  mitk::DataNode* datanode = this->GetDataNode();

  if ( targetInput.IsNull() || targetInput->IsInitialized() == false )
  {
    return;
  }

  mitk::Image::ConstPointer movingInput = this->GetMovingImage();

  if ( movingInput.IsNull() || movingInput->IsInitialized() == false )
  {
    return;
  }

  mitk::MAPRegistrationWrapper::ConstPointer reg = this->GetRegistration();

  //check if there is a valid worldGeometry
  const Geometry2D *worldGeometry = renderer->GetCurrentWorldGeometry2D();
  if( ( worldGeometry == NULL ) || ( !worldGeometry->IsValid() ) || ( !worldGeometry->HasReferenceGeometry() ))
  {
    return;
  }

  if(targetInput->GetMTime()>localStorage->m_LastUpdateTime
    || (localStorage->m_LastUpdateTime < renderer->GetCurrentWorldGeometry2DUpdateTime()) //was the geometry modified?
    || (localStorage->m_LastUpdateTime < renderer->GetCurrentWorldGeometry2D()->GetMTime()))
  { //target input has been modified -> reslice target input
    targetInput->Update();

    // early out if there is no intersection of the current rendering geometry
    // and the geometry of the image that is to be rendered.
    if ( !RenderingGeometryIntersectsImage( worldGeometry, targetInput->GetSlicedGeometry() ) )
    {
      // set image to NULL, to clear the texture in 3D, because
      // the latest image is used there if the plane is out of the geometry
      // see bug-13275
      localStorage->m_EvaluationImage = NULL;
      localStorage->m_Mapper->SetInputData( localStorage->m_EmptyPolyData );
      return;
    }

    //set main input for ExtractSliceFilter
    localStorage->m_Reslicer->SetInput(targetInput);
    localStorage->m_Reslicer->SetWorldGeometry(worldGeometry);
    localStorage->m_Reslicer->SetTimeStep( this->GetTimestep() );


    //set the transformation of the image to adapt reslice axis
    localStorage->m_Reslicer->SetResliceTransformByGeometry( targetInput->GetTimeGeometry()->GetGeometryForTimeStep( this->GetTimestep() ) );


    //is the geometry of the slice based on the input image or the worldgeometry?
    bool inPlaneResampleExtentByGeometry = false;
    datanode->GetBoolProperty("in plane resample extent by geometry", inPlaneResampleExtentByGeometry, renderer);
    localStorage->m_Reslicer->SetInPlaneResampleExtentByGeometry(inPlaneResampleExtentByGeometry);


    // Initialize the interpolation mode for resampling; switch to nearest
    // neighbor if the input image is too small.
    if ( (targetInput->GetDimension() >= 3) && (targetInput->GetDimension(2) > 1) )
    {
      VtkResliceInterpolationProperty *resliceInterpolationProperty;
      datanode->GetProperty(
        resliceInterpolationProperty, "reslice interpolation" );

      int interpolationMode = VTK_RESLICE_NEAREST;
      if ( resliceInterpolationProperty != NULL )
      {
        interpolationMode = resliceInterpolationProperty->GetInterpolation();
      }

      switch ( interpolationMode )
      {
      case VTK_RESLICE_NEAREST:
        localStorage->m_Reslicer->SetInterpolationMode(ExtractSliceFilter::RESLICE_NEAREST);
        break;
      case VTK_RESLICE_LINEAR:
        localStorage->m_Reslicer->SetInterpolationMode(ExtractSliceFilter::RESLICE_LINEAR);
        break;
      case VTK_RESLICE_CUBIC:
        localStorage->m_Reslicer->SetInterpolationMode(ExtractSliceFilter::RESLICE_CUBIC);
        break;
      }
    }
    else
    {
      localStorage->m_Reslicer->SetInterpolationMode(ExtractSliceFilter::RESLICE_NEAREST);
    }

    //this is needed when thick mode was enable bevore. These variable have to be reset to default values
    localStorage->m_Reslicer->SetOutputDimensionality( 2 );
    localStorage->m_Reslicer->SetOutputSpacingZDirection(1.0);
    localStorage->m_Reslicer->SetOutputExtentZDirection( 0, 0 );

    localStorage->m_Reslicer->Modified();
    //start the pipeline with updating the largest possible, needed if the geometry of the input has changed
    localStorage->m_Reslicer->UpdateLargestPossibleRegion();
    localStorage->m_slicedTargetImage = localStorage->m_Reslicer->GetOutput();
    updated = true;
  }

  if(updated ||
    movingInput->GetMTime() > localStorage->m_LastUpdateTime ||
    reg->GetMTime() > localStorage->m_LastUpdateTime)
  {
    //Map moving image
    localStorage->m_slicedMappedImage = mitk::ImageMappingHelper::map(movingInput,reg,false,0,localStorage->m_slicedTargetImage->GetGeometry(),false,0);
    updated = true;
  }

  //Generate evaulation image
  bool isStyleOutdated = mitk::PropertyIsOutdated(datanode,mitk::nodeProp_RegEvalStyle,localStorage->m_LastUpdateTime);
  bool isBlendOutdated = mitk::PropertyIsOutdated(datanode,mitk::nodeProp_RegEvalBlendFactor,localStorage->m_LastUpdateTime);
  bool isCheckerOutdated = mitk::PropertyIsOutdated(datanode,mitk::nodeProp_RegEvalCheckerCount,localStorage->m_LastUpdateTime);
  bool isWipeStyleOutdated = mitk::PropertyIsOutdated(datanode,mitk::nodeProp_RegEvalWipeStyle,localStorage->m_LastUpdateTime);
  bool isContourOutdated = mitk::PropertyIsOutdated(datanode,mitk::nodeProp_RegEvalTargetContour,localStorage->m_LastUpdateTime);

  if (updated ||
    isStyleOutdated ||
    isBlendOutdated ||
    isCheckerOutdated ||
    isWipeStyleOutdated ||
    isContourOutdated)
  {
    mitk::RegEvalStyleProperty::Pointer evalStyleProp = mitk::RegEvalStyleProperty::New();
    datanode->GetProperty(evalStyleProp, mitk::nodeProp_RegEvalStyle);

    switch (evalStyleProp->GetValueAsId())
    {
    case 0 :
      {
        PrepareBlend(datanode, localStorage);
        break;
      }
    case 1 :
      {
        PrepareColorBlend(localStorage);
        break;
      }
    case 2 :
      {
        PrepareCheckerBoard(datanode, localStorage);
        break;
      }
    case 3 :
      {
        PrepareWipe(datanode, localStorage);
        break;
      }
    case 4 :
      {
        PrepareDifference(localStorage);
        break;
      }
    case 5 :
      {
        PrepareContour(datanode, localStorage);
        break;
      }
    }
    updated = true;
  }

  if(updated
    || (localStorage->m_LastUpdateTime < datanode->GetPropertyList()->GetMTime()) //was a property modified?
    || (localStorage->m_LastUpdateTime < datanode->GetPropertyList(renderer)->GetMTime()) )
  {
    // Bounds information for reslicing (only required if reference geometry
    // is present)
    //this used for generating a vtkPLaneSource with the right size
    double sliceBounds[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    localStorage->m_Reslicer->GetClippedPlaneBounds(sliceBounds);

    //get the spacing of the slice
    localStorage->m_mmPerPixel = localStorage->m_Reslicer->GetOutputSpacing();

    // calculate minimum bounding rect of IMAGE in texture
    {
      double textureClippingBounds[6]  = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

      // Calculate the actual bounds of the transformed plane clipped by the
      // dataset bounding box; this is required for drawing the texture at the
      // correct position during 3D mapping.

      const PlaneGeometry *planeGeometry = dynamic_cast< const PlaneGeometry * >( worldGeometry );
      mitk::PlaneClipping::CalculateClippedPlaneBounds( targetInput->GetGeometry(), planeGeometry, textureClippingBounds );

      textureClippingBounds[0] = static_cast< int >( textureClippingBounds[0] / localStorage->m_mmPerPixel[0] + 0.5 );
      textureClippingBounds[1] = static_cast< int >( textureClippingBounds[1] / localStorage->m_mmPerPixel[0] + 0.5 );
      textureClippingBounds[2] = static_cast< int >( textureClippingBounds[2] / localStorage->m_mmPerPixel[1] + 0.5 );
      textureClippingBounds[3] = static_cast< int >( textureClippingBounds[3] / localStorage->m_mmPerPixel[1] + 0.5 );

      //clipping bounds for cutting the image
      localStorage->m_LevelWindowFilter->SetClippingBounds(textureClippingBounds);
    }

    this->ApplyOpacity( renderer );

    this->ApplyLookuptable( renderer );
    this->ApplyLevelWindow( renderer );
    this->ApplyColor( renderer );

    // do not use a VTK lookup table (we do that ourselves in m_LevelWindowFilter)
    localStorage->m_Texture->MapColorScalarsThroughLookupTableOff();

    //connect the input with the levelwindow filter
    localStorage->m_LevelWindowFilter->SetInputData(localStorage->m_EvaluationImage);

    // check for texture interpolation property
    bool textureInterpolation = false;
    GetDataNode()->GetBoolProperty( "texture interpolation", textureInterpolation, renderer );

    //set the interpolation modus according to the property
    localStorage->m_Texture->SetInterpolate(textureInterpolation);

    // connect the texture with the output of the levelwindow filter
    localStorage->m_Texture->SetInputConnection(localStorage->m_LevelWindowFilter->GetOutputPort());

    this->TransformActor( renderer );

    vtkActor* contourShadowActor = dynamic_cast<vtkActor*> (localStorage->m_Actors->GetParts()->GetItemAsObject(0));

    //Connect the mapper with the input texture. This is the standard case.
    //setup the textured plane
    this->GeneratePlane( renderer, sliceBounds );
    //set the plane as input for the mapper
    localStorage->m_Mapper->SetInputConnection(localStorage->m_Plane->GetOutputPort());
    //set the texture for the actor

    localStorage->m_Actor->SetTexture(localStorage->m_Texture);
    contourShadowActor->SetVisibility( false );

    // We have been modified => save this for next Update()
    localStorage->m_LastUpdateTime.Modified();
  }
}


void mitk::RegEvaluationMapper2D::PrepareContour( mitk::DataNode* datanode, LocalStorage * localStorage )
{
  bool targetContour = true;
  datanode->GetBoolProperty(mitk::nodeProp_RegEvalTargetContour,targetContour);

  vtkSmartPointer<vtkImageGradientMagnitude> magFilter =
    vtkSmartPointer<vtkImageGradientMagnitude>::New();
  if(targetContour)
  {
    magFilter->AddInputData(localStorage->m_slicedTargetImage->GetVtkImageData());
  }
  else
  {
    magFilter->AddInputData(localStorage->m_slicedMappedImage->GetVtkImageData());
  }

  vtkSmartPointer<vtkImageAppendComponents> appendFilter =
    vtkSmartPointer<vtkImageAppendComponents>::New();

  appendFilter->AddInputConnection(magFilter->GetOutputPort());
  appendFilter->AddInputConnection(magFilter->GetOutputPort());
  if(targetContour)
  {
    appendFilter->AddInputData(localStorage->m_slicedMappedImage->GetVtkImageData());
  }
  else
  {
    appendFilter->AddInputData(localStorage->m_slicedTargetImage->GetVtkImageData());
  }
  appendFilter->Update();

  localStorage->m_EvaluationImage = appendFilter->GetOutput();
}

void mitk::RegEvaluationMapper2D::PrepareDifference( LocalStorage * localStorage )
{
  vtkSmartPointer<vtkImageMathematics> diffFilter =
    vtkSmartPointer<vtkImageMathematics>::New();
  vtkSmartPointer<vtkImageMathematics> absFilter =
    vtkSmartPointer<vtkImageMathematics>::New();
  diffFilter->SetInput1Data(localStorage->m_slicedTargetImage->GetVtkImageData());
  diffFilter->SetInput2Data(localStorage->m_slicedMappedImage->GetVtkImageData());
  diffFilter->SetOperationToSubtract();
  absFilter->SetInputConnection(diffFilter->GetOutputPort());
  absFilter->SetOperationToAbsoluteValue();
  diffFilter->Update();
  localStorage->m_EvaluationImage = diffFilter->GetOutput();
}

void mitk::RegEvaluationMapper2D::PrepareWipe( mitk::DataNode* datanode, LocalStorage * localStorage )
{
  mitk::RegEvalWipeStyleProperty::Pointer evalWipeStyleProp = mitk::RegEvalWipeStyleProperty::New();
  datanode->GetProperty(evalWipeStyleProp, mitk::nodeProp_RegEvalWipeStyle);

  vtkSmartPointer<vtkImageRectilinearWipe> wipedFilter =
    vtkSmartPointer<vtkImageRectilinearWipe>::New();
  wipedFilter->SetInput1Data(localStorage->m_slicedTargetImage->GetVtkImageData());
  wipedFilter->SetInput2Data(localStorage->m_slicedMappedImage->GetVtkImageData());
  wipedFilter->SetPosition(30,30);

  if (evalWipeStyleProp->GetValueAsId() == 0)
  {
    wipedFilter->SetWipeToQuad();
  }
  else if (evalWipeStyleProp->GetValueAsId() == 1)
  {
    wipedFilter->SetWipeToHorizontal();
  }
  else if (evalWipeStyleProp->GetValueAsId() == 2)
  {
    wipedFilter->SetWipeToVertical();
  }

  wipedFilter->Update();

  localStorage->m_EvaluationImage = wipedFilter->GetOutput();
}

void mitk::RegEvaluationMapper2D::PrepareCheckerBoard( mitk::DataNode* datanode, LocalStorage * localStorage )
{
  int checkerCount = 5;
  datanode->GetIntProperty(mitk::nodeProp_RegEvalCheckerCount,checkerCount);

  vtkSmartPointer<vtkImageCheckerboard> checkerboardFilter =
    vtkSmartPointer<vtkImageCheckerboard>::New();
  checkerboardFilter->SetInput1Data(localStorage->m_slicedTargetImage->GetVtkImageData());
  checkerboardFilter->SetInput2Data(localStorage->m_slicedMappedImage->GetVtkImageData());
  checkerboardFilter->SetNumberOfDivisions(checkerCount,checkerCount,1);
  checkerboardFilter->Update();

  localStorage->m_EvaluationImage = checkerboardFilter->GetOutput();
}

void mitk::RegEvaluationMapper2D::PrepareColorBlend( LocalStorage * localStorage )
{
  vtkSmartPointer<vtkImageAppendComponents> appendFilter =
    vtkSmartPointer<vtkImageAppendComponents>::New();
  //red channel
  appendFilter->AddInputData(localStorage->m_slicedMappedImage->GetVtkImageData());
  //green channel
  appendFilter->AddInputData(localStorage->m_slicedMappedImage->GetVtkImageData());
  //blue channel
  appendFilter->AddInputData(localStorage->m_slicedTargetImage->GetVtkImageData());
  appendFilter->Update();

  localStorage->m_EvaluationImage = appendFilter->GetOutput();
}

void mitk::RegEvaluationMapper2D::PrepareBlend( mitk::DataNode* datanode, LocalStorage * localStorage )
{
  int blendfactor = 50;
  datanode->GetIntProperty(mitk::nodeProp_RegEvalBlendFactor,blendfactor);

  vtkSmartPointer<vtkImageWeightedSum> blendFilter =
    vtkSmartPointer<vtkImageWeightedSum>::New();
  blendFilter->AddInputData(localStorage->m_slicedTargetImage->GetVtkImageData());
  blendFilter->AddInputData(localStorage->m_slicedMappedImage->GetVtkImageData());
  blendFilter->SetWeight(0,(100-blendfactor)/100.);
  blendFilter->SetWeight(1,blendfactor/100.);
  blendFilter->Update();

  localStorage->m_EvaluationImage = blendFilter->GetOutput();
}

void mitk::RegEvaluationMapper2D::ApplyLevelWindow(mitk::BaseRenderer *renderer)
{
  LocalStorage *localStorage = this->GetLocalStorage( renderer );

  LevelWindow levelWindow;
  this->GetDataNode()->GetLevelWindow( levelWindow, renderer, "levelwindow" );
  localStorage->m_LevelWindowFilter->GetLookupTable()->SetRange( levelWindow.GetLowerWindowBound(), levelWindow.GetUpperWindowBound() );

  mitk::LevelWindow opacLevelWindow;
  if( this->GetDataNode()->GetLevelWindow( opacLevelWindow, renderer, "opaclevelwindow" ) )
  {
    //pass the opaque level window to the filter
    localStorage->m_LevelWindowFilter->SetMinOpacity(opacLevelWindow.GetLowerWindowBound());
    localStorage->m_LevelWindowFilter->SetMaxOpacity(opacLevelWindow.GetUpperWindowBound());
  }
  else
  {
    //no opaque level window
    localStorage->m_LevelWindowFilter->SetMinOpacity(0.0);
    localStorage->m_LevelWindowFilter->SetMaxOpacity(255.0);
  }
}

void mitk::RegEvaluationMapper2D::ApplyColor( mitk::BaseRenderer* renderer )
{
  LocalStorage *localStorage = this->GetLocalStorage( renderer );

  float rgb[3]= { 1.0f, 1.0f, 1.0f };

  // check for color prop and use it for rendering if it exists
  // binary image hovering & binary image selection
  bool hover    = false;
  bool selected = false;
  GetDataNode()->GetBoolProperty("binaryimage.ishovering", hover, renderer);
  GetDataNode()->GetBoolProperty("selected", selected, renderer);
  if(hover && !selected)
  {
    mitk::ColorProperty::Pointer colorprop = dynamic_cast<mitk::ColorProperty*>(GetDataNode()->GetProperty
      ("binaryimage.hoveringcolor", renderer));
    if(colorprop.IsNotNull())
    {
      memcpy(rgb, colorprop->GetColor().GetDataPointer(), 3*sizeof(float));
    }
    else
    {
      GetDataNode()->GetColor( rgb, renderer, "color" );
    }
  }
  if(selected)
  {
    mitk::ColorProperty::Pointer colorprop = dynamic_cast<mitk::ColorProperty*>(GetDataNode()->GetProperty
      ("binaryimage.selectedcolor", renderer));
    if(colorprop.IsNotNull()) {
      memcpy(rgb, colorprop->GetColor().GetDataPointer(), 3*sizeof(float));
    }
    else
    {
      GetDataNode()->GetColor(rgb, renderer, "color");
    }
  }
  if(!hover && !selected)
  {
    GetDataNode()->GetColor( rgb, renderer, "color" );
  }

  double rgbConv[3] = {(double)rgb[0], (double)rgb[1], (double)rgb[2]}; //conversion to double for VTK
  dynamic_cast<vtkActor*> (localStorage->m_Actors->GetParts()->GetItemAsObject(0))->GetProperty()->SetColor(rgbConv);
  localStorage->m_Actor->GetProperty()->SetColor(rgbConv);

  if ( localStorage->m_Actors->GetParts()->GetNumberOfItems() > 1 )
  {
    float rgb[3]= { 1.0f, 1.0f, 1.0f };
    mitk::ColorProperty::Pointer colorprop = dynamic_cast<mitk::ColorProperty*>(GetDataNode()->GetProperty
      ("outline binary shadow color", renderer));
    if(colorprop.IsNotNull())
    {
      memcpy(rgb, colorprop->GetColor().GetDataPointer(), 3*sizeof(float));
    }
    double rgbConv[3] = {(double)rgb[0], (double)rgb[1], (double)rgb[2]}; //conversion to double for VTK
    dynamic_cast<vtkActor*>( localStorage->m_Actors->GetParts()->GetItemAsObject(0) )->GetProperty()->SetColor(rgbConv);
  }
}

void mitk::RegEvaluationMapper2D::ApplyOpacity( mitk::BaseRenderer* renderer )
{
  LocalStorage* localStorage = this->GetLocalStorage( renderer );
  float opacity = 1.0f;
  // check for opacity prop and use it for rendering if it exists
  GetDataNode()->GetOpacity( opacity, renderer, "opacity" );
  //set the opacity according to the properties
  localStorage->m_Actor->GetProperty()->SetOpacity(opacity);
  if ( localStorage->m_Actors->GetParts()->GetNumberOfItems() > 1 )
  {
    dynamic_cast<vtkActor*>( localStorage->m_Actors->GetParts()->GetItemAsObject(0) )->GetProperty()->SetOpacity(opacity);
  }
}

void mitk::RegEvaluationMapper2D::ApplyLookuptable( mitk::BaseRenderer* renderer )
{
  LocalStorage* localStorage = m_LSH.GetLocalStorage(renderer);
  vtkLookupTable* usedLookupTable = localStorage->m_ColorLookupTable;

  // If lookup table or transferfunction use is requested...
  mitk::LookupTableProperty::Pointer lookupTableProp = dynamic_cast<mitk::LookupTableProperty*>(this->GetDataNode()->GetProperty("LookupTable"));

  if( lookupTableProp.IsNotNull() ) // is a lookuptable set?
  {
    usedLookupTable = lookupTableProp->GetLookupTable()->GetVtkLookupTable();
  }
  else
  {
    //"Image Rendering.Mode was set to use a lookup table but there is no property 'LookupTable'.
    //A default (rainbow) lookup table will be used.
    //Here have to do nothing. Warning for the user has been removed, due to unwanted console output
    //in every interation of the rendering.
  }
  localStorage->m_LevelWindowFilter->SetLookupTable(usedLookupTable);
}

void mitk::RegEvaluationMapper2D::Update(mitk::BaseRenderer* renderer)
{

  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");

  if ( !visible )
  {
    return;
  }

  mitk::Image* data  = const_cast<mitk::Image *>( this->GetTargetImage() );
  if ( data == NULL )
  {
    return;
  }

  // Calculate time step of the input data for the specified renderer (integer value)
  this->CalculateTimeStep( renderer );

  // Check if time step is valid
  const TimeGeometry *dataTimeGeometry = data->GetTimeGeometry();
  if ( ( dataTimeGeometry == NULL )
    || ( dataTimeGeometry->CountTimeSteps() == 0 )
    || ( !dataTimeGeometry->IsValidTimeStep( this->GetTimestep() ) ) )
  {
    return;
  }

  const DataNode *node = this->GetDataNode();
  data->UpdateOutputInformation();
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  //check if something important has changed and we need to rerender
  if ( (localStorage->m_LastUpdateTime < node->GetMTime()) //was the node modified?
    || (localStorage->m_LastUpdateTime < data->GetPipelineMTime()) //Was the data modified?
    || (localStorage->m_LastUpdateTime < renderer->GetCurrentWorldGeometry2DUpdateTime()) //was the geometry modified?
    || (localStorage->m_LastUpdateTime < renderer->GetCurrentWorldGeometry2D()->GetMTime())
    || (localStorage->m_LastUpdateTime < node->GetPropertyList()->GetMTime()) //was a property modified?
    || (localStorage->m_LastUpdateTime < node->GetPropertyList(renderer)->GetMTime()) )
  {
    this->GenerateDataForRenderer( renderer );
  }

  // since we have checked that nothing important has changed, we can set
  // m_LastUpdateTime to the current time
  localStorage->m_LastUpdateTime.Modified();
}

void mitk::RegEvaluationMapper2D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  mitk::RegEvaluationObject* regEval = dynamic_cast<mitk::RegEvaluationObject*>(node->GetData());

  if(!regEval)
  {
    return;
  }

  // Properties common for both images and segmentations
  node->AddProperty( "depthOffset", mitk::FloatProperty::New( 0.0 ), renderer, overwrite );
  if(regEval->GetTargetImage() && regEval->GetTargetImage()->IsRotated()) node->AddProperty( "reslice interpolation", mitk::VtkResliceInterpolationProperty::New(VTK_RESLICE_CUBIC) );
  else node->AddProperty( "reslice interpolation", mitk::VtkResliceInterpolationProperty::New() );
  node->AddProperty( "texture interpolation", mitk::BoolProperty::New( false ) );  // set to user configurable default value (see global options)
  node->AddProperty( "in plane resample extent by geometry", mitk::BoolProperty::New( false ) );
  node->AddProperty( "bounding box", mitk::BoolProperty::New( false ) );

  mitk::RenderingModeProperty::Pointer renderingModeProperty = mitk::RenderingModeProperty::New();
  node->AddProperty( "Image Rendering.Mode", renderingModeProperty);

  // Set default grayscale look-up table
  mitk::LookupTable::Pointer mitkLut = mitk::LookupTable::New();
  mitk::LookupTableProperty::Pointer mitkLutProp = mitk::LookupTableProperty::New();
  mitkLutProp->SetLookupTable(mitkLut);
  node->SetProperty("LookupTable", mitkLutProp);

  node->AddProperty( "opacity", mitk::FloatProperty::New(1.0f), renderer, overwrite );
  node->AddProperty( "color", ColorProperty::New(1.0,1.0,1.0), renderer, overwrite );
  node->AddProperty( "binary", mitk::BoolProperty::New( false ), renderer, overwrite );
  node->AddProperty("layer", mitk::IntProperty::New(0), renderer, overwrite);

  node->AddProperty(mitk::nodeProp_RegEvalStyle, mitk::RegEvalStyleProperty::New(0), renderer, overwrite);
  node->AddProperty(mitk::nodeProp_RegEvalBlendFactor, mitk::IntProperty::New(50), renderer, overwrite);
  node->AddProperty(mitk::nodeProp_RegEvalCheckerCount, mitk::IntProperty::New(3), renderer, overwrite);
  node->AddProperty(mitk::nodeProp_RegEvalTargetContour, mitk::BoolProperty::New(true), renderer, overwrite);
  node->AddProperty(mitk::nodeProp_RegEvalWipeStyle, mitk::RegEvalWipeStyleProperty::New(0), renderer, overwrite);

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}

mitk::RegEvaluationMapper2D::LocalStorage* mitk::RegEvaluationMapper2D::GetLocalStorage(mitk::BaseRenderer* renderer)
{
  return m_LSH.GetLocalStorage(renderer);
}

void mitk::RegEvaluationMapper2D::TransformActor(mitk::BaseRenderer* renderer)
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);
  //get the transformation matrix of the reslicer in order to render the slice as axial, coronal or saggital
  vtkSmartPointer<vtkTransform> trans = vtkSmartPointer<vtkTransform>::New();
  vtkSmartPointer<vtkMatrix4x4> matrix = localStorage->m_Reslicer->GetResliceAxes();
  trans->SetMatrix(matrix);
  //transform the plane/contour (the actual actor) to the corresponding view (axial, coronal or saggital)
  localStorage->m_Actor->SetUserTransform(trans);
  //transform the origin to center based coordinates, because MITK is center based.
  localStorage->m_Actor->SetPosition( -0.5*localStorage->m_mmPerPixel[0], -0.5*localStorage->m_mmPerPixel[1], 0.0);

  if ( localStorage->m_Actors->GetNumberOfPaths() > 1 )
  {
    vtkActor* secondaryActor = dynamic_cast<vtkActor*>( localStorage->m_Actors->GetParts()->GetItemAsObject(0) );
    secondaryActor->SetUserTransform(trans);
    secondaryActor->SetPosition( -0.5*localStorage->m_mmPerPixel[0], -0.5*localStorage->m_mmPerPixel[1], 0.0);
  }
}

bool mitk::RegEvaluationMapper2D::RenderingGeometryIntersectsImage( const Geometry2D* renderingGeometry, SlicedGeometry3D* imageGeometry )
{
  // if either one of the two geometries is NULL we return true
  // for safety reasons
  if ( renderingGeometry == NULL || imageGeometry == NULL )
    return true;

  // get the distance for the first cornerpoint
  ScalarType initialDistance = renderingGeometry->SignedDistance( imageGeometry->GetCornerPoint( 0 ) );
  for( int i=1; i<8; i++ )
  {
    mitk::Point3D cornerPoint = imageGeometry->GetCornerPoint( i );

    // get the distance to the other cornerpoints
    ScalarType distance = renderingGeometry->SignedDistance( cornerPoint );

    // if it has not the same signing as the distance of the first point
    if ( initialDistance * distance < 0 )
    {
      // we have an intersection and return true
      return true;
    }
  }

  // all distances have the same sign, no intersection and we return false
  return false;
}

mitk::RegEvaluationMapper2D::LocalStorage::~LocalStorage()
{
}

mitk::RegEvaluationMapper2D::LocalStorage::LocalStorage()
{

  m_LevelWindowFilter = vtkSmartPointer<vtkMitkLevelWindowFilter>::New();

  //Do as much actions as possible in here to avoid double executions.
  m_Plane = vtkSmartPointer<vtkPlaneSource>::New();
  //m_Texture = vtkSmartPointer<vtkNeverTranslucentTexture>::New().GetPointer();
  m_Texture = vtkSmartPointer<vtkOpenGLTexture>::New().GetPointer();
  m_DefaultLookupTable = vtkSmartPointer<vtkLookupTable>::New();
  m_ColorLookupTable = vtkSmartPointer<vtkLookupTable>::New();
  m_Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_Actor = vtkSmartPointer<vtkActor>::New();
  m_Actors = vtkSmartPointer<vtkPropAssembly>::New();
  m_Reslicer = mitk::ExtractSliceFilter::New();
  m_EvaluationImage = vtkSmartPointer<vtkImageData>::New();
  m_EmptyPolyData = vtkSmartPointer<vtkPolyData>::New();

  mitk::LookupTable::Pointer mitkLUT = mitk::LookupTable::New();
  //built a default lookuptable
  mitkLUT->SetType(mitk::LookupTable::GRAYSCALE);
  m_DefaultLookupTable = mitkLUT->GetVtkLookupTable();

  mitkLUT->SetType(mitk::LookupTable::JET);
  m_ColorLookupTable = mitkLUT->GetVtkLookupTable();

  //do not repeat the texture (the image)
  m_Texture->RepeatOff();

  //set the mapper for the actor
  m_Actor->SetMapper( m_Mapper );

  vtkSmartPointer<vtkActor> outlineShadowActor = vtkSmartPointer<vtkActor>::New();
  outlineShadowActor->SetMapper( m_Mapper );

  m_Actors->AddPart( outlineShadowActor );
  m_Actors->AddPart( m_Actor );
}
