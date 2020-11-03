/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "mitkNavigationDataSliceVisualization.h"

#include "mitkBaseRenderer.h"

mitk::NavigationDataSliceVisualization::NavigationDataSliceVisualization() : mitk::NavigationDataToNavigationDataFilter(),
  m_Renderer(nullptr),
  m_ViewDirection(Axial)
{
  m_TipOffset[0] = 0.0f;
  m_TipOffset[1] = 0.0f;
  m_TipOffset[2] = 0.0f;

  m_ToolTrajectory[0] = 0;
  m_ToolTrajectory[1] = 0;
  m_ToolTrajectory[2] = -1;

  m_WorldVerticalVector[0] = 0.0;
  m_WorldVerticalVector[1] = 1.0;
  m_WorldVerticalVector[2] = 0.0;
}

void mitk::NavigationDataSliceVisualization::SetToolTrajectory(Vector3D direction)
{
  if (Equal(direction.GetNorm(), 0.0))
  {
    MITK_WARN << "Ignoring invalid direction of projection: " << direction;
    return;
  }

  if (m_ToolTrajectory != direction)
  {
    m_ToolTrajectory = direction;
    this->SetViewDirection(Oblique);
    this->Modified();
  }
}

void mitk::NavigationDataSliceVisualization::GenerateData()
{
  // check if renderer was set
  if (m_Renderer.IsNull())
  {
    itkExceptionMacro(<< "Renderer was not properly set");
  }

  /* update outputs with tracking data from tools */
  unsigned int numberOfInputs = this->GetNumberOfInputs();
  if (numberOfInputs == 0)
  {
    return;
  }
  for (unsigned int i = 0; i < numberOfInputs ; ++i)
  {
    NavigationData* output = this->GetOutput(i);
    assert(output);
    const NavigationData* input = this->GetInput(i);
    assert(input);

    if (!input->IsDataValid())
      continue;

    output->Graft(input); // First, copy all information from input to output
  }

  // Nothing left to do if we don't have an input with valid data
  if (numberOfInputs == 0 || !this->GetInput()->IsDataValid())
    return;

  // get position from NavigationData to move the slice to this position
  Point3D slicePosition = this->GetInput()->GetPosition();

  {
    NavigationData::OrientationType orientation = this->GetInput()->GetOrientation();

    Vector3D transformedTipOffset;
    transformedTipOffset.SetVnlVector(orientation.rotate(m_TipOffset.GetVnlVector()));

    slicePosition += transformedTipOffset;

    mitk::SliceNavigationController::Pointer snc = m_Renderer->GetSliceNavigationController();

    if (Axial == m_ViewDirection)
    {
      snc->SetViewDirection(mitk::SliceNavigationController::Axial);
      snc->SelectSliceByPoint(slicePosition);
    }
    else if (Sagittal == m_ViewDirection)
    {
      snc->SetViewDirection(mitk::SliceNavigationController::Sagittal);
      snc->SelectSliceByPoint(slicePosition);
    }
    else if (Frontal == m_ViewDirection)
    {
      snc->SetViewDirection(mitk::SliceNavigationController::Frontal);
      snc->SelectSliceByPoint(slicePosition);
    }
    else if (AxialOblique == m_ViewDirection || SagittalOblique == m_ViewDirection)
    {
      const int slicingPlaneXAxis = AxialOblique == m_ViewDirection ? 0 : 2;

      // The column 0 is the slicing plane's x-axis, column 1 is the slicing plane's y-axis
      const mitk::PlaneGeometry::TransformType::MatrixType &m =
            m_Renderer->GetCurrentWorldPlaneGeometry()->GetIndexToWorldTransform()->GetMatrix();

      // Rotate the tool trajectory vector into world coordinate frame (assuming
      // NavigationData has passed through a NavigationDataTransformFilter to
      // convert it into world coordinate frame)
      Vector3D slicingPlaneYAxisVector;
      slicingPlaneYAxisVector.SetVnlVector(orientation.rotate(m_ToolTrajectory.GetVnlVector()));

      // Project the tool trajectory onto the plane normal to x-axis of this
      // oblique slicing. This defines the y-axis ("up") of the oblique slicing
      // plane
      slicingPlaneYAxisVector[slicingPlaneXAxis] = 0.0;

      // Do nothing for ambigous/undefined cases:
      //   - the R-L component of the x-axis is zero (for AxialOblique)
      //   - the S-I component of the x-axis is zero (for SagittalOblique)
      //   - the A-P component of the y-axis is zero
      if ( m(slicingPlaneXAxis,0) == 0.0 ||
           m(1,1) == 0.0 ||
           (slicingPlaneXAxis != 0 && slicingPlaneYAxisVector[0] == 0.0) ||
           (slicingPlaneXAxis != 1 && slicingPlaneYAxisVector[1] == 0.0) ||
           (slicingPlaneXAxis != 2 && slicingPlaneYAxisVector[2] == 0.0) )
      {
        return;
      }

      // Maintain the A-P orientation of the slice's y-axis regardless of what
      // direction the tool trajectory points
      /// @todo<C++11> Use std::signbit
      if ( (m(1,1) > 0) != (slicingPlaneYAxisVector[1] > 0) )
      {
        slicingPlaneYAxisVector *= -1;
      }

      Vector3D slicingPlaneXAxisVector;
      slicingPlaneXAxisVector.Fill(0.0);
      // For AxialOblique: maintain the Left/Right direction of the slice's x-axis
      // For SagittalOblique: maintain the Superior/Inferior direction of the slice's x-axis
      /// @todo<C++11> Use std::copysign
      slicingPlaneXAxisVector[slicingPlaneXAxis] = m(slicingPlaneXAxis,0) > 0 ? 1.0 : -1.0;

      Point3D origin;
      FillVector3D(origin, 0.0, 0.0, 0.0);
      snc->ReorientSlices(origin, slicingPlaneXAxisVector, slicingPlaneYAxisVector);
      snc->SelectSliceByPoint(slicePosition);
    }
    else if (Oblique == m_ViewDirection)
    {
      Vector3D slicingPlaneNormalVector;
      slicingPlaneNormalVector.SetVnlVector(orientation.rotate(m_ToolTrajectory.GetVnlVector()));

      // The second column of the Index-to-World matrix is the positive y-axis
      // of the current slicing plane in world coordinates.
      const mitk::PlaneGeometry::TransformType::MatrixType &m =
            m_Renderer->GetCurrentWorldPlaneGeometry()->GetIndexToWorldTransform()->GetMatrix();
      mitk::Vector3D currentSlicingPlaneUpVector;
      mitk::FillVector3D(currentSlicingPlaneUpVector, m[0][1], m[1][1], m[2][1]);
      mitk::Vector3D worldUpVector = m_WorldVerticalVector;
      if (angle(worldUpVector.GetVnlVector(), currentSlicingPlaneUpVector.GetVnlVector()) > vnl_math::pi_over_2 )
      {
        worldUpVector *= -1;
      }

      mitk::PlaneGeometry::Pointer slicingPlane = mitk::PlaneGeometry::New();
      Point3D origin;
      FillVector3D(origin, 0.0, 0.0, 0.0);
      slicingPlane->InitializePlane(origin, slicingPlaneNormalVector);

      // Now that we have the direction of WorldVerticalVector chosen to be the
      // most "up" direction, project it onto the slicing plane to define the
      // up vector (y-axis) of the reoriented slices
      mitk::Vector3D slicingPlaneUpVector;
      if ( slicingPlane->Project(worldUpVector, slicingPlaneUpVector) )
      {
        // slicingPlaneUpVector CROSS slicingPlaneNormalVector -> slicingPlaneRightVector
        // Math is done in double precision as much as possible to get more
        // orthogonal right and up vectors which fixes a VNL SVD error when
        // the WorldGeometry matrix is later inverted
        itk::Vector<double,3> slicingPlaneUpVector_double;
        FillVector3D(slicingPlaneUpVector_double,
                     slicingPlaneUpVector[0], slicingPlaneUpVector[1], slicingPlaneUpVector[2]);
        itk::Vector<double,3> slicingPlaneNormalVector_double;
        FillVector3D(slicingPlaneNormalVector_double,
                     slicingPlaneNormalVector[0], slicingPlaneNormalVector[1], slicingPlaneNormalVector[2]);
        itk::Vector<double,3> slicingPlaneRightVector_double = itk::CrossProduct(slicingPlaneUpVector_double,
                                                                                 slicingPlaneNormalVector_double);

        mitk::Vector3D slicingPlaneRightVector;
        mitk::FillVector3D(slicingPlaneRightVector,
                           slicingPlaneRightVector_double[0], slicingPlaneRightVector_double[1], slicingPlaneRightVector_double[2]);
        mitk::FillVector3D(slicingPlaneUpVector,
                           slicingPlaneUpVector_double[0], slicingPlaneUpVector_double[1], slicingPlaneUpVector_double[2]);

        snc->ReorientSlices(origin, slicingPlaneRightVector, slicingPlaneUpVector);
        snc->SelectSliceByPoint(slicePosition);
      }
    }
    else
    {
      MITK_ERROR << "Unsupported ViewDirection: " << m_ViewDirection;
    }

    m_Renderer->RequestUpdate();
  }
}

