/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef BASEDATAIMPLEMENTATION_H_HEADER_INCLUDED
#define BASEDATAIMPLEMENTATION_H_HEADER_INCLUDED

#include "mitkBaseData.h"

namespace mitk {

  //##Documentation
  //## @brief Implementation of BaseData (for testing) 
  //##
  //## As BaseData is an abstract class, we need an implementation for testing its methods
  //## @ingroup Data

  class BaseDataImplementation : public BaseData
  {
  public:

    mitkClassMacro(BaseDataImplementation, BaseData);

    itkNewMacro(Self);

    virtual void InitializeTimeSlicedGeometry( unsigned int timeSteps /* = 1 */ )
    {
      Superclass::InitializeTimeSlicedGeometry(timeSteps);
    }

  protected:

    virtual bool VerifyRequestedRegion(){return false;};
    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion(){return false;};
    virtual void SetRequestedRegionToLargestPossibleRegion(){};
    virtual void SetRequestedRegion(itk::DataObject * /*data*/){};

    BaseDataImplementation(){};
    virtual ~BaseDataImplementation(){};
    
  };

} // namespace

#endif // BASEDATA_H_HEADER_INCLUDED