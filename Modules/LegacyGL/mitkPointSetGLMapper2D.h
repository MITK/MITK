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


#ifndef MITKPointSetMAPPER2D_H_HEADER_INCLUDED
#define MITKPointSetMAPPER2D_H_HEADER_INCLUDED

#include <MitkLegacyGLExports.h>
#include "mitkGLMapper.h"

namespace mitk {

class BaseRenderer;
class PointSet;

/**
 * @brief OpenGL-based mapper to display a mitk::PointSet in a 2D window.
 *
 * This mapper can actually more than just draw a number of points of a
 * mitk::PointSet. If you set the right properties of the mitk::DataNode,
 * which contains the point set, then this mapper will also draw lines
 * connecting the points, and calculate and display distances and angles
 * between adjacent points. Here is a complete list of boolean properties,
 * which might be of interest:
 *
 * - \b "show contour": Draw not only the points but also the connections between
 *     them (default false)
 * - \b "line width": IntProperty which gives the width of the contour lines
 * - \b "show points": Wheter or not to draw the actual points (default true)
 * - \b "show distances": Wheter or not to calculate and print the distance
 *     between adjacent points (default false)
 * - \b "show angles": Wheter or not to calculate and print the angle between
 *     adjacent points (default false)
 * - \b "show distant lines": When true, the mapper will also draw contour
 *     lines that are far away form the current slice (default true)
 * - \b "label": StringProperty with a label for this point set
 *
 * BUG 1321 - possible new features:
 * point-2d-size (length of lines in cross/diamond)
 * point-linewidth
 *
 * @ingroup Mapper
 */

/** \deprecatedSince{2013_06} This mapper is replaced by PointSetVtkMapper2D. The child classes of this class are deprecated.
 * To further ensure their functionality PointSetGLMapper2D cannot be removed and is set deprecated too.
 */
class MITKLEGACYGL_EXPORT PointSetGLMapper2D : public GLMapper
{
public:
  mitkClassMacro(PointSetGLMapper2D, GLMapper);

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /** @brief Get the PointDataList to map */
  virtual const mitk::PointSet * GetInput(void);

  virtual void Paint(mitk::BaseRenderer * renderer);

  virtual void ApplyAllProperties(mitk::BaseRenderer* renderer);

  static void SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer = NULL, bool overwrite = false);

protected:
  PointSetGLMapper2D();

  virtual ~PointSetGLMapper2D();

  bool m_Polygon;
  bool m_PolygonClosed;
  bool m_ShowPoints;
  bool m_ShowDistances;
  int m_DistancesDecimalDigits;
  bool m_ShowAngles;
  bool m_ShowDistantLines;
  int  m_LineWidth;
  int m_PointLineWidth;
  int m_Point2DSize;
};

} // namespace mitk



#endif /* MITKPointSetMapper2D_H_HEADER_INCLUDED */
