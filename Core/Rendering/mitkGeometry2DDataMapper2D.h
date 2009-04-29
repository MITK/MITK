/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKGEOMETRY2DDATAMAPPER2D_H_HEADER_INCLUDED_C19C0BFB
#define MITKGEOMETRY2DDATAMAPPER2D_H_HEADER_INCLUDED_C19C0BFB

#include "mitkCommon.h"
#include "mitkGLMapper2D.h"
#include "mitkSurfaceMapper2D.h"
#include "mitkDataStorage.h"
#include "mitkDataTreeNode.h"

namespace mitk {

class BaseRenderer;

/**
 * \brief OpenGL-based mapper to display a Geometry2D in a 2D window
 *
 * Currently implemented for mapping on PlaneGeometry. 
 * The result is normally a line. An important usage of this class is to show
 * the orientation of the slices displayed in other 2D windows.
 *
 * \todo implement for AbstractTransformGeometry.
 * \ingroup Mapper
 */
class MITK_CORE_EXPORT Geometry2DDataMapper2D : public GLMapper2D
{

public:
  mitkClassMacro(Geometry2DDataMapper2D, GLMapper2D);

  itkNewMacro(Self);

  /**
    * \brief Get the Geometry2DData to map
    */
  const Geometry2DData *GetInput();

  virtual void Paint( BaseRenderer *renderer );

  /**
    * \brief Leave a little gap when crossing other nodes containing 
    * Geometry2DData accessible by traversing the iterator
    *
    * \note works currently for PlaneGeometry only
    */
  virtual void SetDataIteratorToOtherGeometry2Ds(const DataTreeIteratorBase *iterator);

  virtual void SetDatastorageAndGeometryBaseNode(mitk::DataStorage::Pointer ds, mitk::DataTreeNode::Pointer parent);

  /** Applies properties specific to this mapper */
  virtual void ApplyProperties( BaseRenderer *renderer );


protected:
  Geometry2DDataMapper2D();

  virtual ~Geometry2DDataMapper2D();

  virtual void GenerateData();

  void DrawOrientationArrow( Point2D &outerPoint, Point2D &innerPoint, 
    const PlaneGeometry *planeGeometry, 
    const PlaneGeometry *rendererPlaneGeometry, 
    const DisplayGeometry *displayGeometry,
    bool positiveOrientation = true );

  SurfaceMapper2D::Pointer m_SurfaceMapper;

  /**
    * \brief Leave a little gap when crossing other nodes containing 
    * Geometry2DData accessible by traversing this iterator (if set)
    *
    * \note works currently for PlaneGeometry only
    */
  DataTreeIteratorClone m_IteratorToOtherGeometry2Ds;
  DataStorage::Pointer m_DataStorage;
  DataTreeNode::Pointer m_ParentNode;

  typedef std::vector<DataTreeNode*> NodesVectorType;
  NodesVectorType m_OtherGeometry2Ds;

  bool m_RenderOrientationArrows;
  bool m_ArrowOrientationPositive;
};

} // namespace mitk

#endif /* MITKGEOMETRY2DDATAMAPPER2D_H_HEADER_INCLUDED_C19C0BFB */
