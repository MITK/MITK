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
      MITK_WARN("NodeDisplacementFilter")
        << "Null Node passed to NodeDisplacementFilter. Ignoring Node....";
      return false;
    }
    if (node->GetData() == 0)
    {
      MITK_WARN("NodeDisplacementFilter")
        << "Empty Node passed to NodeDisplacementFilter. Ignoring Node....";
      return false;
    }
    if(m_SelectedInput == -1)
    {
      MITK_ERROR("NodeDisplacementFilter")
        << "Cannot add nodes before input Stream was selected";
      mitkThrow() << "Cannot add nodes before input Stream was selected";
    }

    this->Update(); // make sure we are working on current data
    mitk::NavigationData::Pointer reference = this->GetOutput(m_SelectedInput);

    if (! reference->IsDataValid())
    {
      MITK_WARN("NodeDisplacementFilter")
        << "Cannot add node while selected tool is not tracked. Ignoring Node....";
      return false;
    }

    // find transformation and add node
    mitk::AffineTransform3D::Pointer inverseAffineTransform = mitk::AffineTransform3D::New();
    if ( ! reference->GetAffineTransform3D()->GetInverse(inverseAffineTransform) )
    {
      MITK_ERROR("NodeDisplacementFilter")
        << "Could not get the inverse transformation of the navigation data transformation.";
      mitkThrow() << "Could not get the inverse transformation of the navigation data transformation.";
    }

    inverseAffineTransform->Compose(node->GetData()->GetGeometry()->GetIndexToWorldTransform(), true);
    m_Transforms.push_back(inverseAffineTransform);
    m_Nodes.push_back(node);

    return true;
  }

  bool mitk::NodeDisplacementFilter::RemoveNode(unsigned int i)
  {
    if ( i >= m_Nodes.size() ) { return false; }

    m_Nodes.erase(m_Nodes.begin()+i);
    m_Transforms.erase(m_Transforms.begin()+i);
    return true;
  }

  int mitk::NodeDisplacementFilter::GetNumberOfNodes()
  {
    return m_Nodes.size();
  }

  mitk::DataNode::Pointer mitk::NodeDisplacementFilter::GetNode (unsigned int i)
  {
    if (i < m_Nodes.size() ) { return m_Nodes.at(i); }
    else { return NULL; }
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
      MITK_ERROR("NodeDisplacementFilter")
        << "Selected input index is larger than actual number of inputs.";
      mitkThrow() << "Selected input index is larger than actual number of inputs in NodeDisplacementFilter";
    }
    m_SelectedInput = i;
  }

  mitk::NavigationData::Pointer mitk::NodeDisplacementFilter::GetRawDisplacementNavigationData(unsigned int i)
  {
    mitk::NavigationData::Pointer returnValue = mitk::NavigationData::New();
    if((m_Nodes.size()>i) && (m_Nodes.at(i).IsNotNull()))
      {
      try
        {
          returnValue = mitk::NavigationData::New(m_Nodes.at(i)->GetData()->GetGeometry()->GetIndexToWorldTransform());
        }
      catch (mitk::Exception& e)
        {
          returnValue->SetDataValid(false);
          MITK_WARN << "Excetion while returning navigation data: " << e.GetDescription();
        }
      }
    else
      {
        returnValue->SetDataValid(false);
        MITK_WARN << "Node Nr. " << i << " does not exist!";
      }

    return returnValue;
  }

  void mitk::NodeDisplacementFilter::GenerateData()
  {
    // copy the navigation data from the inputs to the outputs
    mitk::NavigationDataPassThroughFilter::GenerateData();

    // if no reference has been set yet, warn and abort
    if (m_SelectedInput == -1)
    {
      MITK_INFO("NodeDisplacementFilter")
        << "No input has been selected. Only forwarding NavigationData...";
      return;
    }

    // cancel, if selected tool is currently not being tracked
    if ( ! this->GetInput(m_SelectedInput)->IsDataValid() ) { return; }

    // outputs have been updated, now to transform the nodes
    // 1) Generate Pseudo-Geometry for Reference
    mitk::Geometry3D::Pointer refGeom = this->TransformToGeometry(
      this->GetInput(m_SelectedInput)->GetAffineTransform3D());

    // 2) For each node, calculate new position
    for (unsigned int index=0; index < m_Nodes.size(); index++)
    {
      mitk::Geometry3D::Pointer transformGeometry = refGeom->Clone();

      // create transformation to the reference position and from there to
      // the node position (node has fixed transformation from reference position)
      transformGeometry->Compose(m_Transforms.at(index), true);

      m_Nodes.at(index)->GetData()->SetGeometry(transformGeometry);
    }
  }

  void mitk::NodeDisplacementFilter::ResetNodes()
  {
    m_Nodes.clear();
    m_Transforms.clear();
  }

mitk::Geometry3D::Pointer mitk::NodeDisplacementFilter::TransformToGeometry(mitk::AffineTransform3D::Pointer transform){
  mitk::Geometry3D::Pointer g3d = mitk::Geometry3D::New();
  g3d->SetIndexToWorldTransform(transform);
  //g3d->TransferItkToVtkTransform(); // update VTK Transform for rendering too //Included in SetIndexToWorldTransform
  g3d->Modified();
  return g3d;
}
