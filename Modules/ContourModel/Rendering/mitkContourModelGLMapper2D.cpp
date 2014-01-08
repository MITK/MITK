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


#include "mitkContourModelGLMapper2D.h"
#include "mitkBaseRenderer.h"
#include "mitkPlaneGeometry.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkContourModel.h"
#include "mitkContourModelSubDivisionFilter.h"
#include <vtkLinearTransform.h>

#include "mitkGL.h"

mitk::ContourModelGLMapper2D::ContourModelGLMapper2D() :
  m_SubdivisionContour(mitk::ContourModel::New()),
  m_InitSubdivisionCurve(true)
{
}

mitk::ContourModelGLMapper2D::~ContourModelGLMapper2D()
{
}


void mitk::ContourModelGLMapper2D::Paint(mitk::BaseRenderer * renderer)
{
  BaseLocalStorage *ls = m_LSH.GetLocalStorage(renderer);

  mitk::DataNode* dataNode = this->GetDataNode();

  bool visible = true;
  dataNode->GetVisibility(visible, renderer, "visible");

  if ( !visible ) return;

  mitk::ContourModel* input =  this->GetInput();

  mitk::ContourModel::Pointer renderingContour = input;

  bool subdivision = false;

  dataNode->GetBoolProperty( "subdivision curve", subdivision, renderer );
  if (subdivision)
  {
    if(this->m_SubdivisionContour->GetMTime() < renderingContour->GetMTime() || m_InitSubdivisionCurve)
    {

      //mitk::ContourModel::Pointer subdivContour = mitk::ContourModel::New();

      mitk::ContourModelSubDivisionFilter::Pointer subdivFilter = mitk::ContourModelSubDivisionFilter::New();

      subdivFilter->SetInput(input);
      subdivFilter->Update();

      this->m_SubdivisionContour = subdivFilter->GetOutput();

      m_InitSubdivisionCurve = false;
    }
    renderingContour = this->m_SubdivisionContour;
  }

  this->DrawContour(renderingContour, renderer);

  ls->UpdateGenerateDataTime();
}

mitk::ContourModel* mitk::ContourModelGLMapper2D::GetInput(void)
{
  return const_cast< mitk::ContourModel* >(static_cast< const mitk::ContourModel* > ( GetDataNode()->GetData() ));
}

void mitk::ContourModelGLMapper2D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  node->AddProperty( "contour.color", ColorProperty::New(0.9, 1.0, 0.1), renderer, overwrite );
  node->AddProperty( "contour.points.color", ColorProperty::New(1.0, 0.0, 0.1), renderer, overwrite );
  node->AddProperty( "contour.points.show", mitk::BoolProperty::New( false ), renderer, overwrite );
  node->AddProperty( "contour.segments.show", mitk::BoolProperty::New( true ), renderer, overwrite );
  node->AddProperty( "contour.controlpoints.show", mitk::BoolProperty::New( false ), renderer, overwrite );
  node->AddProperty( "contour.width", mitk::FloatProperty::New( 1.0 ), renderer, overwrite );
  node->AddProperty( "contour.hovering.width", mitk::FloatProperty::New( 3.0 ), renderer, overwrite );
  node->AddProperty( "contour.hovering", mitk::BoolProperty::New( false ), renderer, overwrite );
  node->AddProperty( "contour.points.text", mitk::BoolProperty::New( false ), renderer, overwrite );
  node->AddProperty( "contour.controlpoints.text", mitk::BoolProperty::New( false ), renderer, overwrite );

  node->AddProperty( "subdivision curve", mitk::BoolProperty::New( false ), renderer, overwrite );
  node->AddProperty( "contour.project-onto-plane", mitk::BoolProperty::New( false ), renderer, overwrite );

  node->AddProperty( "opacity", mitk::FloatProperty::New(1.0f), renderer, overwrite );

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}
