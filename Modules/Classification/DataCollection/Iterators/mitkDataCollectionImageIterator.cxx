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

#ifndef mitkDataCollectionImageIterator_CXX
#define mitkDataCollectionImageIterator_CXX

#include <Iterators/mitkDataCollectionImageIterator.h>
#include <mitkImageCast.h>

template <typename TDataType, int TImageDimension>
mitk::DataCollectionImageIterator<TDataType, TImageDimension>::
  DataCollectionImageIterator(DataCollection::Pointer collection, std::string imageName) :
  m_Collection(collection), m_ImageName(imageName), m_IsAtEnd(false),
  m_IteratingImages(true), m_CurrentIndex(0),
  m_CurrentElement(0), m_CurrentCollectionIterator(nullptr)
{
  ToBegin();
}

template <typename TDataType, int TImageDimension>
void
  mitk::DataCollectionImageIterator<TDataType, TImageDimension>::
  ToBegin()
{
  m_IsAtEnd = false;
  m_IteratingImages = false;
  m_CurrentIndex = 0;
  m_CurrentElement = 0;
  m_ImageIndex = 0;

  if (m_Collection->HasElement(m_ImageName))
  {
    {
      mitk::Image *image = dynamic_cast<mitk::Image*>(m_Collection->GetData(m_ImageName).GetPointer());

      //TODO: check whether image is valid... image != 0 if empty smart pointer was inserted into collection!!!!
      if (image != nullptr)
      {
        typename ImageType::Pointer itkImage = ImageType::New();
        mitk::CastToItkImage(image, itkImage);
        itk::DataObject::Pointer itkObject = dynamic_cast<itk::DataObject* >(itkImage.GetPointer());
        m_Collection->SetData(itkObject, m_ImageName);
      }
    }
    ImageType * image = dynamic_cast<ImageType*>(m_Collection->GetData(m_ImageName).GetPointer());
    if (image != nullptr)
    {
      m_IteratingImages = true;
      m_CurrentIterator = ImageIterator(image, image->GetLargestPossibleRegion());
    }
  }
  if (!m_IteratingImages)
  {
    if (m_CurrentCollectionIterator != nullptr)
    {
      delete m_CurrentCollectionIterator;
      m_CurrentCollectionIterator = nullptr;
    }
    m_CurrentElement = 0;
    m_CurrentCollectionIterator = GetNextDataCollectionIterator(m_CurrentElement);
    if (m_CurrentCollectionIterator == nullptr)
    {
      m_IsAtEnd = true;
    } else
    {
      m_CurrentIterator = m_CurrentCollectionIterator->GetImageIterator();
    }
  }
}

template <typename TDataType, int TImageDimension>
bool
  mitk::DataCollectionImageIterator<TDataType, TImageDimension>::
  IsAtEnd()
{
  return m_IsAtEnd;
}

template <typename TDataType, int TImageDimension>
void
  mitk::DataCollectionImageIterator<TDataType, TImageDimension>::
  operator++()
{
  ++m_CurrentIndex;
  ++m_CurrentIterator;

  if (m_CurrentIterator.IsAtEnd())
  {
    ++m_ImageIndex;
    NextObject();
  }
}

template <typename TDataType, int TImageDimension>
void
  mitk::DataCollectionImageIterator<TDataType, TImageDimension>::
  operator++(int)
{
  ++m_CurrentIndex;
  ++m_CurrentIterator;

  if (m_CurrentIterator.IsAtEnd())
  {
    ++m_ImageIndex;
    NextObject();
  }
}

template <typename TDataType, int TImageDimension>
mitk::DataCollectionImageIterator<TDataType, TImageDimension>*
  mitk::DataCollectionImageIterator<TDataType, TImageDimension>::
  GetNextDataCollectionIterator(size_t start)
{
  DataCollectionImageIterator<TDataType, TImageDimension>* iterator = nullptr;
  size_t index =start;
  while (index < m_Collection->Size() && iterator == nullptr)
  {
    DataCollection* collection;
    collection = dynamic_cast<DataCollection*>(m_Collection->GetData(index).GetPointer());
    if (collection != nullptr)
    {
      iterator = new DataCollectionImageIterator<TDataType, TImageDimension>(collection, m_ImageName);
      if (iterator->IsAtEnd())
      {
        delete iterator;
        iterator = nullptr;
        ++index;
      }
    }
    else
    {
      ++index;
    }
  }
  m_CurrentElement = index;
  return iterator;
}

template <typename TDataType, int TImageDimension>
TDataType
  mitk::DataCollectionImageIterator<TDataType, TImageDimension>::
  GetVoxel()
{
  return m_CurrentIterator.Get();
}

template <typename TDataType, int TImageDimension>
void
  mitk::DataCollectionImageIterator<TDataType, TImageDimension>::
  SetVoxel(TDataType value)
{
  m_CurrentIterator.Set(value);
}

template <typename TDataType, int TImageDimension>
size_t
  mitk::DataCollectionImageIterator<TDataType, TImageDimension>::
  GetIndex()
{
  return m_CurrentIndex;
}

template <typename TDataType, int TImageDimension>
std::string
  mitk::DataCollectionImageIterator<TDataType, TImageDimension>::
  GetFilePrefix()
{
  if (m_IteratingImages)
  {
    return m_ImageName;
  }
  else
  {
    return  m_Collection->IndexToName(m_CurrentElement) + "/" + m_CurrentCollectionIterator->GetFilePrefix();
  }
}

template <typename TDataType, int TImageDimension>
void mitk::DataCollectionImageIterator<TDataType, TImageDimension>::NextObject()
{
  if (m_IteratingImages)
  {
    if (m_CurrentCollectionIterator != nullptr)
    {
      delete m_CurrentCollectionIterator;
      m_CurrentCollectionIterator = nullptr;
    }
    m_IteratingImages = false;
    m_CurrentElement = 0;
    m_CurrentCollectionIterator = GetNextDataCollectionIterator(m_CurrentElement);
    if (m_CurrentCollectionIterator == nullptr)
    {
      m_IsAtEnd = true;
      return;
    }
  }
  else
  {
    m_CurrentCollectionIterator->NextObject();
    if (m_CurrentCollectionIterator->IsAtEnd()) //Current collection is finished iterated
    {
      delete m_CurrentCollectionIterator;
      m_CurrentCollectionIterator = GetNextDataCollectionIterator(m_CurrentElement+1);
    }
    if (m_CurrentCollectionIterator == nullptr) //If no collection is known
    {
      m_IsAtEnd = true;
      return;
    }
  }
  m_CurrentIterator = m_CurrentCollectionIterator->GetImageIterator();
}

template <typename TDataType, int TImageDimension>
typename mitk::DataCollectionImageIterator<TDataType, TImageDimension>::ImageIterator
  mitk::DataCollectionImageIterator<TDataType, TImageDimension>::GetImageIterator()
{
  return m_CurrentIterator;
};

#endif // mitkDataCollectionImageIterator_CXX