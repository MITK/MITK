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

#ifndef mitkDataCollectionSingleImageIterator_CXX
#define mitkDataCollectionSingleImageIterator_CXX

#include <Iterators/mitkDataCollectionSingleImageIterator.h>
#include <mitkImageCast.h>

template <typename TDataType, int TImageDimension>
mitk::DataCollectionSingleImageIterator<TDataType, TImageDimension>::
DataCollectionSingleImageIterator(DataCollection *collection, std::string imageName) :
  m_Collection(collection),
  m_ImageName(imageName),
  m_IsAtEnd(false),
  m_IteratingImages(true),
  m_CurrentIndex(0),
  m_CurrentElement(0),
  m_CurrentSingleCollectionIterator(nullptr)
{
  ToBegin();
}

template <typename TDataType, int TImageDimension>
void
mitk::DataCollectionSingleImageIterator<TDataType, TImageDimension>::
ToBegin()
{
  m_IsAtEnd = false;
  m_Image = nullptr;
  m_IteratingImages = false;
  m_CurrentIndex = 0;
  m_CurrentElement = 0;

  if (m_Collection->HasElement(m_ImageName))
  {
    {
      mitk::Image *image = dynamic_cast<mitk::Image*>(m_Collection->GetData(m_ImageName).GetPointer());
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
      m_Image = image;
    }
  }
  if (m_Image == nullptr)
  {
    m_CurrentElement = 0;
    m_CurrentSingleCollectionIterator = GetNextDataCollectionIterator(m_CurrentElement);
    if (m_Image  == nullptr)
    {
      m_IsAtEnd = true;
    } else
    {
        m_Image = m_CurrentSingleCollectionIterator->GetImage();
    }
  }
}

template <typename TDataType, int TImageDimension>
bool
mitk::DataCollectionSingleImageIterator<TDataType, TImageDimension>::
IsAtEnd()
{
  return m_IsAtEnd;
}

template <typename TDataType, int TImageDimension>
void
mitk::DataCollectionSingleImageIterator<TDataType, TImageDimension>::
operator++()
{
  ++m_CurrentIndex;
  NextObject();
}

template <typename TDataType, int TImageDimension>
void
mitk::DataCollectionSingleImageIterator<TDataType, TImageDimension>::
operator++(int)
{
    ++m_CurrentIndex;
    NextObject();
}

template <typename TDataType, int TImageDimension>
size_t
mitk::DataCollectionSingleImageIterator<TDataType, TImageDimension>::
GetIndex()
{
  return m_CurrentIndex;
}

template <typename TDataType, int TImageDimension>
std::string
mitk::DataCollectionSingleImageIterator<TDataType, TImageDimension>::
GetFilePrefix()
{
  if (m_IteratingImages)
  {
    return m_ImageName;
  }
  else
  {
    return  m_Collection->IndexToName(m_CurrentElement) + "/" + m_CurrentSingleCollectionIterator->GetFilePrefix();
  }
}

template <typename TDataType, int TImageDimension>
void mitk::DataCollectionSingleImageIterator<TDataType, TImageDimension>::
NextObject()
{
    if (m_Collection->HasElement(m_ImageName))
    {
        if (m_CurrentSingleCollectionIterator != nullptr)
        {
            delete m_CurrentSingleCollectionIterator;
            m_CurrentSingleCollectionIterator = nullptr;
        }
        m_CurrentElement = 0;
        m_Image = nullptr;
        m_CurrentSingleCollectionIterator = GetNextDataCollectionIterator(m_CurrentElement);
        if (m_CurrentSingleCollectionIterator == nullptr)
        {
            m_IsAtEnd = true;
            return;
        }
    }
    else
    {
        m_CurrentSingleCollectionIterator->NextObject();
        if (m_CurrentSingleCollectionIterator->IsAtEnd()) //Current collection is finished iterated
        {
          delete m_CurrentSingleCollectionIterator;
          m_CurrentSingleCollectionIterator = GetNextDataCollectionIterator(m_CurrentElement+1);
        }
        if (m_CurrentSingleCollectionIterator == nullptr) //If no collection is known
        {
          m_IsAtEnd = true;
          return;
        }
    }
}

template <typename TDataType, int TImageDimension>
typename mitk::DataCollectionSingleImageIterator<TDataType, TImageDimension>::ImageType *
mitk::DataCollectionSingleImageIterator<TDataType, TImageDimension>::
GetImage()
{
  return m_Image;
}

template <typename TDataType, int TImageDimension>
void
mitk::DataCollectionSingleImageIterator<TDataType, TImageDimension>::
AddImage(ImageType *image, std::string name)
{
  if (m_Collection->HasElement(m_ImageName))
  {
    itk::DataObject::Pointer dataObject = dynamic_cast<itk::DataObject*>(image);
    m_Collection->AddData(dataObject, name);
  } else
  {
    m_CurrentSingleCollectionIterator->AddImage(image, name);
  }
}

template <typename TDataType, int TImageDimension>
mitk::DataCollectionSingleImageIterator<TDataType, TImageDimension>*
mitk::DataCollectionSingleImageIterator<TDataType, TImageDimension>::
GetNextDataCollectionIterator(size_t start)
{
  DataCollectionSingleImageIterator<TDataType, TImageDimension>* iterator = nullptr;
  size_t index =start;
  while (index < m_Collection->Size() && iterator == nullptr)
  {
    DataCollection* collection;
    collection = dynamic_cast<DataCollection*>(m_Collection->GetData(index).GetPointer());
    if (collection != nullptr)
    {
      iterator = new DataCollectionSingleImageIterator<TDataType, TImageDimension>(collection, m_ImageName);
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
  if (iterator != nullptr)
    m_Image = iterator->GetImage();
  return iterator;
}

// mitkDataCollectionSingleImageIterator_CXX
#endif 
