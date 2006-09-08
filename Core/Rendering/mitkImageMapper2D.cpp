/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#include "mitkImageMapper2D.h"
#include "widget.h"
#include "picimage.h"
#include "pic2vtk.h"
#include "mitkTimeSlicedGeometry.h"
#include "mitkPlaneGeometry.h"
#include "mitkBaseRenderer.h"
#include "mitkDataTreeNode.h"

#include "mitkLookupTableProperty.h"
#include "mitkProperties.h"
#include "mitkLevelWindowProperty.h"

#include "mitkRenderWindow.h"
#include "mitkAbstractTransformGeometry.h"
#include "mitkAbstractTransformGeometry.h"

#include <vtkImageReslice.h>
#include <vtkLinearTransform.h>
#include <vtkGeneralTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkLookupTable.h>
#include <vtkImageData.h>
#include <vtkImageChangeInformation.h>

int mitk::ImageMapper2D::numRenderer = 0;
 

mitk::ImageMapper2D::ImageMapper2D() : m_SliceSelector(NULL)
{
  m_SliceSelector = ImageSliceSelector::New();
  m_Reslicer = vtkImageReslice::New();
  m_ComposedResliceTransform = vtkGeneralTransform::New();
  m_UnitSpacingImageFilter = vtkImageChangeInformation::New();
  m_UnitSpacingImageFilter->SetOutputSpacing( 1.0, 1.0, 1.0 );
}

//##ModelId=3E32DCF60043
mitk::ImageMapper2D::~ImageMapper2D()
{
  m_Reslicer->Delete();
  m_ComposedResliceTransform->Delete();
  m_UnitSpacingImageFilter->Delete();

  Clear();
}

void mitk::ImageMapper2D::Paint(mitk::BaseRenderer * renderer)
{
  mitk::Image* input  = const_cast<mitk::ImageMapper2D::InputImageType *>(
    this->GetInput()
  );

  if ( input == NULL ) 
  {
    return;
  }

  if ( IsVisible(renderer)==false )
  {
    return;
  }

  Update(renderer);

  RendererInfo& renderinfo = AccessRendererInfo(renderer);
  iil4mitkPicImage* image = renderinfo.Get_iil4mitkImage();

  if ( image==NULL )
  {
    return;
  }

  const mitk::DisplayGeometry* displayGeometry = renderer->GetDisplayGeometry();

  Vector2D oldtopLeft = displayGeometry->GetOriginInMM();
  Vector2D oldbottomRight = displayGeometry->GetOriginInMM()
    + displayGeometry->GetSizeInMM();

  Vector2D topLeft;
  Vector2D bottomRight;
  topLeft=displayGeometry->GetOriginInMM();
  bottomRight=topLeft+displayGeometry->GetSizeInMM();

  topLeft[0]*=renderinfo.m_PixelsPerMM[0];
  topLeft[1]*=renderinfo.m_PixelsPerMM[1];

  bottomRight[0]*=renderinfo.m_PixelsPerMM[0];
  bottomRight[1]*=renderinfo.m_PixelsPerMM[1];

  //test - small differences noticed for unisotropic datasets.
  if ( (Vector2D(oldtopLeft-topLeft).GetSquaredNorm() > 0.1)
    || (Vector2D(oldbottomRight-bottomRight).GetSquaredNorm() > 0.1) )
  {
    //itkWarningMacro("oldtopLeft!=topLeft in ImageMapper2D");
  }

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  glOrtho(topLeft[0], bottomRight[0], topLeft[1]/*+1*/, bottomRight[1],0.0,1.0);
  glMatrixMode( GL_MODELVIEW );

  GLdouble eqn0[4] = {0.0, 1.0, 0.0, 0.0};
  GLdouble eqn1[4] = {1.0, 0.0, 0.0, 0.0};
  GLdouble eqn2[4] = {-1.0, 0.0 , 0.0, image->width()};
  GLdouble eqn3[4] = {0, -1.0, 0.0, image->height() };

  glClipPlane (GL_CLIP_PLANE0, eqn0);
  glEnable (GL_CLIP_PLANE0);
  glClipPlane (GL_CLIP_PLANE1, eqn1);
  glEnable (GL_CLIP_PLANE1);
  glClipPlane (GL_CLIP_PLANE2, eqn2);
  glEnable (GL_CLIP_PLANE2);
  glClipPlane (GL_CLIP_PLANE3, eqn3);
  glEnable (GL_CLIP_PLANE3);

  image->setInterpolation(renderinfo.m_IilInterpolation);
  image->display(renderer->GetRenderWindow());

  glDisable (GL_CLIP_PLANE0);
  glDisable (GL_CLIP_PLANE1);
  glDisable (GL_CLIP_PLANE2);
  glDisable (GL_CLIP_PLANE3);

  glPushMatrix ();
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  glOrtho(0, displayGeometry->GetDisplayWidth(), 0,
    displayGeometry->GetDisplayHeight(),0.0,1.0
  );

  glMatrixMode( GL_MODELVIEW );
  glPopMatrix ();
}

//##ModelId=3E3D834B0008
const mitk::ImageMapper2D::InputImageType *mitk::ImageMapper2D::GetInput(void)
{
  return static_cast<const mitk::ImageMapper2D::InputImageType * >
    ( GetData() );
}

//##ModelId=3E6E83B00343
int mitk::ImageMapper2D::GetAssociatedChannelNr(mitk::BaseRenderer *renderer)
{
  RendererInfo& renderinfo=AccessRendererInfo(renderer);

  return renderinfo.GetRendererId();
}

//##ModelId=3ED932B00140
void mitk::ImageMapper2D::GenerateData(mitk::BaseRenderer *renderer)
{
  RendererInfo& renderinfo=AccessRendererInfo(renderer);

  mitk::Image* input  = const_cast<mitk::ImageMapper2D::InputImageType *>(
    this->GetInput()
  );

  renderinfo.Squeeze();

  if (input == NULL)
  {
    return;
  }

  const TimeSlicedGeometry* inputtimegeometry = input->GetTimeSlicedGeometry();
  if ( ( inputtimegeometry == NULL )
    || ( inputtimegeometry->GetTimeSteps() == 0 ) )
  {
    return;
  }

  iil4mitkPicImage* image;
  image = new iil4mitkPicImage(512);
  renderinfo.Set_iil4mitkImage(image);

  ApplyProperties(renderer);

  const Geometry2D* worldgeometry = renderer->GetCurrentWorldGeometry2D();

  assert(worldgeometry!=NULL);

  if (worldgeometry->IsValid()==false)
  {
    return;
  }

  int timestep=0;
  ScalarType time = worldgeometry->GetTimeBounds()[0];
  if(time>-ScalarTypeNumericTraits::max())
    timestep = inputtimegeometry->MSToTimeStep(time);

  if(inputtimegeometry->IsValidTime(timestep)==false)
    return;

  Image::RegionType requestedRegion = input->GetLargestPossibleRegion();
  requestedRegion.SetIndex(3, timestep);
  requestedRegion.SetSize(3, 1);
  requestedRegion.SetSize(4, 1);
  input->SetRequestedRegion(&requestedRegion);
  input->Update();

  vtkImageData* inputData = input->GetVtkImageData(timestep);

  if (inputData==NULL)
  {
    return;
  }

#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
  double spacing[3];
#else
  float spacing[3];
#endif
  
  inputData->GetSpacing(spacing);

  //how many pixels we really want to sample: width x height pixels
  mitk::ScalarType width, height;
  //how big the area is in physical coordinates: widthInMM x heightInMM pixels
  mitk::ScalarType widthInMM, heightInMM;

  //where we want to sample
  Point3D origin;
  Vector3D right, bottom, normal;
  Vector3D rightInIndex, bottomInIndex;

  assert(input->GetTimeSlicedGeometry()==inputtimegeometry);

  //take transform of input image into account
  Geometry3D* inputGeometry = inputtimegeometry->GetGeometry3D(timestep);

  ScalarType MMperPixel[2];
  if ( dynamic_cast<const PlaneGeometry *>(worldgeometry) != NULL )
  {
    const PlaneGeometry *planeview =
      static_cast<const PlaneGeometry *>(worldgeometry);  

    origin = planeview->GetOrigin();
    right  = planeview->GetAxisVector(0);
    bottom = planeview->GetAxisVector(1);
    normal = planeview->GetNormal();

    bool inPlaneResampleExtentByGeometry = false;
    GetDataTreeNode()->GetBoolProperty(
      "in plane resample extent by geometry", inPlaneResampleExtentByGeometry, renderer
    );
    if(inPlaneResampleExtentByGeometry)
    {
      // let's use the values of worldgeometry->GetExtent(0) and
      // worldgeometry->GetExtent(1) for that purpose
      // maybe it is useful to add here a more sophisticated rule that depends
      // on the actual size of the current display, so not to sample
      // 1000x1000 pixels for a display of 10x10 pixels
      width = worldgeometry->GetExtent(0);
      height = worldgeometry->GetExtent(1);
    }
    else
    {
      inputGeometry->WorldToIndex(origin, right, rightInIndex);
      inputGeometry->WorldToIndex(origin, bottom, bottomInIndex);
      width  = rightInIndex.GetNorm();
      height = bottomInIndex.GetNorm();
    }
 
    widthInMM = worldgeometry->GetExtentInMM(0);
    heightInMM = worldgeometry->GetExtentInMM(1);

    MMperPixel[0] = widthInMM/width;
    MMperPixel[1] = heightInMM/height;

    right.Normalize();
    bottom.Normalize();
    normal.Normalize();

    origin += right * (MMperPixel[0]*0.5);
    origin += bottom * (MMperPixel[1]*0.5);

    widthInMM -= MMperPixel[0];
    heightInMM -= MMperPixel[1];

    vtkLinearTransform *inputGeometryTransform =
      inputGeometry->GetVtkTransform();
          
    vtkLinearTransform *inverseTransform =
      inputGeometryTransform->GetLinearInverse();

    m_Reslicer->SetResliceTransform( inverseTransform); 
  }
  else
  if ( dynamic_cast<const AbstractTransformGeometry *>( worldgeometry ) != NULL )
  {
    const mitk::AbstractTransformGeometry* abstractGeometry =
      dynamic_cast<const AbstractTransformGeometry *>(worldgeometry);

    if ( abstractGeometry != NULL )
    {
      // let's use the values of worldgeometry->GetExtent(0) and
      // worldgeometry->GetExtent(1) for that purpose maybe it is useful to
      // add here a more sophisticated rule that depends on the actual size
      // of the current display, so not to sample 1000x1000 pixels for a
      // display of 10x10 pixels
      width =worldgeometry->GetParametricExtent(0);
      height=worldgeometry->GetParametricExtent(1);

      widthInMM = abstractGeometry->GetParametricExtentInMM(0);
      heightInMM= abstractGeometry->GetParametricExtentInMM(1);

      MMperPixel[0] = widthInMM/width;
      MMperPixel[1] = heightInMM/height;

      origin = abstractGeometry->GetPlane()->GetOrigin();

      right  = abstractGeometry->GetPlane()->GetAxisVector(0);
      right.Normalize();

      bottom = abstractGeometry->GetPlane()->GetAxisVector(1);
      bottom.Normalize();

      normal = abstractGeometry->GetPlane()->GetNormal();
      normal.Normalize();

      vtkLinearTransform *inputGeometryTransform =
        inputGeometry->GetVtkTransform();

      vtkLinearTransform *inverseTransform =
        inputGeometryTransform->GetLinearInverse();

      m_ComposedResliceTransform->Identity();
      m_ComposedResliceTransform->Concatenate( inverseTransform );
      m_ComposedResliceTransform->Concatenate(
        abstractGeometry->GetVtkAbstractTransform() );
    }
    else
    {
      return;
    }
    m_Reslicer->SetResliceTransform( m_ComposedResliceTransform );
  }
  else
  {
    return;
  }

  assert(normal.GetSquaredNorm()>0.1);

  if ( (width <= 2) && (height <= 2) )
  {
    return;
  }

  if((input->GetDimension() >= 3) && (input->GetDimension(2) > 1))
  {
    bool vtkInterpolation=false;
    GetDataTreeNode()->GetBoolProperty(
      "vtkInterpolation", vtkInterpolation, renderer
    );

    if (vtkInterpolation) 
      m_Reslicer->SetInterpolationModeToLinear();
    else 
      m_Reslicer->SetInterpolationModeToNearestNeighbor();
  }
  else
    m_Reslicer->SetInterpolationModeToNearestNeighbor();


  vtkMatrix4x4* geometry = vtkMatrix4x4::New();
  geometry->Identity();

  m_UnitSpacingImageFilter->SetInput( inputData );

  m_Reslicer->SetInput( m_UnitSpacingImageFilter->GetOutput() );
  m_Reslicer->SetOutputDimensionality( 2 );
  m_Reslicer->SetOutputOrigin( 0, 0, 0 );

  m_Reslicer->SetBackgroundLevel( -1024 );

  renderinfo.m_PixelsPerMM[0]= 1.0 / MMperPixel[0];
  renderinfo.m_PixelsPerMM[1]= 1.0 / MMperPixel[1];

  m_Reslicer->SetOutputSpacing( MMperPixel[0], MMperPixel[1], 1.0 );
  m_Reslicer->SetOutputExtent( 0, (int)width-1, 0, (int)height-1, 0, 1 );

  //calulate the originarray and the orientations for the reslice-filter
  double originarray[3];
  itk2vtk(origin, originarray);

  m_Reslicer->SetResliceAxes(geometry);
  m_Reslicer->SetResliceAxesOrigin(originarray);
  //m_Reslicer->SetInterpolationModeToLinear();
  double cosines[9];

  // direction of the X-axis of the sampled result
  vnl2vtk(right.Get_vnl_vector(), cosines);

  // direction of the Y-axis of the sampled result
  vnl2vtk(bottom.Get_vnl_vector(), cosines+3);

  // normal of the plane
  vnl2vtk(normal.Get_vnl_vector(), cosines+6);

  m_Reslicer->SetResliceAxesDirectionCosines(cosines);

  //do the reslicing
  m_Reslicer->Update();

  vtkImageData* vtkoutput = m_Reslicer->GetOutput();

  assert(vtkoutput);

  ipPicDescriptor* pic = Pic2vtk::convert(vtkoutput);
  assert(pic);
  if(pic->dim==1)
  {
    pic->dim=2;
    pic->n[1]=1;
  }
  assert(pic->dim == 2);

  if(renderinfo.m_Pic!=NULL)
  {
    ipPicFree(renderinfo.m_Pic);
  }

  renderinfo.m_Pic = pic;

  image->setImage(pic, m_iil4mitkMode);
  image->setInterpolation( false );
  image->setRegion(0,0,pic->n[0],pic->n[1]);

  renderinfo.m_LastUpdateTime.Modified();
}

void mitk::ImageMapper2D::GenerateAllData()
{
  RenderInfoMap::iterator it=m_RendererInfo.begin(), end=m_RendererInfo.end();
  for(;it!=end;++it)
    Update(it->first);
}

void mitk::ImageMapper2D::Clear()
{
  RenderInfoMap::iterator it=m_RendererInfo.begin(), end=m_RendererInfo.end();
  for(;it!=end;++it)
  {
    it->second.Squeeze();
  }
  //@FIXME: durch die folgende Zeile sollte doch wohl der desctructor von
  // RendererInfo aufgerufen werden. Das passiert aber nie. Deshalb wird bei
  // der Programm-Beendung auch das iil4mitkImage und damit die textur nicht
  // rechtzeitig freigegeben und das Programm crashed.
  m_RendererInfo.clear();
}

void mitk::ImageMapper2D::ApplyProperties(mitk::BaseRenderer* renderer)
{
  RendererInfo& renderinfo=AccessRendererInfo(renderer);
  iil4mitkPicImage* image = renderinfo.Get_iil4mitkImage();

  assert(image != NULL);

  float rgba[4]={1.0f,1.0f,1.0f,1.0f};
  float opacity = 1.0f;
  // check for color prop and use it for rendering if it exists
  GetColor(rgba, renderer);
  // check for opacity prop and use it for rendering if it exists
  GetOpacity(opacity, renderer);
  rgba[3]=opacity;

  // check for interpolation properties
  bool iilInterpolation=false;
  GetDataTreeNode()->GetBoolProperty(
    "iilInterpolation", iilInterpolation, renderer
  );

  renderinfo.m_IilInterpolation = iilInterpolation;

  bool useColor=false;
  GetDataTreeNode()->GetBoolProperty("use color", useColor, renderer);
  mitk::LookupTableProperty::Pointer LookupTableProp;
  if (useColor==false)
  {
    LookupTableProp = dynamic_cast<mitk::LookupTableProperty*>(
      this->GetDataTreeNode()->GetProperty("LookupTable").GetPointer()
    );
    if(LookupTableProp.IsNull())
    useColor=true;
  }
  if(useColor)
  {
    m_iil4mitkMode = iil4mitkImage::INTENSITY_ALPHA;
    image->setColor(rgba[0], rgba[1], rgba[2], rgba[3]);
  }
  else 
  {
    m_iil4mitkMode = iil4mitkImage::COLOR_ALPHA;
    // only update the lut, when the properties have changed...
    if ( LookupTableProp->GetLookupTable()->GetMTime() <= this->GetDataTreeNode()->GetPropertyList()->GetMTime() )
    {
      LookupTableProp->GetLookupTable()->ChangeOpacityForAll( opacity );
      LookupTableProp->GetLookupTable()->ChangeOpacity(0, 0.0);
    }
    image->setColors(LookupTableProp->GetLookupTable()->GetRawLookupTable());	
  }

  mitk::LevelWindow levelWindow;

  bool binary=false;
  GetDataTreeNode()->GetBoolProperty("binary", binary, renderer);

  if (binary)
    image->setExtrema(0, 1);
  else 
  {
    if(!GetLevelWindow(levelWindow,renderer,"levelWindow"))
      GetLevelWindow(levelWindow,renderer);

    image->setExtrema(levelWindow.GetMin(), levelWindow.GetMax()); 
  }
}

void mitk::ImageMapper2D::Update(mitk::BaseRenderer* renderer)
{
  mitk::Image* input  = const_cast<mitk::ImageMapper2D::InputImageType *>(
    this->GetInput()
  );

  if ( input == NULL )
  {
    return;
  }

  if ( IsVisible(renderer)==false ) 
  {
    return;
  }

  const DataTreeNode* node = GetDataTreeNode();

  RendererInfo& renderinfo = AccessRendererInfo(renderer);
  iil4mitkPicImage* image = renderinfo.Get_iil4mitkImage();

  input->UpdateOutputInformation();
  if ( (image == NULL)
    || (renderinfo.m_LastUpdateTime < node->GetMTime())
    || (renderinfo.m_LastUpdateTime < input->GetPipelineMTime())
    || (renderinfo.m_LastUpdateTime
          < renderer->GetCurrentWorldGeometry2DUpdateTime())
    || (renderinfo.m_LastUpdateTime
          < renderer->GetDisplayGeometryUpdateTime()) )
  {
    GenerateData(renderer);
  }
  else if ( renderinfo.m_LastUpdateTime
              < renderer->GetCurrentWorldGeometry2D()->GetMTime() )
  {
    GenerateData(renderer);
  }
  else if( (renderinfo.m_LastUpdateTime < node->GetPropertyList()->GetMTime())
        || (renderinfo.m_LastUpdateTime
              < node->GetPropertyList(renderer)->GetMTime()) )
  {
    GenerateData(renderer);
    // since we have checked that nothing important has changed, we can set
    // m_LastUpdateTime to the current time
    renderinfo.m_LastUpdateTime.Modified();
  }
}

void mitk::ImageMapper2D::RendererInfo::RendererDeleted()
{
  //delete texture due to its dependency on the renderer
  delete m_iil4mitkImage;
  m_iil4mitkImage = NULL;
  m_Renderer = NULL;
}

void
mitk::ImageMapper2D::RendererInfo
::Set_iil4mitkImage(iil4mitkPicImage* iil4mitkImage)
{
  assert(iil4mitkImage!=NULL);

  delete m_iil4mitkImage;
  m_iil4mitkImage = iil4mitkImage;
}

void mitk::ImageMapper2D::RendererInfo::Squeeze()
{
  delete m_iil4mitkImage;
  m_iil4mitkImage = NULL;
  if(m_Pic!=NULL)
  {
    ipPicFree(m_Pic);
    m_Pic = NULL;
  }
}
