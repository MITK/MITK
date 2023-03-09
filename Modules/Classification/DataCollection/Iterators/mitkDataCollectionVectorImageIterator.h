/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDataCollectionVectorImageIterator_h
#define mitkDataCollectionVectorImageIterator_h
#include <mitkDataCollection.h>
#include <mitkDataCollectionSingleImageIterator.h>

#include "itkImage.h"

namespace mitk
{

  /*
   * Iterate over a data collection and return a vector image for all selected
   * modalities.
   * TODO SW: this class needs a module test urgently.
   */
  template <typename TDataType, int ImageDimension>
  class DataCollectionVectorImageIterator
  {

  public:
    typedef itk::VectorImage<TDataType, ImageDimension> VectorImageType;
    typedef typename VectorImageType::Pointer VectorImagePointerType;
    typedef itk::ImageRegionIterator<VectorImageType> VectorImageIteratorType;

    DataCollectionVectorImageIterator(DataCollection::Pointer collection, std::vector<std::string> featureNames);

    void ToBegin();

    bool IsAtEnd();

    DataCollectionVectorImageIterator& operator++();

    DataCollectionVectorImageIterator operator++(int);

    /**
     * Get the itk::VectorImage summarizing all the components of the collection. Attention:
     * this is a copy. Each component is one modality.
     *
     * @return the itk::VectorImage
     */
    VectorImagePointerType GetVectorImage();

    void
    AddImage(typename itk::Image<TDataType, ImageDimension>::Pointer image, std::string name);

    typedef typename mitk::DataCollectionSingleImageIterator<TDataType, ImageDimension> SingleImageIterator;
    typedef typename std::vector< SingleImageIterator > DataCollectionSingleImageIterators;

  private:

    DataCollection::Pointer m_Collection;
    DataCollectionSingleImageIterators m_FeatureIterators;
  };

} /* namespace mitk */


#include <Iterators/mitkDataCollectionVectorImageIterator.cxx>

#endif
