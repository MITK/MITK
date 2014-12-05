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

#ifndef MITKGEOMETRY2DDATAMAPPER2D_H_HEADER_INCLUDED_C19C0BFB
#define MITKGEOMETRY2DDATAMAPPER2D_H_HEADER_INCLUDED_C19C0BFB

#include <MitkCoreExports.h>
#include "mitkGLMapper.h"
#include "mitkSurfaceGLMapper2D.h"
#include "mitkDataStorage.h"
#include "mitkDataNode.h"
#include "mitkWeakPointer.h"
#include "mitkLine.h"

namespace mitk {
  class BaseRenderer;
  class PlaneGeometryDataMapper2D;
  /** \deprecatedSince{2014_10} This class is deprecated. Please use PlaneGeometryDataMapper2D instead. */
  DEPRECATED( typedef PlaneGeometryDataMapper2D Geometry2DDataMapper2D);

  /**
  * \brief OpenGL-based mapper to display a PlaneGeometry in a 2D window
  *
  * Currently implemented for mapping on PlaneGeometry.
  * The result is normally a line. An important usage of this class is to show
  * the orientation of the slices displayed in other 2D windows.
  *
  *
  * Properties that can be set and influence the PlaneGeometryDataMapper2D are:
  *
  *   - \b "PlaneOrientationProperty": (PlaneOrientationProperty)

  * \todo implement for AbstractTransformGeometry.
  * \ingroup Mapper
  */
  class MITK_CORE_EXPORT PlaneGeometryDataMapper2D : public GLMapper
  {
  public:
    mitkClassMacro(PlaneGeometryDataMapper2D, GLMapper);

    itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      /**
      * \brief Get the PlaneGeometryData to map
      */
      const PlaneGeometryData *GetInput();

    virtual void Paint( BaseRenderer *renderer );

    virtual void SetDatastorageAndGeometryBaseNode(mitk::DataStorage::Pointer ds, mitk::DataNode::Pointer parent);

    /** Applies properties specific to this mapper */
    virtual void ApplyAllProperties( BaseRenderer *renderer );

    LocalStorageHandler<BaseLocalStorage> m_LSH;

  protected:
    PlaneGeometryDataMapper2D();

    virtual ~PlaneGeometryDataMapper2D();

    virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer);
    /**
    * \brief Returns the thick slice mode for the given datanode.
    *
    * This method returns the value of the 'reslice.thickslices' property for
    * the given datanode.
    *   '0': thick slice mode disabled
    *   '1': thick slice mode enabled
    *
    * The variable 'thickSlicesNum' contains the value of the 'reslice.thickslices.num'
    * property that defines how many slices are shown at once.
    */
    int DetermineThickSliceMode( DataNode * dn, int &thickSlicesNum );

    /**
    * \brief Determines the cross position of two lines and stores them as parametric coordinates
    *
    * This method determines the parametric position at which a line 'otherLine' crosses another line
    * 'mainLine'. The result is stored in 'crossPositions'.
    */
    void DetermineParametricCrossPositions( Line<ScalarType,2> &mainLine, Line<ScalarType,2> &otherLine, std::vector<ScalarType> &crossPositions );

    void DrawLine( BaseRenderer * renderer, ScalarType lengthInDisplayUnits,
      Line< ScalarType, 2 > &line, std::vector< ScalarType > &gapPositions,
      const PlaneGeometry * inputPlaneGeometry, bool drawDashed,
      ScalarType gapSizeInPixel
      );

    void DrawOrientationArrow( Point2D &outerPoint, Point2D &innerPoint,
      const PlaneGeometry *planeGeometry,
      const PlaneGeometry *rendererPlaneGeometry,
      const DisplayGeometry *displayGeometry,
      bool positiveOrientation = true );

    SurfaceGLMapper2D::Pointer m_SurfaceMapper;

    mitk::WeakPointer<mitk::DataStorage> m_DataStorage;  ///< DataStorage that will be searched for sub nodes
    DataNode::Pointer m_ParentNode;  ///< parent node that will be used to search for sub nodes

    typedef std::vector<DataNode*> NodesVectorType;
    NodesVectorType m_OtherPlaneGeometries;

    bool m_RenderOrientationArrows;
    bool m_ArrowOrientationPositive;
  };
} // namespace mitk
#endif /* MITKGEOMETRY2DDATAMAPPER2D_H_HEADER_INCLUDED_C19C0BFB */
