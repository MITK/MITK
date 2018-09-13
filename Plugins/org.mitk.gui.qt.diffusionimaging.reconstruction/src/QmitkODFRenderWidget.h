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

#ifndef QmitkODFRenderWidget_H_
#define QmitkODFRenderWidget_H_


#include <org_mitk_gui_qt_diffusionimaging_reconstruction_Export.h>

#include <QWidget>
#include <QmitkRenderWindow.h>
#include <QBoxLayout>
#include <vtkPolyData.h>
#include <mitkSurface.h>
#include <mitkDataNode.h>
#include <itkOrientationDistributionFunction.h>
#include <mitkStandaloneDataStorage.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkLookupTable.h>
#include <vtkDoubleArray.h>
#include <vtkCellData.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkCamera.h>
#include <mitkLookupTable.h>
#include <mitkLookupTableProperty.h>

class DIFFUSIONIMAGING_RECONSTRUCTION_EXPORT QmitkODFRenderWidget : public QWidget
{
  Q_OBJECT

public:

  QmitkODFRenderWidget( QWidget* parent = nullptr, Qt::WindowFlags f = nullptr );
  virtual ~QmitkODFRenderWidget();

  template<unsigned int ODF_SIZE=ODF_SAMPLING_SIZE>
  void GenerateODF( itk::OrientationDistributionFunction<float, ODF_SIZE> odf, int normalization, bool dir_color)
  {
    try
    {
      switch(normalization)
      {
      case 0:
        odf = odf.MinMaxNormalize();
        break;
      case 1:
        odf = odf.MaxNormalize();
        break;
      case 2:
        odf = odf.MaxNormalize();
        break;
      default:
        odf = odf.MinMaxNormalize();
      }

      m_Surface = mitk::Surface::New();
      m_ds = mitk::StandaloneDataStorage::New();
      m_Node = mitk::DataNode::New();

      vtkPolyData* m_TemplateOdf = itk::OrientationDistributionFunction<float,ODF_SIZE>::GetBaseMesh();

      vtkPolyData *polyData = vtkPolyData::New();
      vtkPoints *points = vtkPoints::New();
      vtkFloatArray *scalars = vtkFloatArray::New();

      for (unsigned int i=0; i<ODF_SIZE; i++){
        double p[3];
        m_TemplateOdf->GetPoints()->GetPoint(i,p);
        double val = odf[i];

        p[0] *= val;
        p[1] *= val;
        p[2] *= val;
        points->InsertPoint(i,p);

        scalars->InsertTuple1(i, 1-val);
      }

      polyData->SetPoints(points);
      vtkCellArray* polys = m_TemplateOdf->GetPolys();
      polyData->SetPolys(polys);
      polyData->GetPointData()->SetScalars(scalars);

      polys->Delete();
      scalars->Delete();
      points->Delete();

      m_Surface->SetVtkPolyData(polyData);
      m_Node->SetData(m_Surface);

      mitk::LookupTable::Pointer mitkLut = mitk::LookupTable::New();

      // assign an empty vtk lookup table to the odf renderer, it is the same
      // the ODF 2D Mapper has
      vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
      mitkLut->SetVtkLookupTable( lut );

      mitk::LookupTableProperty::Pointer mitkLutProp = mitk::LookupTableProperty::New();
      mitkLutProp->SetLookupTable(mitkLut);
      m_Node->SetProperty( "LookupTable", mitkLutProp );

      m_Node->SetProperty("scalar visibility", mitk::BoolProperty::New(true));
      m_Node->SetProperty("color mode", mitk::BoolProperty::New(true));
      m_Node->SetProperty("material.specularCoefficient", mitk::FloatProperty::New(0.5));

      if (dir_color)
      {
        m_Node->SetProperty("scalar visibility", mitk::BoolProperty::New(false));
        m_Node->SetProperty("color mode", mitk::BoolProperty::New(false));
        vnl_vector_fixed<double,3> d = odf.GetPrincipalDiffusionDirection();
        mitk::Color c;
        c.SetRed(fabs(static_cast<float>(d[0])));
        c.SetGreen(fabs(static_cast<float>(d[1])));
        c.SetBlue(fabs(static_cast<float>(d[2])));
        m_Node->SetColor(c);
      }

      m_ds->Add(m_Node);

      m_RenderWindow->GetRenderer()->SetDataStorage( m_ds );

      // adjust camera to current plane rotation
      mitk::PlaneGeometry::ConstPointer worldPlaneGeometry = mitk::BaseRenderer::GetInstance(m_RenderWindow->GetVtkRenderWindow())->GetCurrentWorldPlaneGeometry();
      mitk::Vector3D normal = worldPlaneGeometry->GetNormal();
      mitk::Vector3D up = worldPlaneGeometry->GetAxisVector(1);
      normal.Normalize();
      up.Normalize();

      vtkSmartPointer<vtkCamera> cam = vtkSmartPointer<vtkCamera>::New();
      const double camPos[3] = {normal[0],normal[1],normal[2]};
      const double camUp[3] = {up[0],up[1],up[2]};
      cam->SetPosition(camPos);
      cam->SetViewUp(camUp);
      cam->SetParallelProjection(1);
      m_RenderWindow->GetRenderer()->GetVtkRenderer()->SetActiveCamera(cam);
      m_RenderWindow->update();
    }
    catch (...)
    {

    }
  }

protected:

  QHBoxLayout* QmitkODFRenderWidgetLayout;
  QmitkRenderWindow* m_RenderWindow;

  mitk::Surface::Pointer m_Surface;
  mitk::StandaloneDataStorage::Pointer m_ds;
  mitk::DataNode::Pointer m_Node;
};
#endif /*QmitkODFRenderWidget_H_*/
