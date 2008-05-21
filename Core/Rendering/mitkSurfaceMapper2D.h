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


#ifndef MITKSURFACEDATAMAPPER2D_H_HEADER_INCLUDED_C10EB2E8
#define MITKSURFACEDATAMAPPER2D_H_HEADER_INCLUDED_C10EB2E8

#include "mitkCommon.h"
#include "mitkGLMapper2D.h"
#include "mitkSurface.h"

class vtkCutter;
class vtkPlane;
class vtkLookupTable;
class vtkLinearTransform;
class vtkPKdTree;
class vtkStripper;

namespace mitk {

class BaseRenderer;
class Geometry2D;
class DisplayGeometry;

/**
 * @brief OpenGL-based mapper to display a Geometry2D in a 2D window 
 * OpenGL-based mapper to display a Geometry2D in a 2D window. 
 *
 * The result is normally a line. An important usage of this class 
 * is to show the orientation of the slices displayed in other 2D windows.
 * In not-so-common cases the result of mapping is NOT a line but an arbitrary cut
 * between a 2D geometry (e.g. a curved planed like mitk::ThinPlateSplineCurvedGeometry)
 * and another curved plane.
 */
class MITK_CORE_EXPORT SurfaceMapper2D : public GLMapper2D
{
public:
  mitkClassMacro(SurfaceMapper2D, GLMapper2D);

  itkNewMacro(Self);

  const Surface* GetInput(void);

  virtual void Paint(BaseRenderer* renderer);

  /**
   * @brief The Surface to map can be explicitly set by this method. 
   *
   * If it is set, it is used instead of the data stored in the DataTreeNode.
   * This enables to use the mapper also internally from other mappers.
   */
  itkSetConstObjectMacro(Surface, Surface);

  /**
   * @brief Get the Surface set explicitly.
   *
   * @return NULL is returned if no Surface is set to be used instead of DataTreeNode::GetData().
   * @sa SetSurface
   */
  itkGetConstObjectMacro(Surface, Surface);

  /**
   *\brief Overwritten to initialize lookup table for point scalar data
   */
  void SetDataTreeNode( DataTreeNode::Pointer node );

  /**
   * \brief Generate OpenGL primitives for the VTK contour held in contour.
   */
  void PaintCells(BaseRenderer* renderer, vtkPolyData* contour, 
                  const Geometry2D* worldGeometry, 
                  const DisplayGeometry* displayGeometry, 
                  vtkLinearTransform* vtktransform, 
                  vtkLookupTable* lut = NULL,
                  vtkPolyData* original3DObject = NULL);

  static void SetDefaultProperties(DataTreeNode* node, BaseRenderer* renderer = NULL, bool overwrite = false);

  virtual void ApplyProperties(BaseRenderer* renderer);

protected:

  SurfaceMapper2D();

  virtual ~SurfaceMapper2D();

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

