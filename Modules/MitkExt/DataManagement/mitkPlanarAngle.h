/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 17258 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef _MITK_PLANAR_CIRCLE_H_
#define _MITK_PLANAR_CIRCLE_H_

#include "mitkPlanarFigure.h"


namespace mitk 
{

class Geometry2D;

/**
 * \brief Implementation of PlanarFigure to display an angle
 * through three control points
 */
class MITKEXT_CORE_EXPORT PlanarAngle : public PlanarFigure
{
public:
  mitkClassMacro( PlanarAngle, BaseData );

  itkNewMacro( Self );

//  typedef itk::VectorContainer< unsigned long, mitk::Point2D > VertexContainerType;


  virtual bool IsClosed() const { return true; };

  /** \brief Place figure in its minimal configuration (a point at least)
   * onto the given 2D geometry.
   *
   * Must be implemented in sub-classes.
   */
  virtual void Initialize();

  
protected:
  PlanarAngle();
  virtual ~PlanarAngle();

  /** \brief Generates the poly-line representation of the planar figure. */
  virtual void GeneratePolyLine();

  virtual void PrintSelf( std::ostream &os, itk::Indent indent ) const;


private:

};

} // namespace mitk

#endif //_MITK_PLANAR_CIRCLE_H_
