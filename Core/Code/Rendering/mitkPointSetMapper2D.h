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


#ifndef MITKPointSetMAPPER2D_H_HEADER_INCLUDED
#define MITKPointSetMAPPER2D_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkGLMapper2D.h"

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
class MITK_CORE_EXPORT PointSetMapper2D : public GLMapper2D
{
public:
  mitkClassMacro(PointSetMapper2D, GLMapper2D);

  itkNewMacro(Self);

  /** @brief Get the PointDataList to map */
  virtual const mitk::PointSet * GetInput(void);

  virtual void Paint(mitk::BaseRenderer * renderer);

  virtual void ApplyProperties(mitk::BaseRenderer* renderer);

  static void SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer = NULL, bool overwrite = false);

protected:
  PointSetMapper2D();

  virtual ~PointSetMapper2D();

  bool m_Polygon;
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
