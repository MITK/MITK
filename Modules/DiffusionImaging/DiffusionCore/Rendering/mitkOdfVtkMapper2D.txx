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
#include <math.h>

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
bool mitk::OdfVtkMapper2D<T, N>::m_toggleTensorEllipsoidView = false;

template<class T, int N>
bool mitk::OdfVtkMapper2D<T, N>::m_toggleColourisationMode = false;

template<class T, int N>
bool mitk::OdfVtkMapper2D<T, N>::m_toggleGlyphPlacementMode = true;

template<class T, int N>
vtkSmartPointer<vtkDoubleArray> mitk::OdfVtkMapper2D<T, N>::m_colourScalars = nullptr;

#define ODF_MAPPER_PI M_PI


template<class T, int N>
mitk::OdfVtkMapper2D<T,N>::LocalStorage::LocalStorage()
{
    m_PropAssemblies.push_back(vtkPropAssembly::New());
    m_PropAssemblies.push_back(vtkPropAssembly::New());
    m_PropAssemblies.push_back(vtkPropAssembly::New());

    m_OdfsPlanes.push_back(vtkAppendPolyData::New());
    m_OdfsPlanes.push_back(vtkAppendPolyData::New());
    m_OdfsPlanes.push_back(vtkAppendPolyData::New());

    m_OdfsPlanes[0]->AddInputData(vtkPolyData::New());
    m_OdfsPlanes[1]->AddInputData(vtkPolyData::New());
    m_OdfsPlanes[2]->AddInputData(vtkPolyData::New());

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
    vtkDataArray* odfvals = data->GetArray("vector");
    vtkIdType id = pfilter->GetPointId();
    m_OdfTransform->Identity();
    m_OdfTransform->Translate(point[0],point[1],point[2]);

    typedef itk::OrientationDistributionFunction<float,N> OdfType;
    OdfType odf;

    if( odfvals->GetNumberOfComponents()==6 and not m_toggleTensorEllipsoidView )
    {
        float tensorelems[6] = {
            (float)odfvals->GetComponent(id,0),
            (float)odfvals->GetComponent(id,1),
            (float)odfvals->GetComponent(id,2),
            (float)odfvals->GetComponent(id,3),
            (float)odfvals->GetComponent(id,4),
            (float)odfvals->GetComponent(id,5),
        };
        itk::DiffusionTensor3D<float> tensor(tensorelems);
        odf.InitFromTensor(tensor);

        if( (pfilter-> GetColorMode()) == 0 ) // m_ColourisationMode; VTK_COLOR_BY_INPUT=0, VTK_COLOR_BY_SOURCE=1.
        { /// \brief Colourisation of glyph like in MitkWorkbench's dti visualisation, r,g,b,a=x,y,z,fa of main direction of diffusion.
//          unsigned char rgba[4] = {0,0,0,0};
          double rgba[4] = {0,0,0,0};
          rgba[3] = fabs(tensor.GetFractionalAnisotropy()); //* 255); // fa = FractionalAnisotropy => Helligkeit = Value, and alpha.
          typename itk::DiffusionTensor3D<T>::EigenValuesArrayType eigenValues;
          typename itk::DiffusionTensor3D<T>::EigenVectorsMatrixType eigenVectors;
          tensor.ComputeEigenAnalysis( eigenValues, eigenVectors ); // normalized eigenvectors as rows in ascending order.
          rgba[0] = (fabs( eigenVectors(2, 0) ) * rgba[3]);
          rgba[1] = (fabs( eigenVectors(2, 1) ) * rgba[3]);
          rgba[2] = (fabs( eigenVectors(2, 2) ) * rgba[3]);
//#define __IMG_DAT_ITEM__CLIP_00_FF__(val) (val) = ( (val) < 0 ) ? ( 0 ) : ( ( (val) > 255 ) ? ( 255 ) : ( (val) ) );
#define __IMG_DAT_ITEM__CLIP_0_1__(val) (val) = ( (val) < 0.0 ) ? ( 0.0 ) : ( ( (val) > 1.0 ) ? ( 1.0 ) : ( (val) ) );
        __IMG_DAT_ITEM__CLIP_0_1__(rgba[0]);
        __IMG_DAT_ITEM__CLIP_0_1__(rgba[1]);
        __IMG_DAT_ITEM__CLIP_0_1__(rgba[2]);
        __IMG_DAT_ITEM__CLIP_0_1__(rgba[3]);
          m_colourScalars-> InsertNextTuple4( rgba[0], rgba[1], rgba[2], rgba[3] ); // Allocates space automaticaly.

          data-> AddArray( m_colourScalars );
          data-> SetActiveScalars( "GLYPH_COLORS" );
        }
        else
        {
          data-> SetActiveScalars( "vector" );
          data-> RemoveArray( "GLYPH_COLORS" );
        }
    }

    else if( odfvals->GetNumberOfComponents()==6 and m_toggleTensorEllipsoidView )
    {
      float tensorElements[6] = {
          (float)odfvals->GetComponent(id, 0), (float)odfvals->GetComponent(id, 1), (float)odfvals->GetComponent(id, 2),
          (float)odfvals->GetComponent(id, 3), (float)odfvals->GetComponent(id, 4), (float)odfvals->GetComponent(id, 5),
      };
      itk::DiffusionTensor3D<float> tensor( tensorElements );
      odf.InitFromEllipsoid( tensor );

      if( (pfilter-> GetColorMode()) == 0 ) // m_ColourisationMode; VTK_COLOR_BY_INPUT=0, VTK_COLOR_BY_SOURCE=1.
      { /// \brief Colourisation of glyph like in MitkWorkbench's dti visualisation, r,g,b,a=x,y,z,fa of main direction of diffusion.
        double rgba[4] = {0,0,0,0};
        rgba[3] = fabs( tensor.GetFractionalAnisotropy()); // * 255 ); // fa = FractionalAnisotropy => Helligkeit = Value
        typename itk::DiffusionTensor3D<T>::EigenValuesArrayType eigenValues;
        typename itk::DiffusionTensor3D<T>::EigenVectorsMatrixType eigenVectors;
        tensor.ComputeEigenAnalysis( eigenValues, eigenVectors ); // normalized eigenvectors as rows in ascending order.
        rgba[0] = (fabs( eigenVectors(2, 0) ) * rgba[3]);
        rgba[1] = (fabs( eigenVectors(2, 1) ) * rgba[3]);
        rgba[2] = (fabs( eigenVectors(2, 2) ) * rgba[3]);
        __IMG_DAT_ITEM__CLIP_0_1__(rgba[0]);
        __IMG_DAT_ITEM__CLIP_0_1__(rgba[1]);
        __IMG_DAT_ITEM__CLIP_0_1__(rgba[2]);
        __IMG_DAT_ITEM__CLIP_0_1__(rgba[3]);

        m_colourScalars-> InsertNextTuple4( rgba[0], rgba[1], rgba[2], rgba[3] );
        data-> CopyScalarsOn();
        data-> SetCopyAttribute(id, 1);
        data-> SetCopyScalars(id);
        data-> CopyAllOn();
        data-> SetDebug('1');

        pfilter-> DebugOn();
        pfilter-> GetOutput()-> DebugOn();
        pfilter-> GetOutput()-> GetPointData()-> CopyScalarsOn();

        data-> AddArray( m_colourScalars );
        data-> SetActiveScalars( "GLYPH_COLORS" );

#ifndef NDEBUG
        {
          vtkIndent indent;
          std::cout << "<data>\n";
          data-> PrintSelf(std::cout, indent.GetNextIndent());
          std::cout << "</data>\n<pfilter>\n";
          pfilter-> PrintSelf(std::cout, indent.GetNextIndent());
          std::cout << pfilter->GetOutput()->GetClassName() << "\n";
          pfilter-> GetOutput()-> PrintSelf(std::cout, indent.GetNextIndent());
          pfilter-> GetOutput()->GetPointData()-> PrintSelf(std::cout, indent.GetNextIndent());
          pfilter-> GetOutput()->GetPointData()->GetScalars()-> PrintSelf(std::cout, indent.GetNextIndent());
          std::cout << "</pfilter>";
        }
#endif

      }
      else
      {
        data-> SetActiveScalars( "vector" );
        data-> RemoveArray( "GLYPH_COLORS" );
      }
    }

    else
    {
        for(int i=0; i<N; i++)
            odf[i] = (double)odfvals->GetComponent(id,i);
    }

    switch(m_ScaleBy)
    {
    case ODFSB_NONE:
        m_OdfSource->SetScale(m_Scaling);
        break;
    case ODFSB_GFA:
        m_OdfSource->SetScale(m_Scaling*odf.GetGeneralizedGFA(m_IndexParam1, m_IndexParam2));
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
    inversetransform->PostMultiply();
    inversetransform->Scale(1*myscale[0],1*myscale[1],1*myscale[2]);
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


    m_Planes[index]->SetTransform( (vtkAbstractTransform*)NULL );
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
        {
          pointdata->SetTuple( i, m_VtkImage->GetPointData()->GetScalars()->GetTuple(i) );
        }
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
        delaunay->SetInputData( polydata );
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
        m_ThickPlanes1[index]->SetTransform((vtkAbstractTransform*)NULL );
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
        m_ThickPlanes2[index]->SetTransform((vtkAbstractTransform*)NULL );
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

            glyphGenerator->SetRandomMode( m_toggleGlyphPlacementMode );

            glyphGenerator->SetUseMaskPoints(1);
            glyphGenerator->SetSourceConnection(trans->GetOutputPort() );
            glyphGenerator->SetInput(cuttedPlane);

            if( not m_toggleColourisationMode )
            {
              glyphGenerator-> SetColorModeToColorBySource();
              glyphGenerator-> SetInputArrayToProcess(0,0,0, vtkDataObject::FIELD_ASSOCIATION_POINTS , "vector");
              if(m_colourScalars != nullptr) { m_colourScalars-> Initialize(); }
            }
            else if ( m_toggleColourisationMode )
            {
              m_colourScalars = vtkSmartPointer<vtkDoubleArray>::New();
              m_colourScalars-> SetNumberOfComponents( 4 ); // red, green, blue and alpha are the 4 components per tuple.
              m_colourScalars-> SetName( "GLYPH_COLORS" );
              glyphGenerator-> SetColorModeToColorByInput();
              glyphGenerator-> SetInputArrayToProcess(0,0,0, vtkDataObject::FIELD_ASSOCIATION_POINTS , "GLYPH_COLORS");
            }

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
    if(inputTimeGeometry==NULL || inputTimeGeometry->CountTimeSteps()==0 || !inputTimeGeometry->IsValidTimeStep(this->GetTimestep()))
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

    std::string qclassname("QBallImage");
    if(qclassname.compare(input->GetNameOfClass())==0)
        m_VtkImage = dynamic_cast<mitk::QBallImage*>( this->GetInput() )->GetNonRgbVtkImageData();

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
        else if ( pointData->GetArray(0)->GetNumberOfComponents() != N
                  && pointData->GetArray(0)->GetNumberOfComponents() != 6 /*for tensor visualization*/)
        {
            itkWarningMacro( << "number of components != number of directions in ODF!" );
            return;
        }
        else if ( pointData->GetArrayName( 0 ) == NULL )
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


    if ( (localStorage->m_LastUpdateTime >= m_DataNode->GetMTime()) //was the node modified?
      && (localStorage->m_LastUpdateTime >= m_DataNode->GetPropertyList()->GetMTime()) //was a property modified?
      && (localStorage->m_LastUpdateTime >= m_DataNode->GetPropertyList(renderer)->GetMTime())
      && dispGeo.Equals(m_LastDisplayGeometry))
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

      for(unsigned iter=0; iter<3; ++iter)
      {
        if( m_toggleColourisationMode == true )
        {
          localStorage-> m_OdfsMappers[iter]-> SetColorModeToDirectScalars();
          localStorage-> m_OdfsMappers[iter]-> SelectColorArray("GLYPH_COLORS");
          localStorage-> m_OdfsMappers[iter]-> ScalarVisibilityOn();
          localStorage-> m_OdfsMappers[iter]-> SetScalarMode(3); // 0=Default, 1=UsePointData, 2=UseCellData, _3=UsePointFieldData_
          // 4=UseCellFieldData, 5=UseFieldData, rot oder weiss, manchmal schwarz danach ?... double [0;1] statt char [0;255] ?
        }
        else if ( m_toggleColourisationMode == false )
        {
          localStorage-> m_OdfsMappers[iter]-> SetColorModeToDefault();
          localStorage-> m_OdfsMappers[iter]-> SelectColorArray("vector");
          localStorage-> m_OdfsMappers[iter]-> ScalarVisibilityOn();
          localStorage-> m_OdfsMappers[iter]-> SetScalarMode(0); // 0=Default
        }
      }

      Slice(renderer, dispGeo);
      m_LastDisplayGeometry = dispGeo;
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

    this-> GetDataNode()-> GetBoolProperty( "DiffusionCore.Rendering.OdfVtkMapper.SwitchTensorView", m_toggleTensorEllipsoidView );
    this-> GetDataNode()-> GetBoolProperty( "DiffusionCore.Rendering.OdfVtkMapper.ColourisationModeBit", m_toggleColourisationMode );
    this-> GetDataNode()-> GetBoolProperty( "DiffusionCore.Rendering.OdfVtkMapper.RandomModeBit", m_toggleGlyphPlacementMode);
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
    if( currentImage == NULL )
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
    node->SetProperty( "ShowMaxNumber", mitk::IntProperty::New( 150 ) );
    node->SetProperty( "Scaling", mitk::FloatProperty::New( 1.0 ) );
    node->SetProperty( "Normalization", mitk::OdfNormalizationMethodProperty::New());
    node->SetProperty( "ScaleBy", mitk::OdfScaleByProperty::New());
    node->SetProperty( "IndexParam1", mitk::FloatProperty::New(2));
    node->SetProperty( "IndexParam2", mitk::FloatProperty::New(1));
    node->SetProperty( "visible", mitk::BoolProperty::New( true ) );
    node->SetProperty( "VisibleOdfs_T", mitk::BoolProperty::New( false ) );
    node->SetProperty( "VisibleOdfs_C", mitk::BoolProperty::New( false ) );
    node->SetProperty( "VisibleOdfs_S", mitk::BoolProperty::New( false ) );
    node->SetProperty( "layer", mitk::IntProperty::New(100));
    node->SetProperty( "DoRefresh", mitk::BoolProperty::New( true ) );
    node-> AddProperty( "DiffusionCore.Rendering.OdfVtkMapper.SwitchTensorView", mitk::BoolProperty::New( false) );
    node-> AddProperty( "DiffusionCore.Rendering.OdfVtkMapper.ColourisationModeBit", mitk::BoolProperty::New( false ) );
    node-> AddProperty( "DiffusionCore.Rendering.OdfVtkMapper.RandomModeBit", mitk::BoolProperty::New( true ) );
}

#endif // __mitkOdfVtkMapper2D_txx__

