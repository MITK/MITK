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

#ifndef IndexedValueFunctorPolicy_H
#define IndexedValueFunctorPolicy_H

#include "itkIndex.h"
#include "mitkIndexedValueFunctorBase.h"
#include "MitkModelFitExports.h"

namespace mitk
{

  class MITKMODELFIT_EXPORT IndexedValueFunctorPolicy
  {
  public:
    typedef IndexedValueFunctorBase               FunctorType;
    typedef IndexedValueFunctorBase::ConstPointer FunctorConstPointer;

    typedef FunctorType::InputPixelVectorType InputPixelVectorType;
    typedef FunctorType::OutputPixelVectorType OutputPixelVectorType;
    typedef InputPixelVectorType InputPixelArrayType;

    typedef FunctorType::IndexType IndexType;
    typedef std::vector<mitk::ScalarType> OutputPixelArrayType;

    IndexedValueFunctorPolicy();

    ~IndexedValueFunctorPolicy();

    unsigned int GetNumberOfOutputs() const;

    void SetFunctor(const FunctorType *functor);


    bool operator!=(const IndexedValueFunctorPolicy & other) const;

    bool operator==(const IndexedValueFunctorPolicy & other) const;

    OutputPixelVectorType operator()(const InputPixelVectorType & value, const IndexType& currentIndex) const;

  private:

    FunctorConstPointer m_Functor;
  };

}


#endif // IndexedValueFunctorPolicy_H
