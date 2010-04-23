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

namespace mitk {

class BaseRenderer;
class Contour;


/** 
 * \brief OpenGL-based mapper to render display sub-class instances of
 * mitk::PlanarFigure
 * 
 * Properties that are evaluated during rendering:
 * <ul>
 * <li> "color": ColorProperty for color of the lines that represent the planar figure (e.g. a circle)
 * <li> "opacity": FloatProperty for opacity of the lines that represent the planar figure (e.g. a circle)
 * <li> "width": FloatProperty for widthof the lines that represent the planar figure (e.g. a circle)
 * <li> "draw outline": BoolProperty. If set, the mapper will draw a second, broader line behind the main lines. This may look like an outline
 * <li> "outline color": ColorProperty for the color of the outline
 * <li> "outline opacity": FloatProperty for the opacity of the outline
 * <li> "outline width": FloatProperty for the width of the outline
 * <li> "control point color": ColorProperty for the color of control points
 * <li> "control point opacity": FloatProperty for the opacity of control points
 * <li> "control point width": FloatProperty for the width of the lines used for drawing control points
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
