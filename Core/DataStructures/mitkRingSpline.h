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

#ifndef MITKRingSpline_H_HEADER_INCLUDED
#define MITKRingSpline_H_HEADER_INCLUDED

#include <mitkClosedSpline.h>
#include <mitkCommon.h>
#include <mitkPlaneFit.h>
#include <mitkMesh.h>

namespace mitk {

class RingSpline: public ClosedSpline 
{
public:
  mitkClassMacro(RingSpline, ClosedSpline);

  itkNewMacro(Self);

  typedef mitk::Mesh::DataType MeshType;
  typedef MeshType::CellType CellType;
  typedef CellType::CellAutoPointer CellAutoPointer;
  typedef itk::PolygonCell<MeshType::CellType> PolygonCellType;

protected:

  RingSpline();
  virtual ~RingSpline();

  //!@brief sorting points
  virtual void DoSortPoints();


private:

  PlaneFit::Pointer m_PlaneFit;
 
};

} // namespace mitk

#endif /* MITKRingSpline_H_HEADER_INCLUDED */

