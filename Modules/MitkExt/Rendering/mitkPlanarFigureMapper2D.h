/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-13 18:06:46 +0200 (Mi, 13 Mai 2009) $
Version:   $Revision: 17258 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITK_PLANAR_FIGURE_MAPPER_2D_H_
#define MITK_PLANAR_FIGURE_MAPPER_2D_H_

#include "mitkCommon.h"
#include "MitkExtExports.h"
#include "mitkGLMapper2D.h"
#include "mitkPlanarFigure.h"
#include "mitkPlanarFigureControlPointStyleProperty.h"

namespace mitk {

class BaseRenderer;
class Contour;


/** 
 * \brief OpenGL-based mapper to render display sub-class instances of
 * mitk::PlanarFigure
 * 
 * A couple of properties are evaluated to determine 
 * <ol>
 *   <li> color (ColorProperty)
 *   <li> opacity  (FloatProperty)
 *   <li> line width (FloatProperty)
 * </ol>
 *   of several rendering elements:
 * <ul>
 *   <li> Main lines that constitute the planar figure (e.g. a circle)
 *   <ul>
 *     <li> "color"
 *     <li> "opacity"
 *     <li> "width"
 *   </ul>
 *   <li> An outline for the main lines. This is a second line behind the main lines. By default, this line is broader and looks like an outline
 *   <ul>
 *     <li> "draw outline": BoolProperty to control whether the outline is drawn.
 *     <li> "outline color"
 *     <li> "outline opacity"
 *     <li> "outline width"
 *   </ul>
 *   <li> Helper elements, such as arrow heads or the arcs that signal a measured angle
 *   <ul>
 *     <li> "helper color"
 *     <li> "helper opacity"
 *     <li> "helper width"
 *   </ul>
 *   <li> Control points (handles) that are used to edit the planar figure
 *   <ul>
 *     <li> "control point color"
 *     <li> "control point opacity"
 *     <li> "control point width"
 *   </ul>
 * </ul>
 * 
 * \ingroup Mapper
 */
class MitkExt_EXPORT PlanarFigureMapper2D : public GLMapper2D
{
public:
    
  mitkClassMacro(PlanarFigureMapper2D, Mapper2D);

  itkNewMacro(Self);

  /**
  * reimplemented from Baseclass
  */
  virtual void Paint(BaseRenderer * renderer);
  static void SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer = NULL, bool overwrite = false);
protected:
  
  typedef PlanarFigure::VertexContainerType VertexContainerType;

  PlanarFigureMapper2D();

  virtual ~PlanarFigureMapper2D();

  void TransformObjectToDisplay(
    const mitk::Point2D &point2D,
    mitk::Point2D &displayPoint,
    const mitk::Geometry2D *objectGeometry,
    const mitk::Geometry2D *rendererGeometry,
    const mitk::DisplayGeometry *displayGeometry );

  void DrawMarker(
    const mitk::Point2D &point,
    bool selected,
    float* color,
    float opacity,
    float width,
    PlanarFigureControlPointStyleProperty::Shape shape,
    const mitk::Geometry2D *objectGeometry,
    const mitk::Geometry2D *rendererGeometry,
    const mitk::DisplayGeometry *displayGeometry );

  void PaintPolyLine(
    const VertexContainerType* vertices, 
    bool closed,
    float* color, 
    float opacity, 
    float lineWidth, 
    Point2D& firstPoint,
    const Geometry2D* planarFigureGeometry2D, 
    const Geometry2D* rendererGeometry2D, 
    const DisplayGeometry* displayGeometry);
};

} // namespace mitk



#endif /* MITK_PLANAR_FIGURE_MAPPER_2D_H_ */
