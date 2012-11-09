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

#ifndef mitkReduceContourSetFilter_h_Included
#define mitkReduceContourSetFilter_h_Included

#include "SegmentationExports.h"
#include "mitkSurfaceToSurfaceFilter.h"
#include "mitkSurface.h"
#include "mitkProgressBar.h"

#include "vtkSmartPointer.h"
#include "vtkPolyData.h"
#include "vtkPolygon.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkMath.h"

#include <stack>

namespace mitk {

/**
  \brief A filter that reduces the number of points of contours represented by a mitk::Surface

  The type of the reduction can be set via SetReductionType. The two ways provided by this filter is the

  \li NTH_POINT Algorithm which reduces the contours according to a certain stepsize
  \li DOUGLAS_PEUCKER Algorithm which incorpates an error tolerance into the reduction.

  Stepsize and error tolerance can be set via SetStepSize and SetTolerance.

  Additional if more than one input contour is provided this filter tries detect contours which occur just because
  of an intersection. These intersection contours are eliminated. In oder to ensure a correct elimination the min and max
  spacing of the original image must be provided.

  The output is a mitk::Surface.

  $Author: fetzer$
*/

    class Segmentation_EXPORT ReduceContourSetFilter : public SurfaceToSurfaceFilter
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

        itkGetMacro(NumberOfPointsAfterReduction, unsigned int);

        //Resets the filter, i.e. removes all inputs and outputs
        void Reset();

        /**
          \brief Set whether the mitkProgressBar should be used

          \a Parameter true for using the progress bar, false otherwise
        */
        void SetUseProgressBar(bool);

        /**
          \brief Set the stepsize which the progress bar should proceed

          \a Parameter The stepsize for progressing
        */
        void SetProgressStepSize(unsigned int stepSize);

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

        bool m_UseProgressBar;
        unsigned int m_ProgressStepSize;

        unsigned int m_NumberOfPointsAfterReduction;

    };//class

}//namespace
#endif
