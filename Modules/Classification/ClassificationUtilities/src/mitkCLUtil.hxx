#ifndef _mitkCLUtil_HXX
#define _mitkCLUtil_HXX

#include <mitkCLUtil.h>
#include <itkImageRegionIterator.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>

#include <Eigen/Dense>
#include <itkImage.h>

// itk includes
// Morphologic Operations
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryDilateImageFilter.h>
#include <itkBinaryErodeImageFilter.h>
#include <itkBinaryFillholeImageFilter.h>

// Image Filter
#include <itkDiscreteGaussianImageFilter.h>


template<class TImageType>
void mitk::CLUtil::CountSamplesPerLabels( TImageType * image, std::map<unsigned int, unsigned int> & map)
{
    auto it = itk::ImageRegionIterator< TImageType >(image,image->GetLargestPossibleRegion());
    while(!it.IsAtEnd())
    {
        if(map.find(it.Value()) == map.end())
            map[it.Value()] = 0;
        map[it.Value()]++;
        ++it;
    }
}


template <class TImageType>
void mitk::CLUtil::CountVoxelForLabel(TImageType* image, typename TImageType::PixelType label, unsigned int & count )
{
    itk::ImageRegionConstIterator<TImageType> inputIter(image, image->GetLargestPossibleRegion());
    while(!inputIter.IsAtEnd())
    {
        if(inputIter.Value() == label) ++count;
        ++inputIter;
    }
}

template <class TImageType>
void mitk::CLUtil::SumVoxelForLabel(TImageType* image, const mitk::Image::Pointer & source , typename TImageType::PixelType label, double & val )
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
void mitk::CLUtil::SqSumVoxelForLabel(TImageType* image, const mitk::Image::Pointer & source, typename TImageType::PixelType label, double & val )
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


template <class TImageType1, class TImageType2>
void mitk::CLUtil::SampleLabel(TImageType1* image, TImageType2* output, double acceptrate, unsigned int label)
{

    std::srand (time(NULL));

    itk::ImageRegionConstIterator< TImageType1 > inputIter(image, image->GetLargestPossibleRegion());
    itk::ImageRegionIterator< TImageType2 > outputIter(output, output->GetLargestPossibleRegion());

    while (!inputIter.IsAtEnd())
    {
        double r = (double)(rand()) / RAND_MAX;
        if(inputIter.Get() == label && r < acceptrate)
            outputIter.Set(label);

        ++inputIter;
        ++outputIter;
    }
}



template<typename TImageType>
void mitk::CLUtil::CountLabledVoxels(TImageType * mask, unsigned int & n_numSamples)
{
    auto mit = itk::ImageRegionConstIterator<TImageType>(mask, mask->GetLargestPossibleRegion());
    while (!mit.IsAtEnd())
    {
        if(mit.Value() > 0)
            n_numSamples++;
        ++mit;
    }
}



template<typename TMatrixElementType>
mitk::Image::Pointer mitk::CLUtil::Transform(const Eigen::Matrix<TMatrixElementType, Eigen::Dynamic, Eigen::Dynamic> & matrix, const mitk::Image::Pointer & mask)
{
    itk::Image<unsigned int, 3>::Pointer itkMask;
    mitk::CastToItkImage(mask,itkMask);

    typename itk::Image<TMatrixElementType, 3>::Pointer itk_img = itk::Image<TMatrixElementType, 3>::New();
    itk_img->SetRegions(itkMask->GetLargestPossibleRegion());
    itk_img->SetOrigin(itkMask->GetOrigin());
    itk_img->SetSpacing(itkMask->GetSpacing());
    itk_img->SetDirection(itkMask->GetDirection());
    itk_img->Allocate();


    unsigned int n_numSamples = 0;
    mitk::CLUtil::CountLabledVoxels(itkMask.GetPointer(),n_numSamples);

    if(n_numSamples != matrix.rows())
        MITK_ERROR << "Number of samples in matrix and number of points under the masks is not the same!";

    auto mit = itk::ImageRegionConstIterator<itk::Image<unsigned int, 3> >(itkMask, itkMask->GetLargestPossibleRegion());
    auto oit = itk::ImageRegionIterator<itk::Image<TMatrixElementType, 3> >(itk_img, itk_img->GetLargestPossibleRegion());

    unsigned int current_row = 0;
    while(!mit.IsAtEnd())
    {
        if(mit.Value() > 0)
            oit.Set(matrix(current_row++,0));
        else
            oit.Set(0.0);
        ++mit;
        ++oit;
    }

    mitk::Image::Pointer out_img = mitk::Image::New();
    mitk::GrabItkImageMemory(itk_img,out_img);
    return out_img;
}



template<typename TMatrixElementType>
Eigen::Matrix<TMatrixElementType, Eigen::Dynamic, Eigen::Dynamic> mitk::CLUtil::Transform(const mitk::Image::Pointer & img, const mitk::Image::Pointer & mask)
{
    itk::Image<unsigned int, 3>::Pointer current_mask;
    mitk::CastToItkImage(mask,current_mask);

    unsigned int n_numSamples = 0;
    mitk::CLUtil::CountLabledVoxels(current_mask.GetPointer(),n_numSamples);

    typename itk::Image<TMatrixElementType, 3>::Pointer current_img;
    mitk::CastToItkImage(img,current_img);

    Eigen::Matrix<TMatrixElementType, Eigen::Dynamic, Eigen::Dynamic> out_matrix(n_numSamples,1);

    auto mit = itk::ImageRegionConstIterator<itk::Image<unsigned int, 3> >(current_mask, current_mask->GetLargestPossibleRegion());
    auto iit = itk::ImageRegionConstIterator<itk::Image<TMatrixElementType, 3> >(current_img,current_img->GetLargestPossibleRegion());
    unsigned int current_row = 0;
    while (!mit.IsAtEnd()) {
        if(mit.Value() > 0)
            out_matrix(current_row++) = iit.Value();
        ++mit;
        ++iit;
    }

    return out_matrix;
}

template<typename TImageType>
void mitk::CLUtil::itkDilateBinary(TImageType * sourceImage, mitk::Image::Pointer& resultImage, int factor)
{
    typedef TImageType ImageType;
    typedef itk::BinaryBallStructuringElement<typename TImageType::PixelType, 3> BallType;
    typedef typename itk::BinaryDilateImageFilter<TImageType, TImageType, BallType> BallDilateFilterType;

    BallType strElem;
    typename BallType::SizeType size;
    size.Fill(0);
    size.SetElement(0,factor);
    size.SetElement(1,factor);

    strElem.SetRadius(size);
    strElem.CreateStructuringElement();


    typename BallDilateFilterType::Pointer erodeFilter = BallDilateFilterType::New();
    erodeFilter->SetKernel(strElem);
    erodeFilter->SetInput(sourceImage);
    erodeFilter->SetDilateValue(1);
    erodeFilter->UpdateLargestPossibleRegion();

    mitk::CastToMitkImage(erodeFilter->GetOutput(), resultImage);

}

template<typename TImageType>
void mitk::CLUtil::itkErodeBinary(TImageType * sourceImage, mitk::Image::Pointer& resultImage, int factor)
{
    typedef TImageType ImageType;
    typedef itk::BinaryBallStructuringElement<typename TImageType::PixelType, 3> BallType;
    typedef typename itk::BinaryErodeImageFilter<TImageType, TImageType, BallType> BallErodeFilterType;

    BallType strElem;
    typename BallType::SizeType size;
    size.Fill(0);
    size.SetElement(0,factor);
    size.SetElement(1,factor);

    strElem.SetRadius(size);
    strElem.CreateStructuringElement();


    typename BallErodeFilterType::Pointer erodeFilter = BallErodeFilterType::New();
    erodeFilter->SetKernel(strElem);
    erodeFilter->SetInput(sourceImage);
    erodeFilter->SetErodeValue(1);
    erodeFilter->UpdateLargestPossibleRegion();

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
    fillHoleFilter->UpdateLargestPossibleRegion();

    mitk::CastToMitkImage(fillHoleFilter->GetOutput(), resultImage);
}

///
/// \brief itkLogicalAndImages
/// \param image1
/// \param image2
///
template<typename TImageType1, typename TImageType2>
void mitk::CLUtil::itkLogicalAndImages(TImageType1 * image1, TImageType2 * image2)
{
    itk::ImageRegionIterator<TImageType1> it1(image1, image1->GetLargestPossibleRegion());
    itk::ImageRegionIterator<TImageType2> it2(image2, image2->GetLargestPossibleRegion());
    while(!it1.IsAtEnd())
    {
        if(it1.Value() == 0 || it2.Value() == 0)
        {
            it1.Set(0);
            it2.Set(0);
        }
        ++it1;
        ++it2;
    }
}

///
/// \brief GaussianFilter
/// \param image
/// \param smoothed
/// \param sigma
///
template<class TImageType>
void mitk::CLUtil::GaussianFilter(TImageType * image, mitk::Image::Pointer & smoothed ,double sigma)
{
    typedef itk::DiscreteGaussianImageFilter<TImageType,TImageType> FilterType;
    typename FilterType::Pointer filter = FilterType::New();
    filter->SetInput(image);
    filter->SetVariance(sigma);
    filter->Update();

    mitk::CastToMitkImage(filter->GetOutput(),smoothed);
}


#endif
