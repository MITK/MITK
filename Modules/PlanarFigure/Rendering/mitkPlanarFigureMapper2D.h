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
#include "PlanarFigureExports.h"
#include "mitkGLMapper2D.h"
#include "mitkPlanarFigure.h"
#include "mitkPlanarFigureControlPointStyleProperty.h"

namespace mitk {

class BaseRenderer;
class Contour;


/**
* \brief OpenGL-based mapper to render display sub-class instances of mitk::PlanarFigure
* 
* The appearance of planar figures can be configured through  properties. If no properties are specified,
* default values will be used. There are four elements a planar figure consists of:
* 
* <ol>
* <li>"line": the main line segments of the planar figure (note: text is drawn in the same style)
* <li>"helperline": additional line segments of planar figures, such as arrow tips, arches of angles, etc.
* <li>"outline": background which is drawn behind the lines and helperlines of the planar figure (optional)
* <li>"marker": the markers (control points) of a planar figure
* <li>"markerline": the lines by which markers (control points) are surrounded
* </ol>
* 
* In the following, all appearance-related planar figure properties are listed:
* 
* <ol>
* <li>General properties for the planar figure
*   <ul>
*     <li>"planarfigure.drawoutline": if true, the "outline" lines is drawn
*     <li>"planarfigure.drawquantities": if true, the quantities (text) associated with the planar figure is drawn
*     <li>"planarfigure.drawshadow": if true, a black shadow is drawn around the planar figure
*     <li>"planarfigure.controlpointshape": style of the control points (enum)
*   </ul>
* <li>Line widths of planar figure elements
*   <ul>
*     <li>"planarfigure.line.width": width of "line" segments (float value, in mm)
*     <li>"planarfigure.shadow.widthmodifier": the width of the shadow is defined by width of the "line" * this modifier
*     <li>"planarfigure.outline.width": width of "outline" segments (float value, in mm)
*     <li>"planarfigure.helperline.width": width of "helperline" segments (float value, in mm)
*   </ul>
* <li>Color/opacity of planar figure elements in normal mode (unselected)
*   <ul>
*     <li>"planarfigure.default.line.color"
*     <li>"planarfigure.default.line.opacity"
*     <li>"planarfigure.default.outline.color"
*     <li>"planarfigure.default.outline.opacity"
*     <li>"planarfigure.default.helperline.color"
*     <li>"planarfigure.default.helperline.opacity"
*     <li>"planarfigure.default.markerline.color"
*     <li>"planarfigure.default.markerline.opacity"
*     <li>"planarfigure.default.marker.color"
*     <li>"planarfigure.default.marker.opacity"
*   </ul>
* <li>Color/opacity of planar figure elements in hover mode (mouse-over)
*   <ul>
*     <li>"planarfigure.hover.line.color"
*     <li>"planarfigure.hover.line.opacity"
*     <li>"planarfigure.hover.outline.color"
*     <li>"planarfigure.hover.outline.opacity"
*     <li>"planarfigure.hover.helperline.color"
*     <li>"planarfigure.hover.helperline.opacity"
*     <li>"planarfigure.hover.markerline.color"
*     <li>"planarfigure.hover.markerline.opacity"
*     <li>"planarfigure.hover.marker.color"
*     <li>"planarfigure.hover.marker.opacity"
*   </ul>
* <li>Color/opacity of planar figure elements in selected mode
*   <ul>
*     <li>"planarfigure.selected.line.color"
*     <li>"planarfigure.selected.line.opacity"
*     <li>"planarfigure.selected.outline.color"
*     <li>"planarfigure.selected.outline.opacity"
*     <li>"planarfigure.selected.helperline.color"
*     <li>"planarfigure.selected.helperline.opacity"
*     <li>"planarfigure.selected.markerline.color;"
*     <li>"planarfigure.selected.markerline.opacity"
*     <li>"planarfigure.selected.marker.color"
*     <li>"planarfigure.selected.marker.opacity"
*   </ul>
* </ol>
*
* \ingroup Mapper
*/

class PlanarFigure_EXPORT PlanarFigureMapper2D : public GLMapper2D
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
  
  enum PlanarFigureDisplayMode
  {
    PF_DEFAULT = 0,
    PF_HOVER,
    PF_SELECTED
  };

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
    float* lineColor,
    float lineOpacity,
    float* markerColor,
    float markerOpacity,
    float lineWidth,
    PlanarFigureControlPointStyleProperty::Shape shape,
    const mitk::Geometry2D *objectGeometry,
    const mitk::Geometry2D *rendererGeometry,
    const mitk::DisplayGeometry *displayGeometry );

  void PaintPolyLine( mitk::PlanarFigure::PolyLineType vertices,
    bool closed,
    float* color, 
    float opacity, 
    float lineWidth, 
    Point2D& firstPoint,
    const Geometry2D* planarFigureGeometry2D, 
    const Geometry2D* rendererGeometry2D, 
    const DisplayGeometry* displayGeometry);

  void DrawMainLines( mitk::PlanarFigure* figure, 
    float* color, 
    float opacity, 
    bool drawShadow,
    float lineWidth, 
    float shadowWidthFactor,
    Point2D& firstPoint,
    const Geometry2D* planarFigureGeometry2D, 
    const Geometry2D* rendererGeometry2D, 
    const DisplayGeometry* displayGeometry) ;

  void DrawHelperLines( mitk::PlanarFigure* figure,
    float* color, 
    float opacity, 
    bool drawShadow,
    float lineWidth, 
    float shadowWidthFactor,
    Point2D& firstPoint,
    const Geometry2D* planarFigureGeometry2D, 
    const Geometry2D* rendererGeometry2D, 
    const DisplayGeometry* displayGeometry) ;

  void InitializeDefaultPlanarFigureProperties();

  void InitializePlanarFigurePropertiesFromDataNode( const mitk::DataNode* node );

  void SetColorProperty( float property[3][3], PlanarFigureDisplayMode mode, float red, float green, float blue )
  {
    property[mode][0] = red;
    property[mode][1] = green;
    property[mode][2] = blue;
  }

  void SetFloatProperty( float* property, PlanarFigureDisplayMode mode, float value )
  {
    property[mode] = value;
  }



private:
  bool m_IsSelected;
  bool m_IsHovering;
  bool m_DrawOutline;
  bool m_DrawQuantities;
  bool m_DrawShadow;
  bool m_DrawControlPoints;

  // the width of the shadow is defined as 'm_LineWidth * m_ShadowWidthFactor'
  float m_LineWidth;
  float m_ShadowWidthFactor;
  float m_OutlineWidth;
  float m_HelperlineWidth;
  float m_PointWidth;

  PlanarFigureControlPointStyleProperty::Shape m_ControlPointShape;

  float m_LineColor[3][3];
  float m_LineOpacity[3];
  float m_OutlineColor[3][3];
  float m_OutlineOpacity[3];
  float m_HelperlineColor[3][3];
  float m_HelperlineOpacity[3];
  float m_MarkerlineColor[3][3];
  float m_MarkerlineOpacity[3];
  float m_MarkerColor[3][3];
  float m_MarkerOpacity[3];

};

} // namespace mitk



#endif /* MITK_PLANAR_FIGURE_MAPPER_2D_H_ */
