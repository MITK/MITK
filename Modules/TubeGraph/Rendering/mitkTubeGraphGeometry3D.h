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

#ifndef _mitk_TubeGraphGeometry3D_h
#define _mitk_TubeGraphGeometry3D_h

#include "MitkTubeGraphExports.h"

#include "mitkGeometry3D.h"
#include "mitkCommon.h"

#include "mitkTubeGraph.h"

namespace mitk
{

/**
 * @brief Adapted version of class Geometry3D
 *
 * Adds the possibility to initialize a Geometry3D object with a tube graph. Additionally, the
 * default implementation of GetBoundingBox(...) is overriden, since it
 * can not be applied to tube graphs.
 * \note currently only one point in time is supported. That means, that
 * only one tree not a whole time series can be handled!
 * @ingroup Geometry
 */
class MitkTubeGraph_EXPORT TubeGraphGeometry3D : public Geometry3D
{
  public:

    mitkClassMacro( TubeGraphGeometry3D, Geometry3D );
    itkNewMacro( Self );

    /**
     * Initializes the Geometry3D object with the properties of the given
     * graph. That means: dimension is always 3, dimensions are set to the
     * maximal extent of the graph. Even spacing is assumed.
     * @param graph the graph whose properties should be used to initialize
     *             the Geometry3D object
     */
    virtual void Initialize( TubeGraph* graph );

    /**
     * Calculates the bounding box of the associated graph and returns it.
     * The current implementation ignores the parameter t, since a time
     * series of tube graphs is not yet supported.
     * @returns the bounding box of the associated tube graph
     */
    virtual const BoundingBox* GetBoundingBox() const;

  protected:

    TubeGraphGeometry3D();

    TubeGraph::Pointer m_TubeGraph;
};

} //namespace mitk

#endif
