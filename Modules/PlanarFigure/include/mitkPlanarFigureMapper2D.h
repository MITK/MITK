/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarFigureMapper2D_h
#define mitkPlanarFigureMapper2D_h

#include "mitkCommon.h"
#include "mitkMapper.h"
#include "mitkPlanarFigure.h"
#include "mitkPlanarFigureControlPointStyleProperty.h"
#include <MitkPlanarFigureExports.h>
#include "vtkNew.h"
#include "vtkPen.h"

class vtkContext2D;

namespace mitk
{
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
  *     <li>"planarfigure.shadow.widthmodifier": the width of the shadow is defined by width of the "line" * this
  * modifier
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
  * @ingroup MitkPlanarFigureModule
  */

  class MITKPLANARFIGURE_EXPORT PlanarFigureMapper2D : public Mapper
  {
  public:
    mitkClassMacro(PlanarFigureMapper2D, Mapper);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /**
    * reimplemented from Baseclass
    */
      void MitkRender(mitk::BaseRenderer *renderer, mitk::VtkPropRenderer::RenderType type) override;

    static void SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer = nullptr, bool overwrite = false);

    /** \brief Apply color and opacity properties read from the PropertyList.
    * The actor is not used in the GLMappers. Called by mapper subclasses.
    */
    void ApplyColorAndOpacityProperties(mitk::BaseRenderer *renderer, vtkActor *actor = nullptr) override;

  protected:
    enum PlanarFigureDisplayMode
    {
      PF_DEFAULT = 0,
      PF_HOVER = 1,
      PF_SELECTED = 2,

      PF_COUNT = 3 // helper variable
    };

    PlanarFigureMapper2D();

    ~PlanarFigureMapper2D() override;

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
    void RenderLines(const PlanarFigureDisplayMode lineDisplayMode,
                     mitk::PlanarFigure *planarFigure,
                     mitk::Point2D &anchorPoint,
                     const mitk::PlaneGeometry *planarFigurePlaneGeometry,
                     const mitk::PlaneGeometry *rendererPlaneGeometry,
                     const mitk::BaseRenderer *renderer);

    /**
    * \brief Renders the quantities of the figure below the text annotations.
    */
    void RenderQuantities(const mitk::PlanarFigure *planarFigure,
                          mitk::BaseRenderer *renderer,
                          const mitk::Point2D anchorPoint,
                          double &annotationOffset,
                          float globalOpacity,
                          const PlanarFigureDisplayMode lineDisplayMode);

    /**
    * \brief Renders the text annotations.
    */
    void RenderAnnotations(mitk::BaseRenderer *renderer,
                           const std::string name,
                           const mitk::Point2D anchorPoint,
                           float globalOpacity,
                           const PlanarFigureDisplayMode lineDisplayMode,
                           double &annotationOffset);

    /**
    * \brief Renders the control-points.
    */
    void RenderControlPoints(const mitk::PlanarFigure *planarFigure,
                             const PlanarFigureDisplayMode lineDisplayMode,
                             const mitk::PlaneGeometry *planarFigurePlaneGeometry,
                             const mitk::PlaneGeometry *rendererPlaneGeometry,
                             mitk::BaseRenderer *renderer);

    void TransformObjectToDisplay(const mitk::Point2D &point2D,
                                  mitk::Point2D &displayPoint,
                                  const mitk::PlaneGeometry *objectGeometry,
                                  const mitk::PlaneGeometry *,
                                  const mitk::BaseRenderer *renderer);

    void DrawMarker(const mitk::Point2D &point,
                    float *lineColor,
                    float lineOpacity,
                    float *markerColor,
                    float markerOpacity,
                    float lineWidth,
                    PlanarFigureControlPointStyleProperty::Shape shape,
                    const mitk::PlaneGeometry *objectGeometry,
                    const mitk::PlaneGeometry *rendererGeometry,
                    const mitk::BaseRenderer *renderer);

    /**
    * \brief Actually paints the polyline defined by the figure.
    */
    void PaintPolyLine(const mitk::PlanarFigure::PolyLineType vertices,
                       bool closed,
                       Point2D &anchorPoint,
                       const PlaneGeometry *planarFigurePlaneGeometry,
                       const PlaneGeometry *rendererPlaneGeometry,
                       const mitk::BaseRenderer *renderer);

    /**
    * \brief Internally used by RenderLines() to draw the mainlines using
    * PaintPolyLine().
    */
    void DrawMainLines(mitk::PlanarFigure *figure,
                       Point2D &anchorPoint,
                       const PlaneGeometry *planarFigurePlaneGeometry,
                       const PlaneGeometry *rendererPlaneGeometry,
                       const mitk::BaseRenderer *renderer);

    /**
    * \brief Internally used by RenderLines() to draw the helperlines using
    * PaintPolyLine().
    */
    void DrawHelperLines(mitk::PlanarFigure *figure,
                         Point2D &anchorPoint,
                         const PlaneGeometry *planarFigurePlaneGeometry,
                         const PlaneGeometry *rendererPlaneGeometry,
                         const mitk::BaseRenderer *renderer);

    void InitializeDefaultPlanarFigureProperties();

    void InitializePlanarFigurePropertiesFromDataNode(const mitk::DataNode *node);

    void SetColorProperty(float property[3][3], PlanarFigureDisplayMode mode, float red, float green, float blue)
    {
      property[mode][0] = red;
      property[mode][1] = green;
      property[mode][2] = blue;
    }

    void SetFloatProperty(float *property, PlanarFigureDisplayMode mode, float value) { property[mode] = value; }
    /**
    * \brief Callback that sets m_NodeModified to true.
    *
    * This method set the bool flag m_NodeModified to true. It's a callback
    * that is executed when a itk::ModifiedEvet is invoked on our
    * DataNode.
    */
    void OnNodeModified();

    void Initialize(mitk::BaseRenderer *renderer);

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
    bool m_AnnotationsShadow;

    std::string m_AnnotationFontFamily;
    bool m_DrawAnnotationBold;
    bool m_DrawAnnotationItalic;
    int m_AnnotationSize;

    // the width of the shadow is defined as 'm_LineWidth * m_ShadowWidthFactor'
    float m_LineWidth;
    float m_ShadowWidthFactor;
    float m_OutlineWidth;
    float m_HelperlineWidth;
    // float m_PointWidth;

    float m_DevicePixelRatio;

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
    float m_AnnotationColor[3][3];

    // Bool flag that represents whether or not the DataNode has been modified.
    bool m_NodeModified;

    // Observer-tag for listening to itk::ModifiedEvents on the DataNode
    unsigned long m_NodeModifiedObserverTag;

    // Bool flag that indicates if a node modified observer was added
    bool m_NodeModifiedObserverAdded;

    bool m_Initialized;

    vtkNew<vtkContext2D> m_Context;
    vtkSmartPointer<vtkPen> m_Pen;
  };

} // namespace mitk

#endif
