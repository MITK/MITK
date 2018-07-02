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

#ifndef SIMPLEFUNCTORBASE_H
#define SIMPLEFUNCTORBASE_H

#include <itkObject.h>

#include <mitkVector.h>

#include "mitkModelBase.h"

#include "MitkModelFitExports.h"

namespace mitk
{

  /**Simple functor base class for the itkMultiOutputNaryFunctorImageFilter. */
  class MITKMODELFIT_EXPORT SimpleFunctorBase: public ::itk::Object
  {
  public:
    typedef SimpleFunctorBase Self;
    typedef itk::Object Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkTypeMacro(SimpleFunctorBase, itk::Object);

    typedef ScalarType InputImagePixelType;
    typedef std::vector<InputImagePixelType> InputPixelVectorType;
    typedef std::vector<InputImagePixelType> OutputPixelVectorType;

    typedef itk::Array<double> GridArrayType;

    virtual GridArrayType GetGrid() const = 0;

    virtual OutputPixelVectorType Compute(const InputPixelVectorType & value) const = 0;

    /** @todo #3 Function needs to be implemented in every derived Functor
     * The function is already declared here to ensure that derived models give feedback on how many output parameters they produce
     * This is requested by several generators
     */
    virtual unsigned int GetNumberOfOutputs() const = 0;

  protected:

    SimpleFunctorBase();

    virtual ~SimpleFunctorBase();

  };

}


#endif // SIMPLEFUNCTORBASE_H
