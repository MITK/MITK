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

#ifndef mitkImageToPointCloudFilter_h_Included
#define mitkImageToPointCloudFilter_h_Included

#include <MitkSurfaceInterpolationExports.h>
#include <mitkSurfaceToSurfaceFilter.h>

#include "mitkSurface.h"

namespace mitk
{

class MitkSurfaceInterpolation_EXPORT PointCloudScoringFilter:
    public SurfaceToSurfaceFilter
{

public:

  typedef std::pair<int, double> ScorePair;

  mitkClassMacro( PointCloudScoringFilter, SurfaceToSurfaceFilter)

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  itkGetMacro(NumberOfOutpPoints, int)
  itkGetMacro(FilteredScores, std::vector< ScorePair >)

protected:

  virtual void GenerateData();

  virtual void GenerateOutputInformation();

  PointCloudScoringFilter();

  virtual ~PointCloudScoringFilter();

private:

  mitk::Surface::Pointer m_OutpSurface;

  std::vector< ScorePair > m_FilteredScores;

  int m_NumberOfOutpPoints;

};

}
#endif
