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
#include <mitkDataNodeFactory.h>
#include <mitkImageSliceSelector.h>
#include <mitkLevelWindowProperty.h>
#include <mitkLookupTableProperty.h>
#include <mitkPlaneGeometry.h>
#include <mitkProperties.h>
#include <mitkResliceMethodProperty.h>
#include <mitkTimeSlicedGeometry.h>
#include <mitkVtkResliceInterpolationProperty.h>
#include <mitkPixelType.h>
//#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>
#include "mitkImageStatisticsHolder.h"

//MITK Rendering
#include "mitkImageVtkMapper2D.h"
#include "vtkMitkThickSlicesFilter.h"
#include "vtkMitkApplyLevelWindowToRGBFilter.h"

//VTK
#include <vtkProperty.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkLookupTable.h>
#include <vtkImageData.h>
#include <vtkPoints.h>
#include <vtkGeneralTransform.h>
#include <vtkImageReslice.h>
#include <vtkImageChangeInformation.h>
#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkTexture.h>
#include <vtkCellArray.h>
#include <vtkCamera.h>
#include <vtkColorTransferFunction.h>

//ITK
#include <itkRGBAPixel.h>

mitk::ImageVtkMapper2D::ImageVtkMapper2D()
{
}

mitk::ImageVtkMapper2D::~ImageVtkMapper2D()
{
  //The 3D RW Mapper (Geometry2DDataVtkMapper3D) is listening to this event,
  //in order to delete the images from the 3D RW.
  this->InvokeEvent( itk::DeleteEvent() );
}

//set the two points defining the textured plane according to the dimension and spacing
void mitk::ImageVtkMapper2D::GeneratePlane(mitk::BaseRenderer* renderer, vtkFloatingPointType planeBounds[6])
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

float mitk::ImageVtkMapper2D::CalculateLayerDepth(mitk::BaseRenderer* renderer)
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

const mitk::Image* mitk::ImageVtkMapper2D::GetInput( void )
{
  return static_cast< const mitk::Image * >( this->GetData() );
}

vtkProp* mitk::ImageVtkMapper2D::GetVtkProp(mitk::BaseRenderer* renderer)
{
  //return the actor corresponding to the renderer
  return m_LSH.GetLocalStorage(renderer)->m_Actors;
}

void mitk::ImageVtkMapper2D::MitkRenderOverlay(BaseRenderer* renderer)
{
  if ( this->IsVisible(renderer)==false )
    return;
  if ( this->GetVtkProp(renderer)->GetVisibility() )
  {
    this->GetVtkProp(renderer)->RenderOverlay(renderer->GetVtkRenderer());
  }
}

void mitk::ImageVtkMapper2D::MitkRenderOpaqueGeometry(BaseRenderer* renderer)
{
  if ( this->IsVisible( renderer )==false )
    return;
  if ( this->GetVtkProp(renderer)->GetVisibility() )
  {
    this->GetVtkProp(renderer)->RenderOpaqueGeometry( renderer->GetVtkRenderer() );
  }
}

void mitk::ImageVtkMapper2D::MitkRenderTranslucentGeometry(BaseRenderer* renderer)
{
  if ( this->IsVisible(renderer)==false )
    return;
  if ( this->GetVtkProp(renderer)->GetVisibility() )
  {
    this->GetVtkProp(renderer)->RenderTranslucentPolygonalGeometry(renderer->GetVtkRenderer());
  }
}

void mitk::ImageVtkMapper2D::MitkRenderVolumetricGeometry(BaseRenderer* renderer)
{
  if(IsVisible(renderer)==false)
    return;
  if ( GetVtkProp(renderer)->GetVisibility() )
  {
    this->GetVtkProp(renderer)->RenderVolumetricGeometry(renderer->GetVtkRenderer());
  }
}

void mitk::ImageVtkMapper2D::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  mitk::Image *input = const_cast< mitk::Image * >( this->GetInput() );
  mitk::DataNode* datanode = this->GetDataNode();

  if ( input == NULL || input->IsInitialized() == false )
  {
    return;
  }

  //check if there is a valid worldGeometry
  const Geometry2D *worldGeometry = renderer->GetCurrentWorldGeometry2D();
  if( ( worldGeometry == NULL ) || ( !worldGeometry->IsValid() ) || ( !worldGeometry->HasReferenceGeometry() ))
  {
    return;
  }

  input->Update();

  // early out if there is no intersection of the current rendering geometry
  // and the geometry of the image that is to be rendered.
  if ( !RenderingGeometryIntersectsImage( worldGeometry, input->GetSlicedGeometry() ) )
  {
    // set image to NULL, to clear the texture in 3D, because
    // the latest image is used there if the plane is out of the geometry
    // see bug-13275
    localStorage->m_ReslicedImage = NULL;
    localStorage->m_Mapper->SetInput( localStorage->m_EmptyPolyData );
    return;
  }


  //set main input for ExtractSliceFilter
  localStorage->m_Reslicer->SetInput(input);
  localStorage->m_Reslicer->SetWorldGeometry(worldGeometry);
  localStorage->m_Reslicer->SetTimeStep( this->GetTimestep() );


  //set the transformation of the image to adapt reslice axis
  localStorage->m_Reslicer->SetResliceTransformByGeometry( input->GetTimeSlicedGeometry()->GetGeometry3D( this->GetTimestep() ) );


  //is the geometry of the slice based on the input image or the worldgeometry?
  bool inPlaneResampleExtentByGeometry = false;
  datanode->GetBoolProperty("in plane resample extent by geometry", inPlaneResampleExtentByGeometry, renderer);
  localStorage->m_Reslicer->SetInPlaneResampleExtentByGeometry(inPlaneResampleExtentByGeometry);


  // Initialize the interpolation mode for resampling; switch to nearest
  // neighbor if the input image is too small.
  if ( (input->GetDimension() >= 3) && (input->GetDimension(2) > 1) )
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

  //set the vtk output property to true, makes sure that no unneeded mitk image convertion
  //is done.
  localStorage->m_Reslicer->SetVtkOutputRequest(true);


  //Thickslicing
  int thickSlicesMode = 0;
  int thickSlicesNum = 1;
  // Thick slices parameters
  if( input->GetPixelType().GetNumberOfComponents() == 1 ) // for now only single component are allowed
  {
    DataNode *dn=renderer->GetCurrentWorldGeometry2DNode();
    if(dn)
    {
      ResliceMethodProperty *resliceMethodEnumProperty=0;

      if( dn->GetProperty( resliceMethodEnumProperty, "reslice.thickslices" ) && resliceMethodEnumProperty )
        thickSlicesMode = resliceMethodEnumProperty->GetValueAsId();

      IntProperty *intProperty=0;
      if( dn->GetProperty( intProperty, "reslice.thickslices.num" ) && intProperty )
      {
        thickSlicesNum = intProperty->GetValue();
        if(thickSlicesNum < 1) thickSlicesNum=1;
        if(thickSlicesNum > 10) thickSlicesNum=10;
      }
    }
    else
    {
      MITK_WARN << "no associated widget plane data tree node found";
    }
  }


  if(thickSlicesMode > 0)
  {
    double dataZSpacing = 1.0;

    Vector3D normInIndex, normal;
    const PlaneGeometry *planeGeometry = dynamic_cast< const PlaneGeometry * >( worldGeometry );
    if ( planeGeometry != NULL ){
      normal = planeGeometry->GetNormal();
    }else{
      const mitk::AbstractTransformGeometry* abstractGeometry = dynamic_cast< const AbstractTransformGeometry * >(worldGeometry);
      if(abstractGeometry != NULL)
        normal = abstractGeometry->GetPlane()->GetNormal();
      else
        return; //no fitting geometry set
    }
    normal.Normalize();

    input->GetTimeSlicedGeometry()->GetGeometry3D( this->GetTimestep() )->WorldToIndex( normal, normInIndex );

    dataZSpacing = 1.0 / normInIndex.GetNorm();

    localStorage->m_Reslicer->SetOutputDimensionality( 3 );
    localStorage->m_Reslicer->SetOutputSpacingZDirection(dataZSpacing);
    localStorage->m_Reslicer->SetOutputExtentZDirection( -thickSlicesNum, 0+thickSlicesNum );

    // Do the reslicing. Modified() is called to make sure that the reslicer is
    // executed even though the input geometry information did not change; this
    // is necessary when the input /em data, but not the /em geometry changes.
    localStorage->m_TSFilter->SetThickSliceMode( thickSlicesMode-1 );
    localStorage->m_TSFilter->SetInput( localStorage->m_Reslicer->GetVtkOutput() );

    //vtkFilter=>mitkFilter=>vtkFilter update mechanism will fail without calling manually
    localStorage->m_Reslicer->Modified();
    localStorage->m_Reslicer->Update();

    localStorage->m_TSFilter->Modified();
    localStorage->m_TSFilter->Update();
    localStorage->m_ReslicedImage = localStorage->m_TSFilter->GetOutput();
  }
  else
  {
    //this is needed when thick mode was enable bevore. These variable have to be reset to default values
    localStorage->m_Reslicer->SetOutputDimensionality( 2 );
    localStorage->m_Reslicer->SetOutputSpacingZDirection(1.0);
    localStorage->m_Reslicer->SetOutputExtentZDirection( 0, 0 );


    localStorage->m_Reslicer->Modified();
    //start the pipeline with updating the largest possible, needed if the geometry of the input has changed
    localStorage->m_Reslicer->UpdateLargestPossibleRegion();
    localStorage->m_ReslicedImage = localStorage->m_Reslicer->GetVtkOutput();
  }


  // Bounds information for reslicing (only reuqired if reference geometry
  // is present)
  //this used for generating a vtkPLaneSource with the right size
  vtkFloatingPointType sliceBounds[6];
  for ( int i = 0; i < 6; ++i )
  {
    sliceBounds[i] = 0.0;
  }
  localStorage->m_Reslicer->GetClippedPlaneBounds(sliceBounds);

  //get the spacing of the slice
  localStorage->m_mmPerPixel = localStorage->m_Reslicer->GetOutputSpacing();


  //get the number of scalar components to distinguish between different image types
  int numberOfComponents = localStorage->m_ReslicedImage->GetNumberOfScalarComponents();
  //get the binary property
  bool binary = false;
  bool binaryOutline = false;
  datanode->GetBoolProperty( "binary", binary, renderer );
  if(binary) //binary image
  {
    datanode->GetBoolProperty( "outline binary", binaryOutline, renderer );
    if(binaryOutline) //contour rendering
    {
      if ( input->GetPixelType().GetBpe() <= 8 )
      {
        //generate contours/outlines
        localStorage->m_OutlinePolyData = CreateOutlinePolyData(renderer);

        float binaryOutlineWidth(1.0);
        if ( datanode->GetFloatProperty( "outline width", binaryOutlineWidth, renderer ) )
        {
          if ( localStorage->m_Actors->GetNumberOfPaths() > 1 )
          {
            float binaryOutlineShadowWidth(1.5);
            datanode->GetFloatProperty( "outline shadow width", binaryOutlineShadowWidth, renderer );

            dynamic_cast<vtkActor*>(localStorage->m_Actors->GetParts()->GetItemAsObject(0))
            ->GetProperty()->SetLineWidth( binaryOutlineWidth * binaryOutlineShadowWidth );
          }

          localStorage->m_Actor->GetProperty()->SetLineWidth( binaryOutlineWidth );
        }
      }
      else
      {
        binaryOutline = false;
        this->ApplyLookuptable(renderer);
        MITK_WARN << "Type of all binary images should be (un)signed char. Outline does not work on other pixel types!";
      }
    }
    else //standard binary image
    {
      if(numberOfComponents != 1)
      {
        MITK_ERROR << "Rendering Error: Binary Images with more then 1 component are not supported!";
      }
    }
    this->ApplyLookuptable(renderer);
    //Interpret the values as binary values
    localStorage->m_Texture->MapColorScalarsThroughLookupTableOn();
  }
  else if( numberOfComponents == 1 ) //gray images
  {
    //Interpret the values as gray values
    localStorage->m_Texture->MapColorScalarsThroughLookupTableOn();

    this->ApplyLookuptable(renderer);
  }
  else if ( (numberOfComponents == 3) || (numberOfComponents == 4) ) //RBG(A) images
  {
    //Interpret the RGB(A) images values correctly
    localStorage->m_Texture->MapColorScalarsThroughLookupTableOff();

    this->ApplyLookuptable(renderer);
    this->ApplyRBGALevelWindow(renderer);
  }
  else
  {
    MITK_ERROR << "2D Reindering Error: Unknown number of components!!! Please report to rendering task force or check your data!";
  }

  this->ApplyColor( renderer );
  this->ApplyOpacity( renderer );
  this->TransformActor( renderer );

  vtkActor* contourShadowActor = dynamic_cast<vtkActor*> (localStorage->m_Actors->GetParts()->GetItemAsObject(0));

  if(binary && binaryOutline) //connect the mapper with the polyData which contains the lines
  {
    //We need the contour for the binary outline property as actor
    localStorage->m_Mapper->SetInput(localStorage->m_OutlinePolyData);
    localStorage->m_Actor->SetTexture(NULL); //no texture for contours

    bool binaryOutlineShadow( false );
    datanode->GetBoolProperty( "outline binary shadow", binaryOutlineShadow, renderer );

    if ( binaryOutlineShadow )
      contourShadowActor->SetVisibility( true );
    else
      contourShadowActor->SetVisibility( false );

  }
  else
  { //Connect the mapper with the input texture. This is the standard case.
    //setup the textured plane
    this->GeneratePlane( renderer, sliceBounds );
    //set the plane as input for the mapper
    localStorage->m_Mapper->SetInputConnection(localStorage->m_Plane->GetOutputPort());
    //set the texture for the actor

    localStorage->m_Actor->SetTexture(localStorage->m_Texture);
    contourShadowActor->SetVisibility( false );
  }

  // We have been modified => save this for next Update()
  localStorage->m_LastUpdateTime.Modified();
}

void mitk::ImageVtkMapper2D::ApplyColor( mitk::BaseRenderer* renderer )
{
  LocalStorage *localStorage = this->GetLocalStorage( renderer );

  // check for interpolation properties
  bool textureInterpolation = false;
  GetDataNode()->GetBoolProperty( "texture interpolation", textureInterpolation, renderer );

  //set the interpolation modus according to the property
  localStorage->m_Texture->SetInterpolate(textureInterpolation);

  bool useColor = true;
  this->GetDataNode()->GetBoolProperty( "use color", useColor, renderer );
  if( useColor )
  {
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
        GetColor( rgb, renderer );
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
        GetColor( rgb, renderer );
      }
    }
    if(!hover && !selected)
    {
      GetColor( rgb, renderer );
    }

    double rgbConv[3] = {(double)rgb[0], (double)rgb[1], (double)rgb[2]}; //conversion to double for VTK
    dynamic_cast<vtkActor*> (localStorage->m_Actors->GetParts()->GetItemAsObject(0))->GetProperty()->SetColor(rgbConv);
    localStorage->m_Actor->GetProperty()->SetColor(rgbConv);
  }
  else
  {
    //If the user defines a lut, we dont want to use the color and take white instead.
    dynamic_cast<vtkActor*> (localStorage->m_Actors->GetParts()->GetItemAsObject(0))->GetProperty()->SetColor(1.0, 1.0, 1.0);
    localStorage->m_Actor->GetProperty()->SetColor(1.0, 1.0, 1.0);
  }

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

void mitk::ImageVtkMapper2D::ApplyOpacity( mitk::BaseRenderer* renderer )
{
  LocalStorage* localStorage = this->GetLocalStorage( renderer );
  float opacity = 1.0f;
  // check for opacity prop and use it for rendering if it exists
  GetOpacity( opacity, renderer );
  //set the opacity according to the properties
  localStorage->m_Actor->GetProperty()->SetOpacity(opacity);
  if ( localStorage->m_Actors->GetParts()->GetNumberOfItems() > 1 )
  {
    dynamic_cast<vtkActor*>( localStorage->m_Actors->GetParts()->GetItemAsObject(0) )->GetProperty()->SetOpacity(opacity);
  }

}

void mitk::ImageVtkMapper2D::ApplyLookuptable( mitk::BaseRenderer* renderer )
{
  bool binary = false;
  bool CTFcanBeApplied = false;
  this->GetDataNode()->GetBoolProperty( "binary", binary, renderer );
  LocalStorage* localStorage = this->GetLocalStorage(renderer);

  //default lookuptable
  localStorage->m_Texture->SetLookupTable( localStorage->m_LookupTable );

  if(binary)
  {
    //default lookuptable for binary images
    localStorage->m_Texture->GetLookupTable()->SetRange(0.0, 1.0);
  }
  else
  {
    bool useColor = true;
    this->GetDataNode()->GetBoolProperty( "use color", useColor, renderer );
    if((!useColor))
    {
      //BEGIN PROPERTY user-defined lut
      //currently we do not allow a lookuptable if it is a binary image
      // If lookup table use is requested...
      mitk::LookupTableProperty::Pointer LookupTableProp;
      LookupTableProp = dynamic_cast<mitk::LookupTableProperty*>
        (this->GetDataNode()->GetProperty("LookupTable"));
      //...check if there is a lookuptable provided by the user
      if ( LookupTableProp.IsNotNull() )
      {
        // If lookup table use is requested and supplied by the user:
        // only update the lut, when the properties have changed...
        if( LookupTableProp->GetLookupTable()->GetMTime()
          <= this->GetDataNode()->GetPropertyList()->GetMTime() )
        {
          LookupTableProp->GetLookupTable()->ChangeOpacityForAll( LookupTableProp->GetLookupTable()->GetVtkLookupTable()->GetAlpha()*localStorage->m_Actor->GetProperty()->GetOpacity() );
          LookupTableProp->GetLookupTable()->ChangeOpacity(0, 0.0);
        }
        //we use the user-defined lookuptable
        localStorage->m_Texture->SetLookupTable( LookupTableProp->GetLookupTable()->GetVtkLookupTable() );
      }
      else
      {
        CTFcanBeApplied = true;
      }
    }//END PROPERTY user-defined lut
    LevelWindow levelWindow;
    this->GetLevelWindow( levelWindow, renderer );
    //set up the lookuptable with the level window range
    localStorage->m_Texture->GetLookupTable()->SetRange( levelWindow.GetLowerWindowBound(), levelWindow.GetUpperWindowBound() );
  }
  //the color function can be applied if the user does not want to use color
  //and does not provide a lookuptable
  if(CTFcanBeApplied)
  {
    ApplyColorTransferFunction(renderer);
  }
  localStorage->m_Texture->SetInput( localStorage->m_ReslicedImage );
}

void mitk::ImageVtkMapper2D::ApplyColorTransferFunction(mitk::BaseRenderer* renderer)
{
  mitk::TransferFunctionProperty::Pointer transferFunctionProperty =
    dynamic_cast<mitk::TransferFunctionProperty*>(this->GetDataNode()->GetProperty("Image Rendering.Transfer Function",renderer ));
  LocalStorage* localStorage = m_LSH.GetLocalStorage(renderer);
  if(transferFunctionProperty.IsNotNull())
  {
    localStorage->m_Texture->SetLookupTable(transferFunctionProperty->GetValue()->GetColorTransferFunction());
  }
  else
  {
    MITK_WARN << "Neither a lookuptable nor a transfer function is set and use color is off.";
  }
}



void mitk::ImageVtkMapper2D::ApplyRBGALevelWindow( mitk::BaseRenderer* renderer )
{
  LocalStorage* localStorage = this->GetLocalStorage( renderer );
  //pass the LuT to the RBG filter
  localStorage->m_LevelWindowToRGBFilterObject->SetLookupTable(localStorage->m_Texture->GetLookupTable());
  mitk::LevelWindow opacLevelWindow;
  if( this->GetLevelWindow( opacLevelWindow, renderer, "opaclevelwindow" ) )
  {//pass the opaque level window to the filter
    localStorage->m_LevelWindowToRGBFilterObject->SetMinOpacity(opacLevelWindow.GetLowerWindowBound());
    localStorage->m_LevelWindowToRGBFilterObject->SetMaxOpacity(opacLevelWindow.GetUpperWindowBound());
  }
  else
  {//no opaque level window
    localStorage->m_LevelWindowToRGBFilterObject->SetMinOpacity(0.0);
    localStorage->m_LevelWindowToRGBFilterObject->SetMaxOpacity(255.0);
  }
  localStorage->m_LevelWindowToRGBFilterObject->SetInput(localStorage->m_ReslicedImage);
  //connect the texture with the output of the RGB filter
  localStorage->m_Texture->SetInputConnection(localStorage->m_LevelWindowToRGBFilterObject->GetOutputPort());
}

void mitk::ImageVtkMapper2D::Update(mitk::BaseRenderer* renderer)
{
  if ( !this->IsVisible( renderer ) )
  {
    return;
  }

  mitk::Image* data  = const_cast<mitk::Image *>( this->GetInput() );
  if ( data == NULL )
  {
    return;
  }

  // Calculate time step of the input data for the specified renderer (integer value)
  this->CalculateTimeStep( renderer );

  // Check if time step is valid
  const TimeSlicedGeometry *dataTimeGeometry = data->GetTimeSlicedGeometry();
  if ( ( dataTimeGeometry == NULL )
    || ( dataTimeGeometry->GetTimeSteps() == 0 )
    || ( !dataTimeGeometry->IsValidTime( this->GetTimestep() ) ) )
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

void mitk::ImageVtkMapper2D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());

  // Properties common for both images and segmentations
  node->AddProperty( "use color", mitk::BoolProperty::New( true ), renderer, overwrite );
  node->AddProperty( "depthOffset", mitk::FloatProperty::New( 0.0 ), renderer, overwrite );
  node->AddProperty( "outline binary", mitk::BoolProperty::New( false ), renderer, overwrite );
  node->AddProperty( "outline width", mitk::FloatProperty::New( 1.0 ), renderer, overwrite );
  node->AddProperty( "outline binary shadow", mitk::BoolProperty::New( false ), renderer, overwrite );
  node->AddProperty( "outline binary shadow color", ColorProperty::New(0.0,0.0,0.0), renderer, overwrite );
  node->AddProperty( "outline shadow width", mitk::FloatProperty::New( 1.5 ), renderer, overwrite );
  if(image->IsRotated()) node->AddProperty( "reslice interpolation", mitk::VtkResliceInterpolationProperty::New(VTK_RESLICE_CUBIC) );
  else node->AddProperty( "reslice interpolation", mitk::VtkResliceInterpolationProperty::New() );
  node->AddProperty( "texture interpolation", mitk::BoolProperty::New( mitk::DataNodeFactory::m_TextureInterpolationActive ) );  // set to user configurable default value (see global options)
  node->AddProperty( "in plane resample extent by geometry", mitk::BoolProperty::New( false ) );
  node->AddProperty( "bounding box", mitk::BoolProperty::New( false ) );

  std::string photometricInterpretation; // DICOM tag telling us how pixel values should be displayed
  if ( node->GetStringProperty( "dicom.pixel.PhotometricInterpretation", photometricInterpretation ) )
  {
    // modality provided by DICOM or other reader
    if ( photometricInterpretation.find("MONOCHROME1") != std::string::npos ) // meaning: display MINIMUM pixels as WHITE
    {
      // generate LUT (white to black)
      mitk::LookupTable::Pointer mitkLut = mitk::LookupTable::New();
      vtkLookupTable* bwLut = mitkLut->GetVtkLookupTable();
      bwLut->SetTableRange (0, 1);
      bwLut->SetSaturationRange (0, 0);
      bwLut->SetHueRange (0, 0);
      bwLut->SetValueRange (1, 0);
      bwLut->SetAlphaRange (1, 1);
      bwLut->SetRampToLinear();
      bwLut->Build();
      mitk::LookupTableProperty::Pointer mitkLutProp = mitk::LookupTableProperty::New();
      mitkLutProp->SetLookupTable(mitkLut);
      node->SetProperty( "LookupTable", mitkLutProp );

      node->SetProperty( "use color", mitk::BoolProperty::New( false ), renderer );
    }
    else
    if ( photometricInterpretation.find("MONOCHROME2") != std::string::npos ) // meaning: display MINIMUM pixels as BLACK
    {
      // apply default LUT (black to white)
      node->SetProperty( "use color", mitk::BoolProperty::New( true ), renderer );
      node->SetProperty( "color", mitk::ColorProperty::New( 1,1,1 ), renderer );
    }
    // PALETTE interpretation should be handled ok by RGB loading
  }

  bool isBinaryImage(false);
  if ( ! node->GetBoolProperty("binary", isBinaryImage) )
  {

    // ok, property is not set, use heuristic to determine if this
    // is a binary image
    mitk::Image::Pointer centralSliceImage;
    ScalarType minValue = 0.0;
    ScalarType maxValue = 0.0;
    ScalarType min2ndValue = 0.0;
    ScalarType max2ndValue = 0.0;
    mitk::ImageSliceSelector::Pointer sliceSelector = mitk::ImageSliceSelector::New();

    sliceSelector->SetInput(image);
    sliceSelector->SetSliceNr(image->GetDimension(2)/2);
    sliceSelector->SetTimeNr(image->GetDimension(3)/2);
    sliceSelector->SetChannelNr(image->GetDimension(4)/2);
    sliceSelector->Update();
    centralSliceImage = sliceSelector->GetOutput();
    if ( centralSliceImage.IsNotNull() && centralSliceImage->IsInitialized() )
    {
      minValue    = centralSliceImage->GetStatistics()->GetScalarValueMin();
      maxValue    = centralSliceImage->GetStatistics()->GetScalarValueMax();
      min2ndValue = centralSliceImage->GetStatistics()->GetScalarValue2ndMin();
      max2ndValue = centralSliceImage->GetStatistics()->GetScalarValue2ndMax();
    }
    if ((maxValue == min2ndValue && minValue == max2ndValue) || minValue == maxValue)
    {
      // centralSlice is strange, lets look at all data
      minValue    = image->GetStatistics()->GetScalarValueMin();
      maxValue    = image->GetStatistics()->GetScalarValueMaxNoRecompute();
      min2ndValue = image->GetStatistics()->GetScalarValue2ndMinNoRecompute();
      max2ndValue = image->GetStatistics()->GetScalarValue2ndMaxNoRecompute();
    }
    isBinaryImage = ( maxValue == min2ndValue && minValue == max2ndValue );
  }

  // some more properties specific for a binary...
  if (isBinaryImage)
  {
    node->AddProperty( "opacity", mitk::FloatProperty::New(0.3f), renderer, overwrite );
    node->AddProperty( "color", ColorProperty::New(1.0,0.0,0.0), renderer, overwrite );
    node->AddProperty( "binaryimage.selectedcolor", ColorProperty::New(1.0,0.0,0.0), renderer, overwrite );
    node->AddProperty( "binaryimage.selectedannotationcolor", ColorProperty::New(1.0,0.0,0.0), renderer, overwrite );
    node->AddProperty( "binaryimage.hoveringcolor", ColorProperty::New(1.0,0.0,0.0), renderer, overwrite );
    node->AddProperty( "binaryimage.hoveringannotationcolor", ColorProperty::New(1.0,0.0,0.0), renderer, overwrite );
    node->AddProperty( "binary", mitk::BoolProperty::New( true ), renderer, overwrite );
    node->AddProperty("layer", mitk::IntProperty::New(10), renderer, overwrite);
  }
  else          //...or image type object
  {
    node->AddProperty( "opacity", mitk::FloatProperty::New(1.0f), renderer, overwrite );
    node->AddProperty( "color", ColorProperty::New(1.0,1.0,1.0), renderer, overwrite );
    node->AddProperty( "binary", mitk::BoolProperty::New( false ), renderer, overwrite );
    node->AddProperty("layer", mitk::IntProperty::New(0), renderer, overwrite);
  }

  if(image.IsNotNull() && image->IsInitialized())
  {
    if((overwrite) || (node->GetProperty("levelwindow", renderer)==NULL))
    {
      /* initialize level/window from DICOM tags */
      std::string sLevel;
      std::string sWindow;
      if ( image->GetPropertyList()->GetStringProperty( "dicom.voilut.WindowCenter", sLevel )
        && image->GetPropertyList()->GetStringProperty( "dicom.voilut.WindowWidth", sWindow ) )
      {
        float level = atof( sLevel.c_str() );
        float window = atof( sWindow.c_str() );

        mitk::LevelWindow contrast;
        std::string sSmallestPixelValueInSeries;
        std::string sLargestPixelValueInSeries;

        if ( image->GetPropertyList()->GetStringProperty( "dicom.series.SmallestPixelValueInSeries", sSmallestPixelValueInSeries )
          && image->GetPropertyList()->GetStringProperty( "dicom.series.LargestPixelValueInSeries", sLargestPixelValueInSeries ) )
        {
          float smallestPixelValueInSeries = atof( sSmallestPixelValueInSeries.c_str() );
          float largestPixelValueInSeries = atof( sLargestPixelValueInSeries.c_str() );
          contrast.SetRangeMinMax( smallestPixelValueInSeries-1, largestPixelValueInSeries+1 ); // why not a little buffer?
          // might remedy some l/w widget challenges
        }
        else
        {
          contrast.SetAuto( static_cast<mitk::Image*>(node->GetData()), false, true ); // we need this as a fallback
        }

        contrast.SetLevelWindow( level, window, true );
        node->SetProperty( "levelwindow", LevelWindowProperty::New( contrast ), renderer );
      }
    }
    if(((overwrite) || (node->GetProperty("opaclevelwindow", renderer)==NULL))
       && (image->GetPixelType().GetPixelTypeId() == itk::ImageIOBase::RGBA)
       && (image->GetPixelType().GetTypeId() == typeid( unsigned char)) )
    {
      mitk::LevelWindow opaclevwin;
      opaclevwin.SetRangeMinMax(0,255);
      opaclevwin.SetWindowBounds(0,255);
      mitk::LevelWindowProperty::Pointer prop = mitk::LevelWindowProperty::New(opaclevwin);
      node->SetProperty( "opaclevelwindow", prop, renderer );
    }
    if((overwrite) || (node->GetProperty("LookupTable", renderer)==NULL))
    {
      // add a default rainbow lookup table for color mapping
      mitk::LookupTable::Pointer mitkLut = mitk::LookupTable::New();
      vtkLookupTable* vtkLut = mitkLut->GetVtkLookupTable();
      vtkLut->SetHueRange(0.6667, 0.0);
      vtkLut->SetTableRange(0.0, 20.0);
      vtkLut->Build();
      mitk::LookupTableProperty::Pointer mitkLutProp = mitk::LookupTableProperty::New();
      mitkLutProp->SetLookupTable(mitkLut);
      node->SetProperty( "LookupTable", mitkLutProp );
    }
  }
  Superclass::SetDefaultProperties(node, renderer, overwrite);
}

mitk::ImageVtkMapper2D::LocalStorage* mitk::ImageVtkMapper2D::GetLocalStorage(mitk::BaseRenderer* renderer)
{
  return m_LSH.GetLocalStorage(renderer);
}

vtkSmartPointer<vtkPolyData> mitk::ImageVtkMapper2D::CreateOutlinePolyData(mitk::BaseRenderer* renderer ){
  LocalStorage* localStorage = this->GetLocalStorage(renderer);

  //get the min and max index values of each direction
  int* extent = localStorage->m_ReslicedImage->GetExtent();
  int xMin = extent[0];
  int xMax = extent[1];
  int yMin = extent[2];
  int yMax = extent[3];

  int* dims = localStorage->m_ReslicedImage->GetDimensions(); //dimensions of the image
  int line = dims[0]; //how many pixels per line?
  int x = xMin; //pixel index x
  int y = yMin; //pixel index y
  char* currentPixel;


  //get the depth for each contour
  float depth = CalculateLayerDepth(renderer);

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New(); //the points to draw
  vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New(); //the lines to connect the points

  // We take the pointer to the first pixel of the image
  currentPixel = static_cast<char*>(localStorage->m_ReslicedImage->GetScalarPointer() );

  while (y <= yMax)
  {
    //if the current pixel value is set to something
    if ((currentPixel) && (*currentPixel != 0))
    {
      //check in which direction a line is necessary
      //a line is added if the neighbor of the current pixel has the value 0
      //and if the pixel is located at the edge of the image

      //if   vvvvv  not the first line vvvvv
      if (y > yMin && *(currentPixel-line) == 0)
      { //x direction - bottom edge of the pixel
        //add the 2 points
        vtkIdType p1 = points->InsertNextPoint(x*localStorage->m_mmPerPixel[0], y*localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint((x+1)*localStorage->m_mmPerPixel[0], y*localStorage->m_mmPerPixel[1], depth);
        //add the line between both points
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }

      //if   vvvvv  not the last line vvvvv
      if (y < yMax && *(currentPixel+line) == 0)
      { //x direction - top edge of the pixel
        vtkIdType p1 = points->InsertNextPoint(x*localStorage->m_mmPerPixel[0], (y+1)*localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint((x+1)*localStorage->m_mmPerPixel[0], (y+1)*localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }

      //if   vvvvv  not the first pixel vvvvv
      if ( (x > xMin || y > yMin) && *(currentPixel-1) == 0)
      { //y direction - left edge of the pixel
        vtkIdType p1 = points->InsertNextPoint(x*localStorage->m_mmPerPixel[0], y*localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint(x*localStorage->m_mmPerPixel[0], (y+1)*localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }

      //if   vvvvv  not the last pixel vvvvv
      if ( (y < yMax || (x < xMax) ) && *(currentPixel+1) == 0)
      { //y direction - right edge of the pixel
        vtkIdType p1 = points->InsertNextPoint((x+1)*localStorage->m_mmPerPixel[0], y*localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint((x+1)*localStorage->m_mmPerPixel[0], (y+1)*localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }

      /*  now consider pixels at the edge of the image  */

      //if   vvvvv  left edge of image vvvvv
      if (x == xMin)
      { //draw left edge of the pixel
        vtkIdType p1 = points->InsertNextPoint(x*localStorage->m_mmPerPixel[0], y*localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint(x*localStorage->m_mmPerPixel[0], (y+1)*localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }

      //if   vvvvv  right edge of image vvvvv
      if (x == xMax)
      { //draw right edge of the pixel
        vtkIdType p1 = points->InsertNextPoint((x+1)*localStorage->m_mmPerPixel[0], y*localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint((x+1)*localStorage->m_mmPerPixel[0], (y+1)*localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }

      //if   vvvvv  bottom edge of image vvvvv
      if (y == yMin)
      { //draw bottom edge of the pixel
        vtkIdType p1 = points->InsertNextPoint(x*localStorage->m_mmPerPixel[0], y*localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint((x+1)*localStorage->m_mmPerPixel[0], y*localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }

      //if   vvvvv  top edge of image vvvvv
      if (y == yMax)
      { //draw top edge of the pixel
        vtkIdType p1 = points->InsertNextPoint(x*localStorage->m_mmPerPixel[0], (y+1)*localStorage->m_mmPerPixel[1], depth);
        vtkIdType p2 = points->InsertNextPoint((x+1)*localStorage->m_mmPerPixel[0], (y+1)*localStorage->m_mmPerPixel[1], depth);
        lines->InsertNextCell(2);
        lines->InsertCellPoint(p1);
        lines->InsertCellPoint(p2);
      }
    }//end if currentpixel is set

    x++;

    if (x > xMax)
    { //reached end of line
      x = xMin;
      y++;
    }

    // Increase the pointer-position to the next pixel.
    // This is safe, as the while-loop and the x-reset logic above makes
    // sure we do not exceed the bounds of the image
    currentPixel++;


}//end of while


  // Create a polydata to store everything in
  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  // Add the points to the dataset
  polyData->SetPoints(points);
  // Add the lines to the dataset
  polyData->SetLines(lines);
  return polyData;
}

void mitk::ImageVtkMapper2D::TransformActor(mitk::BaseRenderer* renderer)
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

bool mitk::ImageVtkMapper2D::RenderingGeometryIntersectsImage( const Geometry2D* renderingGeometry, SlicedGeometry3D* imageGeometry )
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

mitk::ImageVtkMapper2D::LocalStorage::LocalStorage()
{
  //Do as much actions as possible in here to avoid double executions.
  m_Plane = vtkSmartPointer<vtkPlaneSource>::New();
  m_Texture = vtkSmartPointer<vtkTexture>::New();
  m_LookupTable = vtkSmartPointer<vtkLookupTable>::New();
  m_Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  m_Actor = vtkSmartPointer<vtkActor>::New();
  m_Actors = vtkSmartPointer<vtkPropAssembly>::New();
  m_Reslicer = mitk::ExtractSliceFilter::New();
  m_TSFilter = vtkSmartPointer<vtkMitkThickSlicesFilter>::New();
  m_OutlinePolyData = vtkSmartPointer<vtkPolyData>::New();
  m_ReslicedImage = vtkSmartPointer<vtkImageData>::New();
  m_EmptyPolyData = vtkSmartPointer<vtkPolyData>::New();

  //the following actions are always the same and thus can be performed
  //in the constructor for each image (i.e. the image-corresponding local storage)
  m_TSFilter->ReleaseDataFlagOn();

  //built a default lookuptable
  m_LookupTable->SetRampToLinear();
  m_LookupTable->SetSaturationRange( 0.0, 0.0 );
  m_LookupTable->SetHueRange( 0.0, 0.0 );
  m_LookupTable->SetValueRange( 0.0, 1.0 );
  m_LookupTable->Build();
  //map all black values to transparent
  m_LookupTable->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);

  //do not repeat the texture (the image)
  m_Texture->RepeatOff();

  //set the mapper for the actor
  m_Actor->SetMapper( m_Mapper );

  vtkSmartPointer<vtkActor> outlineShadowActor = vtkSmartPointer<vtkActor>::New();
  outlineShadowActor->SetMapper( m_Mapper );

  m_Actors->AddPart( outlineShadowActor );
  m_Actors->AddPart( m_Actor );

  //filter for RGB(A) images
  m_LevelWindowToRGBFilterObject = new vtkMitkApplyLevelWindowToRGBFilter();
}
