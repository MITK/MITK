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

#include "mitkNodeDisplacementFilter.h"



  mitk::NodeDisplacementFilter::NodeDisplacementFilter()
    : m_SelectedInput(-1)
  {
  }

  mitk::NodeDisplacementFilter::~NodeDisplacementFilter()
  {
  }

  bool mitk::NodeDisplacementFilter::AddNode( mitk::DataNode::Pointer node )
  {
    // Consistency Checks
    if (node.IsNull())
    {
      MITK_WARN << "Null Node passed to NodeDisplacementFilter. Ignoring Node....";
      return false;
    }
    if (node->GetData() == 0)
    {
      MITK_WARN << "Empty Node passed to NodeDisplacementFilter. Ignoring Node....";
      return false;
    }
    if(m_SelectedInput == -1)
      mitkThrow() << "Cannot add nodes before input Stream was selected";

    this->Update(); // Make sure we are working on current data
    mitk::NavigationData::Pointer reference = GetOutput(m_SelectedInput);

    if (! reference->IsDataValid())
    {
      MITK_WARN << "Cannot add node while selected tool is not tracked. Ignoring Node....";
      return false;
    }

    // Find Offset and add Node
    m_Nodes.push_back(node);
    mitk::Geometry3D::Pointer g3d = TransformToGeometry(this->NavigationDataToTransform(reference));
    mitk::Point3D referenceOffset;
    g3d->WorldToIndex(node->GetData()->GetGeometry()->GetOrigin(), referenceOffset);
    m_Offsets.push_back(referenceOffset);

    return true;
  }

  bool mitk::NodeDisplacementFilter::RemoveNode(unsigned int i)
  {
    if ( i >= m_Nodes.size() ) { return false; }

    m_Nodes.erase(m_Nodes.begin()+i);
    m_Offsets.erase(m_Offsets.begin()+i);
    return true;
  }

  int mitk::NodeDisplacementFilter::GetNumberOfNodes()
  {
    return m_Nodes.size();
  }

  mitk::DataNode::Pointer mitk::NodeDisplacementFilter::GetNode (unsigned int i)
  {
    if (i < m_Nodes.size() )
    {
      return m_Nodes.at(i);
    }
    else return NULL;
  }

  std::vector< mitk::DataNode::Pointer > mitk::NodeDisplacementFilter::GetNodes()
  {
    return m_Nodes;
  }

  void mitk::NodeDisplacementFilter::SelectInput(int i)
  {
    if (i < 0) { mitkThrow() << "Negative Input selected in NodeDisplacementFilter"; }
    if (! (static_cast<unsigned int>(i) < this->GetInputs().size()))
    {
      mitkThrow() << "Selected input index is larger than actual number of inputs in NodeDisplacementFilter";
    }
    m_SelectedInput = i;
  }

  void mitk::NodeDisplacementFilter::GenerateData()
  {
    // copy the navigation data from the inputs to the outputs
    mitk::NavigationDataPassThroughFilter::GenerateData();

    // If no reference has been set yet, warn and abort
    if (m_SelectedInput == -1)
    {
      MITK_INFO << "No input has been selected in NodeDisplacement Filter. Only forwarding NavigationData...";
      return;
    }

    // Cancel, if selected tool is currently not being tracked
    if (! this->GetInput(m_SelectedInput)->IsDataValid()) return;

    // Outputs have been updated, now to transform the nodes
    // 1) Generate Pseudo-Geometry for Reference
    mitk::AffineTransform3D::Pointer refTrans = this->NavigationDataToTransform(this->GetInput(m_SelectedInput));
    mitk::Geometry3D::Pointer refGeom = this->TransformToGeometry(refTrans);
    // 2) For each node, calculate new position
    for (unsigned int index=0; index < m_Nodes.size(); index++)
    {
      m_Nodes.at(index)->GetData()->SetGeometry( dynamic_cast<mitk::Geometry3D *> (refGeom->Clone().GetPointer()) );
      mitk::Point3D offset;
      m_Nodes.at(index)->GetData()->GetGeometry()->IndexToWorld(m_Offsets.at(index), offset);
      m_Nodes.at(index)->GetData()->GetGeometry()->SetOrigin(offset);
    }
  }

  void mitk::NodeDisplacementFilter::ResetNodes()
  {
    m_Nodes.clear();
    m_Offsets.clear();
  }

mitk::AffineTransform3D::Pointer mitk::NodeDisplacementFilter::NavigationDataToTransform(const mitk::NavigationData * nd)
{
  mitk::AffineTransform3D::Pointer affineTransform = mitk::AffineTransform3D::New();
  affineTransform->SetIdentity();

  //calculate the transform from the quaternions
  static itk::QuaternionRigidTransform<double>::Pointer quatTransform = itk::QuaternionRigidTransform<double>::New();

  mitk::NavigationData::OrientationType orientation = nd->GetOrientation();
  // convert mitk::ScalarType quaternion to double quaternion because of itk bug
  vnl_quaternion<double> doubleQuaternion(orientation.x(), orientation.y(), orientation.z(), orientation.r());
  quatTransform->SetIdentity();
  quatTransform->SetRotation(doubleQuaternion);
  quatTransform->Modified();

  /* because of an itk bug, the transform can not be calculated with float data type.
  To use it in the mitk geometry classes, it has to be transfered to mitk::ScalarType which is float */
  static AffineTransform3D::MatrixType m;
  mitk::TransferMatrix(quatTransform->GetMatrix(), m);
  affineTransform->SetMatrix(m);

  /*set the offset by convert from itkPoint to itkVector and setting offset of transform*/
  mitk::Vector3D pos;
  pos.SetVnlVector(nd->GetPosition().GetVnlVector());
  affineTransform->SetOffset(pos);

  affineTransform->Modified();
  return affineTransform;
}

mitk::Geometry3D::Pointer mitk::NodeDisplacementFilter::TransformToGeometry(mitk::AffineTransform3D::Pointer transform){
  mitk::Geometry3D::Pointer g3d = mitk::Geometry3D::New();
  g3d->SetIndexToWorldTransform(transform);
  g3d->TransferItkToVtkTransform(); // update VTK Transform for rendering too
  g3d->Modified();
  return g3d;
}
