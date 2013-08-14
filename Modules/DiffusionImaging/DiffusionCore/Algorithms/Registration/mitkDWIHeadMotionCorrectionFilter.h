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

#ifndef MITKDWIHEADMOTIONCORRECTIONFILTER_H
#define MITKDWIHEADMOTIONCORRECTIONFILTER_H

#include "mitkDiffusionImageToDiffusionImageFilter.h"

namespace mitk
{

/**
 * @class DWIHeadMotionCorrectionFilter
 */
template< typename DiffusionPixelType>
class DWIHeadMotionCorrectionFilter
    : public DiffusionImageToDiffusionImageFilter< DiffusionPixelType >
{
public:

  // class macros
  mitkClassMacro( DWIHeadMotionCorrectionFilter,
                  DiffusionImageToDiffusionImageFilter<DiffusionPixelType> )

  itkSimpleNewMacro(Self)

  // public typedefs
  typedef typename Superclass::DiffusionImageType         DiffusionImageType;
  typedef typename Superclass::DiffusionImagePointerType  DiffusionImagePointerType;


protected:
  DWIHeadMotionCorrectionFilter();
  virtual ~DWIHeadMotionCorrectionFilter() {}

  virtual void GenerateData();

};

} //end namespace mitk

#include "mitkDWIHeadMotionCorrectionFilter.cpp"


#endif // MITKDWIHEADMOTIONCORRECTIONFILTER_H
