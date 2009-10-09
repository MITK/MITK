/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-08-25 18:10:57 +0200 (Mo, 25 Aug 2008) $
Version:   $Revision: 15062 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef __mitkOdfVtkMapper2D_txx__
#define __mitkOdfVtkMapper2D_txx__

#include "mitkOdfVtkMapper2D.h"
#include "mitkDataTreeNode.h"
#include "mitkBaseRenderer.h"
#include "mitkMatrixConvert.h"
#include "mitkGeometry3D.h"
#include "mitkOdfNormalizationMethodProperty.h" 
#include "mitkOdfScaleByProperty.h" 

#include "vtkSphereSource.h"
#include "vtkPropCollection.h"
#include "vtkMaskedGlyph3D.h"
#include "vtkGlyph2D.h"
#include "vtkGlyph3D.h"
#include "vtkMaskedProgrammableGlyphFilter.h"
#include "vtkImageData.h"
#include "vtkLinearTransform.h"
#include "vtkCamera.h"
#include "vtkPointData.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTransform.h"
#include "vtkOdfSource.h"
#include "vtkDoubleArray.h"
#include "vtkLookupTable.h"
#include "vtkProperty.h"
#include "vtkPolyDataNormals.h"
#include "vtkLight.h"
#include "vtkLightCollection.h"
#include "vtkMath.h"
#include "vtkFloatArray.h"
#include "vtkDelaunay2D.h"
#include "vtkMapper.h"

#include "vtkRenderer.h"
#include "vtkCamera.h"

#include "itkOrientationDistributionFunction.h"

#include "itkFixedArray.h"

#include <GL/gl.h>
#include "vtkOpenGLRenderer.h"

template<class T, int N>
vtkTransform* mitk::OdfVtkMapper2D<T,N>::m_OdfTransform
= vtkTransform::New();

template<class T, int N>
vtkDoubleArray* mitk::OdfVtkMapper2D<T,N>::m_OdfVals
= vtkDoubleArray::New();

template<class T, int N>
vtkOdfSource* mitk::OdfVtkMapper2D<T,N>::m_OdfSource
= vtkOdfSource::New();

template<class T, int N>
float mitk::OdfVtkMapper2D<T,N>::m_Scaling;

template<class T, int N>
int mitk::OdfVtkMapper2D<T,N>::m_Normalization;

template<class T, int N>
int mitk::OdfVtkMapper2D<T,N>::m_ScaleBy;

template<class T, int N>
float mitk::OdfVtkMapper2D<T,N>::m_IndexParam1;

template<class T, int N>
float mitk::OdfVtkMapper2D<T,N>::m_IndexParam2;

#define ODF_MAPPER_PI 3.1415926535897932384626433832795

//#include "vtkSphereSource.h"
//#include "vtkPolyDataMapper.h"
//#include "vtkActor.h"
//#include "vtkRenderWindow.h"
//#include "vtkRenderer.h"
//#include "vtkRenderWindowInteractor.h"
//#include "vtkProperty.h"
//
//void bla(vtkPolyData* poly)
//{
//
//  // map to graphics library
//  vtkPolyDataMapper *map = vtkPolyDataMapper::New();
//  map->SetInput(poly);
//
//  // actor coordinates geometry, properties, transformation
//  vtkActor *aSphere = vtkActor::New();
//  aSphere->SetMapper(map);
//  aSphere->GetProperty()->SetColor(0,0,1); // sphere color blue
//
//  // a renderer and render window
//  vtkRenderer *ren1 = vtkRenderer::New();
//  vtkRenderWindow *renWin = vtkRenderWindow::New();
//  renWin->AddRenderer(ren1);
//
//  // an interactor
//  vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
//  iren->SetRenderWindow(renWin);
//
//  // add the actor to the scene
//  ren1->AddActor(aSphere);
//  ren1->SetBackground(1,1,1); // Background color white
//
//  // render an image (lights and cameras are created automatically)
//  renWin->Render();
//
//  // begin mouse interaction
//  iren->Start();
//}

template<class T, int N>
mitk::OdfVtkMapper2D<T,N>
::OdfVtkMapper2D()
{
  m_VtkBased = true;

  m_PropAssemblies.push_back(vtkPropAssembly::New());
  m_PropAssemblies.push_back(vtkPropAssembly::New());
  m_PropAssemblies.push_back(vtkPropAssembly::New());

  m_OdfsPlanes.push_back(vtkAppendPolyData::New());
  m_OdfsPlanes.push_back(vtkAppendPolyData::New());
  m_OdfsPlanes.push_back(vtkAppendPolyData::New());

  m_OdfsPlanes[0]->AddInput(vtkPolyData::New());
  m_OdfsPlanes[1]->AddInput(vtkPolyData::New());
  m_OdfsPlanes[2]->AddInput(vtkPolyData::New());

  m_OdfsActors.push_back(vtkActor::New());
  m_OdfsActors.push_back(vtkActor::New());
  m_OdfsActors.push_back(vtkActor::New());

  m_OdfsActors[0]->GetProperty()->SetInterpolationToGouraud();
  m_OdfsActors[1]->GetProperty()->SetInterpolationToGouraud();
  m_OdfsActors[2]->GetProperty()->SetInterpolationToGouraud();

  m_OdfsMappers.push_back(vtkPolyDataMapper::New());
  m_OdfsMappers.push_back(vtkPolyDataMapper::New());
  m_OdfsMappers.push_back(vtkPolyDataMapper::New());

  vtkLookupTable *lut = vtkLookupTable::New();
  //lut->SetMinimumTableValue(0,0,1,1);
  //lut->SetMaximumTableValue(1,0,0,1);
  //lut->SetWindow(0.1);
  //lut->SetLevel(0.05);   <== not recognized or reset by mapper ??
  //lut->Build();
  m_OdfsMappers[0]->SetLookupTable(lut);
  m_OdfsMappers[1]->SetLookupTable(lut);
  m_OdfsMappers[2]->SetLookupTable(lut);

  m_OdfsActors[0]->SetMapper(m_OdfsMappers[0]);
  m_OdfsActors[1]->SetMapper(m_OdfsMappers[1]);
  m_OdfsActors[2]->SetMapper(m_OdfsMappers[2]);

  m_Planes.push_back(vtkPlane::New());
  m_Planes.push_back(vtkPlane::New());
  m_Planes.push_back(vtkPlane::New());

  m_Cutters.push_back(vtkCutter::New());
  m_Cutters.push_back(vtkCutter::New());
  m_Cutters.push_back(vtkCutter::New());

  m_Cutters[0]->SetCutFunction( m_Planes[0] );
  m_Cutters[0]->GenerateValues( 1, 0, 1 );

  m_Cutters[1]->SetCutFunction( m_Planes[1] );
  m_Cutters[1]->GenerateValues( 1, 0, 1 );

  m_Cutters[2]->SetCutFunction( m_Planes[2] );
  m_Cutters[2]->GenerateValues( 1, 0, 1 );

  // Windowing the cutted planes in direction 1
  m_ThickPlanes1.push_back(vtkThickPlane::New());
  m_ThickPlanes1.push_back(vtkThickPlane::New());
  m_ThickPlanes1.push_back(vtkThickPlane::New());

  m_Clippers1.push_back(vtkClipPolyData::New());
  m_Clippers1.push_back(vtkClipPolyData::New());
  m_Clippers1.push_back(vtkClipPolyData::New());

  m_Clippers1[0]->SetClipFunction( m_ThickPlanes1[0] );
  m_Clippers1[1]->SetClipFunction( m_ThickPlanes1[1] );
  m_Clippers1[2]->SetClipFunction( m_ThickPlanes1[2] );

  // Windowing the cutted planes in direction 2
  m_ThickPlanes2.push_back(vtkThickPlane::New());
  m_ThickPlanes2.push_back(vtkThickPlane::New());
  m_ThickPlanes2.push_back(vtkThickPlane::New());

  m_Clippers2.push_back(vtkClipPolyData::New());
  m_Clippers2.push_back(vtkClipPolyData::New());
  m_Clippers2.push_back(vtkClipPolyData::New());

  m_Clippers2[0]->SetClipFunction( m_ThickPlanes2[0] );
  m_Clippers2[1]->SetClipFunction( m_ThickPlanes2[1] );
  m_Clippers2[2]->SetClipFunction( m_ThickPlanes2[2] );

  m_TemplateOdf = itk::OrientationDistributionFunction<T,N>::GetBaseMesh();

  vtkPoints* points = m_TemplateOdf->GetPoints();

  m_OdfVals->Allocate(N);
  m_OdfSource->SetTemplateOdf(m_TemplateOdf);
  m_OdfSource->SetOdfVals(m_OdfVals);

  m_ShowMaxNumber = 500;

  //vtkMapper::GlobalImmediateModeRenderingOn();
}

template<class T, int N>
mitk::OdfVtkMapper2D<T,N>
::~OdfVtkMapper2D()
{
  m_PropAssemblies[0]->Delete();
  m_PropAssemblies[1]->Delete();
  m_PropAssemblies[2]->Delete();
  m_OdfsPlanes[0]->Delete();
  m_OdfsPlanes[1]->Delete();
  m_OdfsPlanes[2]->Delete();
  m_OdfsActors[0]->Delete();
  m_OdfsActors[1]->Delete();
  m_OdfsActors[2]->Delete();
  m_OdfsMappers[0]->Delete();
  m_OdfsMappers[1]->Delete();
  m_OdfsMappers[2]->Delete();
  m_Planes[0]->Delete();
  m_Planes[1]->Delete();
  m_Planes[2]->Delete();
  m_Cutters[0]->Delete();
  m_Cutters[1]->Delete();
  m_Cutters[2]->Delete();
  m_ThickPlanes1[0]->Delete();
  m_ThickPlanes1[1]->Delete();
  m_ThickPlanes1[2]->Delete();
  m_ThickPlanes2[0]->Delete();
  m_ThickPlanes2[1]->Delete();
  m_ThickPlanes2[2]->Delete();
  m_Clippers1[0]->Delete();
  m_Clippers1[1]->Delete();
  m_Clippers1[2]->Delete();
  m_Clippers2[0]->Delete();
  m_Clippers2[1]->Delete();
  m_Clippers2[2]->Delete();
}

template<class T, int N>
const mitk::Image* mitk::OdfVtkMapper2D<T,N>
::GetInput()
{
  return static_cast<const mitk::Image * > ( m_DataTreeNode->GetData() );
}

template<class T, int N>
vtkProp*  mitk::OdfVtkMapper2D<T,N>
::GetProp(mitk::BaseRenderer* renderer)
{
  return m_PropAssemblies[GetIndex(renderer)];
}

template<class T, int N>
int  mitk::OdfVtkMapper2D<T,N>
::GetIndex(mitk::BaseRenderer* renderer)
{
  if(!strcmp(renderer->GetName(),"stdmulti.widget1"))
    return 0;

  if(!strcmp(renderer->GetName(),"stdmulti.widget2"))
    return 1;

  if(!strcmp(renderer->GetName(),"stdmulti.widget3"))
    return 2;

  return 0;
}

template<class T, int N>
void  mitk::OdfVtkMapper2D<T,N>
::GlyphMethod(void *arg)
{
  vtkMaskedProgrammableGlyphFilter
    *pfilter=(vtkMaskedProgrammableGlyphFilter*)arg;

  double point[3];
  pfilter->GetPoint(point);

  itk::Point<double,3> p(point);
  Point3D origin = pfilter->GetGeometry()->GetOrigin();
  p[0] += origin[0];
  p[1] += origin[1];
  p[2] += origin[2];
  mitk::Point3D p2;
  pfilter->GetGeometry()->ItkPhysicalPointToWorld( p, p2 );
  point[0] = p2[0];
  point[1] = p2[1];
  point[2] = p2[2];

  vtkPointData* data = pfilter->GetPointData();
  vtkDataArray* odfvals = data->GetArray("vector");
  vtkIdType id = pfilter->GetPointId();
  m_OdfTransform->Identity();
  m_OdfTransform->Translate(point[0],point[1],point[2]);

  typedef itk::OrientationDistributionFunction<float,N> OdfType;
  OdfType odf;

  for(int i=0; i<N; i++)
    odf[i] = (double)odfvals->GetComponent(id,i);

  switch(m_Normalization)
  {
  case ODFN_MINMAX:
    odf = odf.MinMaxNormalize();
    break;
  case ODFN_MAX:
    odf = odf.MaxNormalize();
    break;
  case ODFN_NONE:
    // nothing
    break;
  case ODFN_GLOBAL_MAX:
    // global max not implemented yet
    break;
  default:
    odf = odf.MinMaxNormalize();
  }

  switch(m_ScaleBy)
  {
  case ODFSB_NONE:
    break;
  case ODFSB_GFA:
    odf = odf * odf.GetGeneralizedGFA(m_IndexParam1, m_IndexParam2);
    break;
  case ODFSB_PC:
    odf = odf * odf.GetPrincipleCurvature(m_IndexParam1, m_IndexParam2, 1);
    break;
  }

  for(int i=0; i<N; i++)
    m_OdfVals->SetComponent(0,i,0.5*odf[i]*m_Scaling);

  m_OdfSource->Modified();
}

template<class T, int N>
void  mitk::OdfVtkMapper2D<T,N>
::Slice(mitk::BaseRenderer* renderer)
{
  int index = GetIndex(renderer);

  vtkLinearTransform * vtktransform = 
    this->GetDataTreeNode()->GetVtkTransform(this->GetTimestep());
  Geometry2D::ConstPointer worldGeometry = 
    renderer->GetCurrentWorldGeometry2D();
  PlaneGeometry::ConstPointer worldPlaneGeometry = 
    dynamic_cast<const PlaneGeometry*>( worldGeometry.GetPointer() );

  // set up the cutter orientation according to the current geometry of
  // the renderers plane
  vtkFloatingPointType vp[ 3 ], vnormal[ 3 ];
  Point3D point = worldPlaneGeometry->GetOrigin();
  Vector3D normal = worldPlaneGeometry->GetNormal(); normal.Normalize();
  vnl2vtk( point.Get_vnl_vector(), vp );
  vnl2vtk( normal.Get_vnl_vector(), vnormal );

  mitk::DisplayGeometry::Pointer dispGeometry = renderer->GetDisplayGeometry();
  mitk::Vector2D size = dispGeometry->GetSizeInMM();
  mitk::Vector2D origin = dispGeometry->GetOriginInMM();

  //
  //  |------O------|
  //  |      d2     |
  //  L  d1  M      |
  //  |             |
  //  |-------------|
  //

  mitk::Vector2D M;
  mitk::Vector2D L;
  mitk::Vector2D O;

  M[0] = origin[0] + size[0]/2;
  M[1] = origin[1] + size[1]/2;

  L[0] = origin[0];
  L[1] = origin[1] + size[1]/2;

  O[0] = origin[0] + size[0]/2;
  O[1] = origin[1] + size[1];

  mitk::Point2D point1;
  point1[0] = M[0]; point1[1] = M[1]; point1[2] = M[2];
  mitk::Point3D M3D;
  dispGeometry->Map(point1, M3D);

  point1[0] = L[0]; point1[1] = L[1]; point1[2] = L[2];
  mitk::Point3D L3D;
  dispGeometry->Map(point1, L3D);

  point1[0] = O[0]; point1[1] = O[1]; point1[2] = O[2];
  mitk::Point3D O3D;
  dispGeometry->Map(point1, O3D);

  double d1 = sqrt((M3D[0]-L3D[0])*(M3D[0]-L3D[0])
    + (M3D[1]-L3D[1])*(M3D[1]-L3D[1])
    + (M3D[2]-L3D[2])*(M3D[2]-L3D[2]));
  double d2 = sqrt((M3D[0]-O3D[0])*(M3D[0]-O3D[0])
    + (M3D[1]-O3D[1])*(M3D[1]-O3D[1])
    + (M3D[2]-O3D[2])*(M3D[2]-O3D[2]));
  double d = d1>d2 ? d1 : d2;
  d = d2;

  double viewAngle = renderer->GetVtkRenderer()->GetActiveCamera()->GetViewAngle();
  viewAngle = viewAngle * (ODF_MAPPER_PI/180.0);
  viewAngle /= 2;
  double dist = d/tan(viewAngle);

  renderer->GetVtkRenderer()->GetActiveCamera()->SetParallelProjection(true);
  renderer->GetVtkRenderer()->GetActiveCamera()->SetParallelScale(dist/3.74);

  mitk::Point3D mfoc; 
  mfoc[0]=M3D[0];
  mfoc[1]=M3D[1];
  mfoc[2]=M3D[2];

  mitk::Point3D mpos; 
  mpos[0]=mfoc[0]+dist*normal[0];
  mpos[1]=mfoc[1]+dist*normal[1];
  mpos[2]=mfoc[2]+dist*normal[2];

  mitk::Point3D mup; 
  mup[0]=O3D[0]-M3D[0];
  mup[1]=O3D[1]-M3D[1];
  mup[2]=O3D[2]-M3D[2];

  
  vtkCamera*   camera = renderer->GetVtkRenderer()->GetActiveCamera();
  if (camera)
  {
    camera->SetPosition(mpos[0],mpos[1],mpos[2]);
    camera->SetFocalPoint(mfoc[0], mfoc[1],mfoc[2]);
    camera->SetViewUp(mup[0],mup[1],mup[2]);
  }  
  renderer->GetVtkRenderer()->ResetCameraClippingRange();

  vtkTransform* inversetransform = vtkTransform::New();
  inversetransform->Identity();
  inversetransform->Concatenate(vtktransform->GetLinearInverse());
  double myscale[3];
  ((vtkTransform*)vtktransform)->GetScale(myscale);
  inversetransform->PostMultiply();
  inversetransform->Scale(1*myscale[0],1*myscale[1],1*myscale[2]);
  //inversetransform->GetScale(myscale);
  //inversetransform->PostMultiply();
  //inversetransform->Scale(1/myscale[0],1/myscale[1],1/myscale[2]);

  inversetransform->TransformPoint( vp, vp );
  inversetransform->TransformNormalAtPoint( vp, vnormal, vnormal );

  // vtk works in axis align coords
  // thus the normal also must be axis align, since 
  // we do not allow arbitrary cutting through volume
  //
  // vnormal should already be axis align, but in order
  // to get rid of precision effects, we set the two smaller
  // components to zero here
  int dims[3];
  m_VtkImage->GetDimensions(dims);
  double spac[3];
  m_VtkImage->GetSpacing(spac);
  if(fabs(vnormal[0]) > fabs(vnormal[1]) && fabs(vnormal[0]) > fabs(vnormal[2]) )
  {
    if(fabs(vp[0]/spac[0]) < 0.4)
      vp[0] = 0.4*spac[0];
    if(fabs(vp[0]/spac[0]) > (dims[0]-1)-0.4)
      vp[0] = ((dims[0]-1)-0.4)*spac[0];
    vnormal[1] = 0;
    vnormal[2] = 0;
  }

  if(fabs(vnormal[1]) > fabs(vnormal[0]) && fabs(vnormal[1]) > fabs(vnormal[2]) )
  {
    if(fabs(vp[1]/spac[1]) < 0.4)
      vp[1] = 0.4*spac[1];
    if(fabs(vp[1]/spac[1]) > (dims[1]-1)-0.4)
      vp[1] = ((dims[1]-1)-0.4)*spac[1];
    vnormal[0] = 0;
    vnormal[2] = 0;
  }

  if(fabs(vnormal[2]) > fabs(vnormal[1]) && fabs(vnormal[2]) > fabs(vnormal[0]) )
  {
    if(fabs(vp[2]/spac[2]) < 0.4)
      vp[2] = 0.4*spac[2];
    if(fabs(vp[2]/spac[2]) > (dims[2]-1)-0.4)
      vp[2] = ((dims[2]-1)-0.4)*spac[2];
    vnormal[0] = 0;
    vnormal[1] = 0;
  }


  m_Planes[index]->SetTransform( (vtkAbstractTransform*)NULL );
  m_Planes[index]->SetOrigin( vp );
  m_Planes[index]->SetNormal( vnormal );
  
  vtkPoints* points = NULL;
  vtkPoints* tmppoints = NULL;
  vtkPolyData* polydata = NULL;
  vtkFloatArray* pointdata = NULL;
  vtkDelaunay2D *delaunay = NULL;
  vtkPolyData* cuttedPlane = NULL;
  if(!( (dims[0] == 1 && vnormal[0] != 0) || 
        (dims[1] == 1 && vnormal[1] != 0) ||
        (dims[2] == 1 && vnormal[2] != 0) ))
  {
    m_Cutters[index]->SetCutFunction( m_Planes[index] );
    m_Cutters[index]->SetInput( m_VtkImage );
    m_Cutters[index]->Update();
    cuttedPlane = m_Cutters[index]->GetOutput();
  }
  else
  {
    // cutting of a 2D-Volume does not work, 
    // so we have to build up our own polydata object
    cuttedPlane = vtkPolyData::New();
    points = vtkPoints::New();
    points->SetNumberOfPoints(m_VtkImage->GetNumberOfPoints());
    for(int i=0; i<m_VtkImage->GetNumberOfPoints(); i++)
    {
      points->SetPoint(i, m_VtkImage->GetPoint(i));
    }
    cuttedPlane->SetPoints(points);

    pointdata = vtkFloatArray::New();
    int comps  = m_VtkImage->GetPointData()->GetScalars()->GetNumberOfComponents();
    pointdata->SetNumberOfComponents(comps);
    int tuples = m_VtkImage->GetPointData()->GetScalars()->GetNumberOfTuples();
    pointdata->SetNumberOfTuples(tuples);
    for(int i=0; i<tuples; i++)
      pointdata->SetTuple(i,m_VtkImage->GetPointData()->GetScalars()->GetTuple(i));
    pointdata->SetName( "vector" );
    cuttedPlane->GetPointData()->AddArray(pointdata);

    int nZero1, nZero2;
    if(dims[0]==1)
    {
      nZero1 = 1; nZero2 = 2;
    }
    else if(dims[1]==1)
    {
      nZero1 = 0; nZero2 = 2;
    }
    else
    {
      nZero1 = 0; nZero2 = 1;
    }

    tmppoints = vtkPoints::New();
    for(int j=0; j<m_VtkImage->GetNumberOfPoints(); j++){
      double pt[3];
      m_VtkImage->GetPoint(j,pt);
      tmppoints->InsertNextPoint(pt[nZero1],pt[nZero2],0);
    }

    polydata = vtkPolyData::New();
    polydata->SetPoints( tmppoints );
    delaunay = vtkDelaunay2D::New();
    delaunay->SetInput( polydata );
    delaunay->Update();
    vtkCellArray* polys = delaunay->GetOutput()->GetPolys();
    cuttedPlane->SetPolys(polys);
  }

  if(cuttedPlane->GetNumberOfPoints())
  {
    //  WINDOWING HERE
    inversetransform = vtkTransform::New();
    inversetransform->Identity();
    inversetransform->Concatenate(vtktransform->GetLinearInverse());
    double myscale[3];
    ((vtkTransform*)vtktransform)->GetScale(myscale);
    inversetransform->PostMultiply();
    inversetransform->Scale(1*myscale[0],1*myscale[1],1*myscale[2]);
    
    vnormal[0] = M3D[0]-O3D[0];
    vnormal[1] = M3D[1]-O3D[1];
    vnormal[2] = M3D[2]-O3D[2];
    vtkMath::Normalize(vnormal);
    vp[0] = M3D[0];
    vp[1] = M3D[1];
    vp[2] = M3D[2];

    inversetransform->TransformPoint( vp, vp );
    inversetransform->TransformNormalAtPoint( vp, vnormal, vnormal );

    m_ThickPlanes1[index]->count = 0;
    m_ThickPlanes1[index]->SetTransform((vtkAbstractTransform*)NULL );
    m_ThickPlanes1[index]->SetPose( vnormal, vp );
    m_ThickPlanes1[index]->SetThickness(d2);
    m_Clippers1[index]->SetClipFunction( m_ThickPlanes1[index] );
    m_Clippers1[index]->SetInput( cuttedPlane );
    m_Clippers1[index]->SetInsideOut(1);
    m_Clippers1[index]->Update();

    vnormal[0] = M3D[0]-L3D[0];
    vnormal[1] = M3D[1]-L3D[1];
    vnormal[2] = M3D[2]-L3D[2];
    vtkMath::Normalize(vnormal);
    vp[0] = M3D[0];
    vp[1] = M3D[1];
    vp[2] = M3D[2];

    inversetransform->TransformPoint( vp, vp );
    inversetransform->TransformNormalAtPoint( vp, vnormal, vnormal );

    m_ThickPlanes2[index]->count = 0;
    m_ThickPlanes2[index]->SetTransform((vtkAbstractTransform*)NULL );
    m_ThickPlanes2[index]->SetPose( vnormal, vp );
    m_ThickPlanes2[index]->SetThickness(d1);
    m_Clippers2[index]->SetClipFunction( m_ThickPlanes2[index] );
    m_Clippers2[index]->SetInput( m_Clippers1[index]->GetOutput() );
    m_Clippers2[index]->SetInsideOut(1);
    m_Clippers2[index]->Update();

    cuttedPlane = m_Clippers2[index]->GetOutput ();

    if(cuttedPlane->GetNumberOfPoints())
    {
      m_OdfsPlanes[index]->RemoveAllInputs();

      vtkPolyDataNormals* normals = vtkPolyDataNormals::New();
      normals->SetInputConnection( m_OdfSource->GetOutputPort() );
      normals->SplittingOff();
      normals->ConsistencyOff();
      normals->AutoOrientNormalsOff();
      normals->ComputePointNormalsOn();
      normals->ComputeCellNormalsOff();
      normals->FlipNormalsOff();
      normals->NonManifoldTraversalOff();

      vtkTransformPolyDataFilter* trans = vtkTransformPolyDataFilter::New();
      trans->SetInputConnection( normals->GetOutputPort() );
      trans->SetTransform(m_OdfTransform);

      vtkMaskedProgrammableGlyphFilter* glyphGenerator = vtkMaskedProgrammableGlyphFilter::New();
      glyphGenerator->SetMaximumNumberOfPoints(m_ShowMaxNumber);
      glyphGenerator->SetRandomMode(1);
      glyphGenerator->SetUseMaskPoints(1);
      glyphGenerator->SetSource( trans->GetOutput() );
      glyphGenerator->SetInput(cuttedPlane);
      glyphGenerator->SetColorModeToColorBySource();
      glyphGenerator->SetInputArrayToProcess(0,0,0, vtkDataObject::FIELD_ASSOCIATION_POINTS , "vector");
      glyphGenerator->SetGeometry(this->GetDataTreeNode()->GetData()->GetGeometry());
      glyphGenerator->SetGlyphMethod(&(GlyphMethod),(void *)glyphGenerator);
      try
      {
        glyphGenerator->Update();
      }
      catch( itk::ExceptionObject& err ) 
      {
        std::cout << err << std::endl;
      }
      m_OdfsPlanes[index]->AddInput(glyphGenerator->GetOutput());

      trans->Delete();
      glyphGenerator->Delete();
      normals->Delete();

      m_OdfsPlanes[index]->Update();
    }

  }
  m_PropAssemblies[index]->VisibilityOn();
  if(m_PropAssemblies[index]->GetParts()->IsItemPresent(m_OdfsActors[index]))
    m_PropAssemblies[index]->RemovePart(m_OdfsActors[index]);
  m_OdfsMappers[index]->SetInput(m_OdfsPlanes[index]->GetOutput());
  m_PropAssemblies[index]->AddPart(m_OdfsActors[index]);

  if(inversetransform) inversetransform->Delete();
  if(points) points->Delete();
  if(pointdata) pointdata->Delete();
  if(tmppoints) tmppoints->Delete();
  if(polydata) polydata->Delete();
  if(delaunay) delaunay->Delete();

}

template<class T, int N>
void  mitk::OdfVtkMapper2D<T,N>
::MitkRenderOverlay(mitk::BaseRenderer* renderer)
{
  //std::cout << "MitkRenderOverlay(" << renderer->GetName() << ")" << std::endl;
  if ( this->IsVisible(renderer)==false ) 
    return;

  if ( this->GetProp(renderer)->GetVisibility() )
  {
    this->GetProp(renderer)->RenderOverlay(renderer->GetVtkRenderer());
  }
}

template<class T, int N>
void  mitk::OdfVtkMapper2D<T,N>
::MitkRenderOpaqueGeometry(mitk::BaseRenderer* renderer)
{
  //std::cout << "MitkRenderOpaqueGeometry(" << renderer->GetName() << ")" << std::endl;
  if ( this->IsVisible( renderer )==false ) 
    return;

  if ( this->GetProp(renderer)->GetVisibility() )
  {
    if(this->GetDataTreeNode()->IsOn("DoRefresh",NULL))
    {
      glMatrixMode( GL_PROJECTION );  
      glPushMatrix();
      glLoadIdentity(); 

      glMatrixMode( GL_MODELVIEW );  
      glPushMatrix();
      glLoadIdentity(); 

      renderer->GetVtkRenderer()->SetErase(false);
      renderer->GetVtkRenderer()->GetActiveCamera()->Render(renderer->GetVtkRenderer());
      renderer->GetVtkRenderer()->SetErase(true);

      //GLfloat matrix[16];
      //glGetFloatv(GL_MODELVIEW_MATRIX, matrix);

      float LightPos[4] = {0,0,0,0};
      int index = GetIndex(renderer);
      if(index==0)
      {
        LightPos[2] = -1000;
      }
      if(index==1)
      {
        LightPos[0] = 1000;
      }
      if(index==2)
      {
        LightPos[1] = -1000;
      }
      glLightfv(GL_LIGHT0,GL_POSITION,LightPos);     
      glLightfv(GL_LIGHT1,GL_POSITION,LightPos);     
      glLightfv(GL_LIGHT2,GL_POSITION,LightPos);     
      glLightfv(GL_LIGHT3,GL_POSITION,LightPos);     
      glLightfv(GL_LIGHT4,GL_POSITION,LightPos);     
      glLightfv(GL_LIGHT5,GL_POSITION,LightPos);     
      glLightfv(GL_LIGHT6,GL_POSITION,LightPos);     
      glLightfv(GL_LIGHT7,GL_POSITION,LightPos);     

    }

    this->GetProp(renderer)->RenderOpaqueGeometry( renderer->GetVtkRenderer() );

    if(this->GetDataTreeNode()->IsOn("DoRefresh",NULL))
    {
      glMatrixMode( GL_PROJECTION );  
      glPopMatrix();

      glMatrixMode( GL_MODELVIEW );  
      glPopMatrix();
    }
  }
}

template<class T, int N>
void  mitk::OdfVtkMapper2D<T,N>
::MitkRenderTranslucentGeometry(mitk::BaseRenderer* renderer)
{
  //std::cout << "MitkRenderTranslucentGeometry(" << renderer->GetName() << ")" << std::endl;
  if ( this->IsVisible(renderer)==false ) 
    return;

  if ( this->GetProp(renderer)->GetVisibility() )
    //BUG (#1551) changed VTK_MINOR_VERSION FROM 3 to 2 cause RenderTranslucentGeometry was changed in minor version 2
#if ( ( VTK_MAJOR_VERSION >= 5 ) && ( VTK_MINOR_VERSION>=2)  )
    this->GetProp(renderer)->RenderTranslucentPolygonalGeometry(renderer->GetVtkRenderer());
#else
    this->GetProp(renderer)->RenderTranslucentGeometry(renderer->GetVtkRenderer());
#endif
}

template<class T, int N>
void  mitk::OdfVtkMapper2D<T,N>
::GenerateData()
{
  //std::cout << "GenerateData()" << std::endl;

  mitk::Image::Pointer input = const_cast<mitk::Image*>( this->GetInput() );
  if ( input.IsNull() )  return ;
  m_VtkImage = input->GetVtkImageData();

  if( m_VtkImage )
  {
    // make sure, that we have point data with more than 1 component (as vectors)
    vtkPointData* pointData = m_VtkImage->GetPointData();
    if ( pointData == NULL )
    {
      itkWarningMacro( << "m_VtkImage->GetPointData() returns NULL!" );
      return ;
    }
    if ( pointData->GetNumberOfArrays() == 0 )
    {
      itkWarningMacro( << "m_VtkImage->GetPointData()->GetNumberOfArrays() is 0!" );
      return ;
    }
    else if ( pointData->GetArray(0)->GetNumberOfComponents() != N)
    {
      itkWarningMacro( << "number of components != number of directions in ODF!" );
      return;
    }
    else if ( pointData->GetArrayName( 0 ) == NULL )
    {
      m_VtkImage->GetPointData()->GetArray(0)->SetName("vector");
    }
  }
  else
  {
    itkWarningMacro( << "m_VtkImage is NULL!" );
    return ;
  }
}

template<class T, int N>
void  mitk::OdfVtkMapper2D<T,N>
::GenerateData( mitk::BaseRenderer *renderer )
{
  if(!m_VtkImage)
  {
    itkWarningMacro( << "m_VtkImage is NULL!" );
    return ;
  }

  int index = GetIndex(renderer);
  LOG_INFO << index;

  
  // Spacing adapted scaling
  double spacing[3];
  m_VtkImage->GetSpacing(spacing);
  double min;
  if(index==0)
  {
    min = spacing[0];
    min = min > spacing[1] ? spacing[1] : min;
  }
  if(index==1)
  {
    min = spacing[1];
    min = min > spacing[2] ? spacing[2] : min;
  }
  if(index==2)
  {
    min = spacing[0];
    min = min > spacing[2] ? spacing[2] : min;
  }
  m_OdfSource->SetScale(min);

  // Light Sources
  vtkCollectionSimpleIterator sit;
  vtkLight* light;
  for(renderer->GetVtkRenderer()->GetLights()->InitTraversal(sit); 
      (light = renderer->GetVtkRenderer()->GetLights()->GetNextLight(sit)); )
    {
      renderer->GetVtkRenderer()->RemoveLight(light);
    }

  light = vtkLight::New();
  light->SetFocalPoint(0,0,0);
  light->SetLightTypeToSceneLight();
  light->SwitchOn();
  light->SetIntensity(1.0);
  light->PositionalOff();

  itk::Point<float> p;
  if(index==0)
  {
    p[0] = 0; p[1] = 0; p[2] = 10000;
  }
  if(index==1)
  {
    p[0] = 0; p[1] = 10000; p[2] = 0;
  }
  if(index==2)
  {
    p[0] = 10000; p[1] = 0; p[2] = 0;
  }

  mitk::Point3D p2;
  this->GetInput()->GetGeometry()->IndexToWorld(p,p2);
  light->SetPosition(p2[0],p2[1],p2[2]);
  renderer->GetVtkRenderer()->AddLight(light);

  this->GetDataTreeNode()->GetFloatProperty( "Scaling", m_Scaling );
  this->GetDataTreeNode()->GetIntProperty( "ShowMaxNumber", m_ShowMaxNumber );
  
  OdfNormalizationMethodProperty* nmp = dynamic_cast
    <OdfNormalizationMethodProperty*>(
    this->GetDataTreeNode()->GetProperty( "Normalization" ));
  if(nmp)
  {
    m_Normalization = nmp->GetNormalization();
  }

  OdfScaleByProperty* sbp = dynamic_cast
    <OdfScaleByProperty*>(
    this->GetDataTreeNode()->GetProperty( "ScaleBy" ));
  if(sbp)
  {
    m_ScaleBy = sbp->GetScaleBy();
  }
  
  this->GetDataTreeNode()->GetFloatProperty( "IndexParam1", m_IndexParam1);
  this->GetDataTreeNode()->GetFloatProperty( "IndexParam2", m_IndexParam2);
  
  if(IsVisible(renderer)==false)
  {
    m_OdfsActors[0]->VisibilityOff();
    m_OdfsActors[1]->VisibilityOff();
    m_OdfsActors[2]->VisibilityOff();
    return;
  }
  else
  {
    Slice(renderer);
  }

  // Get the TimeSlicedGeometry of the input object
  mitk::Image::Pointer input  = const_cast<mitk::Image*>(this->GetInput());
  const TimeSlicedGeometry *inputTimeGeometry = input->GetTimeSlicedGeometry();
  if (( inputTimeGeometry == NULL ) || ( inputTimeGeometry->GetTimeSteps() == 0 ))
  {
    m_PropAssemblies[0]->VisibilityOff();
    m_PropAssemblies[1]->VisibilityOff();
    m_PropAssemblies[2]->VisibilityOff();
    return;
  }

  if( inputTimeGeometry->IsValidTime( this->GetTimestep() ) == false )
  {
    m_PropAssemblies[0]->VisibilityOff();
    m_PropAssemblies[1]->VisibilityOff();
    m_PropAssemblies[2]->VisibilityOff();
    return;
  }


}

template<class T, int N>
void  mitk::OdfVtkMapper2D<T,N>
::SetDefaultProperties(mitk::DataTreeNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  node->SetProperty( "ShowMaxNumber", mitk::IntProperty::New( 150 ) );
  node->SetProperty( "Scaling", mitk::FloatProperty::New( 1.0 ) );
  node->SetProperty( "Normalization", mitk::OdfNormalizationMethodProperty::New());
  node->SetProperty( "ScaleBy", mitk::OdfScaleByProperty::New());
  node->SetProperty( "IndexParam1", mitk::FloatProperty::New(2));
  node->SetProperty( "IndexParam2", mitk::FloatProperty::New(1));
  node->SetProperty( "visible", mitk::BoolProperty::New( true ) );
  node->SetProperty ("layer", mitk::IntProperty::New(100));
  node->SetProperty( "DoRefresh", mitk::BoolProperty::New( true ) );
  //node->SetProperty( "opacity", mitk::FloatProperty::New(1.0f) );
}

#endif // __mitkOdfVtkMapper2D_txx__

