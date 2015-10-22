#ifndef _mitkCLUtil_HXX
#define _mitkCLUtil_HXX

#include <mitkCLUtil.h>

#include <mitkImageAccessByItk.h>



#include <Eigen/Dense>
#include <itkImage.h>

// itk includes
#include <itkCheckerBoardImageFilter.h>
#include <itkShapedNeighborhoodIterator.h>
// Morphologic Operations
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryDilateImageFilter.h>
#include <itkBinaryErodeImageFilter.h>
#include <itkBinaryFillholeImageFilter.h>
#include <itkBinaryMorphologicalClosingImageFilter.h>
#include <itkGrayscaleErodeImageFilter.h>
#include <itkGrayscaleDilateImageFilter.h>
#include <itkGrayscaleFillholeImageFilter.h>

// Image Filter
#include <itkDiscreteGaussianImageFilter.h>

void mitk::CLUtil::ProbabilityMap(const mitk::Image::Pointer & image , double mean, double stddev, mitk::Image::Pointer & outimage)
{
  AccessFixedDimensionByItk_3(image, mitk::CLUtil::itkProbabilityMap, 3, mean, stddev, outimage);
}

void mitk::CLUtil::ErodeGrayscale(mitk::Image::Pointer & image , unsigned int radius, mitk::CLUtil::MorphologicalDimensions d, mitk::Image::Pointer & outimage)
{
  AccessFixedDimensionByItk_3(image, mitk::CLUtil::itkErodeGrayscale, 3, outimage, radius, d);
}

void mitk::CLUtil::DilateGrayscale(mitk::Image::Pointer & image, unsigned int radius, mitk::CLUtil::MorphologicalDimensions d, mitk::Image::Pointer & outimage)
{
  AccessFixedDimensionByItk_3(image, mitk::CLUtil::itkDilateGrayscale, 3, outimage, radius, d);
}

void mitk::CLUtil::FillHoleGrayscale(mitk::Image::Pointer & image, mitk::Image::Pointer & outimage)
{
  AccessFixedDimensionByItk_1(image, mitk::CLUtil::itkFillHoleGrayscale, 3, outimage);
}

void mitk::CLUtil::InsertLabel(mitk::Image::Pointer & image, mitk::Image::Pointer & maskImage, unsigned int label)
{
  AccessByItk_2(image, mitk::CLUtil::itkInsertLabel, maskImage, label);
}

void mitk::CLUtil::GrabLabel(mitk::Image::Pointer & image, mitk::Image::Pointer & outimage, unsigned int label)
{
  AccessFixedDimensionByItk_2(image, mitk::CLUtil::itkGrabLabel, 3, outimage, label);
}

void mitk::CLUtil::ConnectedComponentsImage(mitk::Image::Pointer & image, mitk::Image::Pointer& mask, mitk::Image::Pointer &outimage, unsigned int& num_components)
{
  AccessFixedDimensionByItk_3(image, mitk::CLUtil::itkConnectedComponentsImage,3, mask, outimage, num_components);
}

void mitk::CLUtil::MergeLabels(mitk::Image::Pointer & img, const std::map<unsigned int, unsigned int> & map)
{
  AccessByItk_1(img, mitk::CLUtil::itkMergeLabels, map);
}

void mitk::CLUtil::CountVoxel(mitk::Image::Pointer image, std::map<unsigned int, unsigned int> & map)
{
  AccessByItk_1(image, mitk::CLUtil::itkCountVoxel, map);
}

void mitk::CLUtil::CountVoxel(mitk::Image::Pointer image, unsigned int label, unsigned int & count)
{
  AccessByItk_2(image, mitk::CLUtil::itkCountVoxel, label, count);
}

void mitk::CLUtil::CountVoxel(mitk::Image::Pointer image, unsigned int & count)
{
  AccessByItk_1(image, mitk::CLUtil::itkCountVoxel, count);
}

void mitk::CLUtil::CreateCheckerboardMask(mitk::Image::Pointer image, mitk::Image::Pointer & outimage)
{
  AccessFixedDimensionByItk_1(image, mitk::CLUtil::itkCreateCheckerboardMask,3, outimage);
}

void mitk::CLUtil::LogicalAndImages(const mitk::Image::Pointer & image1, const mitk::Image::Pointer & image2, mitk::Image::Pointer & outimage)
{
  AccessFixedDimensionByItk_2(image1,itkLogicalAndImages, 3, image2, outimage);

}

void mitk::CLUtil::InterpolateCheckerboardPrediction(mitk::Image::Pointer checkerboard_prediction, mitk::Image::Pointer & checkerboard_mask, mitk::Image::Pointer & outimage)
{
  AccessFixedDimensionByItk_2(checkerboard_prediction, mitk::CLUtil::itkInterpolateCheckerboardPrediction,3, checkerboard_mask, outimage);
}

void mitk::CLUtil::GaussianFilter(mitk::Image::Pointer image, mitk::Image::Pointer & smoothed ,double sigma)
{
  AccessFixedDimensionByItk_2(image, mitk::CLUtil::itkGaussianFilter,3, smoothed, sigma);
}


void mitk::CLUtil::DilateBinary(mitk::Image::Pointer & sourceImage, mitk::Image::Pointer& resultImage, int factor , MorphologicalDimensions d)
{
  AccessFixedDimensionByItk_3(sourceImage, mitk::CLUtil::itkDilateBinary, 3, resultImage, factor, d);
}


void mitk::CLUtil::ErodeBinary(mitk::Image::Pointer & sourceImage, mitk::Image::Pointer& resultImage, int factor, MorphologicalDimensions d)
{
  AccessFixedDimensionByItk_3(sourceImage, mitk::CLUtil::itkErodeBinary, 3, resultImage, factor, d);
}


void mitk::CLUtil::ClosingBinary(mitk::Image::Pointer & sourceImage, mitk::Image::Pointer& resultImage, int factor, MorphologicalDimensions d)
{
  AccessFixedDimensionByItk_3(sourceImage, mitk::CLUtil::itkClosingBinary, 3, resultImage, factor, d);
}

template<typename TImageType>
void mitk::CLUtil::itkProbabilityMap(const TImageType * sourceImage, double mean, double std_dev, mitk::Image::Pointer& resultImage)
{
  itk::Image<double, 3>::Pointer itk_img = itk::Image<double, 3>::New();
  itk_img->SetRegions(sourceImage->GetLargestPossibleRegion());
  itk_img->SetOrigin(sourceImage->GetOrigin());
  itk_img->SetSpacing(sourceImage->GetSpacing());
  itk_img->SetDirection(sourceImage->GetDirection());
  itk_img->Allocate();


  itk::ImageRegionConstIterator<TImageType> it(sourceImage,sourceImage->GetLargestPossibleRegion());
  itk::ImageRegionIterator<itk::Image<double, 3> > outit(itk_img,itk_img->GetLargestPossibleRegion());

  while(!it.IsAtEnd())
  {
    double x = it.Value();

    double prob = (1.0/(std_dev*std::sqrt(2.0*M_PI))) * std::exp(-(((x-mean)*(x-mean))/(2.0*std_dev*std_dev)));
    outit.Set(prob);
    ++it;
    ++outit;
  }

  mitk::CastToMitkImage(itk_img, resultImage);
}

template< typename TImageType >
void mitk::CLUtil::itkInterpolateCheckerboardPrediction(TImageType * checkerboard_prediction, Image::Pointer &checkerboard_mask, mitk::Image::Pointer & outimage)
{
  typename TImageType::Pointer itk_checkerboard_mask;
  mitk::CastToItkImage(checkerboard_mask,itk_checkerboard_mask);

  typename TImageType::Pointer itk_outimage = TImageType::New();
  itk_outimage->SetRegions(checkerboard_prediction->GetLargestPossibleRegion());
  itk_outimage->SetDirection(checkerboard_prediction->GetDirection());
  itk_outimage->SetOrigin(checkerboard_prediction->GetOrigin());
  itk_outimage->SetSpacing(checkerboard_prediction->GetSpacing());
  itk_outimage->Allocate();
  itk_outimage->FillBuffer(0);

  //typedef typename itk::ShapedNeighborhoodIterator<TImageType>::SizeType SizeType;
  typedef itk::Size<3> SizeType;
  SizeType size;
  size.Fill(1);
  itk::ShapedNeighborhoodIterator<TImageType> iit(size,checkerboard_prediction,checkerboard_prediction->GetLargestPossibleRegion());
  itk::ShapedNeighborhoodIterator<TImageType> mit(size,itk_checkerboard_mask,itk_checkerboard_mask->GetLargestPossibleRegion());
  itk::ImageRegionIterator<TImageType> oit(itk_outimage,itk_outimage->GetLargestPossibleRegion());

  typedef typename itk::ShapedNeighborhoodIterator<TImageType>::OffsetType OffsetType;
  OffsetType offset;
  offset.Fill(0);
  offset[0] = 1;       // {1,0,0}
  iit.ActivateOffset(offset);
  mit.ActivateOffset(offset);
  offset[0] = -1;      // {-1,0,0}
  iit.ActivateOffset(offset);
  mit.ActivateOffset(offset);
  offset[0] = 0; offset[1] = 1; //{0,1,0}
  iit.ActivateOffset(offset);
  mit.ActivateOffset(offset);
  offset[1] = -1;      //{0,-1,0}
  iit.ActivateOffset(offset);
  mit.ActivateOffset(offset);

  //    iit.ActivateOffset({{0,0,1}});
  //    iit.ActivateOffset({{0,0,-1}});
  //    mit.ActivateOffset({{0,0,1}});
  //    mit.ActivateOffset({{0,0,-1}});

  while(!iit.IsAtEnd())
  {
    if(mit.GetCenterPixel() == 0)
    {
      typename TImageType::PixelType mean = 0;
      for (auto i = iit.Begin(); ! i.IsAtEnd(); i++)
      { mean += i.Get(); }


      //std::sort(list.begin(),list.end(),[](const typename TImageType::PixelType x,const typename TImageType::PixelType y){return x<=y;});

      oit.Set((mean+0.5)/6.0);
    }
    else
    {
      oit.Set(iit.GetCenterPixel());
    }
    ++iit;
    ++mit;
    ++oit;
  }

  mitk::CastToMitkImage(itk_outimage,outimage);
}

template< typename TImageType >
void mitk::CLUtil::itkCreateCheckerboardMask(TImageType * image, mitk::Image::Pointer & outimage)
{
  typename TImageType::Pointer zeroimg = TImageType::New();
  zeroimg->SetRegions(image->GetLargestPossibleRegion());
  zeroimg->SetDirection(image->GetDirection());
  zeroimg->SetOrigin(image->GetOrigin());
  zeroimg->SetSpacing(image->GetSpacing());

  zeroimg->Allocate();
  zeroimg->FillBuffer(0);

  typedef itk::CheckerBoardImageFilter<TImageType> FilterType;
  typename FilterType::Pointer filter = FilterType::New();
  filter->SetInput1(image);
  filter->SetInput2(zeroimg);
  typename FilterType::PatternArrayType pattern;
  pattern.SetElement(0,(image->GetLargestPossibleRegion().GetSize()[0]));
  pattern.SetElement(1,(image->GetLargestPossibleRegion().GetSize()[1]));
  pattern.SetElement(2,(image->GetLargestPossibleRegion().GetSize()[2]));
  filter->SetCheckerPattern(pattern);

  filter->Update();
  mitk::CastToMitkImage(filter->GetOutput(), outimage);
}


template <class TImageType>
void mitk::CLUtil::itkSumVoxelForLabel(TImageType* image, const mitk::Image::Pointer & source , typename TImageType::PixelType label, double & val )
{
  itk::Image<double,3>::Pointer itk_source;
  mitk::CastToItkImage(source,itk_source);

  itk::ImageRegionConstIterator<TImageType> inputIter(image, image->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator< itk::Image<double,3> > sourceIter(itk_source, itk_source->GetLargestPossibleRegion());
  while(!inputIter.IsAtEnd())
  {
    if(inputIter.Value() == label) val += sourceIter.Value();
    ++inputIter;
    ++sourceIter;
  }
}

template <class TImageType>
void mitk::CLUtil::itkSqSumVoxelForLabel(TImageType* image, const mitk::Image::Pointer & source, typename TImageType::PixelType label, double & val )
{
  itk::Image<double,3>::Pointer itk_source;
  mitk::CastToItkImage(source,itk_source);

  itk::ImageRegionConstIterator<TImageType> inputIter(image, image->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator< itk::Image<double,3> > sourceIter(itk_source, itk_source->GetLargestPossibleRegion());
  while(!inputIter.IsAtEnd())
  {
    if(inputIter.Value() == label) val += sourceIter.Value() * sourceIter.Value();
    ++inputIter;
    ++sourceIter;
  }
}

template<typename TStructuringElement>
void mitk::CLUtil::itkFitStructuringElement(TStructuringElement & se, MorphologicalDimensions d, int factor)
{
  typename TStructuringElement::SizeType size;
  size.Fill(factor);
  switch(d)
  {
  case(All):
  case(Axial):
    size.SetElement(2,0);
    break;
  case(Sagital):
    size.SetElement(0,0);
    break;
  case(Coronal):
    size.SetElement(1,0);
    break;
  }
  se.SetRadius(size);
  se.CreateStructuringElement();
}

template<typename TImageType>
void mitk::CLUtil::itkClosingBinary(TImageType * sourceImage, mitk::Image::Pointer& resultImage, int factor, MorphologicalDimensions d)
{
  typedef itk::BinaryBallStructuringElement<typename TImageType::PixelType, 3> BallType;
  typedef itk::BinaryMorphologicalClosingImageFilter<TImageType, TImageType, BallType> FilterType;

  BallType strElem;
  itkFitStructuringElement(strElem,d,factor);

  typename FilterType::Pointer erodeFilter = FilterType::New();
  erodeFilter->SetKernel(strElem);
  erodeFilter->SetInput(sourceImage);
  erodeFilter->SetForegroundValue(1);
  erodeFilter->Update();

  mitk::CastToMitkImage(erodeFilter->GetOutput(), resultImage);

}

template<typename TImageType>
void mitk::CLUtil::itkDilateBinary(TImageType * sourceImage, mitk::Image::Pointer& resultImage, int factor, MorphologicalDimensions d)
{
  typedef itk::BinaryBallStructuringElement<typename TImageType::PixelType, 3> BallType;
  typedef typename itk::BinaryDilateImageFilter<TImageType, TImageType, BallType> BallDilateFilterType;

  BallType strElem;
  itkFitStructuringElement(strElem,d,factor);

  typename BallDilateFilterType::Pointer erodeFilter = BallDilateFilterType::New();
  erodeFilter->SetKernel(strElem);
  erodeFilter->SetInput(sourceImage);
  erodeFilter->SetDilateValue(1);
  erodeFilter->Update();

  mitk::CastToMitkImage(erodeFilter->GetOutput(), resultImage);

}

template<typename TImageType>
void mitk::CLUtil::itkErodeBinary(TImageType * sourceImage, mitk::Image::Pointer& resultImage, int factor, MorphologicalDimensions d)
{
  typedef itk::BinaryBallStructuringElement<typename TImageType::PixelType, 3> BallType;
  typedef typename itk::BinaryErodeImageFilter<TImageType, TImageType, BallType> BallErodeFilterType;

  BallType strElem;
  itkFitStructuringElement(strElem,d,factor);


  typename BallErodeFilterType::Pointer erodeFilter = BallErodeFilterType::New();
  erodeFilter->SetKernel(strElem);
  erodeFilter->SetInput(sourceImage);
  erodeFilter->SetErodeValue(1);
//  erodeFilter->UpdateLargestPossibleRegion();
  erodeFilter->Update();

  mitk::CastToMitkImage(erodeFilter->GetOutput(), resultImage);

}

///
/// \brief itkFillHolesBinary
/// \param sourceImage
/// \param resultImage
///
template<typename TPixel, unsigned int VDimension>
void mitk::CLUtil::itkFillHolesBinary(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer& resultImage)
{
  typedef itk::Image<TPixel, VDimension> ImageType;
  typedef typename itk::BinaryFillholeImageFilter<ImageType> FillHoleFilterType;

  typename FillHoleFilterType::Pointer fillHoleFilter = FillHoleFilterType::New();
  fillHoleFilter->SetInput(sourceImage);
  fillHoleFilter->SetForegroundValue(1);
  fillHoleFilter->Update();

  mitk::CastToMitkImage(fillHoleFilter->GetOutput(), resultImage);
}

///
/// \brief itkLogicalAndImages
/// \param image1 keep the values of image 1
/// \param image2
///
template<typename TImageType>
void mitk::CLUtil::itkLogicalAndImages(const TImageType * image1, const mitk::Image::Pointer & image2, mitk::Image::Pointer & outimage)
{

  typename TImageType::Pointer itk_outimage = TImageType::New();
  itk_outimage->SetRegions(image1->GetLargestPossibleRegion());
  itk_outimage->SetDirection(image1->GetDirection());
  itk_outimage->SetOrigin(image1->GetOrigin());
  itk_outimage->SetSpacing(image1->GetSpacing());

  itk_outimage->Allocate();
  itk_outimage->FillBuffer(0);

  typename TImageType::Pointer itk_image2;
  mitk::CastToItkImage(image2,itk_image2);

  itk::ImageRegionConstIterator<TImageType> it1(image1, image1->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator<TImageType> it2(itk_image2, itk_image2->GetLargestPossibleRegion());
  itk::ImageRegionIterator<TImageType> oit(itk_outimage,itk_outimage->GetLargestPossibleRegion());

  while(!it1.IsAtEnd())
  {
    if(it1.Value() == 0 || it2.Value() == 0)
    {
      oit.Set(0);
    }else
      oit.Set(it1.Value());
    ++it1;
    ++it2;
    ++oit;
  }

  mitk::CastToMitkImage(itk_outimage, outimage);
}

///
/// \brief GaussianFilter
/// \param image
/// \param smoothed
/// \param sigma
///
template<class TImageType>
void mitk::CLUtil::itkGaussianFilter(TImageType * image, mitk::Image::Pointer & smoothed ,double sigma)
{
  typedef itk::DiscreteGaussianImageFilter<TImageType,TImageType> FilterType;
  typename FilterType::Pointer filter = FilterType::New();
  filter->SetInput(image);
  filter->SetVariance(sigma);
  filter->Update();

  mitk::CastToMitkImage(filter->GetOutput(),smoothed);
}

template<class TImageType>
void mitk::CLUtil::itkErodeGrayscale(TImageType * image, mitk::Image::Pointer & outimage , unsigned int radius, mitk::CLUtil::MorphologicalDimensions d)
{
  typedef itk::BinaryBallStructuringElement<typename TImageType::PixelType, 3>          StructureElementType;
  typedef itk::GrayscaleErodeImageFilter<TImageType,TImageType,StructureElementType>    FilterType;

  StructureElementType ball;
  itkFitStructuringElement(ball,d, radius);

  typename FilterType::Pointer filter = FilterType::New();
  filter->SetKernel(ball);
  filter->SetInput(image);
  filter->Update();

  mitk::CastToMitkImage(filter->GetOutput(),outimage);
}

template<class TImageType>
void mitk::CLUtil::itkDilateGrayscale(TImageType * image, mitk::Image::Pointer & outimage , unsigned int radius, mitk::CLUtil::MorphologicalDimensions d)
{
  typedef itk::BinaryBallStructuringElement<typename TImageType::PixelType, 3>          StructureElementType;
  typedef itk::GrayscaleDilateImageFilter<TImageType,TImageType,StructureElementType>    FilterType;

  StructureElementType ball;
  itkFitStructuringElement(ball,d, radius);

  typename FilterType::Pointer filter = FilterType::New();
  filter->SetKernel(ball);
  filter->SetInput(image);
  filter->Update();

  mitk::CastToMitkImage(filter->GetOutput(),outimage);
}

template<class TImageType>
void mitk::CLUtil::itkFillHoleGrayscale(TImageType * image, mitk::Image::Pointer & outimage)
{
  typedef itk::GrayscaleFillholeImageFilter<TImageType,TImageType>    FilterType;

  typename FilterType::Pointer filter = FilterType::New();
  filter->SetInput(image);
  filter->Update();

  mitk::CastToMitkImage(filter->GetOutput(),outimage);
}


#endif
