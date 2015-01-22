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
#include "mitkPlanePositionManager.h"
#include "mitkInteractionConst.h"

mitk::PlanePositionManagerService::PlanePositionManagerService()
{
}

mitk::PlanePositionManagerService::~PlanePositionManagerService()
{
  for (unsigned int i = 0; i < m_PositionList.size(); ++i)
    delete m_PositionList[i];
}

unsigned int mitk::PlanePositionManagerService::AddNewPlanePosition ( const PlaneGeometry* plane, unsigned int sliceIndex )
{
    for (unsigned int i = 0; i < m_PositionList.size(); ++i)
    {
        if (m_PositionList[i] != 0)
        {
            bool isSameMatrix(true);
            bool isSameOffset(true);
            isSameOffset =  mitk::Equal(m_PositionList[i]->GetTransform()->GetOffset(), plane->GetIndexToWorldTransform()->GetOffset());
            if(!isSameOffset || sliceIndex != m_PositionList[i]->GetPos())
                continue;
            isSameMatrix = mitk::MatrixEqualElementWise(m_PositionList[i]->GetTransform()->GetMatrix(), plane->GetIndexToWorldTransform()->GetMatrix());
            if(isSameMatrix)
                return i;
        }

    }

    AffineTransform3D::Pointer transform = AffineTransform3D::New();
    Matrix3D matrix;
    matrix.GetVnlMatrix().set_column(0, plane->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(0));
    matrix.GetVnlMatrix().set_column(1, plane->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(1));
    matrix.GetVnlMatrix().set_column(2, plane->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(2));
    transform->SetMatrix(matrix);
    transform->SetOffset(plane->GetIndexToWorldTransform()->GetOffset());

    mitk::Vector3D direction;
    direction[0] = plane->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(2)[0];
    direction[1] = plane->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(2)[1];
    direction[2] = plane->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(2)[2];
    direction.Normalize();

    mitk::RestorePlanePositionOperation* newOp = new mitk::RestorePlanePositionOperation (OpRESTOREPLANEPOSITION, plane->GetExtent(0),
                                                                                          plane->GetExtent(1), plane->GetSpacing(), sliceIndex, direction, transform);

    m_PositionList.push_back( newOp );
    return GetNumberOfPlanePositions()-1;
}

bool mitk::PlanePositionManagerService::RemovePlanePosition( unsigned int ID )
{
  if (m_PositionList.size() > ID)
  {
    delete m_PositionList[ID];
    m_PositionList.erase(m_PositionList.begin()+ID);
    return true;
  }
  else
  {
    return false;
  }
}

mitk::RestorePlanePositionOperation* mitk::PlanePositionManagerService::GetPlanePosition ( unsigned int ID )
{
  if ( ID < m_PositionList.size() )
  {
    return m_PositionList[ID];
  }
  else
  {
    MITK_WARN<<"GetPlanePosition returned NULL!";
    return 0;
  }
}

unsigned int mitk::PlanePositionManagerService::GetNumberOfPlanePositions()
{
  return m_PositionList.size();
}

void mitk::PlanePositionManagerService::RemoveAllPlanePositions()
{
  for (unsigned int i = 0; i < m_PositionList.size(); ++i)
    delete m_PositionList[i];

  m_PositionList.clear();
}
