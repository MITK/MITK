/*=========================================================================

  Program:   Medical Imaging & Interaction Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _mitkContourSetToPointSetFilter_h__
#define _mitkContourSetToPointSetFilter_h__

#include "mitkCommon.h"
#include "mitkPointSet.h"
#include "mitkPointSetSource.h"
#include "mitkContourSet.h"

namespace mitk {

/**
 *
 * @brief Converts a contour set to a point set.
 *
 * The resulting pointset consists of sample points of all the contours
 *
 * @ingroup SurfaceFilters
 * @ingroup Process
 */
class MITKEXT_CORE_EXPORT ContourSetToPointSetFilter : public PointSetSource
{
public:
  mitkClassMacro(ContourSetToPointSetFilter, PointSetSource);
  itkNewMacro(Self);


  itkSetMacro(Frequency, unsigned int);

  itkGetMacro(Frequency, unsigned int);

  virtual void GenerateOutputInformation();

  virtual void GenerateData();

  const mitk::ContourSet* GetInput(void);

  virtual void SetInput(const mitk::ContourSet *contourSet);

protected:
  ContourSetToPointSetFilter();

  virtual ~ContourSetToPointSetFilter();

protected:
  unsigned int m_Frequency;

};

} // namespace mitk

#endif // _mitkContourSetToPointSetFilter_h__
