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
  * \brief TODO
  *
  * \ingroup US
  */
  class MitkUSNavigation_EXPORT NodeDisplacementFilter : public NavigationDataToNavigationDataFilter
  {
  public:
    mitkClassMacro(NodeDisplacementFilter, NavigationDataToNavigationDataFilter);
    itkNewMacro(Self);

    virtual void AddNode(  mitk::DataNode::Pointer node );
    virtual int GetNumberOfNodes();
    virtual mitk::DataNode::Pointer GetNode (int i = 0);
    virtual std::vector< mitk::DataNode::Pointer > GetNodes();
    virtual void SelectInput(int i);

    /**Documentation
    * \brief Removes all added Nodes from the Filter but leaves all other configuration intact.
    */
    virtual void ResetNodes();


    /**Documentation
    * \brief Set the input of this filter
    */
    virtual void SetInput( const NavigationData* nd);

    /**Documentation
    * \brief Set input with id idx of this filter
    */
    virtual void SetInput( unsigned int idx, const NavigationData* nd);

    /**Documentation
    * \brief Get the input of this filter
    */
    const NavigationData* GetInput(void);

    /**Documentation
    * \brief Get the input with id idx of this filter
    */
    const NavigationData* GetInput(unsigned int idx);

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
