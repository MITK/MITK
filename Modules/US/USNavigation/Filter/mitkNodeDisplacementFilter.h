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

#ifndef NODEDISPLACEMENTFILTER_H_INCLUDED
#define NODEDISPLACEMENTFILTER_H_INCLUDED

#include "MitkUSNavigationExports.h"

#include <itkProcessObject.h>

#include "mitkBaseRenderer.h"
#include "mitkCommon.h"
#include "mitkNavigationData.h"
#include "mitkNavigationDataToNavigationDataFilter.h"

//MITK

namespace mitk {
  /**Documentation
  * \brief This filter moves DataNodes relatively to tracking Data from a 6-DoF Sensor.
  *
  * This behaviour can for example be used for rigid tracking of risk structures relative to a skin marker.
  * To use it, connect the Filter and select the input that delivers tracking data from the reference marker
  * via SelectInput().
  * Make sure tracking is started before proceeding any further: The filter requires tracking data from the
  * sensor to calculate the relative position of the added node.
  *
  * One can then add Nodes to the filter via AddNode(). Make sure that the node has a geometry and position set
  * in the tracking coordinate system of the reference input. The Filter will then calculate the offset between
  * Node and reference marker and continously update the node position accordign to the tracking data.
  *
  * \ingroup US
  */
  class MitkUSNavigation_EXPORT NodeDisplacementFilter : public NavigationDataToNavigationDataFilter
  {
  public:
    mitkClassMacro(NodeDisplacementFilter, NavigationDataToNavigationDataFilter);
    itkNewMacro(Self);

    /**
    * \brief Adds a note  to the filter, the position of which will then be continously update relatively to the selected input stream.
    *
    * The node should have a geometry and position set in the coordinate system of the selected input stream
    */
    virtual void AddNode(  mitk::DataNode::Pointer node );

    /**
    * \brief Returns the number of nodes that were added to this filter.
    */
    virtual int GetNumberOfNodes();

    /**
    * \brief Returns the nth node that was added to this filter.
    */
    virtual mitk::DataNode::Pointer GetNode (unsigned int i = 0);

    /**
    * \brief Returns a vector containing all nodes that have been added to this filter.
    *
    * Indexes in this vector correspond to indexes in the vector provided by GetOffsets().
    */
    virtual std::vector< mitk::DataNode::Pointer > GetNodes();

    /**
    * \brief Selects an input stream as the reference stream.
    *
    * Position and orientation of all Nodes will be Updated according to information from the selected stream.
    * Make sure to select the input before adding nodes. The input should deliver 6DoF Data. Behaviour is undefined
    * for 5-Dof Data. The selected input can be changed during intervention if both old and new reference input
    * Lie in the same coordinate system. Be aware however that the offsets will not be recalculated, just moved
    * to the new stream.
    */
    virtual void SelectInput(int i);

    /**Documentation
    * \brief Removes all added Nodes from the Filter but leaves all other configuration intact.
    */
    virtual void ResetNodes();

  protected:
    NodeDisplacementFilter();
    virtual ~NodeDisplacementFilter();

    virtual void GenerateData();

    /**
    * \brief Creates an Affine Transformation from a Navigation Data Object.
    */
    mitk::AffineTransform3D::Pointer NavigationDataToTransform(const mitk::NavigationData * nd);
    /**
    * \brief Creates an Geometry 3D Object from an AffineTransformation.
    */
    mitk::Geometry3D::Pointer TransformToGeometry(mitk::AffineTransform3D::Pointer transform);
    /**
    * \brief All Nodes that are being managed by this Filter.
    */
    std::vector<mitk::DataNode::Pointer> m_Nodes;
    /**
    * \brief The Offset that each node has from the selected Navigation Tool.
    *
    * The indexes correspond to indexes in the Node vector
    */
    std::vector<mitk::Point3D> m_Offsets;

    /**
    * \brief The Input that is used as a reference to orient the managed nodes.
    */
    int m_SelectedInput;
  };
} // namespace mitk

#endif