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

#ifndef itkLocalIntensityFilter_cpp
#define itkLocalIntensityFilter_cpp

#include <itkLocalIntensityFilter.h>

#include <itkNeighborhoodIterator.h>
#include <itkImageRegionIterator.h>
#include <itkImageIterator.h>

#include <limits>

#include "itkImageScanlineIterator.h"
#include "itkProgressReporter.h"

namespace itk
{
  template< typename TInputImage >
  LocalIntensityFilter< TInputImage >
    ::LocalIntensityFilter() :m_ThreadLocalMaximum(1), m_ThreadLocalPeakValue(1), m_ThreadGlobalPeakValue(1)
  {
    // first output is a copy of the image, DataObject created by
    // superclass

    // allocate the data objects for the outputs which are
    // just decorators around real types
    for (int i = 1; i < 4; ++i)
    {
      typename RealObjectType::Pointer output =
        static_cast< RealObjectType * >(this->MakeOutput(i).GetPointer());
      this->ProcessObject::SetNthOutput(i, output.GetPointer());
    }
  }

  template< typename TInputImage >
  DataObject::Pointer
    LocalIntensityFilter< TInputImage >
    ::MakeOutput(DataObjectPointerArraySizeType output)
  {
    switch (output)
    {
    case 0:
      return TInputImage::New().GetPointer();
      break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      return RealObjectType::New().GetPointer();
      break;
    default:
      // might as well make an image
      return TInputImage::New().GetPointer();
      break;
    }
  }

  template< typename TInputImage >
  typename LocalIntensityFilter< TInputImage >::RealObjectType *
    LocalIntensityFilter< TInputImage >
    ::GetLocalPeakOutput()
  {
    return static_cast< RealObjectType * >(this->ProcessObject::GetOutput(1));
  }

  template< typename TInputImage >
  const typename LocalIntensityFilter< TInputImage >::RealObjectType *
    LocalIntensityFilter< TInputImage >
    ::GetLocalPeakOutput() const
  {
    return static_cast< const RealObjectType * >(this->ProcessObject::GetOutput(1));
  }

  template< typename TInputImage >
  typename LocalIntensityFilter< TInputImage >::RealObjectType *
    LocalIntensityFilter< TInputImage >
    ::GetGlobalPeakOutput()
  {
    return static_cast< RealObjectType * >(this->ProcessObject::GetOutput(2));
  }

  template< typename TInputImage >
  const typename LocalIntensityFilter< TInputImage >::RealObjectType *
    LocalIntensityFilter< TInputImage >
    ::GetGlobalPeakOutput() const
  {
    return static_cast< const RealObjectType * >(this->ProcessObject::GetOutput(2));
  }

  template< typename TInputImage >
  typename LocalIntensityFilter< TInputImage >::RealObjectType *
    LocalIntensityFilter< TInputImage >
    ::GetLocalMaximumOutput()
  {
    return static_cast< RealObjectType * >(this->ProcessObject::GetOutput(3));
  }

  template< typename TInputImage >
  const typename LocalIntensityFilter< TInputImage >::RealObjectType *
    LocalIntensityFilter< TInputImage >
    ::GetLocalMaximumOutput() const
  {
    return static_cast< const RealObjectType * >(this->ProcessObject::GetOutput(3));
  }

  template< typename TInputImage >
  void
    LocalIntensityFilter< TInputImage >
    ::GenerateInputRequestedRegion()
  {
    Superclass::GenerateInputRequestedRegion();
    if (this->GetInput())
    {
      InputImagePointer image =
        const_cast< typename Superclass::InputImageType * >(this->GetInput());
      image->SetRequestedRegionToLargestPossibleRegion();
    }
  }

  template< typename TInputImage >
  void
    LocalIntensityFilter< TInputImage >
    ::EnlargeOutputRequestedRegion(DataObject *data)
  {
    Superclass::EnlargeOutputRequestedRegion(data);
    data->SetRequestedRegionToLargestPossibleRegion();
  }

  template< typename TInputImage >
  void
    LocalIntensityFilter< TInputImage >
    ::AllocateOutputs()
  {
    // Pass the input through as the output
    InputImagePointer image =
      const_cast< TInputImage * >(this->GetInput());

    this->GraftOutput(image);

    // Nothing that needs to be allocated for the remaining outputs
  }

  template< typename TInputImage >
  void
    LocalIntensityFilter< TInputImage >
    ::BeforeThreadedGenerateData()
  {
    ThreadIdType numberOfThreads = this->GetNumberOfThreads();

    // Resize the thread temporaries
    m_ThreadLocalMaximum.SetSize(numberOfThreads);
    m_ThreadLocalPeakValue.SetSize(numberOfThreads);
    m_ThreadGlobalPeakValue.SetSize(numberOfThreads);

    // Initialize the temporaries
    m_ThreadLocalMaximum.Fill(std::numeric_limits< RealType>::lowest());
    m_ThreadLocalPeakValue.Fill(std::numeric_limits< RealType>::lowest());
    m_ThreadGlobalPeakValue.Fill(std::numeric_limits< RealType>::lowest());

  }

  template< typename TInputImage >
  void
    LocalIntensityFilter< TInputImage >
    ::AfterThreadedGenerateData()
  {
    ThreadIdType    i;
    ThreadIdType numberOfThreads = this->GetNumberOfThreads();

    RealType  localMaximum = std::numeric_limits< RealType >::lowest();
    RealType  localPeakValue = std::numeric_limits< RealType >::lowest();
    RealType  globalPeakValue = std::numeric_limits< RealType >::lowest();

    for (i = 0; i < numberOfThreads; i++)
    {
      globalPeakValue = std::max<RealType>(globalPeakValue, m_ThreadGlobalPeakValue[i]);
      if (localMaximum == m_ThreadLocalMaximum[i])
      {
        localPeakValue = std::max< RealType >(m_ThreadLocalPeakValue[i], localPeakValue);
      }
      else if (localMaximum < m_ThreadLocalMaximum[i]) {
        localMaximum = m_ThreadLocalMaximum[i];
        localPeakValue = m_ThreadLocalPeakValue[i];
      }
    }
        // Set the outputs
    this->GetLocalPeakOutput()->Set(localPeakValue);
    this->GetGlobalPeakOutput()->Set(globalPeakValue);
    this->GetLocalMaximumOutput()->Set(localMaximum);
  }

  template< typename TInputImage >
  void
    LocalIntensityFilter< TInputImage >
    ::ThreadedGenerateData(const RegionType & outputRegionForThread,
      ThreadIdType threadId)
  {
    typename TInputImage::ConstPointer itkImage = this->GetInput();
    typename MaskImageType::Pointer itkMask = m_Mask;

    double range = m_Range;
    double minimumSpacing = 1;
    typename TInputImage::SizeType regionSize;
    int offset = std::ceil(range / minimumSpacing);
    regionSize.Fill(1);
    for (unsigned int i = 0; i <  TInputImage::ImageDimension; ++i)
    {
      minimumSpacing = itkImage->GetSpacing()[i];
      offset = std::ceil(range / minimumSpacing);
      regionSize[i] = offset;
    }

    itk::ConstNeighborhoodIterator<TInputImage> iter(regionSize, itkImage, outputRegionForThread);
    itk::ConstNeighborhoodIterator<MaskImageType> iterMask(regionSize, itkMask, outputRegionForThread);

    typename TInputImage::PointType origin;
    typename TInputImage::PointType localPoint;
    itk::Index<TInputImage::ImageDimension> index;

    double tmpPeakValue;
    double globalPeakValue = std::numeric_limits<double>::lowest();
    double localPeakValue = std::numeric_limits<double>::lowest();
    PixelType localMaximum = std::numeric_limits<PixelType>::lowest();

    std::vector<bool> vectorIsInRange;
    index = iter.GetIndex();
    itkImage->TransformIndexToPhysicalPoint(index, origin);
    for (itk::SizeValueType i = 0; i < iter.Size(); ++i)
    {
      itkImage->TransformIndexToPhysicalPoint(iter.GetIndex(i), localPoint);
      double dist = origin.EuclideanDistanceTo(localPoint);
      vectorIsInRange.push_back((dist < range));
    }

    int count = 0;

    iter.NeedToUseBoundaryConditionOff();
    iterMask.NeedToUseBoundaryConditionOff();

    auto imageSize = itkImage->GetLargestPossibleRegion().GetSize();
    unsigned int imageDimension = itkImage->GetImageDimension();

    while (!iter.IsAtEnd())
    {
      if (iterMask.GetCenterPixel() > 0)
      {
        tmpPeakValue = 0;
        count = 0;
        for (itk::SizeValueType i = 0; i < iter.Size(); ++i)
        {
          if (vectorIsInRange[i])
          {
            auto localIndex = iter.GetIndex(i);
            bool calculatePoint = true;
            for (unsigned int dimension = 0; dimension < imageDimension; ++dimension)
            {
              calculatePoint &= (localIndex[dimension] < static_cast<signed int>(imageSize[dimension]));
              calculatePoint &= (0 <= localIndex[dimension]);
            }
            if (calculatePoint)
            {
              tmpPeakValue += iter.GetPixel(i);
              ++count;
            }
          }
        }
        tmpPeakValue /= count;
        globalPeakValue = std::max<double>(tmpPeakValue, globalPeakValue);
        auto currentCenterPixelValue = iter.GetCenterPixel();
        if (localMaximum == currentCenterPixelValue)
        {
          localPeakValue = std::max<double>(tmpPeakValue, localPeakValue);
        }
        else if (localMaximum < currentCenterPixelValue)
        {
          localMaximum = currentCenterPixelValue;
          localPeakValue = tmpPeakValue;
        }
      }
      ++iterMask;
      ++iter;
    }

    m_ThreadLocalMaximum[threadId] = localMaximum;
    m_ThreadLocalPeakValue[threadId] = localPeakValue;
    m_ThreadGlobalPeakValue[threadId] = globalPeakValue;
  }

  template< typename TImage >
  void
    LocalIntensityFilter< TImage >
    ::PrintSelf(std::ostream & os, Indent indent) const
  {
    Superclass::PrintSelf(os, indent);

    os << indent << "Local Peak: " << this->GetLocalPeak() << std::endl;
    os << indent << "Global Peak: " << this->GetGlobalPeak() << std::endl;
    os << indent << "Local Maximum: " << this->GetLocalMaximum() << std::endl;
  }
} // end namespace itk
#endif