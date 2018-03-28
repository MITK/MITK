#ifndef mitkCLUtil_h
#define mitkCLUtil_h

#include <Eigen/Dense>
#include <MitkCLUtilitiesExports.h>
#include <itkImageRegionIterator.h>


#include <mitkImage.h>
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>

#include <itkConnectedComponentImageFilter.h>
namespace mitk
{

class MITKCLUTILITIES_EXPORT CLUtil
{
public:
  ///
  /// \brief The MorphologicalDimensions enum
  ///
  enum MorphologicalDimensions
  {
    Axial,Coronal,Sagital,All
  };

  ///
  /// \brief CreateCheckerBoardPredictionMask
  /// \param image
  /// \param outimage
  ///
  static void CreateCheckerboardMask(mitk::Image::Pointer image, mitk::Image::Pointer & outimage);

  ///
  /// \brief InterpolateCreateCheckerboardPrediction
  /// \param image
  /// \param outimage
  ///
  static void InterpolateCheckerboardPrediction(mitk::Image::Pointer checkerboard_prediction, mitk::Image::Pointer & checkerboard_mask, mitk::Image::Pointer & outimage);

  ///
  /// \brief CountVoxel
  /// \param image
  /// \param map
  ///
  static void CountVoxel(mitk::Image::Pointer image, std::map<unsigned int, unsigned int> & map);

  ///
  /// \brief CountVoxel
  /// \param image
  /// \param label
  /// \param count
  ///
  static void CountVoxel(mitk::Image::Pointer image, unsigned int label, unsigned int & count);

  ///
  /// \brief CountVoxel
  /// \param image
  /// \param count
  ///
  static void CountVoxel(mitk::Image::Pointer image, unsigned int & count);

  ///
  /// \brief SumVoxelForLabel
  /// \param image
  /// \param source
  /// \param label
  /// \param val
  ///
  static void SumVoxelForLabel(mitk::Image::Pointer image, const mitk::Image::Pointer & source , unsigned int label, double & val );

  ///
  /// \brief SqSumVoxelForLabel
  /// \param image
  /// \param source
  /// \param label
  /// \param val
  ///
  static void SqSumVoxelForLabel(mitk::Image::Pointer image, const mitk::Image::Pointer & source, unsigned int label, double & val );

  ///
  /// \brief LogicalAndImages
  /// \param image1
  /// \param image2
  ///
  static void LogicalAndImages(const Image::Pointer &image1, const Image::Pointer &image2, Image::Pointer &outimage);


  ///
  /// \brief GaussianFilter
  /// \param image
  /// \param smoothed
  /// \param sigma
  ///
  static void GaussianFilter(mitk::Image::Pointer image, mitk::Image::Pointer & smoothed ,double sigma);

  ///
  /// \brief SubtractGaussianFilter
  /// \param image
  /// \param smoothed (Result is sigma1-sigma2)
  /// \param sigma1
  /// \param sigma2
  ///
  static void DifferenceOfGaussianFilter(mitk::Image::Pointer image, mitk::Image::Pointer & smoothed, double sigma1, double sigma2);

  ///
  /// \brief Laplacian of Gaussian
  /// \param image
  /// \param smoothed (Result is sigma1-sigma2)
  /// \param sigma1
  /// \param sigma2
  ///
  static void LaplacianOfGaussianFilter(mitk::Image::Pointer image, mitk::Image::Pointer & smoothed, double sigma1);

  ///
  /// \brief SubtractGaussianFilter
  /// \param image
  /// \param smoothed (Result is sigma1-sigma2)
  /// \param sigma1
  /// \param sigma2
  ///
  static void HessianOfGaussianFilter(mitk::Image::Pointer image, std::vector<mitk::Image::Pointer> &out, double sigma);

  ///
  /// \brief Local Histogram
  /// \param image
  /// \param smoothed (Result is sigma1-sigma2)
  /// \param sigma1
  /// \param sigma2
  ///
  static void LocalHistogram(mitk::Image::Pointer image, std::vector<mitk::Image::Pointer> &out, int Bins, int NeighbourhoodSize);

  ///
  /// \brief transform
  /// \param matrix
  /// \param mask
  /// \param outimage
  ///
  template<typename TMatrixElementType>
  static mitk::Image::Pointer Transform(const Eigen::Matrix<TMatrixElementType, Eigen::Dynamic, Eigen::Dynamic> & matrix, const mitk::Image::Pointer & mask)
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
    mitk::CLUtil::CountVoxel(mask,n_numSamples);

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
  ///
  /// \brief TransformImageToMatrix
  /// \param in_img
  /// \param mask
  /// \param out_matrix
  ///
  template<typename TMatrixElementType>
  static Eigen::Matrix<TMatrixElementType, Eigen::Dynamic, Eigen::Dynamic> Transform(const mitk::Image::Pointer & img, const mitk::Image::Pointer & mask)
  {
    itk::Image<unsigned int, 3>::Pointer current_mask;
    mitk::CastToItkImage(mask,current_mask);

    unsigned int n_numSamples = 0;
    mitk::CLUtil::CountVoxel(mask,n_numSamples);

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

  ///
  /// \brief DilateBinary
  /// \param BinaryImage
  /// \param BinaryImage
  /// \param Size of the StructuringElement
  /// \param Dimension
  ///
  static void DilateBinary(mitk::Image::Pointer & sourceImage, mitk::Image::Pointer& resultImage, int radius , MorphologicalDimensions d);

  ///
  /// \brief ErodeBinary
  /// \param BinaryImage
  /// \param BinaryImage
  /// \param Size of the StructuringElement
  /// \param Dimension
  ///
  static void ErodeBinary(mitk::Image::Pointer & sourceImage, mitk::Image::Pointer& resultImage, int radius, MorphologicalDimensions d);

  ///
  /// \brief ClosingBinary
  /// \param BinaryImage
  /// \param BinaryImage
  /// \param Size of the StructuringElement
  /// \param Dimension
  ///
  static void ClosingBinary(mitk::Image::Pointer & sourceImage, mitk::Image::Pointer& resultImage, int radius, MorphologicalDimensions d);


  ///
  /// \brief MergeLabels
  /// \param MultilabelImage
  /// \param map merge instruction where each map entry defines a mapping instruction. Key <sourcelabel> - Value <targetlabel>
  ///
  static void MergeLabels(mitk::Image::Pointer & img, const std::map<unsigned int, unsigned int> & map);

  ///
  /// \brief ConnectedComponentsImage
  /// \param BinaryImage
  /// \param BinaryImage
  /// \param MultilabelImage
  /// \param Number of components found in the image
  ///
  static void ConnectedComponentsImage(mitk::Image::Pointer & image, mitk::Image::Pointer& mask, mitk::Image::Pointer &outimage, unsigned int& num_components);

  ///
  /// \brief GrabLabel
  /// \param MultiLabelImage
  /// \param outimage
  /// \param label
  ///
  static void GrabLabel(mitk::Image::Pointer & image, mitk::Image::Pointer & outimage, unsigned int label);


  ///
  /// \brief itkInsertLabel
  /// \param image
  /// \param maskImage
  /// \param label
  ///
  static void InsertLabel(mitk::Image::Pointer & image, mitk::Image::Pointer & maskImage, unsigned int label);

  ///
  /// \brief ErodeGrayscale
  /// \param image
  /// \param outimage
  /// \param radius
  /// \param d
  ///
  static void ErodeGrayscale(mitk::Image::Pointer & image, unsigned int radius, mitk::CLUtil::MorphologicalDimensions d, mitk::Image::Pointer & outimage );

  ///
  /// \brief DilateGrayscale
  /// \param image
  /// \param outimage
  /// \param radius
  /// \param d
  ///
  static void DilateGrayscale(mitk::Image::Pointer & image, unsigned int radius, mitk::CLUtil::MorphologicalDimensions d, mitk::Image::Pointer & outimage );

  ///
  /// \brief FillHoleGrayscale
  /// \param image
  /// \param outimage
  ///
  static void FillHoleGrayscale(mitk::Image::Pointer & image, mitk::Image::Pointer & outimage);

  ///
  /// \brief ProbabilityMap
  /// \param sourceImage
  /// \param mean
  /// \param std_dev
  /// \param resultImage
  ///
  static void ProbabilityMap(const mitk::Image::Pointer&  sourceImage, double mean, double std_dev, mitk::Image::Pointer& resultImage);

  template<class TImageType>
  static void itkCountVoxel( TImageType * image, std::map<unsigned int, unsigned int> & map)
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
  static void itkCountVoxel(TImageType* image, typename TImageType::PixelType label, unsigned int & count )
  {
    itk::ImageRegionConstIterator<TImageType> inputIter(image, image->GetLargestPossibleRegion());
    while(!inputIter.IsAtEnd())
    {
      if(inputIter.Value() == label) ++count;
      ++inputIter;
    }
  }

  template<typename TImageType>
  static inline void itkCountVoxel(TImageType * mask, unsigned int & n_numSamples)
  {
    auto mit = itk::ImageRegionConstIterator<TImageType>(mask, mask->GetLargestPossibleRegion());
    while (!mit.IsAtEnd())
    {
      if(mit.Value() > 0)
        n_numSamples++;
      ++mit;
    }
  }

  template <class TImageType1, class TImageType2>
  static void itkSampleLabel(TImageType1* image, TImageType2* output, double acceptrate, unsigned int label)
  {
    std::srand (time(nullptr));

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

  template <class TImageType>
  static void itkSampleLabel(TImageType* image, mitk::Image::Pointer & output, unsigned int n_samples_drawn)
  {
    std::srand (time(nullptr));

    typename TImageType::Pointer itk_out = TImageType::New();
    itk_out->SetRegions(image->GetLargestPossibleRegion());
    itk_out->SetDirection(image->GetDirection());
    itk_out->SetOrigin(image->GetOrigin());
    itk_out->SetSpacing(image->GetSpacing());
    itk_out->Allocate();
    itk_out->FillBuffer(0);

    itk::ImageRegionConstIterator< TImageType > inputIter(image, image->GetLargestPossibleRegion());
    itk::ImageRegionIterator< TImageType > outputIter(itk_out, itk_out->GetLargestPossibleRegion());

    for(unsigned int i = 0 ; i < n_samples_drawn ;)
    {
      double r = (double)(rand()) / RAND_MAX;
      if(inputIter.Value() != 0 && r < 0.01 && outputIter.Value() == 0)
      {
        outputIter.Set(inputIter.Value());
        i++;
      }
      ++inputIter;
      ++outputIter;

      if(inputIter.IsAtEnd())
      {
        inputIter.GoToBegin();
        outputIter.GoToBegin();
      }

    }

    mitk::CastToMitkImage(itk_out, output);
  }

private:

  template<class TImageType>
  static void itkErodeGrayscale(TImageType * image, mitk::Image::Pointer & outimage , unsigned int radius, mitk::CLUtil::MorphologicalDimensions d);

  template<class TImageType>
  static void itkDilateGrayscale(TImageType * image, mitk::Image::Pointer & outimage , unsigned int radius, mitk::CLUtil::MorphologicalDimensions d);

  template<class TImageType>
  static void itkFillHoleGrayscale(TImageType * image, mitk::Image::Pointer & outimage);

  template< typename TImageType >
  static void itkInsertLabel(TImageType * maskImage, mitk::Image::Pointer & outimage, unsigned int label)
  {

    typename TImageType::Pointer itk_out;

    if(outimage.IsNull()) // create if necessary
    {
      MITK_INFO << "Initialize new image";
      itk_out = TImageType::New();
      itk_out->SetSpacing(maskImage->GetSpacing());
      itk_out->SetDirection(maskImage->GetDirection());
      itk_out->SetOrigin(maskImage->GetOrigin());
      itk_out->SetRegions(maskImage->GetLargestPossibleRegion());
      itk_out->Allocate();
      itk_out->FillBuffer(0);
    }else
    {
      mitk::CastToItkImage(outimage, itk_out);
    }

    itk::ImageRegionIterator<TImageType> oit(itk_out,itk_out->GetLargestPossibleRegion());
    itk::ImageRegionConstIterator<TImageType> mit(maskImage,maskImage->GetLargestPossibleRegion());

    while(!mit.IsAtEnd())
    {
      if(mit.Value() != 0)
      {
        oit.Set(label);
      }
      ++oit;
      ++mit;
    }

    mitk::CastToMitkImage(itk_out,outimage);
  }


  template< typename TImageType >
  static void itkGrabLabel(TImageType * image, mitk::Image::Pointer & outimage, unsigned int label)
  {
    typedef itk::Image<unsigned short, 3> TOutType;
    TOutType::Pointer itk_out = TOutType::New();
    itk_out->SetRegions(image->GetLargestPossibleRegion());
    itk_out->SetDirection(image->GetDirection());
    itk_out->SetOrigin(image->GetOrigin());
    itk_out->SetSpacing(image->GetSpacing());
    itk_out->Allocate();

    itk::ImageRegionConstIterator<TImageType> iit(image, image->GetLargestPossibleRegion());
    itk::ImageRegionIterator<TOutType> oit(itk_out,itk_out->GetLargestPossibleRegion());

    while(!iit.IsAtEnd())
    {
      if(iit.Value() == static_cast<typename TImageType::PixelType>(label))
        oit.Set(1);
      else
        oit.Set(0);

      ++iit;
      ++oit;
    }

    mitk::CastToMitkImage(itk_out, outimage);
  }

  template<class TImagetype>
  static void itkMergeLabels(TImagetype * img, const std::map<unsigned int, unsigned int> & map)
  {

    auto it = itk::ImageRegionIterator<TImagetype>(img,img->GetLargestPossibleRegion());

    while(!it.IsAtEnd())
    {
      if(map.find(it.Value())!=map.end())
        it.Set( map.at(it.Value()) );
      ++it;
    }

  }

  template<typename TImageType>
  static void itkConnectedComponentsImage(TImageType * image, mitk::Image::Pointer& mask, mitk::Image::Pointer &outimage, unsigned int& num_components)
  {
    typedef itk::Image<unsigned short, 3> MaskImageType;
    MaskImageType::Pointer itk_mask;
    if(mask.IsNull())
    {
      itk_mask = MaskImageType::New();
      itk_mask->SetRegions(image->GetLargestPossibleRegion());
      itk_mask->SetDirection(image->GetDirection());
      itk_mask->SetOrigin(image->GetOrigin());
      itk_mask->SetSpacing(image->GetSpacing());
      itk_mask->Allocate();
      itk_mask->FillBuffer(1);
    }else{
      mitk::CastToItkImage(mask,itk_mask);
    }

    typedef itk::ConnectedComponentImageFilter<TImageType, MaskImageType, MaskImageType > FilterType;
    typename FilterType::Pointer cc_filter = FilterType::New();
    cc_filter->SetMaskImage(itk_mask.GetPointer());
    cc_filter->SetInput(image);
    cc_filter->SetBackgroundValue(0);
    cc_filter->Update();

    num_components = cc_filter->GetObjectCount();
    mitk::CastToMitkImage(cc_filter->GetOutput(), outimage);
  }

  template< typename TImageType >
  static void itkCreateCheckerboardMask(TImageType * image, mitk::Image::Pointer & outimage);

  template< typename TImageType >
  static void itkInterpolateCheckerboardPrediction(TImageType * checkerboard_prediction, mitk::Image::Pointer & checkerboard_mask, mitk::Image::Pointer & outimage);

  template <class TImageType>
  static void itkSumVoxelForLabel(TImageType* image, const mitk::Image::Pointer & source , typename TImageType::PixelType label, double & val );

  template <class TImageType>
  static void itkSqSumVoxelForLabel(TImageType* image, const mitk::Image::Pointer & source, typename TImageType::PixelType label, double & val );

  template<typename TStructuringElement>
  static void itkFitStructuringElement(TStructuringElement & se, MorphologicalDimensions d, int radius);

  template<typename TImageType>
  static void itkDilateBinary(TImageType * sourceImage, mitk::Image::Pointer& resultImage, int radius , MorphologicalDimensions d);

  template<typename TImageType>
  static void itkErodeBinary(TImageType * sourceImage, mitk::Image::Pointer& resultImage, int radius, MorphologicalDimensions d);

  template<typename TImageType>
  static void itkClosingBinary(TImageType * sourceImage, mitk::Image::Pointer& resultImage, int radius, MorphologicalDimensions d);

  template<typename TPixel, unsigned int VDimension>
  static void itkFillHolesBinary(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer& resultImage);

  template<typename TImageType>
  static void itkLogicalAndImages(const TImageType * image1, const mitk::Image::Pointer & image2, mitk::Image::Pointer & outimage);

  template<class TImageType>
  static void itkGaussianFilter(TImageType * image, mitk::Image::Pointer & smoothed ,double sigma);

  template<class TImageType>
  static void itkDifferenceOfGaussianFilter(TImageType * image, mitk::Image::Pointer & smoothed, double sigma1, double sigma2);

  template<typename TImageType>
  static void itkProbabilityMap(const TImageType * sourceImage, double mean, double std_dev, mitk::Image::Pointer& resultImage);

  template<typename TPixel, unsigned int VImageDimension>
  static void itkHessianOfGaussianFilter(itk::Image<TPixel, VImageDimension>* itkImage, double variance, std::vector<mitk::Image::Pointer> &out);

  template<typename TPixel, unsigned int VImageDimension>
  static void itkLaplacianOfGaussianFilter(itk::Image<TPixel, VImageDimension>* itkImage, double variance, mitk::Image::Pointer &output);

  template<typename TPixel, unsigned int VImageDimension>
  static void itkLocalHistograms(itk::Image<TPixel, VImageDimension>* itkImage, std::vector<mitk::Image::Pointer> &out, int size, int bins);
};

} //namespace MITK

#endif
