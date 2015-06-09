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

#ifndef mitkPointCloudScoringFilter_h_Included
#define mitkPointCloudScoringFilter_h_Included

#include <MitkSurfaceInterpolationExports.h>
#include <mitkUnstructuredGridToUnstructuredGridFilter.h>


namespace mitk
{

class UnstructuredGrid;

/**
 * @brief Scores an UnstructuredGrid as good as one matches to the other.
 *
 * The result UnstructureGrid of the filter are the points where the distance to
 * the closest point of the other UnstructuredGrid is higher than the average
 * distance from all points to their closest neighbours of the other
 * UnstructuredGrid.
 * The second input is the UnstructuredGrid, which you want to score. All Points
 * of the output UnstructuredGrid are from the second input.
 */
class MITKSURFACEINTERPOLATION_EXPORT PointCloudScoringFilter:
    public UnstructuredGridToUnstructuredGridFilter
{

public:

  typedef std::pair<int, double> ScorePair;

  mitkClassMacro( PointCloudScoringFilter, UnstructuredGridToUnstructuredGridFilter)

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /** Number of Points of the scored UnstructuredGrid. These points are far away
   * from their neighbours */
  itkGetMacro(NumberOfOutpPoints, int)

  /** A vector in which the point IDs and their distance to their neighbours
   * is stored */
  itkGetMacro(FilteredScores, std::vector< ScorePair >)

protected:

  /** is called by the Update() method */
  virtual void GenerateData() override;

  /** Defines the output */
  virtual void GenerateOutputInformation() override;

  /** Constructor */
  PointCloudScoringFilter();

  /** Destructor */
  virtual ~PointCloudScoringFilter();

private:

  /** The output UnstructuredGrid containing the scored points, which are far
   * aways from their neighbours */
  mitk::UnstructuredGrid::Pointer m_OutpGrid;

  /** The Point IDs and their distance to their neighbours */
  std::vector< ScorePair > m_FilteredScores;

  /** The number of points which are far aways from their neighbours */
  int m_NumberOfOutpPoints;

};

}
#endif
