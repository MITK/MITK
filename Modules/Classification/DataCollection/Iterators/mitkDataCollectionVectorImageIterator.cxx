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

#include "mitkDataCollectionVectorImageIterator.h"

#include "itkImageRegionConstIterator.h"

#include "itkComposeImageFilter.h"

template<typename TDataType, int ImageDimension>
inline
mitk::DataCollectionVectorImageIterator<TDataType, ImageDimension>::DataCollectionVectorImageIterator(
    DataCollection::Pointer collection, std::vector<std::string> featureNames) :
    m_Collection(collection)
    {
  for(std::vector<std::string>::iterator featureNameIterator = featureNames.begin();
      featureNameIterator != featureNames.end();
      ++featureNameIterator)
  {
    SingleImageIterator featureIterator(collection, *featureNameIterator);
    m_FeatureIterators.push_back(featureIterator);
  }
    }

template<typename TDataType, int ImageDimension>
inline void
mitk::DataCollectionVectorImageIterator<TDataType, ImageDimension>::ToBegin()
{
  for(typename DataCollectionSingleImageIterators::iterator it = m_FeatureIterators.begin();
      it != m_FeatureIterators.end();
      ++it)
  {
    it->ToBegin();
  }
}

template<typename TDataType, int ImageDimension>
inline bool
mitk::DataCollectionVectorImageIterator<TDataType, ImageDimension>::IsAtEnd()
{
  return m_FeatureIterators.at(0).IsAtEnd();
}

template<typename TDataType, int ImageDimension>
inline mitk::DataCollectionVectorImageIterator<TDataType, ImageDimension>&
mitk::DataCollectionVectorImageIterator<TDataType, ImageDimension>::operator ++()
{
  for(typename DataCollectionSingleImageIterators::iterator it = m_FeatureIterators.begin();
      it != m_FeatureIterators.end();
      ++it)
  {
    ++(*it);
  }
  return *this;
}

template<typename TDataType, int ImageDimension>
inline mitk::DataCollectionVectorImageIterator<TDataType, ImageDimension>
mitk::DataCollectionVectorImageIterator<TDataType, ImageDimension>::operator ++(int)
{
  mitk::DataCollectionVectorImageIterator<TDataType, ImageDimension> result (*this);
  ++result;
  return result;
}



template<typename TDataType, int ImageDimension>
inline typename mitk::DataCollectionVectorImageIterator<TDataType, ImageDimension>::VectorImagePointerType
mitk::DataCollectionVectorImageIterator<TDataType, ImageDimension>::GetVectorImage()
{
  typedef typename itk::Image<TDataType, ImageDimension> ImageType;

  typename itk::ComposeImageFilter<ImageType, VectorImageType>::Pointer composeImageFilter =
      itk::ComposeImageFilter<ImageType, VectorImageType>::New();

  // create image iterators from collection iterators
  for (unsigned i = 0; i < m_FeatureIterators.size(); i++)
  {
    composeImageFilter->SetInput(i, m_FeatureIterators.at(i).GetImage());
  }

  composeImageFilter->Update();

  composeImageFilter->GetOutput()->SetRequestedRegionToLargestPossibleRegion();

  return composeImageFilter->GetOutput();
}

template<typename TDataType, int ImageDimension>
inline void
mitk::DataCollectionVectorImageIterator<TDataType, ImageDimension>::AddImage(
    typename itk::Image<TDataType, ImageDimension>::Pointer image, std::string name)
    {
  m_FeatureIterators.at(m_FeatureIterators.size()-1).AddImage(image, name);
    }