/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkHotspotMaskGenerator.h>
#include <mitkImageTimeSelector.h>
#include <mitkImageCast.h>
#include <mitkPoint.h>
#include <itkImageRegionIterator.h>
#include "mitkImageAccessByItk.h"
#include <itkImageDuplicator.h>
#include <itkFFTConvolutionImageFilter.h>
#include <itkVnlFFTImageFilterInitFactory.h>
#include <mitkITKImageImport.h>

namespace mitk
{
    HotspotMaskGenerator::HotspotMaskGenerator():
        m_HotspotRadiusInMM(6.2035049089940),   // radius of a 1cm3 sphere in mm
        m_HotspotMustBeCompletelyInsideImage(true),
        m_Label(1)
    {
        m_InternalMask = mitk::Image::New();
        m_InternalMaskUpdateTime = 0;
    }

    HotspotMaskGenerator::~HotspotMaskGenerator()
    {
    }

    unsigned int HotspotMaskGenerator::GetNumberOfMasks() const
    {
      return 1;
    }

    mitk::Image::ConstPointer HotspotMaskGenerator::DoGetMask(unsigned int)
    {
        if (IsUpdateRequired())
        {
            if ( m_InputImage.IsNull() )
            {
              throw std::runtime_error( "Error: image empty!" );
            }

            if ( m_InputImage->GetTimeGeometry()->IsValidTimePoint(m_TimePoint) )
            {
              throw std::runtime_error( "Error: invalid time point!" );
            }

            auto timeSliceImage = SelectImageByTimePoint(m_InputImage, m_TimePoint);

            m_internalMask2D = nullptr; // is this correct when this variable holds a smart pointer?
            m_internalMask3D = nullptr;

            if ( m_Mask != nullptr )
            {
                m_Mask->SetTimePoint(m_TimePoint);
                mitk::Image::ConstPointer timeSliceMask = m_Mask->GetMask(0);

                if ( timeSliceImage->GetDimension() == 3 )
                {
                    itk::Image<unsigned short, 3>::Pointer noneConstMaskImage; //needed to work around the fact that CastToItkImage currently does not support const itk images.
                    CastToItkImage(timeSliceMask, noneConstMaskImage);
                    m_internalMask3D = noneConstMaskImage;
                    AccessFixedDimensionByItk_2(timeSliceImage, CalculateHotspotMask, 3, m_internalMask3D.GetPointer(), m_Label);
                }
                else if ( timeSliceImage->GetDimension() == 2 )
                {
                    itk::Image<unsigned short, 2>::Pointer noneConstMaskImage; //needed to work around the fact that CastToItkImage currently does not support const itk images.
                    CastToItkImage(timeSliceMask, noneConstMaskImage);
                    m_internalMask2D = noneConstMaskImage;
                    AccessFixedDimensionByItk_2(timeSliceImage, CalculateHotspotMask, 2, m_internalMask2D.GetPointer(), m_Label);
                }
                else
                {
                    throw std::runtime_error( "Error: invalid image dimension" );
                }
            }
            else
            {

                if ( timeSliceImage->GetDimension() == 3 )
                {
                    AccessFixedDimensionByItk_2(timeSliceImage, CalculateHotspotMask, 3, m_internalMask3D.GetPointer(), m_Label);
                }
                else if ( timeSliceImage->GetDimension() == 2 )
                {
                    AccessFixedDimensionByItk_2(timeSliceImage, CalculateHotspotMask, 2, m_internalMask2D.GetPointer(), m_Label);
                }
                else
                {
                    throw std::runtime_error( "Error: invalid image dimension" );
                }
            }
            this->Modified();
        }

        m_InternalMaskUpdateTime = m_InternalMask->GetMTime();
        return m_InternalMask;
    }

    template <typename TPixel, unsigned int VImageDimension  >
    HotspotMaskGenerator::ImageExtrema
      HotspotMaskGenerator::CalculateExtremaWorld( const itk::Image<TPixel, VImageDimension>* inputImage,
                                                    const itk::Image<unsigned short, VImageDimension>* maskImage,
                                                    double neccessaryDistanceToImageBorderInMM,
                                                    unsigned int label )
    {
      typedef itk::Image< TPixel, VImageDimension > ImageType;
      typedef itk::Image< unsigned short, VImageDimension > MaskImageType;

      typedef itk::ImageRegionConstIteratorWithIndex<MaskImageType> MaskImageIteratorType;
      typedef itk::ImageRegionConstIteratorWithIndex<ImageType> InputImageIndexIteratorType;

      typename ImageType::SpacingType spacing = inputImage->GetSpacing();

      ImageExtrema minMax;
      minMax.Defined = false;
      minMax.MaxIndex.set_size(VImageDimension);
      minMax.MaxIndex.set_size(VImageDimension);

      typename ImageType::RegionType allowedExtremaRegion = inputImage->GetLargestPossibleRegion();

      bool keepDistanceToImageBorders( neccessaryDistanceToImageBorderInMM > 0 );
      if (keepDistanceToImageBorders)
      {
        itk::IndexValueType distanceInPixels[VImageDimension];
        for(unsigned short dimension = 0; dimension < VImageDimension; ++dimension)
        {
          // To confirm that the whole hotspot is inside the image we have to keep a specific distance to the image-borders, which is as long as
          // the radius. To get the amount of indices we divide the radius by spacing and add 0.5 because voxels are center based:
          // For example with a radius of 2.2 and a spacing of 1 two indices are enough because 2.2 / 1 + 0.5 = 2.7 => 2.
          // But with a radius of 2.7 we need 3 indices because 2.7 / 1 + 0.5 = 3.2 => 3
          distanceInPixels[dimension] = int( neccessaryDistanceToImageBorderInMM / spacing[dimension] + 0.5);
        }

        allowedExtremaRegion.ShrinkByRadius(distanceInPixels);
      }

      InputImageIndexIteratorType imageIndexIt(inputImage, allowedExtremaRegion);

      float maxValue = itk::NumericTraits<float>::min();
      float minValue = itk::NumericTraits<float>::max();

      typename ImageType::IndexType maxIndex;
      typename ImageType::IndexType minIndex;

      for(unsigned short i = 0; i < VImageDimension; ++i)
      {
        maxIndex[i] = 0;
        minIndex[i] = 0;
      }

      if (maskImage != nullptr)
      {
        MaskImageIteratorType maskIt(maskImage, maskImage->GetLargestPossibleRegion());
        typename ImageType::IndexType imageIndex;
        typename ImageType::IndexType maskIndex;

        for(maskIt.GoToBegin(); !maskIt.IsAtEnd(); ++maskIt)
        {
          imageIndex = maskIndex = maskIt.GetIndex();

          if(maskIt.Get() == label)
          {
            if( allowedExtremaRegion.IsInside(imageIndex) )
            {
              imageIndexIt.SetIndex( imageIndex );
              double value = imageIndexIt.Get();
              minMax.Defined = true;

              //Calculate minimum, maximum and corresponding index-values
              if( value > maxValue )
              {
                maxIndex = imageIndexIt.GetIndex();
                maxValue = value;
              }

              if(value < minValue )
              {
                minIndex = imageIndexIt.GetIndex();
                minValue = value;
              }
            }
          }
        }
      }
      else
      {
        for(imageIndexIt.GoToBegin(); !imageIndexIt.IsAtEnd(); ++imageIndexIt)
        {
          double value = imageIndexIt.Get();
          minMax.Defined = true;

          //Calculate minimum, maximum and corresponding index-values
          if( value > maxValue )
          {
            maxIndex = imageIndexIt.GetIndex();
            maxValue = value;
          }

          if(value < minValue )
          {
            minIndex = imageIndexIt.GetIndex();
            minValue = value;
          }
        }
      }

      minMax.MaxIndex.set_size(VImageDimension);
      minMax.MinIndex.set_size(VImageDimension);

      for(unsigned int i = 0; i < minMax.MaxIndex.size(); ++i)
      {
        minMax.MaxIndex[i] = maxIndex[i];
      }

      for(unsigned int i = 0; i < minMax.MinIndex.size(); ++i)
      {
        minMax.MinIndex[i] = minIndex[i];
      }

      minMax.Max = maxValue;
      minMax.Min = minValue;

      return minMax;
    }

    template <unsigned int VImageDimension>
    itk::Size<VImageDimension>
      HotspotMaskGenerator::CalculateConvolutionKernelSize( double spacing[VImageDimension],
                                                             double radiusInMM )
    {
      typedef itk::Image< float, VImageDimension > KernelImageType;
      typedef typename KernelImageType::SizeType SizeType;
      SizeType maskSize;

      for(unsigned int i = 0; i < VImageDimension; ++i)
      {
        maskSize[i] = static_cast<int>( 2 * radiusInMM / spacing[i]);

        // We always want an uneven size to have a clear center point in the convolution mask
        if(maskSize[i] % 2 == 0 )
        {
          ++maskSize[i];
        }
      }
      return maskSize;
    }

    template <unsigned int VImageDimension>
    itk::SmartPointer< itk::Image<float, VImageDimension> >
      HotspotMaskGenerator::GenerateHotspotSearchConvolutionKernel(double mmPerPixel[VImageDimension],
                                                                    double radiusInMM )
    {
      std::stringstream ss;
      for (unsigned int i = 0; i < VImageDimension; ++i)
      {
        ss << mmPerPixel[i];
        if (i < VImageDimension -1)
          ss << ",";
      }
      MITK_DEBUG << "Update convolution kernel for spacing (" << ss.str() << ") and radius " << radiusInMM << "mm";


      double radiusInMMSquared = radiusInMM * radiusInMM;
      typedef itk::Image< float, VImageDimension > KernelImageType;
      typename KernelImageType::Pointer convolutionKernel = KernelImageType::New();

      // Calculate size and allocate mask image
      typedef typename KernelImageType::SizeType SizeType;
      SizeType maskSize = this->CalculateConvolutionKernelSize<VImageDimension>(mmPerPixel, radiusInMM);

      mitk::Point3D convolutionMaskCenterIndex;
      convolutionMaskCenterIndex.Fill(0.0);
      for(unsigned int i = 0; i < VImageDimension; ++i)
      {
        convolutionMaskCenterIndex[i] = 0.5 * (double)(maskSize[i]-1);
      }

      typedef typename KernelImageType::IndexType IndexType;
      IndexType maskIndex;
      maskIndex.Fill(0);

      typedef typename KernelImageType::RegionType RegionType;
      RegionType maskRegion;
      maskRegion.SetSize(maskSize);
      maskRegion.SetIndex(maskIndex);

      convolutionKernel->SetRegions(maskRegion);
      convolutionKernel->SetSpacing(mmPerPixel);
      convolutionKernel->Allocate();

      // Fill mask image values by subsampling the image grid
      typedef itk::ImageRegionIteratorWithIndex<KernelImageType> MaskIteratorType;
      MaskIteratorType maskIt(convolutionKernel,maskRegion);

      int numberOfSubVoxelsPerDimension = 2; // per dimension!
      int numberOfSubVoxels = ::pow( static_cast<float>(numberOfSubVoxelsPerDimension), static_cast<float>(VImageDimension) );
      double subVoxelSizeInPixels = 1.0 / (double)numberOfSubVoxelsPerDimension;
      double valueOfOneSubVoxel = 1.0 / (double)numberOfSubVoxels;
      mitk::Point3D subVoxelIndexPosition;
      double distanceSquared = 0.0;

      typedef itk::ContinuousIndex<double, VImageDimension> ContinuousIndexType;
      for(maskIt.GoToBegin(); !maskIt.IsAtEnd(); ++maskIt)
      {
        ContinuousIndexType indexPoint(maskIt.GetIndex());
        mitk::Point3D voxelPosition;
        for (unsigned int dimension = 0; dimension < VImageDimension; ++dimension)
        {
          voxelPosition[dimension] = indexPoint[dimension];
        }

        double maskValue = 0.0;
        mitk::Vector3D subVoxelOffset; subVoxelOffset.Fill(0.0);
        // iterate sub-voxels by iterating all possible offsets
        for (subVoxelOffset[0] = -0.5 + subVoxelSizeInPixels / 2.0;
          subVoxelOffset[0] < +0.5;
          subVoxelOffset[0] += subVoxelSizeInPixels)
        {
          for (subVoxelOffset[1] = -0.5 + subVoxelSizeInPixels / 2.0;
            subVoxelOffset[1] < +0.5;
            subVoxelOffset[1] += subVoxelSizeInPixels)
          {
            for (subVoxelOffset[2] = -0.5 + subVoxelSizeInPixels / 2.0;
              subVoxelOffset[2] < +0.5;
              subVoxelOffset[2] += subVoxelSizeInPixels)
            {
              subVoxelIndexPosition = voxelPosition + subVoxelOffset; // this COULD be integrated into the for-loops if necessary (add voxelPosition to initializer and end condition)
              distanceSquared =
                (subVoxelIndexPosition[0]-convolutionMaskCenterIndex[0]) * mmPerPixel[0] * (subVoxelIndexPosition[0]-convolutionMaskCenterIndex[0]) * mmPerPixel[0]
              + (subVoxelIndexPosition[1]-convolutionMaskCenterIndex[1]) * mmPerPixel[1] * (subVoxelIndexPosition[1]-convolutionMaskCenterIndex[1]) * mmPerPixel[1]
              + (subVoxelIndexPosition[2]-convolutionMaskCenterIndex[2]) * mmPerPixel[2] * (subVoxelIndexPosition[2]-convolutionMaskCenterIndex[2]) * mmPerPixel[2];

              if (distanceSquared <= radiusInMMSquared)
              {
                maskValue += valueOfOneSubVoxel;
              }
            }
          }
        }
        maskIt.Set( maskValue );
      }

      return convolutionKernel;
    }

    template <typename TPixel, unsigned int VImageDimension>
    itk::SmartPointer<itk::Image<TPixel, VImageDimension> >
      HotspotMaskGenerator::GenerateConvolutionImage( const itk::Image<TPixel, VImageDimension>* inputImage )
    {
      double mmPerPixel[VImageDimension];
      for (unsigned int dimension = 0; dimension < VImageDimension; ++dimension)
      {
        mmPerPixel[dimension] = inputImage->GetSpacing()[dimension];
      }

      // update convolution kernel
      typedef itk::Image< float, VImageDimension > KernelImageType;
      typename KernelImageType::Pointer convolutionKernel = this->GenerateHotspotSearchConvolutionKernel<VImageDimension>(mmPerPixel, m_HotspotRadiusInMM);

      // update convolution image
      typedef itk::Image< TPixel, VImageDimension > InputImageType;
      typedef itk::Image< TPixel, VImageDimension > ConvolutionImageType;
      typedef itk::FFTConvolutionImageFilter<InputImageType,
        KernelImageType,
        ConvolutionImageType> ConvolutionFilterType;

      itk::VnlFFTImageFilterInitFactory::RegisterFactories();

      typename ConvolutionFilterType::Pointer convolutionFilter = ConvolutionFilterType::New();
      typedef itk::ConstantBoundaryCondition<InputImageType, InputImageType> BoundaryConditionType;
      BoundaryConditionType boundaryCondition;
      boundaryCondition.SetConstant(0.0);

      if (m_HotspotMustBeCompletelyInsideImage)
      {
        // overwrite default boundary condition
        convolutionFilter->SetBoundaryCondition(&boundaryCondition);
      }

      convolutionFilter->SetInput(inputImage);
      convolutionFilter->SetKernelImage(convolutionKernel);
      convolutionFilter->SetNormalize(true);
      MITK_DEBUG << "Update Convolution image for hotspot search";
      convolutionFilter->UpdateLargestPossibleRegion();

      typename ConvolutionImageType::Pointer convolutionImage = convolutionFilter->GetOutput();
      convolutionImage->SetSpacing( inputImage->GetSpacing() ); // only workaround because convolution filter seems to ignore spacing of input image

      return convolutionImage;
    }

    template < typename TPixel, unsigned int VImageDimension>
    void
      HotspotMaskGenerator::FillHotspotMaskPixels( itk::Image<TPixel, VImageDimension>* maskImage,
                                                itk::Point<double, VImageDimension> sphereCenter,
                                                double sphereRadiusInMM )
    {
      typedef itk::Image< TPixel, VImageDimension > MaskImageType;
      typedef itk::ImageRegionIteratorWithIndex<MaskImageType> MaskImageIteratorType;

      MaskImageIteratorType maskIt(maskImage, maskImage->GetLargestPossibleRegion());

      typename MaskImageType::IndexType maskIndex;
      typename MaskImageType::PointType worldPosition;

      // this is not very smart. I would rather use a 0 initialized mask (not the case here -> blame CalculateHotspotMask) and find the region where I need to iterate over, then iterate only over the small region
      for(maskIt.GoToBegin(); !maskIt.IsAtEnd(); ++maskIt)
      {
        maskIndex = maskIt.GetIndex();
        maskImage->TransformIndexToPhysicalPoint(maskIndex, worldPosition);
        maskIt.Set( worldPosition.EuclideanDistanceTo(sphereCenter) <= sphereRadiusInMM ? 1 : 0 );
      }
    }

    template <typename TPixel, unsigned int VImageDimension>
    void
      HotspotMaskGenerator::CalculateHotspotMask(const itk::Image<TPixel, VImageDimension>* inputImage,
                                              const itk::Image<unsigned short, VImageDimension>* maskImage,
                                              unsigned int label)
    {
        typedef itk::Image< TPixel, VImageDimension > InputImageType;
        typedef itk::Image< TPixel, VImageDimension > ConvolutionImageType;
        typedef itk::Image< unsigned short, VImageDimension > MaskImageType;

        typename ConvolutionImageType::Pointer convolutionImage = this->GenerateConvolutionImage(inputImage);

        if (convolutionImage.IsNull())
        {
          MITK_ERROR << "Empty convolution image in CalculateHotspotStatistics(). We should never reach this state (logic error).";
          throw std::logic_error("Empty convolution image in CalculateHotspotStatistics()");
        }

        typename MaskImageType::ConstPointer usedMask = maskImage;
        // if mask image is not defined, create an image of the same size as inputImage and fill it with 1's
        // there is maybe a better way to do this!?
        if (maskImage == nullptr)
        {
            auto defaultMask = MaskImageType::New();
            typename MaskImageType::RegionType maskRegion = inputImage->GetLargestPossibleRegion();
            typename MaskImageType::SpacingType maskSpacing = inputImage->GetSpacing();
            typename MaskImageType::PointType maskOrigin = inputImage->GetOrigin();
            typename MaskImageType::DirectionType maskDirection = inputImage->GetDirection();
            defaultMask->SetRegions(maskRegion);
            defaultMask->Allocate();
            defaultMask->SetOrigin(maskOrigin);
            defaultMask->SetSpacing(maskSpacing);
            defaultMask->SetDirection(maskDirection);

            defaultMask->FillBuffer(1);

            usedMask = defaultMask;
            label = 1;
        }

        // find maximum in convolution image, given the current mask
        double requiredDistanceToBorder = m_HotspotMustBeCompletelyInsideImage ? m_HotspotRadiusInMM : -1.0;
        ImageExtrema convolutionImageInformation = CalculateExtremaWorld(convolutionImage.GetPointer(), usedMask.GetPointer(), requiredDistanceToBorder, label);

        bool isHotspotDefined = convolutionImageInformation.Defined;

        if (!isHotspotDefined)
        {
          MITK_ERROR << "No origin of hotspot-sphere was calculated!";
          m_InternalMask = nullptr;
        }
        else
        {
          // create a binary mask around the "hotspot" region, fill the shape of a sphere around our hotspot center
//          typename DuplicatorType::Pointer copyMachine = DuplicatorType::New();
//          copyMachine->SetInputImage(inputImage);
//          copyMachine->Update();

//          typename CastFilterType::Pointer caster = CastFilterType::New();
//          caster->SetInput( copyMachine->GetOutput() );
//          caster->Update();
          typename MaskImageType::Pointer hotspotMaskITK = MaskImageType::New();
          hotspotMaskITK->SetOrigin(inputImage->GetOrigin());
          hotspotMaskITK->SetSpacing(inputImage->GetSpacing());
          hotspotMaskITK->SetLargestPossibleRegion(inputImage->GetLargestPossibleRegion());
          hotspotMaskITK->SetBufferedRegion(inputImage->GetBufferedRegion());
          hotspotMaskITK->SetDirection(inputImage->GetDirection());
          hotspotMaskITK->SetNumberOfComponentsPerPixel(inputImage->GetNumberOfComponentsPerPixel());
          hotspotMaskITK->Allocate();
          hotspotMaskITK->FillBuffer(1);

          typedef typename InputImageType::IndexType IndexType;
          IndexType maskCenterIndex;
          for (unsigned int d =0; d< VImageDimension;++d)
          {
              maskCenterIndex[d]=convolutionImageInformation.MaxIndex[d];
          }

          typename ConvolutionImageType::PointType maskCenter;
          inputImage->TransformIndexToPhysicalPoint(maskCenterIndex,maskCenter);

          FillHotspotMaskPixels(hotspotMaskITK.GetPointer(), maskCenter, m_HotspotRadiusInMM);

          //obtain mitk::Image::Pointer from itk::Image
          mitk::Image::Pointer hotspotMaskAsMITKImage = mitk::GrabItkImageMemory(hotspotMaskITK);

          m_InternalMask = hotspotMaskAsMITKImage;
          m_ConvolutionImageMaxIndex = convolutionImageInformation.MaxIndex;
          m_ConvolutionImageMinIndex = convolutionImageInformation.MinIndex;
        }
    }

    bool HotspotMaskGenerator::IsUpdateRequired() const
    {
        unsigned long thisClassTimeStamp = this->GetMTime();
        unsigned long internalMaskTimeStamp = m_InternalMask->GetMTime();
        unsigned long maskGeneratorTimeStamp = m_Mask->GetMTime();
        unsigned long inputImageTimeStamp = m_InputImage->GetMTime();

        if (thisClassTimeStamp > m_InternalMaskUpdateTime) // inputs have changed
        {
            return true;
        }

        if (m_InternalMaskUpdateTime < maskGeneratorTimeStamp || m_InternalMaskUpdateTime < inputImageTimeStamp) // mask image has changed outside of this class
        {
            return true;
        }

        if (internalMaskTimeStamp > m_InternalMaskUpdateTime) // internal mask has been changed outside of this class
        {
            return true;
        }

        return false;
    }
}
