/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkContourModelSetMapper2D.h>

#include <mitkColorProperty.h>
#include <mitkContourModelSet.h>
#include <mitkPlaneGeometry.h>
#include <mitkProperties.h>
#include <vtkLinearTransform.h>

mitk::ContourModelSetMapper2D::ContourModelSetMapper2D()
{
}

mitk::ContourModelSetMapper2D::~ContourModelSetMapper2D()
{
}

int mitk::ContourModelSetMapper2D::MitkRender(mitk::BaseRenderer *renderer, mitk::VtkPropRenderer::RenderType type)
{
    // Drawing happens immediately through a vtkContext2D without a vtkProp, so the
    // contours have to be painted in exactly one pass. Overlay is the last one and
    // therefore the pass that determines the final pixels.
    if (type != mitk::VtkPropRenderer::Overlay)
        return 0;

    BaseLocalStorage *ls = m_LSH.GetLocalStorage(renderer);

    mitk::DataNode::Pointer dataNode = this->GetDataNode();
    bool visible = true;
    dataNode->GetVisibility(visible, nullptr);

    if (!visible)
        return 0;

    mitk::ContourModelSet::Pointer input = this->GetInput();

    if (input->GetSize() < 1)
        return 0;

    auto centerOfViewPointZ = renderer->GetCurrentWorldPlaneGeometry()->GetCenter()[2];
    auto it = input->Begin();

    auto end = input->End();

    int numberOfRenderedContours = 0;

    while (it != end)
    {
        //we have the assumption that each contour model vertex has the same z coordinate
        auto currentZValue = (*it)->GetVertexAt(0)->Coordinates[2];
        double acceptedDeviationInMM = 5.0;
        //only draw contour if it is visible
        if (currentZValue - acceptedDeviationInMM < centerOfViewPointZ && currentZValue + acceptedDeviationInMM > centerOfViewPointZ){
            this->DrawContour(it->GetPointer(), renderer);
            ++numberOfRenderedContours;
        }
        ++it;
    }

    ls->UpdateGenerateDataTime();

    return numberOfRenderedContours;
}

mitk::ContourModelSet *mitk::ContourModelSetMapper2D::GetInput(void)
{
  return static_cast<mitk::ContourModelSet *>(GetDataNode()->GetData());
}

void mitk::ContourModelSetMapper2D::SetDefaultProperties(mitk::DataNode *node,
                                                           mitk::BaseRenderer *renderer,
                                                           bool overwrite)
{
  node->AddProperty("contour.color", ColorProperty::New(0.9, 1.0, 0.1), renderer, overwrite);
  node->AddProperty("contour.points.color", ColorProperty::New(1.0, 0.0, 0.1), renderer, overwrite);
  node->AddProperty("contour.points.show", mitk::BoolProperty::New(false), renderer, overwrite);
  node->AddProperty("contour.segments.show", mitk::BoolProperty::New(true), renderer, overwrite);
  node->AddProperty("contour.controlpoints.show", mitk::BoolProperty::New(false), renderer, overwrite);
  node->AddProperty("contour.width", mitk::FloatProperty::New(1.0), renderer, overwrite);
  node->AddProperty("contour.hovering.width", mitk::FloatProperty::New(3.0), renderer, overwrite);
  node->AddProperty("contour.hovering", mitk::BoolProperty::New(false), renderer, overwrite);

  node->AddProperty("contour.project-onto-plane", mitk::BoolProperty::New(false), renderer, overwrite);

  node->AddProperty("opacity", mitk::FloatProperty::New(1.0f), renderer, overwrite);

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}
