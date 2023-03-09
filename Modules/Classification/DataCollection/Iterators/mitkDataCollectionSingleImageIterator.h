/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkDataCollectionSingleImageIterator_h
#define mitkDataCollectionSingleImageIterator_h


#include <mitkDataCollection.h>
#include <itkImageRegionIterator.h>

/**
  \brief Follow Up Storage - Class to facilitate loading/accessing structured follow-up data

  Data is into a collection that may contain further (sub) collections or images.
*/

namespace mitk
{
  template <typename TDataType, int ImageDimension>
  class DataCollectionSingleImageIterator
  {
  public:
    typedef itk::Image<TDataType, ImageDimension> ImageType;
    typedef itk::ImageRegionIterator<ImageType> ImageIterator;

    DataCollectionSingleImageIterator(mitk::DataCollection * collection, std::string imageName);

    void ToBegin();

    bool IsAtEnd();

    void operator++();

    // TODO refactor implementation
    void operator++(int);

    size_t GetIndex();

    std::string GetFilePrefix();

    void NextObject();

    ImageType * GetImage();

    void AddImage(ImageType * image, std::string name);

  private:
    // Variables
    DataCollection * m_Collection;
    std::string m_ImageName;
    bool m_IsAtEnd;
    bool m_IteratingImages;
    size_t m_CurrentIndex;
    size_t m_CurrentElement;
    ImageType * m_Image;
    DataCollectionSingleImageIterator<TDataType, ImageDimension>* m_CurrentSingleCollectionIterator;

    // Methods
    DataCollectionSingleImageIterator<TDataType, ImageDimension>* GetNextDataCollectionIterator(size_t start);

  };
} // end namespace

#include <Iterators/mitkDataCollectionSingleImageIterator.cxx>
#endif
