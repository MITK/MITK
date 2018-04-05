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

#include "mitkUSNavigationTargetUpdateFilter.h"

#include <mitkBaseGeometry.h>
#include "mitkDataNode.h"
#include "mitkSurface.h"

#include "vtkSmartPointer.h"
#include "vtkUnsignedCharArray.h"
#include "vtkPolyData.h"
#include "vtkPointData.h"
#include "vtkLookupTable.h"
#include "mitkLookupTable.h"
#include "mitkLookupTableProperty.h"
#include "vtkFloatArray.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkLinearTransform.h"

mitk::USNavigationTargetUpdateFilter::USNavigationTargetUpdateFilter()
  : m_NumberOfTargets(0), m_OptimalAngle(0),
    m_ScalarArrayIdentifier("USNavigation::NoIdentifierSet"),
    m_UseMaximumScore(false)
{
}

mitk::USNavigationTargetUpdateFilter::~USNavigationTargetUpdateFilter()
{
}

void mitk::USNavigationTargetUpdateFilter::SetTargetStructure(DataNode::Pointer targetStructure)
{
  m_TargetStructure = targetStructure;

  // get vtk surface and the points
  vtkSmartPointer<vtkPolyData> targetSurfaceVtk = this->GetVtkPolyDataOfTarget();
  int numberOfPoints = targetSurfaceVtk->GetNumberOfPoints();

  if ( numberOfPoints > 0 )
  {
    // create vtk scalar array for score values
    vtkSmartPointer<vtkFloatArray> colors = vtkSmartPointer<vtkFloatArray>::New();
    colors->SetNumberOfComponents(1);
    colors->SetName(m_ScalarArrayIdentifier.c_str());

    // initialize with green color
    float color = 1;
    for (int n = 0; n < numberOfPoints ; ++n)
    {
      colors->InsertNextTuple1(color);
    }

    // add the scores array to the target surface
    targetSurfaceVtk->GetPointData()->AddArray(colors);
    targetSurfaceVtk->GetPointData()->Update();
  }
}

bool mitk::USNavigationTargetUpdateFilter::SetNumberOfTargets(unsigned int numberOfTargets)
{
  if ( numberOfTargets < 1 || numberOfTargets > 4 )
  {
    MITK_WARN << "Number of targets can only be between 1 and 4.";
    return false;
  }

  if ( numberOfTargets == 1 )
  {
    m_OptimalAngle = 0;
  }
  else if ( numberOfTargets < 5 )
  {
    // for every number of targets between 2 and 4 the optimal angle can be
    // calculated using the arcus cosinus
    m_OptimalAngle = acos(-1.0/(numberOfTargets-1));
  }

  m_NumberOfTargets = numberOfTargets;
  return true;
}

void mitk::USNavigationTargetUpdateFilter::SetOptimalAngle(double optimalAngle)
{
  m_OptimalAngle = optimalAngle;
}

double mitk::USNavigationTargetUpdateFilter::GetOptimalAngle()
{
  return m_OptimalAngle;
}

void mitk::USNavigationTargetUpdateFilter::SetScalarArrayIdentifier(std::string scalarArrayIdentifier)
{
  m_ScalarArrayIdentifier = scalarArrayIdentifier;
}

void mitk::USNavigationTargetUpdateFilter::SetUseMaximumScore(bool useMaximumScore)
{
  m_UseMaximumScore = useMaximumScore;
}

void mitk::USNavigationTargetUpdateFilter::SetControlNode(unsigned int id, itk::SmartPointer<DataNode> controlNode)
{
  // make sure that the node fit into the vector and insert it then
  if ( m_ControlNodesVector.size() <= id ) { m_ControlNodesVector.resize(id+1); }
  m_ControlNodesVector[id] = controlNode;

  this->UpdateTargetScores();
}

void mitk::USNavigationTargetUpdateFilter::RemovePositionOfTarget(unsigned int id)
{
  if ( id >= m_ControlNodesVector.size() )
  {
    mitkThrow() << "Given id is larger than the vector size.";
  }

  m_ControlNodesVector.erase(m_ControlNodesVector.begin()+id);

  this->UpdateTargetScores();
}

void mitk::USNavigationTargetUpdateFilter::Reset()
{
  m_ControlNodesVector.clear();

  if ( m_TargetStructure.IsNotNull() ) { this->UpdateTargetScores(); }
}

void mitk::USNavigationTargetUpdateFilter::UpdateTargetScores()
{
  mitk::BaseGeometry::Pointer targetStructureGeometry = this->GetGeometryOfTarget();
  mitk::Point3D targetStructureOrigin = targetStructureGeometry->GetOrigin();

  // get vtk surface and the points
  vtkSmartPointer<vtkPolyData> targetSurfaceVtk = this->GetVtkPolyDataOfTarget();
  int numberOfPoints = targetSurfaceVtk->GetNumberOfPoints();

  // transform vtk polydata according to mitk geometry
  vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter =
    vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  transformFilter->SetInputData(0, targetSurfaceVtk);
  transformFilter->SetTransform(targetStructureGeometry->GetVtkTransform());
  transformFilter->Update();
  vtkSmartPointer<vtkPolyData> targetSurfaceVtkTransformed = transformFilter->GetOutput();

  if ( numberOfPoints > 0 )
  {
    vtkSmartPointer<vtkFloatArray> colors =
      vtkSmartPointer<vtkFloatArray>::New();
    colors->SetNumberOfComponents(1);
    colors->SetName(m_ScalarArrayIdentifier.c_str());

    for (int n = 0; n < numberOfPoints ; ++n)
    {
      float score = m_UseMaximumScore ? 0.0 : 1.0;

      if ( m_ControlNodesVector.empty() )
      {
        // first target can be placed everywhere
        score = 1;
      }
      else if ( m_ControlNodesVector.size() == m_NumberOfTargets )
      {
        // if all targets are placed, there is no
        // good position for another target
        score = 0;
      }
      else
      {
        double coordinates[3];
        mitk::Point3D coordinatesMitk;
        targetSurfaceVtkTransformed->GetPoint(n, coordinates);
        coordinatesMitk[0] = coordinates[0];
        coordinatesMitk[1] = coordinates[1];
        coordinatesMitk[2] = coordinates[2];

        // vector pointing from the current surface coordinates to the origin
        // of the target structure
        itk::Vector<float, 3u> vector = targetStructureOrigin - coordinatesMitk;
        vector.Normalize();

        for (std::vector<mitk::DataNode::Pointer>::iterator it = m_ControlNodesVector.begin();
          it != m_ControlNodesVector.end(); ++it)
        {
          if ( (*it)->GetData() == 0 || (*it)->GetData()->GetGeometry() == 0 )
          {
            mitkThrow() << "Control data node and geometry of the node must not be null.";
          }

          itk::Vector<float, 3u> controlPointToOriginVector = targetStructureOrigin - (*it)->GetData()->GetGeometry()->GetOrigin();
          controlPointToOriginVector.Normalize();

          float angle = acos( vector * controlPointToOriginVector );
          float angleDifference = angle - m_OptimalAngle;
          float tmpScore = 1 - (angleDifference >= 0 ? angleDifference : -angleDifference);

          // update the score if the current score is larger (or lower) than
          // this score
          if ( (m_UseMaximumScore && tmpScore > score) || (!m_UseMaximumScore && tmpScore < score) )
          {
            score = tmpScore;
          }
        }
      }
      colors->InsertNextTuple1(score);
    }

    targetSurfaceVtk->GetPointData()->AddArray(colors);
    targetSurfaceVtk->GetPointData()->Update();
  }
}

vtkSmartPointer<vtkPolyData> mitk::USNavigationTargetUpdateFilter::GetVtkPolyDataOfTarget()
{
  if ( m_TargetStructure.IsNull() )
  {
    mitkThrow() << "Target structure must not be null.";
  }

  mitk::Surface::Pointer targetSurface = dynamic_cast<mitk::Surface*>(m_TargetStructure->GetData());
  if ( targetSurface.IsNull() )
  {
    mitkThrow() << "A mitk::Surface has to be set to the data node.";
  }

  vtkSmartPointer<vtkPolyData> targetSurfaceVtk = targetSurface->GetVtkPolyData();
  if( targetSurfaceVtk == 0 )
  {
    mitkThrow() << "VtkPolyData of the mitk::Surface of the data node must not be null.";
  }

  return targetSurfaceVtk;
}

mitk::BaseGeometry::Pointer mitk::USNavigationTargetUpdateFilter::GetGeometryOfTarget()
{
  if ( m_TargetStructure.IsNull() )
  {
    mitkThrow() << "Target structure must be set before position of target is set.";
  }

  mitk::BaseData* targetStructureData = m_TargetStructure->GetData();
  if ( ! targetStructureData )
  {
    mitkThrow() << "Data of target structure must not be null.";
  }

  mitk::BaseGeometry::Pointer targetStructureGeometry = targetStructureData->GetGeometry();
  if ( targetStructureGeometry.IsNull() )
  {
    mitkThrow() << "Geometry of target structure must not be null.";
  }

  return targetStructureGeometry;
}
