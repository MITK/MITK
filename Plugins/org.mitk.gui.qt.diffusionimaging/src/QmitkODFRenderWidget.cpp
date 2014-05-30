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

#include "QmitkODFRenderWidget.h"
#include <mitkLookupTable.h>
#include <mitkLookupTableProperty.h>
#include <vtkSmartPointer.h>
#include <mitkPlaneGeometry.h>

QmitkODFRenderWidget::QmitkODFRenderWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
{
  //create Layouts
  QmitkODFRenderWidgetLayout = new QHBoxLayout( this );

  //Set Layout to widget
  this->setLayout(QmitkODFRenderWidgetLayout);

  //Create RenderWindow
  m_RenderWindow = new QmitkRenderWindow(this, "odf render widget");
  m_RenderWindow->setMaximumSize(300,300);
  m_RenderWindow->GetRenderer()->SetMapperID( mitk::BaseRenderer::Standard3D );

  //m_RenderWindow->SetLayoutIndex( 3 );
  QmitkODFRenderWidgetLayout->addWidget( m_RenderWindow );
}


QmitkODFRenderWidget::~QmitkODFRenderWidget()
{
}

void QmitkODFRenderWidget::GenerateODF( itk::OrientationDistributionFunction<double, QBALL_ODFSIZE> odf )
{
  try
  {
  m_Surface = mitk::Surface::New();
  m_ds = mitk::StandaloneDataStorage::New();
  m_Node = mitk::DataNode::New();

  vtkPolyData* m_TemplateOdf = itk::OrientationDistributionFunction<float,QBALL_ODFSIZE>::GetBaseMesh();

  vtkPolyData *polyData = vtkPolyData::New();
  vtkPoints *points = vtkPoints::New();
  vtkFloatArray *scalars = vtkFloatArray::New();

  for (int i=0; i<QBALL_ODFSIZE; i++){
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
  vtkLookupTable *lut = vtkLookupTable::New();
  mitkLut->SetVtkLookupTable( lut );

  mitk::LookupTableProperty::Pointer mitkLutProp = mitk::LookupTableProperty::New();
  mitkLutProp->SetLookupTable(mitkLut);
  m_Node->SetProperty( "LookupTable", mitkLutProp );

  m_Node->SetProperty("scalar visibility", mitk::BoolProperty::New(true));
  m_Node->SetProperty("color mode", mitk::BoolProperty::New(true));
  m_Node->SetProperty("material.specularCoefficient", mitk::FloatProperty::New(0.5));

  m_ds->Add(m_Node);

  m_RenderWindow->GetRenderer()->SetDataStorage( m_ds );

  // adjust camera to current plane rotation
  mitk::Geometry2D::ConstPointer worldGeometry = mitk::GlobalInteraction::GetInstance()->GetFocus()->GetCurrentWorldGeometry2D();
  mitk::PlaneGeometry::ConstPointer worldPlaneGeometry = dynamic_cast<const mitk::PlaneGeometry*>( worldGeometry.GetPointer() );
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
