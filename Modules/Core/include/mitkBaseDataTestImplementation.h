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

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual void InitializeTimeGeometry( unsigned int timeSteps /* = 1 */ ) override
    {
      Superclass::InitializeTimeGeometry(timeSteps);
    }

  protected:

    mitkCloneMacro(Self);

    virtual bool VerifyRequestedRegion() override{return false;};
    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion() override{return false;};
    virtual void SetRequestedRegionToLargestPossibleRegion() override{};
    virtual void SetRequestedRegion( const itk::DataObject * /*data*/) override{};

    BaseDataTestImplementation(){};
    virtual ~BaseDataTestImplementation(){};

  };

} // namespace

#endif // BASEDATA_H_HEADER_INCLUDED
