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

#include "QmitkODFRenderWidget.h"
#include <mitkLookupTable.h>
#include <mitkLookupTableProperty.h>

QmitkODFRenderWidget::QmitkODFRenderWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
{
  //create Layouts
  QmitkODFRenderWidgetLayout = new QHBoxLayout( this );

  //Set Layout to widget
  this->setLayout(QmitkODFRenderWidgetLayout);

  //Create RenderWindow
  m_RenderWindow = new QmitkRenderWindow(this, "odf render widget");
  m_RenderWindow->setMaximumSize(300,300);
  //m_RenderWindow->SetLayoutIndex( 3 );
  QmitkODFRenderWidgetLayout->addWidget( m_RenderWindow );
}


QmitkODFRenderWidget::~QmitkODFRenderWidget()
{
}

void QmitkODFRenderWidget::GenerateODF( std::vector<double> odfVals )
{
  m_Surface = mitk::Surface::New();
  m_ds = mitk::StandaloneDataStorage::New();
  m_Node = mitk::DataNode::New();

  vtkPolyData* m_TemplateOdf = itk::OrientationDistributionFunction<float,QBALL_ODFSIZE>::GetBaseMesh();

  vtkPolyData *polyData = vtkPolyData::New();
  vtkPoints *points = vtkPoints::New();
  vtkFloatArray *scalars = vtkFloatArray::New();

  double max = -10000;
  double min = 10000;
  for (int i=0; i<odfVals.size(); i++)
    if(odfVals.at(i)<min)
      min = odfVals.at(i);

  if (min<0)
    return;

  for (int i=0; i<odfVals.size(); i++)
    if(odfVals.at(i)>max)
      max = odfVals.at(i);

  if (std::abs(max)<mitk::eps)
    max = 0.0001;

  for (int i=0; i<odfVals.size(); i++){
    double p[3];
    m_TemplateOdf->GetPoints()->GetPoint(i,p);
    double val = odfVals.at(i);

    p[0] *= val;
    p[1] *= val;
    p[2] *= val;
    points->InsertPoint(i,p);
    scalars->InsertTuple1(i, 1-val/max);
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
  vtkLookupTable* vtkLut = mitkLut->GetVtkLookupTable();
  vtkLut->SetTableRange(0, 1);

  vtkLut->Build();
  mitk::LookupTableProperty::Pointer mitkLutProp = mitk::LookupTableProperty::New();
  mitkLutProp->SetLookupTable(mitkLut);
  m_Node->SetProperty( "LookupTable", mitkLutProp );

  m_Node->SetProperty("scalar visibility", mitk::BoolProperty::New(true));
  m_Node->SetProperty("color mode", mitk::BoolProperty::New(true));
  m_Node->SetProperty("material.specularCoefficient", mitk::FloatProperty::New(0.5));
//  m_Node->SetProperty("material.diffuseCoefficient", mitk::FloatProperty::New(0.05));
//  m_Node->SetProperty("material.ambientCoefficient", mitk::FloatProperty::New(0.5));
//  m_Node->SetProperty("material.interpolation", mitk::IntProperty::New(2));

  m_ds->Add(m_Node);

  m_RenderWindow->GetRenderer()->SetDataStorage( m_ds );
  m_RenderWindow->GetRenderer()->SetMapperID( mitk::BaseRenderer::Standard3D );

}
