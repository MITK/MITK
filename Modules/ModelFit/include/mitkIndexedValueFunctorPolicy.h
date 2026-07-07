/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIndexedValueFunctorPolicy_h
#define mitkIndexedValueFunctorPolicy_h

#include <itkIndex.h>
#include <mitkIndexedValueFunctorBase.h>
#include <MitkModelFitExports.h>

namespace mitk
{

  /**
   * \class IndexedValueFunctorPolicy
   * \brief Policy class adapting an IndexedValueFunctorBase for use with itkMultiOutputNaryFunctorImageFilter.
   *
   * Wraps an IndexedValueFunctorBase and provides the operator() interface expected
   * by the ITK image filter, passing both the pixel values and the current index.
   *
   * \sa IndexedValueFunctorBase, SimpleFunctorPolicy
   */
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


#endif
