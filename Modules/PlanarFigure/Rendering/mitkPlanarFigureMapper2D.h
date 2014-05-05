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


#ifndef MITK_PLANAR_FIGURE_MAPPER_2D_H_
#define MITK_PLANAR_FIGURE_MAPPER_2D_H_

#include "mitkCommon.h"
#include <MitkPlanarFigureExports.h>
#include "mitkGLMapper.h"
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
*     <li>"planarfigure.drawname": if true, the name specified by the dataNode is drawn
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

class MitkPlanarFigure_EXPORT PlanarFigureMapper2D : public GLMapper
{
public:

  mitkClassMacro(PlanarFigureMapper2D, GLMapper);

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /**
  * reimplemented from Baseclass
  */
  virtual void Paint(BaseRenderer * renderer);


  static void SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer = NULL, bool overwrite = false);


protected:

  enum PlanarFigureDisplayMode
  {
    PF_DEFAULT = 0,
    PF_HOVER = 1,
    PF_SELECTED = 2,

    PF_COUNT = 3 //helper variable
  };

  PlanarFigureMapper2D();

  virtual ~PlanarFigureMapper2D();

  /**
  * \brief Renders all the lines defined by the PlanarFigure.
  *
  * This method renders all the lines that are defined by the PlanarFigure.
  * That includes the mainlines and helperlines as well as their shadows
  * and the outlines.
  *
  * This method already takes responsibility for the setting of the relevant
  * openGL attributes to reduce unnecessary setting of these attributes.
  * (e.g. no need to set color twice if it's the same)
  */
  void RenderLines( PlanarFigureDisplayMode lineDisplayMode,
                    mitk::PlanarFigure * planarFigure,
                    mitk::Point2D &anchorPoint,
                    mitk::PlaneGeometry * planarFigurePlaneGeometry,
                    const mitk::PlaneGeometry * rendererPlaneGeometry,
                    mitk::DisplayGeometry * displayGeometry );

  /**
  * \brief Renders the quantities of the figure below the text annotations.
  */
  void RenderQuantities( mitk::PlanarFigure * planarFigure,
                         mitk::BaseRenderer * renderer,
                         mitk::Point2D anchorPoint,
                         double &annotationOffset,
                         float globalOpacity,
                         PlanarFigureDisplayMode lineDisplayMode );

  /**
  * \brief Renders the text annotations.
  */
  void RenderAnnotations( mitk::BaseRenderer * renderer,
                          std::string name,
                          mitk::Point2D anchorPoint,
                          float globalOpacity,
                          PlanarFigureDisplayMode lineDisplayMode,
                          double &annotationOffset );

  /**
  * \brief Renders the control-points.
  */
  void RenderControlPoints( mitk::PlanarFigure * planarFigure,
                            PlanarFigureDisplayMode lineDisplayMode,
                            mitk::PlaneGeometry * planarFigurePlaneGeometry,
                            const mitk::PlaneGeometry * rendererPlaneGeometry,
                            mitk::DisplayGeometry * displayGeometry );


  void TransformObjectToDisplay(
    const mitk::Point2D &point2D,
    mitk::Point2D &displayPoint,
    const mitk::PlaneGeometry *objectGeometry,
    const mitk::PlaneGeometry *rendererGeometry,
    const mitk::DisplayGeometry *displayGeometry );

  void DrawMarker(
    const mitk::Point2D &point,
    float* lineColor,
    float lineOpacity,
    float* markerColor,
    float markerOpacity,
    float lineWidth,
    PlanarFigureControlPointStyleProperty::Shape shape,
    const mitk::PlaneGeometry *objectGeometry,
    const mitk::PlaneGeometry *rendererGeometry,
    const mitk::DisplayGeometry *displayGeometry );

  /**
  * \brief Actually paints the polyline defined by the figure.
  */
  void PaintPolyLine( mitk::PlanarFigure::PolyLineType vertices,
    bool closed,
    Point2D& anchorPoint,
    const PlaneGeometry* planarFigurePlaneGeometry,
    const PlaneGeometry* rendererPlaneGeometry,
    const DisplayGeometry* displayGeometry);

  /**
  * \brief Internally used by RenderLines() to draw the mainlines using
  * PaintPolyLine().
  */
  void DrawMainLines( mitk::PlanarFigure* figure,
    Point2D& anchorPoint,
    const PlaneGeometry* planarFigurePlaneGeometry,
    const PlaneGeometry* rendererPlaneGeometry,
    const DisplayGeometry* displayGeometry) ;

  /**
  * \brief Internally used by RenderLines() to draw the helperlines using
  * PaintPolyLine().
  */
  void DrawHelperLines( mitk::PlanarFigure* figure,
    Point2D& anchorPoint,
    const PlaneGeometry* planarFigurePlaneGeometry,
    const PlaneGeometry* rendererPlaneGeometry,
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

  /**
  * \brief Callback that sets m_NodeModified to true.
  *
  * This method set the bool flag m_NodeModified to true. It's a callback
  * that is executed when a itk::ModifiedEvet is invoked on our
  * DataNode.
  */
  void OnNodeModified();

private:
  bool m_IsSelected;
  bool m_IsHovering;
  bool m_DrawOutline;
  bool m_DrawQuantities;
  bool m_DrawShadow;
  bool m_DrawControlPoints;
  bool m_DrawName;
  bool m_DrawDashed;
  bool m_DrawHelperDashed;

  // the width of the shadow is defined as 'm_LineWidth * m_ShadowWidthFactor'
  float m_LineWidth;
  float m_ShadowWidthFactor;
  float m_OutlineWidth;
  float m_HelperlineWidth;
  //float m_PointWidth;

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

  // Bool flag that represents whether or not the DataNode has been modified.
  bool m_NodeModified;

  // Observer-tag for listening to itk::ModifiedEvents on the DataNode
  unsigned long m_NodeModifiedObserverTag;

  // Bool flag that indicates if a node modified observer was added
  bool m_NodeModifiedObserverAdded;
};

} // namespace mitk



#endif /* MITK_PLANAR_FIGURE_MAPPER_2D_H_ */
