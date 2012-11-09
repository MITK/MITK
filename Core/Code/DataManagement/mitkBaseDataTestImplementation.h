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


#ifndef BASEDATAIMPLEMENTATION_H_HEADER_INCLUDED
#define BASEDATAIMPLEMENTATION_H_HEADER_INCLUDED

#include "mitkBaseData.h"

namespace mitk {

  //##Documentation
  //## @brief Implementation of BaseData (for testing)
  //##
  //## As BaseData is an abstract class, we need an implementation for testing its methods
  //## @ingroup Data

  class BaseDataTestImplementation : public BaseData
  {
  public:

    mitkClassMacro(BaseDataTestImplementation, BaseData);

    itkNewMacro(Self);
    mitkCloneMacro(BaseDataTestImplementation);

    virtual void InitializeTimeSlicedGeometry( unsigned int timeSteps /* = 1 */ )
    {
      Superclass::InitializeTimeSlicedGeometry(timeSteps);
    }

  protected:

    virtual bool VerifyRequestedRegion(){return false;};
    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion(){return false;};
    virtual void SetRequestedRegionToLargestPossibleRegion(){};
    virtual void SetRequestedRegion(itk::DataObject * /*data*/){};

    BaseDataTestImplementation(){};
    virtual ~BaseDataTestImplementation(){};

  };

} // namespace

#endif // BASEDATA_H_HEADER_INCLUDED
