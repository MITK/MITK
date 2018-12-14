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


#ifndef __mitkOdfVtkMapper2D_txx__
#define __mitkOdfVtkMapper2D_txx__

#include "mitkOdfVtkMapper2D.h"
#include "mitkDataNode.h"
#include "mitkBaseRenderer.h"
#include "mitkMatrixConvert.h"
#include "mitkGeometry3D.h"
#include "mitkTimeGeometry.h"
#include "mitkOdfNormalizationMethodProperty.h"
#include "mitkOdfScaleByProperty.h"
#include "mitkProperties.h"
#include "mitkTensorImage.h"
#include "mitkShImage.h"

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
#include <vtkInformationVector.h>
#include <vtkInformation.h>
#include "vtkRenderer.h"

#include "itkOrientationDistributionFunction.h"

#include "itkFixedArray.h"

#include <mitkGL.h>
#include "vtkOpenGLRenderer.h"

#define _USE_MATH_DEFINES
#include <cmath>

#include <ciso646>


template<class T, int N>
vtkSmartPointer<vtkTransform> mitk::OdfVtkMapper2D<T,N>::m_OdfTransform = vtkSmartPointer<vtkTransform>::New();

template<class T, int N>
vtkSmartPointer<vtkOdfSource> mitk::OdfVtkMapper2D<T,N>::m_OdfSource = vtkSmartPointer<vtkOdfSource>::New();

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

template<class T, int N>
bool mitk::OdfVtkMapper2D<T, N>::m_ToggleTensorEllipsoidView = false;

template<class T, int N>
bool mitk::OdfVtkMapper2D<T, N>::m_ToggleColourisationMode = false;

template<class T, int N>
bool mitk::OdfVtkMapper2D<T, N>::m_ToggleGlyphPlacementMode = true;

template<class T, int N>
vtkSmartPointer<vtkDoubleArray> mitk::OdfVtkMapper2D<T, N>::m_ColourScalars = nullptr;


template<class T, int N>
vnl_matrix<float> mitk::OdfVtkMapper2D<T, N>::m_Sh2Basis =  mitk::sh::CalcShBasisForDirections(2, itk::PointShell<N, vnl_matrix_fixed<double, 3, N> >::DistributePointShell()->as_matrix());
template<class T, int N>
vnl_matrix<float> mitk::OdfVtkMapper2D<T, N>::m_Sh4Basis =  mitk::sh::CalcShBasisForDirections(4, itk::PointShell<N, vnl_matrix_fixed<double, 3, N> >::DistributePointShell()->as_matrix());
template<class T, int N>
vnl_matrix<float> mitk::OdfVtkMapper2D<T, N>::m_Sh6Basis =  mitk::sh::CalcShBasisForDirections(6, itk::PointShell<N, vnl_matrix_fixed<double, 3, N> >::DistributePointShell()->as_matrix());
template<class T, int N>
vnl_matrix<float> mitk::OdfVtkMapper2D<T, N>::m_Sh8Basis =  mitk::sh::CalcShBasisForDirections(8, itk::PointShell<N, vnl_matrix_fixed<double, 3, N> >::DistributePointShell()->as_matrix());
template<class T, int N>
vnl_matrix<float> mitk::OdfVtkMapper2D<T, N>::m_Sh10Basis =  mitk::sh::CalcShBasisForDirections(10, itk::PointShell<N, vnl_matrix_fixed<double, 3, N> >::DistributePointShell()->as_matrix());
template<class T, int N>
vnl_matrix<float> mitk::OdfVtkMapper2D<T, N>::m_Sh12Basis =  mitk::sh::CalcShBasisForDirections(12, itk::PointShell<N, vnl_matrix_fixed<double, 3, N> >::DistributePointShell()->as_matrix());


template<class T, int N>
mitk::OdfVtkMapper2D<T,N>::LocalStorage::LocalStorage()
{
  m_PropAssemblies.push_back(vtkSmartPointer<vtkPropAssembly>::New());
  m_PropAssemblies.push_back(vtkSmartPointer<vtkPropAssembly>::New());
  m_PropAssemblies.push_back(vtkSmartPointer<vtkPropAssembly>::New());

  m_OdfsPlanes.push_back(vtkSmartPointer<vtkAppendPolyData>::New());
  m_OdfsPlanes.push_back(vtkSmartPointer<vtkAppendPolyData>::New());
  m_OdfsPlanes.push_back(vtkSmartPointer<vtkAppendPolyData>::New());

  m_OdfsPlanes[0]->AddInputData(vtkSmartPointer<vtkPolyData>::New());
  m_OdfsPlanes[1]->AddInputData(vtkSmartPointer<vtkPolyData>::New());
  m_OdfsPlanes[2]->AddInputData(vtkSmartPointer<vtkPolyData>::New());

  m_OdfsActors.push_back(vtkSmartPointer<vtkActor>::New());
  m_OdfsActors.push_back(vtkSmartPointer<vtkActor>::New());
  m_OdfsActors.push_back(vtkSmartPointer<vtkActor>::New());

  m_OdfsActors[0]->GetProperty()->SetInterpolationToGouraud();
  m_OdfsActors[1]->GetProperty()->SetInterpolationToGouraud();
  m_OdfsActors[2]->GetProperty()->SetInterpolationToGouraud();

  m_OdfsMappers.push_back(vtkSmartPointer<vtkPolyDataMapper>::New());
  m_OdfsMappers.push_back(vtkSmartPointer<vtkPolyDataMapper>::New());
  m_OdfsMappers.push_back(vtkSmartPointer<vtkPolyDataMapper>::New());

  vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();

  m_OdfsMappers[0]->SetLookupTable(lut);
  m_OdfsMappers[1]->SetLookupTable(lut);
  m_OdfsMappers[2]->SetLookupTable(lut);

  m_OdfsActors[0]->SetMapper(m_OdfsMappers[0]);
  m_OdfsActors[1]->SetMapper(m_OdfsMappers[1]);
  m_OdfsActors[2]->SetMapper(m_OdfsMappers[2]);
}

template<class T, int N>
mitk::OdfVtkMapper2D<T,N>
::OdfVtkMapper2D()
{
  m_LastDisplayGeometry.push_back(OdfDisplayGeometry());
  m_LastDisplayGeometry.push_back(OdfDisplayGeometry());
  m_LastDisplayGeometry.push_back(OdfDisplayGeometry());

  m_Planes.push_back(vtkSmartPointer<vtkPlane>::New());
  m_Planes.push_back(vtkSmartPointer<vtkPlane>::New());
  m_Planes.push_back(vtkSmartPointer<vtkPlane>::New());

  m_Cutters.push_back(vtkSmartPointer<vtkCutter>::New());
  m_Cutters.push_back(vtkSmartPointer<vtkCutter>::New());
  m_Cutters.push_back(vtkSmartPointer<vtkCutter>::New());

  m_Cutters[0]->SetCutFunction( m_Planes[0] );
  m_Cutters[0]->GenerateValues( 1, 0, 1 );

  m_Cutters[1]->SetCutFunction( m_Planes[1] );
  m_Cutters[1]->GenerateValues( 1, 0, 1 );

  m_Cutters[2]->SetCutFunction( m_Planes[2] );
  m_Cutters[2]->GenerateValues( 1, 0, 1 );

  // Windowing the cutted planes in direction 1
  m_ThickPlanes1.push_back(vtkSmartPointer<vtkThickPlane>::New());
  m_ThickPlanes1.push_back(vtkSmartPointer<vtkThickPlane>::New());
  m_ThickPlanes1.push_back(vtkSmartPointer<vtkThickPlane>::New());

  m_Clippers1.push_back(vtkSmartPointer<vtkClipPolyData>::New());
  m_Clippers1.push_back(vtkSmartPointer<vtkClipPolyData>::New());
  m_Clippers1.push_back(vtkSmartPointer<vtkClipPolyData>::New());

  m_Clippers1[0]->SetClipFunction( m_ThickPlanes1[0] );
  m_Clippers1[1]->SetClipFunction( m_ThickPlanes1[1] );
  m_Clippers1[2]->SetClipFunction( m_ThickPlanes1[2] );

  // Windowing the cutted planes in direction 2
  m_ThickPlanes2.push_back(vtkSmartPointer<vtkThickPlane>::New());
  m_ThickPlanes2.push_back(vtkSmartPointer<vtkThickPlane>::New());
  m_ThickPlanes2.push_back(vtkSmartPointer<vtkThickPlane>::New());

  m_Clippers2.push_back(vtkSmartPointer<vtkClipPolyData>::New());
  m_Clippers2.push_back(vtkSmartPointer<vtkClipPolyData>::New());
  m_Clippers2.push_back(vtkSmartPointer<vtkClipPolyData>::New());

  m_Clippers2[0]->SetClipFunction( m_ThickPlanes2[0] );
  m_Clippers2[1]->SetClipFunction( m_ThickPlanes2[1] );
  m_Clippers2[2]->SetClipFunction( m_ThickPlanes2[2] );

  m_ShowMaxNumber = 500;
}

template<class T, int N>
mitk::OdfVtkMapper2D<T,N>
::~OdfVtkMapper2D()
{

}

template<class T, int N>
mitk::Image* mitk::OdfVtkMapper2D<T,N>
::GetInput()
{
  return static_cast<mitk::Image * > ( m_DataNode->GetData() );
}

template<class T, int N>
vtkProp*  mitk::OdfVtkMapper2D<T,N>
::GetVtkProp(mitk::BaseRenderer* renderer)
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);
  return localStorage->m_PropAssemblies[GetIndex(renderer)];
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
  vtkMaskedProgrammableGlyphFilter* pfilter=(vtkMaskedProgrammableGlyphFilter*)arg;

  double point[3];
  double debugpoint[3];
  pfilter->GetPoint(point);
  pfilter->GetPoint(debugpoint);

  itk::Point<double,3> p(point);
  Vector3D spacing = pfilter->GetGeometry()->GetSpacing();
  p[0] /= spacing[0];
  p[1] /= spacing[1];
  p[2] /= spacing[2];

  mitk::Point3D p2;
  pfilter->GetGeometry()->IndexToWorld( p, p2 );
  point[0] = p2[0];
  point[1] = p2[1];
  point[2] = p2[2];

  vtkPointData* data = pfilter->GetPointData();
  vtkDataArray* image_vals = data->GetArray("vector");
  vtkIdType id = pfilter->GetPointId();
  m_OdfTransform->Identity();
  m_OdfTransform->Translate(point[0],point[1],point[2]);

  typedef itk::OrientationDistributionFunction<float,N> OdfType;
  OdfType odf;

  if( image_vals->GetNumberOfComponents()==6 && !m_ToggleTensorEllipsoidView )
  {
    float tensorelems[6] = {
      (float)image_vals->GetComponent(id,0),
      (float)image_vals->GetComponent(id,1),
      (float)image_vals->GetComponent(id,2),
      (float)image_vals->GetComponent(id,3),
      (float)image_vals->GetComponent(id,4),
      (float)image_vals->GetComponent(id,5),
    };
    itk::DiffusionTensor3D<float> tensor(tensorelems);
    odf.InitFromTensor(tensor);
  }
  else if( image_vals->GetNumberOfComponents()==6 && m_ToggleTensorEllipsoidView )
  {
    float tensorelems[6] = {
      (float)image_vals->GetComponent(id,0),
      (float)image_vals->GetComponent(id,1),
      (float)image_vals->GetComponent(id,2),
      (float)image_vals->GetComponent(id,3),
      (float)image_vals->GetComponent(id,4),
      (float)image_vals->GetComponent(id,5),
    };
    itk::DiffusionTensor3D<float> tensor( tensorelems );
    odf.InitFromEllipsoid( tensor );
  }
  else if (image_vals->GetNumberOfComponents() == ODF_SAMPLING_SIZE)
  {
    for(int i=0; i<N; i++)
      odf[i] = (double)image_vals->GetComponent(id,i);
  }
  else
  {
    int nrCoeffs = image_vals->GetNumberOfComponents();
    Vector< float, N > odf_vals;
    vnl_vector< float > coeffs(nrCoeffs);
    for(int i=0; i<nrCoeffs; i++)
      coeffs[i] = (float)image_vals->GetComponent(id,i);

    switch (nrCoeffs)
    {
    case 6:
      odf_vals = ( m_Sh2Basis * coeffs ).data_block();
      break;
    case 15:
      odf_vals = ( m_Sh4Basis * coeffs ).data_block();
      break;
    case 28:
      odf_vals = ( m_Sh6Basis * coeffs ).data_block();
      break;
    case 45:
      odf_vals = ( m_Sh8Basis * coeffs ).data_block();
      break;
    case 66:
      odf_vals = ( m_Sh10Basis * coeffs ).data_block();
      break;
    case 91:
      odf_vals = ( m_Sh12Basis * coeffs ).data_block();
      break;
    default :
      mitkThrow() << "SH order larger 12 not supported in current ODF mapper version";
    }

    for(int i=0; i<N; i++)
      odf[i] = odf_vals[i];
  }

  if (m_ToggleColourisationMode)
  {
    m_OdfSource->SetUseCustomColor(true);
    vnl_vector_fixed<double,3> d = odf.GetPrincipalDiffusionDirection();
    m_OdfSource->SetColor(fabs(d[0])*255,fabs(d[1])*255,fabs(d[2])*255);
  }
  else
  {
    m_OdfSource->SetUseCustomColor(false);
  }

  switch(m_ScaleBy)
  {
  case ODFSB_NONE:
    m_OdfSource->SetScale(m_Scaling);
    break;
  case ODFSB_GFA:
    m_OdfSource->SetScale(m_Scaling*odf.GetGeneralizedFractionalAnisotropy());
    break;
  case ODFSB_PC:
    m_OdfSource->SetScale(m_Scaling*odf.GetPrincipleCurvature(m_IndexParam1, m_IndexParam2, 0));
    break;
  }

  m_OdfSource->SetNormalization(m_Normalization);
  m_OdfSource->SetOdf(odf);
  m_OdfSource->Modified();
}

template<class T, int N>
typename mitk::OdfVtkMapper2D<T,N>::OdfDisplayGeometry mitk::OdfVtkMapper2D<T,N>
::MeasureDisplayedGeometry(mitk::BaseRenderer* renderer)
{
  PlaneGeometry::ConstPointer worldPlaneGeometry = renderer->GetCurrentWorldPlaneGeometry();

  // set up the cutter orientation according to the current geometry of
  // the renderers plane
  double vp[ 3 ], vnormal[ 3 ];
  Point3D point = worldPlaneGeometry->GetOrigin();
  Vector3D normal = worldPlaneGeometry->GetNormal(); normal.Normalize();
  vnl2vtk( point.GetVnlVector(), vp );
  vnl2vtk( normal.GetVnlVector(), vnormal );

  Point2D dispSizeInMM = renderer->GetViewportSizeInMM();

  Point2D displayGeometryOriginInMM = renderer->GetOriginInMM();

  mitk::Vector2D size = dispSizeInMM.GetVectorFromOrigin();
  mitk::Vector2D origin = displayGeometryOriginInMM.GetVectorFromOrigin();

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
  point1[0] = M[0]; point1[1] = M[1];
  mitk::Point3D M3D;
  renderer->GetCurrentWorldPlaneGeometry()->Map(point1, M3D);

  point1[0] = L[0]; point1[1] = L[1];
  mitk::Point3D L3D;
  renderer->GetCurrentWorldPlaneGeometry()->Map(point1, L3D);

  point1[0] = O[0]; point1[1] = O[1];
  mitk::Point3D O3D;
  renderer->GetCurrentWorldPlaneGeometry()->Map(point1, O3D);

  double d1 = sqrt((M3D[0]-L3D[0])*(M3D[0]-L3D[0])
      + (M3D[1]-L3D[1])*(M3D[1]-L3D[1])
      + (M3D[2]-L3D[2])*(M3D[2]-L3D[2]));
  double d2 = sqrt((M3D[0]-O3D[0])*(M3D[0]-O3D[0])
      + (M3D[1]-O3D[1])*(M3D[1]-O3D[1])
      + (M3D[2]-O3D[2])*(M3D[2]-O3D[2]));
  double d = d1>d2 ? d1 : d2;
  d = d2;

  OdfDisplayGeometry retval;
  retval.vp[0] = vp[0];
  retval.vp[1] = vp[1];
  retval.vp[2] = vp[2];
  retval.vnormal[0] = vnormal[0];
  retval.vnormal[1] = vnormal[1];
  retval.vnormal[2] = vnormal[2];
  retval.normal[0] = normal[0];
  retval.normal[1] = normal[1];
  retval.normal[2] = normal[2];
  retval.d = d;
  retval.d1 = d1;
  retval.d2 = d2;
  retval.M3D[0] = M3D[0];
  retval.M3D[1] = M3D[1];
  retval.M3D[2] = M3D[2];
  retval.L3D[0] = L3D[0];
  retval.L3D[1] = L3D[1];
  retval.L3D[2] = L3D[2];
  retval.O3D[0] = O3D[0];
  retval.O3D[1] = O3D[1];
  retval.O3D[2] = O3D[2];

  retval.vp_original[0] = vp[0];
  retval.vp_original[1] = vp[1];
  retval.vp_original[2] = vp[2];
  retval.vnormal_original[0] = vnormal[0];
  retval.vnormal_original[1] = vnormal[1];
  retval.vnormal_original[2] = vnormal[2];
  retval.size[0] = size[0];
  retval.size[1] = size[1];
  retval.origin[0] = origin[0];
  retval.origin[1] = origin[1];

  return retval;
}

template<class T, int N>
void  mitk::OdfVtkMapper2D<T,N>
::Slice(mitk::BaseRenderer* renderer, OdfDisplayGeometry dispGeo)
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  vtkLinearTransform * vtktransform =
      this->GetDataNode()->GetVtkTransform(this->GetTimestep());

  int index = GetIndex(renderer);

  vtkSmartPointer<vtkTransform> inversetransform = vtkSmartPointer<vtkTransform>::New();
  inversetransform->Identity();
  inversetransform->Concatenate(vtktransform->GetLinearInverse());
  double myscale[3];
  ((vtkTransform*)vtktransform)->GetScale(myscale);
  myscale[0] = fabs(myscale[0]);
  myscale[1] = fabs(myscale[1]);
  myscale[2] = fabs(myscale[2]);
  inversetransform->PostMultiply();
  inversetransform->Scale(myscale[0],myscale[1],myscale[2]);
  inversetransform->TransformPoint( dispGeo.vp, dispGeo.vp );
  inversetransform->TransformNormalAtPoint( dispGeo.vp, dispGeo.vnormal, dispGeo.vnormal );

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
  if(fabs(dispGeo.vnormal[0]) > fabs(dispGeo.vnormal[1])
     && fabs(dispGeo.vnormal[0]) > fabs(dispGeo.vnormal[2]) )
  {
    if(fabs(dispGeo.vp[0]/spac[0]) < 0.4)
      dispGeo.vp[0] = 0.4*spac[0];
    if(fabs(dispGeo.vp[0]/spac[0]) > (dims[0]-1)-0.4)
      dispGeo.vp[0] = ((dims[0]-1)-0.4)*spac[0];
    dispGeo.vnormal[1] = 0;
    dispGeo.vnormal[2] = 0;
  }

  if(fabs(dispGeo.vnormal[1]) > fabs(dispGeo.vnormal[0]) && fabs(dispGeo.vnormal[1]) > fabs(dispGeo.vnormal[2]) )
  {
    if(fabs(dispGeo.vp[1]/spac[1]) < 0.4)
      dispGeo.vp[1] = 0.4*spac[1];
    if(fabs(dispGeo.vp[1]/spac[1]) > (dims[1]-1)-0.4)
      dispGeo.vp[1] = ((dims[1]-1)-0.4)*spac[1];
    dispGeo.vnormal[0] = 0;
    dispGeo.vnormal[2] = 0;
  }

  if(fabs(dispGeo.vnormal[2]) > fabs(dispGeo.vnormal[1]) && fabs(dispGeo.vnormal[2]) > fabs(dispGeo.vnormal[0]) )
  {
    if(fabs(dispGeo.vp[2]/spac[2]) < 0.4)
      dispGeo.vp[2] = 0.4*spac[2];
    if(fabs(dispGeo.vp[2]/spac[2]) > (dims[2]-1)-0.4)
      dispGeo.vp[2] = ((dims[2]-1)-0.4)*spac[2];
    dispGeo.vnormal[0] = 0;
    dispGeo.vnormal[1] = 0;
  }


  m_Planes[index]->SetTransform( (vtkAbstractTransform*)nullptr );
  m_Planes[index]->SetOrigin( dispGeo.vp );
  m_Planes[index]->SetNormal( dispGeo.vnormal );

  vtkSmartPointer<vtkPoints> points;
  vtkSmartPointer<vtkPoints> tmppoints;
  vtkSmartPointer<vtkPolyData> polydata;
  vtkSmartPointer<vtkFloatArray> pointdata;
  vtkSmartPointer<vtkDelaunay2D> delaunay;
  vtkSmartPointer<vtkPolyData> cuttedPlane;

  // the cutter only works if we do not have a 2D-image
  // or if we have a 2D-image and want to see the whole image.
  //
  // for side views of 2D-images, we need some special treatment
  if(!( (dims[0] == 1 && dispGeo.vnormal[0] != 0) ||
        (dims[1] == 1 && dispGeo.vnormal[1] != 0) ||
        (dims[2] == 1 && dispGeo.vnormal[2] != 0) ))
  {
    m_Cutters[index]->SetCutFunction( m_Planes[index] );
    m_Cutters[index]->SetInputData( m_VtkImage );
    m_Cutters[index]->Update();
    cuttedPlane = m_Cutters[index]->GetOutput();
  }
  else
  {
    // cutting of a 2D-Volume does not work,
    // so we have to build up our own polydata object
    cuttedPlane = vtkSmartPointer<vtkPolyData>::New();
    points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(m_VtkImage->GetNumberOfPoints());
    for(int i=0; i<m_VtkImage->GetNumberOfPoints(); i++)
    {
      points->SetPoint(i, m_VtkImage->GetPoint(i));
    }
    cuttedPlane->SetPoints(points);

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

    tmppoints = vtkSmartPointer<vtkPoints>::New();
    for(int j=0; j<m_VtkImage->GetNumberOfPoints(); j++){
      double pt[3];
      m_VtkImage->GetPoint(j,pt);
      tmppoints->InsertNextPoint(pt[nZero1],pt[nZero2],0);
    }

    polydata = vtkSmartPointer<vtkPolyData>::New();
    polydata->SetPoints( tmppoints );
    delaunay = vtkSmartPointer<vtkDelaunay2D>::New();
    delaunay->SetInputData( polydata );
    delaunay->Update();
    vtkCellArray* polys = delaunay->GetOutput()->GetPolys();
    cuttedPlane->SetPolys(polys);
  }

  if(cuttedPlane->GetNumberOfPoints())
  {
    //  WINDOWING HERE
    dispGeo.vnormal[0] = dispGeo.M3D[0]-dispGeo.O3D[0];
    dispGeo.vnormal[1] = dispGeo.M3D[1]-dispGeo.O3D[1];
    dispGeo.vnormal[2] = dispGeo.M3D[2]-dispGeo.O3D[2];
    vtkMath::Normalize(dispGeo.vnormal);
    dispGeo.vp[0] = dispGeo.M3D[0];
    dispGeo.vp[1] = dispGeo.M3D[1];
    dispGeo.vp[2] = dispGeo.M3D[2];

    inversetransform->TransformPoint( dispGeo.vp, dispGeo.vp );
    inversetransform->TransformNormalAtPoint( dispGeo.vp, dispGeo.vnormal, dispGeo.vnormal );

    m_ThickPlanes1[index]->count = 0;
    m_ThickPlanes1[index]->SetTransform((vtkAbstractTransform*)nullptr );
    m_ThickPlanes1[index]->SetPose( dispGeo.vnormal, dispGeo.vp );
    m_ThickPlanes1[index]->SetThickness(dispGeo.d2);
    m_Clippers1[index]->SetClipFunction( m_ThickPlanes1[index] );
    m_Clippers1[index]->SetInputData( cuttedPlane );
    m_Clippers1[index]->SetInsideOut(1);
    m_Clippers1[index]->Update();

    dispGeo.vnormal[0] = dispGeo.M3D[0]-dispGeo.L3D[0];
    dispGeo.vnormal[1] = dispGeo.M3D[1]-dispGeo.L3D[1];
    dispGeo.vnormal[2] = dispGeo.M3D[2]-dispGeo.L3D[2];
    vtkMath::Normalize(dispGeo.vnormal);
    dispGeo.vp[0] = dispGeo.M3D[0];
    dispGeo.vp[1] = dispGeo.M3D[1];
    dispGeo.vp[2] = dispGeo.M3D[2];

    inversetransform->TransformPoint( dispGeo.vp, dispGeo.vp );
    inversetransform->TransformNormalAtPoint( dispGeo.vp, dispGeo.vnormal, dispGeo.vnormal );

    m_ThickPlanes2[index]->count = 0;
    m_ThickPlanes2[index]->SetTransform((vtkAbstractTransform*)nullptr );
    m_ThickPlanes2[index]->SetPose( dispGeo.vnormal, dispGeo.vp );
    m_ThickPlanes2[index]->SetThickness(dispGeo.d1);
    m_Clippers2[index]->SetClipFunction( m_ThickPlanes2[index] );
    m_Clippers2[index]->SetInputData( m_Clippers1[index]->GetOutput() );
    m_Clippers2[index]->SetInsideOut(1);
    m_Clippers2[index]->Update();

    cuttedPlane = m_Clippers2[index]->GetOutput ();

    if(cuttedPlane->GetNumberOfPoints())
    {
      localStorage->m_OdfsPlanes[index]->RemoveAllInputs();

      vtkSmartPointer<vtkPolyDataNormals> normals = vtkSmartPointer<vtkPolyDataNormals>::New();
      normals->SetInputConnection( m_OdfSource->GetOutputPort() );
      normals->SplittingOff();
      normals->ConsistencyOff();
      normals->AutoOrientNormalsOff();
      normals->ComputePointNormalsOn();
      normals->ComputeCellNormalsOff();
      normals->FlipNormalsOff();
      normals->NonManifoldTraversalOff();

      vtkSmartPointer<vtkTransformPolyDataFilter> trans = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      trans->SetInputConnection( normals->GetOutputPort() );
      trans->SetTransform(m_OdfTransform);

      vtkSmartPointer<vtkMaskedProgrammableGlyphFilter> glyphGenerator = vtkSmartPointer<vtkMaskedProgrammableGlyphFilter>::New();
      glyphGenerator->SetMaximumNumberOfPoints(std::min(m_ShowMaxNumber,(int)cuttedPlane->GetNumberOfPoints()));
      glyphGenerator->SetRandomMode( m_ToggleGlyphPlacementMode );
      glyphGenerator->SetUseMaskPoints(1);
      glyphGenerator->SetSourceConnection(trans->GetOutputPort() );
      glyphGenerator->SetInput(cuttedPlane);
      glyphGenerator->SetColorModeToColorBySource();
      glyphGenerator->SetGeometry(this->GetDataNode()->GetData()->GetGeometry());
      glyphGenerator->SetGlyphMethod(&(GlyphMethod),(void *)glyphGenerator);

      try
      {
        glyphGenerator->Update();
      }
      catch( itk::ExceptionObject& err )
      {
        std::cout << err << std::endl;
      }

      localStorage->m_OdfsPlanes[index]->AddInputConnection(glyphGenerator->GetOutputPort());
      localStorage->m_OdfsPlanes[index]->Update();
    }
  }
  localStorage->m_OdfsMappers[index]->ScalarVisibilityOn();
  localStorage->m_OdfsMappers[index]->SetScalarModeToUsePointFieldData();
  localStorage->m_OdfsMappers[index]->SelectColorArray("ODF_COLORS");

  localStorage->m_PropAssemblies[index]->VisibilityOn();
  if(localStorage->m_PropAssemblies[index]->GetParts()->IsItemPresent(localStorage->m_OdfsActors[index]))
  {
    localStorage->m_PropAssemblies[index]->RemovePart(localStorage->m_OdfsActors[index]);
  }
  localStorage->m_OdfsMappers[index]->SetInputData(localStorage->m_OdfsPlanes[index]->GetOutput());
  localStorage->m_PropAssemblies[index]->AddPart(localStorage->m_OdfsActors[index]);
}

template<class T, int N>
bool mitk::OdfVtkMapper2D<T,N>
::IsVisibleOdfs(mitk::BaseRenderer* renderer)
{
  mitk::Image::Pointer input  = const_cast<mitk::Image*>(this->GetInput());
  const TimeGeometry *inputTimeGeometry = input->GetTimeGeometry();
  if(inputTimeGeometry==nullptr || inputTimeGeometry->CountTimeSteps()==0 || !inputTimeGeometry->IsValidTimeStep(this->GetTimestep()))
    return false;

  if(this->IsPlaneRotated(renderer))
    return false;

  bool retval = false;
  switch(GetIndex(renderer))
  {
  case 0:
    GetDataNode()->GetVisibility(retval, renderer, "VisibleOdfs_T");
    break;
  case 1:
    GetDataNode()->GetVisibility(retval, renderer, "VisibleOdfs_S");
    break;
  case 2:
    GetDataNode()->GetVisibility(retval, renderer, "VisibleOdfs_C");
    break;
  }

  return retval;
}

template<class T, int N>
void  mitk::OdfVtkMapper2D<T,N>
::MitkRenderOverlay(mitk::BaseRenderer* renderer)
{
  if ( this->IsVisibleOdfs(renderer)==false )
    return;

  if ( this->GetVtkProp(renderer)->GetVisibility() )
    this->GetVtkProp(renderer)->RenderOverlay(renderer->GetVtkRenderer());
}

template<class T, int N>
void  mitk::OdfVtkMapper2D<T,N>
::MitkRenderOpaqueGeometry(mitk::BaseRenderer* renderer)
{
  if ( this->IsVisibleOdfs( renderer )==false )
    return;

  if ( this->GetVtkProp(renderer)->GetVisibility() )
  {
    // adapt cam pos
    this->GetVtkProp(renderer)->RenderOpaqueGeometry( renderer->GetVtkRenderer() );
  }
}

template<class T, int N>
void  mitk::OdfVtkMapper2D<T,N>
::MitkRenderTranslucentGeometry(mitk::BaseRenderer* renderer)
{
  if ( this->IsVisibleOdfs(renderer)==false )
    return;

  if ( this->GetVtkProp(renderer)->GetVisibility() )
    this->GetVtkProp(renderer)->RenderTranslucentPolygonalGeometry(renderer->GetVtkRenderer());

}

template<class T, int N>
void mitk::OdfVtkMapper2D<T,N>
::Update(mitk::BaseRenderer* renderer)
{
  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");

  if ( !visible ) return;

  mitk::Image::Pointer input = const_cast<mitk::Image*>( this->GetInput() );
  if ( input.IsNull() ) return ;

  std::string classname("TensorImage");
  if(classname.compare(input->GetNameOfClass())==0)
    m_VtkImage = dynamic_cast<mitk::TensorImage*>( this->GetInput() )->GetNonRgbVtkImageData();

  std::string qclassname("OdfImage");
  if(qclassname.compare(input->GetNameOfClass())==0)
    m_VtkImage = dynamic_cast<mitk::OdfImage*>( this->GetInput() )->GetNonRgbVtkImageData();

  std::string shclassname("ShImage");
  if(shclassname.compare(input->GetNameOfClass())==0)
    m_VtkImage = dynamic_cast<mitk::ShImage*>( this->GetInput() )->GetNonRgbVtkImageData();

  if( m_VtkImage )
  {
    // make sure, that we have point data with more than 1 component (as vectors)
    vtkPointData* pointData = m_VtkImage->GetPointData();
    if ( pointData == nullptr )
    {
      itkWarningMacro( << "m_VtkImage->GetPointData() returns NULL!" );
      return ;
    }
    if ( pointData->GetNumberOfArrays() == 0 )
    {
      itkWarningMacro( << "m_VtkImage->GetPointData()->GetNumberOfArrays() is 0!" );
      return ;
    }
    else if ( pointData->GetArrayName( 0 ) == nullptr )
    {
      m_VtkImage->GetPointData()->GetArray(0)->SetName("vector");
    }

    GenerateDataForRenderer(renderer);
  }
  else
  {
    itkWarningMacro( << "m_VtkImage is NULL!" );
    return ;
  }
}

template<class T, int N>
void  mitk::OdfVtkMapper2D<T,N>
::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
  LocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  OdfDisplayGeometry dispGeo = MeasureDisplayedGeometry( renderer);

  if ((localStorage->m_LastUpdateTime >= m_DataNode->GetMTime()) //was the node modified?
      && (localStorage->m_LastUpdateTime >= m_DataNode->GetPropertyList()->GetMTime()) //was a property modified?
      && (localStorage->m_LastUpdateTime >= m_DataNode->GetPropertyList(renderer)->GetMTime())
      && dispGeo.Equals(m_LastDisplayGeometry.at(GetIndex(renderer))))
  {
    return;
  }

  localStorage->m_LastUpdateTime.Modified();

  if(!IsVisibleOdfs(renderer))
  {
    localStorage->m_OdfsActors[0]->VisibilityOff();
    localStorage->m_OdfsActors[1]->VisibilityOff();
    localStorage->m_OdfsActors[2]->VisibilityOff();
  }
  else
  {
    localStorage->m_OdfsActors[0]->VisibilityOn();
    localStorage->m_OdfsActors[1]->VisibilityOn();
    localStorage->m_OdfsActors[2]->VisibilityOn();

    m_OdfSource->SetAdditionalScale(GetMinImageSpacing(GetIndex(renderer)));
    ApplyPropertySettings();
    Slice(renderer, dispGeo);
    m_LastDisplayGeometry[GetIndex(renderer)] = dispGeo;
  }
}

template<class T, int N>
double  mitk::OdfVtkMapper2D<T,N>::GetMinImageSpacing( int index )
{
  // Spacing adapted scaling
  double spacing[3];
  m_VtkImage->GetSpacing(spacing);
  double min = spacing[0];
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
  return min;
}

template<class T, int N>
void  mitk::OdfVtkMapper2D<T,N>
::ApplyPropertySettings()
{
  this->GetDataNode()->GetFloatProperty( "Scaling", m_Scaling );
  this->GetDataNode()->GetIntProperty( "ShowMaxNumber", m_ShowMaxNumber );

  OdfNormalizationMethodProperty* nmp = dynamic_cast<OdfNormalizationMethodProperty*>(this->GetDataNode()->GetProperty( "Normalization" ));
  if(nmp)
    m_Normalization = nmp->GetNormalization();

  OdfScaleByProperty* sbp = dynamic_cast<OdfScaleByProperty*>(this->GetDataNode()->GetProperty( "ScaleBy" ));
  if(sbp)
    m_ScaleBy = sbp->GetScaleBy();

  this->GetDataNode()->GetFloatProperty( "IndexParam1", m_IndexParam1);
  this->GetDataNode()->GetFloatProperty( "IndexParam2", m_IndexParam2);

  this->GetDataNode()->GetBoolProperty( "DiffusionCore.Rendering.OdfVtkMapper.SwitchTensorView", m_ToggleTensorEllipsoidView );
  this->GetDataNode()->GetBoolProperty( "DiffusionCore.Rendering.OdfVtkMapper.ColourisationModeBit", m_ToggleColourisationMode );
  this->GetDataNode()->GetBoolProperty( "DiffusionCore.Rendering.OdfVtkMapper.RandomModeBit", m_ToggleGlyphPlacementMode);
}

template <class T, int N>
bool mitk::OdfVtkMapper2D<T,N>
::IsPlaneRotated(mitk::BaseRenderer* renderer)
{
  PlaneGeometry::ConstPointer worldPlaneGeometry = renderer->GetCurrentWorldPlaneGeometry();

  double vnormal[ 3 ];
  Vector3D normal = worldPlaneGeometry->GetNormal(); normal.Normalize();
  vnl2vtk( normal.GetVnlVector(), vnormal );

  mitk::Image* currentImage = dynamic_cast<mitk::Image* >( this->GetDataNode()->GetData() );
  if( currentImage == nullptr )
    return false;
  mitk::Vector3D imageNormal0 = currentImage->GetSlicedGeometry()->GetAxisVector(0);
  mitk::Vector3D imageNormal1 = currentImage->GetSlicedGeometry()->GetAxisVector(1);
  mitk::Vector3D imageNormal2 = currentImage->GetSlicedGeometry()->GetAxisVector(2);
  imageNormal0.Normalize();
  imageNormal1.Normalize();
  imageNormal2.Normalize();

  double eps = 0.000001; // Did you mean: std::numeric_limits<T>::epsilon(); ?
  int test = 0;
  if( fabs(fabs(dot_product(normal.GetVnlVector(),imageNormal0.GetVnlVector()))-1) > eps )
    test++;
  if( fabs(fabs(dot_product(normal.GetVnlVector(),imageNormal1.GetVnlVector()))-1) > eps )
    test++;
  if( fabs(fabs(dot_product(normal.GetVnlVector(),imageNormal2.GetVnlVector()))-1) > eps )
    test++;
  if (test==3)
    return true;
  return false;
}

template<class T, int N>
void  mitk::OdfVtkMapper2D<T,N>
::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer*  /*renderer*/, bool  /*overwrite*/)
{
  node->SetProperty( "ShowMaxNumber", mitk::IntProperty::New( 500 ) );
  node->SetProperty( "Normalization", mitk::OdfNormalizationMethodProperty::New());

  mitk::OdfScaleByProperty::Pointer prop = mitk::OdfScaleByProperty::New();
  prop->SetScaleByGFA();
  node->SetProperty( "ScaleBy", prop);

  if (dynamic_cast<mitk::TensorImage*>(node->GetData()))
  {
    node->AddProperty( "DiffusionCore.Rendering.OdfVtkMapper.ColourisationModeBit", mitk::BoolProperty::New( true ) );
    node->SetProperty( "Scaling", mitk::FloatProperty::New( 3.0 ) );
  }
  else
  {
    node->AddProperty( "DiffusionCore.Rendering.OdfVtkMapper.ColourisationModeBit", mitk::BoolProperty::New( false ) );
    node->SetProperty( "Scaling", mitk::FloatProperty::New( 1.5 ) );
  }

  node->SetProperty( "IndexParam1", mitk::FloatProperty::New(2));
  node->SetProperty( "IndexParam2", mitk::FloatProperty::New(1));
  node->SetProperty( "visible", mitk::BoolProperty::New( true ) );
  node->SetProperty( "VisibleOdfs_T", mitk::BoolProperty::New( false ) );
  node->SetProperty( "VisibleOdfs_C", mitk::BoolProperty::New( false ) );
  node->SetProperty( "VisibleOdfs_S", mitk::BoolProperty::New( false ) );
  node->SetProperty( "layer", mitk::IntProperty::New(100));
  node->SetProperty( "DoRefresh", mitk::BoolProperty::New( true ) );
  node->AddProperty( "DiffusionCore.Rendering.OdfVtkMapper.SwitchTensorView", mitk::BoolProperty::New( true) );
  node->AddProperty( "DiffusionCore.Rendering.OdfVtkMapper.RandomModeBit", mitk::BoolProperty::New( true ) );
}

#endif // __mitkOdfVtkMapper2D_txx__

