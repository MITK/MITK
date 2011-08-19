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

#include "MitkExtExports.h"
#include "mitkSurfaceToSurfaceFilter.h"
#include "vtkCellArray.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"

//Test
//#include "mitkNthPointFilter.h"
//#include "mitkDouglasPeuckerFilter.h"

namespace mitk {
    
    class MitkExt_EXPORT ReduceContourSetFilter : public SurfaceToSurfaceFilter
    {
        
    public:

       enum Reduction_Type
        {
            NTH_POINT, DOUGLAS_PEUCKER
        };
        
        struct LineSegment 
        {
            unsigned int StartIndex;
            unsigned int EndIndex;
        };
        
        mitkClassMacro(ReduceContourSetFilter,SurfaceToSurfaceFilter);
        itkNewMacro(Self);
        
        itkSetMacro(MinSpacing, double);
        itkSetMacro(MaxSpacing, double);
        itkSetMacro(ReductionType, Reduction_Type);
        itkSetMacro(StepSize, unsigned int);
        itkSetMacro(Tolerance, double);
        
    protected:
        ReduceContourSetFilter();
        virtual ~ReduceContourSetFilter();
        virtual void GenerateData();
        virtual void GenerateOutputInformation();
        
    private:
        void ReduceNumberOfPointsByNthPoint (vtkIdType cellSize, vtkIdType* cell, vtkPoints* points, vtkPolygon* reducedPolygon, vtkPoints* reducedPoints);
        
        void ReduceNumberOfPointsByDouglasPeucker (vtkIdType cellSize, vtkIdType* cell, vtkPoints* points, vtkPolygon* reducedPolygon, vtkPoints* reducedPoints);
        
        bool CheckForIntersection (vtkIdType* currentCell, vtkIdType currentCellSize, vtkPoints* currentPoints, /*vtkIdType numberOfIntersections, vtkIdType* intersectionPoints,*/ unsigned int currentInputIndex);
        
        double m_MinSpacing; 
        double m_MaxSpacing;
       
        Reduction_Type m_ReductionType;
        unsigned int m_StepSize;
        double m_Tolerance;
        unsigned int m_MaxSegmentLenght;
        
    };//class
    
}//namespace
#endif