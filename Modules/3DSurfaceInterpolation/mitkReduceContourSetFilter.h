/*=========================================================================
Program: Medical Imaging & Interaction Toolkit
Module: $RCSfile$
Language: C++
Date: $Date: $
Version: $Revision: $
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the above copyright notices for more information.
=========================================================================*/

#ifndef mitkReduceContourSetFilter_h_Included
#define mitkReduceContourSetFilter_h_Included

#include <mitk3DSurfaceInterpolationExports.h>
#include "mitkSurfaceToSurfaceFilter.h"
#include "vtkCellArray.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"

namespace mitk {

class mitk3DSurfaceInterpolation_EXPORT ReduceContourSetFilter : public SurfaceToSurfaceFilter
{
public:

  mitkClassMacro(ReduceContourSetFilter,SurfaceToSurfaceFilter);
  itkNewMacro(Self);

  itkSetMacro(MinSpacing, double);
  itkSetMacro(MaxSpacing, double);

protected:
  ReduceContourSetFilter();
  virtual ~ReduceContourSetFilter();
  virtual void GenerateData();
  virtual void GenerateOutputInformation();

private:
  vtkPolyData* ReduceNumberOfPoints (vtkPolyData*, unsigned int currentInputIndex);
  bool CheckForIntersection (vtkIdType* currentCell, vtkIdType currentCellSize, vtkPoints* currentPoints, /*vtkIdType numberOfIntersections, vtkIdType* intersectionPoints,*/ unsigned int currentInputIndex);

  double m_MinSpacing; 
  double m_MaxSpacing;
};//class

}//namespace
#endif