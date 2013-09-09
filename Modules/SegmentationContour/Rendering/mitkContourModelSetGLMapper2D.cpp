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


#include "mitkContourModelSetGLMapper2D.h"
#include "mitkBaseRenderer.h"
#include "mitkPlaneGeometry.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkContourModel.h"
#include "mitkContourModelSubDivisionFilter.h"
#include <vtkLinearTransform.h>

#include "mitkGL.h"

mitk::ContourModelSetGLMapper2D::ContourModelSetGLMapper2D()
{
}

mitk::ContourModelSetGLMapper2D::~ContourModelSetGLMapper2D()
{
}


void mitk::ContourModelSetGLMapper2D::Paint(mitk::BaseRenderer * renderer)
{
}

const mitk::ContourModelSet* mitk::ContourModelSetGLMapper2D::GetInput(void)
{
  return static_cast<const mitk::ContourModelSet * > ( GetDataNode()->GetData() );
}

void mitk::ContourModelSetGLMapper2D::DrawContour(mitk::ContourModel &contour)
{
}

void mitk::ContourModelSetGLMapper2D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
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

  node->AddProperty( "contour.project-onto-plane", mitk::BoolProperty::New( false ), renderer, overwrite );

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}
