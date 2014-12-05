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


#ifndef MITKSURFACEDATAMAPPER2D_H_HEADER_INCLUDED_C10EB2E8
#define MITKSURFACEDATAMAPPER2D_H_HEADER_INCLUDED_C10EB2E8

#include <MitkCoreExports.h>
#include "mitkGLMapper.h"
#include "mitkSurface.h"

class vtkCutter;
class vtkPlane;
class vtkLookupTable;
class vtkLinearTransform;
class vtkPKdTree;
class vtkStripper;

namespace mitk {

class BaseRenderer;
class PlaneGeometry;
class DisplayGeometry;

/**
 * @brief OpenGL-based mapper to display a Surface in a 2D window.
 *
 * Displays a 2D cut through a Surface object (vtkPolyData). This
 * is basically done in two steps:
 *
 * 1. Cut a slice out of a (input) vtkPolyData object. The slice may be a flat plane (PlaneGeometry)
 *    or a curved plane (ThinPlateSplineCurvedGeometry). The actual cutting is done by a vtkCutter.
 *    The result of cutting is a (3D) vtkPolyData object, which contains only points and lines
 *    describing the cut.
 *
 * 2. Paint the cut out slice by means of OpenGL. To do this, all lines of the cut object are traversed.
 *    For each line segment, both end points are transformed from 3D into the 2D system of the associated
 *    renderer and then drawn by OpenGL.
 *
 * There is a mode to display normals of the input surface object (see properties below). If this mode
 * is on, then the drawing of the 2D cut is slightly more complicated. For each line segment of the cut,
 * we take the end point (p2d) of this line and search the input vtkPolyData object for the closest point to p2d (p3D-input).
 * We then read out the surface normal for p3D-input. We map this normal into our 2D coordinate system and
 * then draw a line from p2d to (p2d+mapped normal). This drawing of surface normals will only work if the
 * input vtkPolyData actually HAS normals. If you have a vtkPolyData without normals, use the vtkPolyDataNormals
 * filter to generate normals.
 *
 * Properties that influence rendering are:
 *
 *   - \b "color": (ColorProperty) Color of surface object
 *   - \b "line width": (IntProperty) Width in pixels of the lines drawn.
 *   - \b "scalar visibility": (BoolProperty) Whether point/cell data values (from vtkPolyData) should be used to influence colors
 *   - \b "scalar mode": (BoolProperty) If "scalar visibility" is on, whether to use point data or cell data for coloring.
 *   - \b "LookupTable": (LookupTableProperty) A lookup table to translate point/cell data values (from vtkPolyData) to colors
 *   - \b "ScalarsRangeMinimum": (FloatProperty) Range of the lookup table
 *   - \b "ScalarsRangeMaximum": (FloatProperty) Range of the lookup table
 *   - \b "draw normals 2D": (BoolProperty) If true, normals are drawn (if present in vtkPolyData)
 *   - \b "invert normals": (BoolProperty) Inverts front/back for display.
 *   - \b "front color": (ColorProperty) Color for normals display on front side of the plane
 *   - \b "front normal length (px)": (FloatProperty) Length of the front side normals in pixels.
 *   - \b "back color": (ColorProperty) Color for normals display on back side of the plane
 *   - \b "back normal length (px)": (FloatProperty) Length of the back side normals in pixels.
 *
 */
class MITK_CORE_EXPORT SurfaceGLMapper2D : public GLMapper
{
public:
  mitkClassMacro(SurfaceGLMapper2D, GLMapper);

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  const Surface* GetInput(void);

  virtual void Paint(BaseRenderer* renderer);

  /**
   * @brief The Surface to map can be explicitly set by this method.
   *
   * If it is set, it is used instead of the data stored in the DataNode.
   * This enables to use the mapper also internally from other mappers.
   */
  itkSetConstObjectMacro(Surface, Surface);

  /**
   * @brief Get the Surface set explicitly.
   *
   * @return NULL is returned if no Surface is set to be used instead of DataNode::GetData().
   * @sa SetSurface
   */
  itkGetConstObjectMacro(Surface, Surface);

  /**
   *\brief Overwritten to initialize lookup table for point scalar data
   */
  void SetDataNode( DataNode* node );

  /**
   * \brief Generate OpenGL primitives for the VTK contour held in contour.
   */
  void PaintCells(BaseRenderer* renderer, vtkPolyData* contour,
                  const PlaneGeometry* worldGeometry,
                  const DisplayGeometry* displayGeometry,
                  vtkLinearTransform* vtktransform,
                  vtkLookupTable* lut = NULL,
                  vtkPolyData* original3DObject = NULL);

  static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = NULL, bool overwrite = false);

  virtual void ApplyAllProperties(BaseRenderer* renderer);

protected:

  SurfaceGLMapper2D();

  virtual ~SurfaceGLMapper2D();

  vtkPlane*  m_Plane;
  vtkCutter* m_Cutter;

  Surface::ConstPointer m_Surface;

  vtkLookupTable* m_LUT;

  int m_LineWidth;

  vtkPKdTree* m_PointLocator;

  vtkStripper* m_Stripper;

  bool m_DrawNormals;

  float m_FrontSideColor[4];
  float m_BackSideColor[4];
  float m_LineColor[4];
  float m_FrontNormalLengthInPixels;
  float m_BackNormalLengthInPixels;
};

} // namespace mitk

#endif /* MITKSURFACEDATAMAPPER2D_H_HEADER_INCLUDED_C10EB2E8 */

