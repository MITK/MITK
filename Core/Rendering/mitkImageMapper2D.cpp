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
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkLookupTable.h>
#include <vtkImageData.h>

#include "GL/glu.h"

int mitk::ImageMapper2D::numRenderer = 0;
 

mitk::ImageMapper2D::ImageMapper2D() : m_SliceSelector(NULL)
{
  m_SliceSelector = ImageSliceSelector::New();
  m_Reslicer = vtkImageReslice::New();
}

//##ModelId=3E32DCF60043
mitk::ImageMapper2D::~ImageMapper2D()
{
  Clear();
}

void mitk::ImageMapper2D::Paint(mitk::BaseRenderer * renderer)
{
  mitk::Image* input  = const_cast<mitk::ImageMapper2D::InputImageType *>(this->GetInput());
  if(input == NULL) 
    //|| ((input->GetSource().GetPointer()==NULL) && (input->IsInitialized()==false)))
    return;

  if(IsVisible(renderer)==false) return;

  Update(renderer);

  RendererInfo& renderinfo = AccessRendererInfo(renderer);
  iil4mitkPicImage* image = renderinfo.Get_iil4mitkImage();

  if(image==NULL)
    return;

  const mitk::DisplayGeometry* displayGeometry = renderer->GetDisplayGeometry();

  Vector2D oldtopLeft=displayGeometry->GetOriginInMM();
  Vector2D oldbottomRight=displayGeometry->GetOriginInMM()+displayGeometry->GetSizeInMM();

  Vector2D topLeft;
  Vector2D bottomRight;
  topLeft=displayGeometry->GetOriginInMM();
  bottomRight=topLeft+displayGeometry->GetSizeInMM();

  topLeft[0]*=renderinfo.m_PixelsPerMM[0];  topLeft[1]*=renderinfo.m_PixelsPerMM[1];
  bottomRight[0]*=renderinfo.m_PixelsPerMM[0];  bottomRight[1]*=renderinfo.m_PixelsPerMM[1];

  //test - small differences noticed for unisotropic datasets.
  if((Vector2D(oldtopLeft-topLeft).GetSquaredNorm()>0.1) || (Vector2D(oldbottomRight-bottomRight).GetSquaredNorm()>0.1))
  {
    itkWarningMacro("oldtopLeft!=topLeft in ImageMapper2D");
  }

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluOrtho2D(topLeft[0], bottomRight[0], topLeft[1], bottomRight[1] );
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
  gluOrtho2D(0, displayGeometry->GetDisplayWidth(), 0, displayGeometry->GetDisplayHeight() );
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

  mitk::Image* input  = const_cast<mitk::ImageMapper2D::InputImageType *>(this->GetInput());

  renderinfo.Squeeze();

  if(input!=NULL)
  {
    iil4mitkPicImage* image;
    image = new iil4mitkPicImage(512);
    renderinfo.Set_iil4mitkImage(image);

    ApplyProperties(renderer);
    const TimeSlicedGeometry* inputtimegeometry = dynamic_cast<const TimeSlicedGeometry*>(input->GetUpdatedGeometry());

    assert(inputtimegeometry!=NULL);

    const Geometry2D* worldgeometry = renderer->GetCurrentWorldGeometry2D();

    assert(worldgeometry!=NULL);

    int timestep=0;
    ScalarType time = worldgeometry->GetTimeBoundsInMS()[0];
    if(time>-ScalarTypeNumericTraits::max())
      timestep = inputtimegeometry->MSToTimeStep(time);

    if(inputtimegeometry->IsValidTime(timestep)==false)
      return;

    vtkImageData* inputData = input->GetVtkImageData(timestep);
    if(inputData==NULL)
      return;

    float spacing[3];
    inputData->GetSpacing(spacing);

    //how many pixels we really want to sample: width x height pixels
    mitk::ScalarType width, height;
    //how big the area is in physical coordinates: widthInMM x heightInMM pixels
    mitk::ScalarType widthInMM, heightInMM;

    //where we want to sample
    Point3D origin;
    Vector3D right, bottom, normal;

    if(dynamic_cast<const PlaneGeometry *>(worldgeometry)!=NULL)
    {
      //let's use the values of worldgeometry->GetExtent(0) and worldgeometry->GetExtent(1) for that purpose
      //maybe it is useful to add here a more sophisticated rule that depends on the actual size of the current display, so not to
      //sample 1000x1000 pixels for a display of 10x10 pixels
      width =worldgeometry->GetExtent(0); widthInMM = worldgeometry->GetExtentInMM(0);
      height=worldgeometry->GetExtent(1); heightInMM= worldgeometry->GetExtentInMM(1);

      const PlaneGeometry *planeview=dynamic_cast<const PlaneGeometry *>(worldgeometry);  
      origin = planeview->GetOrigin();
      right  = planeview->GetAxisVector(0); right.Normalize();
      bottom = planeview->GetAxisVector(1); bottom.Normalize();
      normal = planeview->GetNormal();      normal.Normalize();

      vtkTransform * vtktransform = GetDataTreeNode()->GetVtkTransform();
           
      vtkLinearTransform * inversetransform = vtktransform->GetLinearInverse();
      m_Reslicer->SetResliceTransform(inversetransform); 
      inputData->SetSpacing(1,1,1); //spacing already included in transform!
    }
    else
    if(dynamic_cast<const AbstractTransformGeometry *>(worldgeometry)!=NULL)
    {
      const mitk::AbstractTransformGeometry* abstractGeometry = dynamic_cast<const AbstractTransformGeometry *>(worldgeometry);
      if(abstractGeometry!=NULL)
      {
        //let's use the values of worldgeometry->GetExtent(0) and worldgeometry->GetExtent(1) for that purpose
        //maybe it is useful to add here a more sophisticated rule that depends on the actual size of the current display, so not to
        //sample 1000x1000 pixels for a display of 10x10 pixels
        width =worldgeometry->GetParametricExtent(0); widthInMM = abstractGeometry->GetPlane()->GetExtentInMM(0);
        height=worldgeometry->GetParametricExtent(1); heightInMM= abstractGeometry->GetPlane()->GetExtentInMM(1);

        origin = abstractGeometry->GetPlane()->GetOrigin();
        right  = abstractGeometry->GetPlane()->GetAxisVector(0); right.Normalize();
        bottom = abstractGeometry->GetPlane()->GetAxisVector(1); bottom.Normalize();
        normal = abstractGeometry->GetPlane()->GetNormal();      normal.Normalize();
      }
      else
          return;
      m_Reslicer->SetResliceTransform(abstractGeometry->GetVtkAbstractTransform());
    }
    else
      return;

    assert(normal.GetSquaredNorm()>0.1);

    if((width<=1) && (height<=1))
      return;

    vtkMatrix4x4* geometry = vtkMatrix4x4::New();
    geometry->Identity();

    m_Reslicer->SetInput(inputData);
    m_Reslicer->SetOutputDimensionality(2);
    m_Reslicer->SetOutputOrigin(0,0,0);

    m_Reslicer->SetBackgroundLevel(-1024);

    renderinfo.m_PixelsPerMM[0]= width/widthInMM;
    renderinfo.m_PixelsPerMM[1]= height/heightInMM;

    m_Reslicer->SetOutputSpacing(1.0/renderinfo.m_PixelsPerMM[0], 1.0/renderinfo.m_PixelsPerMM[1], 1.0);
    m_Reslicer->SetOutputExtent(0, (int)width-1, 0, (int)height-1, 0, 1);

    //calulate the originarray and the orientations for the reslice-filter
    double originarray[3];
    itk2vtk(origin, originarray);

    m_Reslicer->SetResliceAxes(geometry);
    m_Reslicer->SetResliceAxesOrigin(originarray);
    //        m_Reslicer->SetInterpolationModeToLinear();
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

    //	std::cout << vtkoutput <<std::endl;
    ipPicDescriptor* pic = Pic2vtk::convert(vtkoutput);
    assert(pic);
    if(pic->dim==1)
    {
      pic->dim=2;
      pic->n[1]=1;
    }
    assert(pic->dim == 2);

    if(renderinfo.m_Pic!=NULL)
      ipPicFree(renderinfo.m_Pic);
    renderinfo.m_Pic = pic;

    //std::cout << "Pic dimensions:" << pic->dim << std::endl;

//   image->setImage(pic, iil4mitkImage::INTENSITY_ALPHA);
    image->setImage(pic, m_iil4mitkMode);
    image->setRegion(0,0,pic->n[0],pic->n[1]);

    renderinfo.m_LastUpdateTime.Modified();

    inputData->SetSpacing(spacing);
  }
  return;
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
  //@FIXME: durch die folgende Zeile sollte doch wohl der desctructor von RendererInfo aufgerufen werden. Das passiert aber nie. Deshalb wird bei der Programm-Beendung auch das iil4mitkImage und damit die textur nicht rechtzeitig freigegeben und das Programm crashed.
  m_RendererInfo.clear();
}

void mitk::ImageMapper2D::ApplyProperties(mitk::BaseRenderer* renderer)
{
  RendererInfo& renderinfo=AccessRendererInfo(renderer);
  iil4mitkPicImage* image = renderinfo.Get_iil4mitkImage();

  assert(image != NULL);

  float rgba[4]={1.0f,1.0f,1.0f,1.0f};
  // check for color prop and use it for rendering if it exists
  GetColor(rgba, renderer);
  // check for opacity prop and use it for rendering if it exists
  GetOpacity(rgba[3], renderer);


  // check for interpolation properties
  mitk::BoolProperty::Pointer vtkInterpolation = dynamic_cast<mitk::BoolProperty*>(this->GetDataTreeNode()->GetProperty("vtkInterpolation",renderer).GetPointer());
  mitk::BoolProperty::Pointer iilInterpolation = dynamic_cast<mitk::BoolProperty*>(this->GetDataTreeNode()->GetProperty("iilInterpolation",renderer).GetPointer());

  renderinfo.m_IilInterpolation = iilInterpolation.IsNotNull() && iilInterpolation->GetValue();
  if (vtkInterpolation.IsNotNull() && vtkInterpolation->GetValue()) {
    m_Reslicer->SetInterpolationModeToLinear();
  } else {
    m_Reslicer->SetInterpolationModeToNearestNeighbor();

  }
 
  

  mitk::LevelWindow levelWindow;
  // check for level window prop and use it for display if it exists
  GetLevelWindow(levelWindow, renderer);


  mitk::LookupTableProperty::Pointer LookupTableProp;
  LookupTableProp = dynamic_cast<mitk::LookupTableProperty*>(this->GetDataTreeNode()->GetProperty("LookupTable").GetPointer());
	if (LookupTableProp.IsNull() )
	{
		m_iil4mitkMode = iil4mitkImage::INTENSITY_ALPHA;
	  image->setColor(rgba[0], rgba[1], rgba[2], rgba[3]);
	}
	else {
		m_iil4mitkMode = iil4mitkImage::COLOR_ALPHA;
		image->setColors(LookupTableProp->GetLookupTable().GetRawLookupTable());
		
	}

  mitk::BoolProperty::Pointer binary;
  binary = dynamic_cast<mitk::BoolProperty*>(this->GetDataTreeNode()->GetProperty("binary").GetPointer());

  mitk::LevelWindowProperty::Pointer overwriteLevelWindow;
  overwriteLevelWindow = dynamic_cast<mitk::LevelWindowProperty*>(this->GetDataTreeNode()->GetProperty("levelWindow").GetPointer());

  if (binary.IsNotNull() )
  {
    image->setExtrema(0, 1);
  }
  else if (overwriteLevelWindow.IsNotNull() )
  {
    image->setExtrema(overwriteLevelWindow->GetLevelWindow().GetMin(), overwriteLevelWindow->GetLevelWindow().GetMax());
  }
  else
  {
  // set the properties
    image->setExtrema(levelWindow.GetMin(), levelWindow.GetMax());
  }
//  image->setColor(rgba[0], rgba[1], rgba[2], rgba[3]);
  
}

void mitk::ImageMapper2D::Update(mitk::BaseRenderer* renderer)
{
  mitk::Image* input  = const_cast<mitk::ImageMapper2D::InputImageType *>(this->GetInput());
  if(input == NULL)
    return;

  const DataTreeNode* node = GetDataTreeNode();

  RendererInfo& renderinfo = AccessRendererInfo(renderer);
  iil4mitkPicImage* image = renderinfo.Get_iil4mitkImage();

  if(
      (image == NULL) ||
      (renderinfo.m_LastUpdateTime < node->GetMTime()) ||
      (renderinfo.m_LastUpdateTime < renderer->GetCurrentWorldGeometry2DUpdateTime()) ||
      (renderinfo.m_LastUpdateTime < renderer->GetDisplayGeometryUpdateTime())
    )
    GenerateData(renderer);
  else
  if(
      (renderinfo.m_LastUpdateTime < renderer->GetCurrentWorldGeometry2D()->GetMTime())
      //&&
      //(renderinfo.m_LastUpdateTime < renderer->GetMTime())
    )
    GenerateData(renderer);
  else
  if(
      (renderinfo.m_LastUpdateTime < node->GetPropertyList()->GetMTime()) ||
      (renderinfo.m_LastUpdateTime < node->GetPropertyList(renderer)->GetMTime())
    )
  {
    GenerateData(renderer);
    // since we have checked that nothing important has changed, we can set m_LastUpdateTime
    // to the current time
    renderinfo.m_LastUpdateTime.Modified();
  }
}

void mitk::ImageMapper2D::RendererInfo::RendererDeleted()
{
  //delete texture due to its dependency on the renderer
  delete m_iil4mitkImage;
  m_iil4mitkImage = NULL;
  //renderer will be deleted, so the command is no longer necessary
  m_DeleteRendererCommand = NULL;
  m_Renderer = NULL;
}

void mitk::ImageMapper2D::RendererInfo::Set_iil4mitkImage(iil4mitkPicImage* iil4mitkImage)
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
