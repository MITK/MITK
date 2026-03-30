/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCLUtil_h
#define mitkCLUtil_h

#include <itkeigen/Eigen/Dense>
#include <MitkCLUtilitiesExports.h>
#include <itkImageRegionIterator.h>


#include <mitkImage.h>
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>

#include <itkConnectedComponentImageFilter.h>
namespace mitk
{

/**
 * \brief Collection of static utility functions for classification and image processing.
 *
 * CLUtil provides convenience methods for voxel counting, morphological operations,
 * image filtering (Gaussian, DoG, LoG, Hessian), label manipulation, and conversion
 * between Eigen matrices and MITK images.
 *
 * \sa AbstractGlobalImageFeature
 * \sa AbstractClassifier
 */
class MITKCLUTILITIES_EXPORT CLUtil
{
public:
  /**
   * \brief Specifies the anatomical plane for morphological operations.
   */
  enum MorphologicalDimensions
  {
    Axial,    ///< Axial (transverse) plane.
    Coronal,  ///< Coronal (frontal) plane.
    Sagittal, ///< Sagittal plane.
    All       ///< All three dimensions (3D operation).
  };

  /**
   * \brief Create a checkerboard mask from the given image.
   *
   * \param[in] image The input image.
   * \param[out] outimage The resulting checkerboard mask.
   */
  static void CreateCheckerboardMask(mitk::Image::Pointer image, mitk::Image::Pointer & outimage);

  /**
   * \brief Interpolate a prediction using a checkerboard pattern.
   *
   * \param[in] checkerboard_prediction The prediction computed on the checkerboard pattern.
   * \param[in,out] checkerboard_mask The checkerboard mask.
   * \param[out] outimage The interpolated full prediction.
   */
  static void InterpolateCheckerboardPrediction(mitk::Image::Pointer checkerboard_prediction, mitk::Image::Pointer & checkerboard_mask, mitk::Image::Pointer & outimage);

  /**
   * \brief Count voxels per label in an image.
   *
   * \param[in] image The label image.
   * \param[out] map Map from label value to voxel count.
   */
  static void CountVoxel(mitk::Image::Pointer image, std::map<unsigned int, unsigned int> & map);

  /**
   * \brief Count the number of voxels with a specific label value.
   *
   * \param[in] image The label image.
   * \param[in] label The label value to count.
   * \param[out] count The number of voxels with the given label.
   */
  static void CountVoxel(mitk::Image::Pointer image, unsigned int label, unsigned int & count);

  /**
   * \brief Count the total number of non-zero voxels.
   *
   * \param[in] image The input image.
   * \param[out] count The total count of voxels with value > 0.
   */
  static void CountVoxel(mitk::Image::Pointer image, unsigned int & count);

  /**
   * \brief Sum intensity values from a source image for voxels matching a given label.
   *
   * \param[in] image The label image.
   * \param[in] source The intensity source image.
   * \param[in] label The label value to select voxels.
   * \param[out] val The sum of intensities for the selected voxels.
   */
  static void SumVoxelForLabel(mitk::Image::Pointer image, const mitk::Image::Pointer & source , unsigned int label, double & val );

  /**
   * \brief Sum squared intensity values from a source image for voxels matching a given label.
   *
   * \param[in] image The label image.
   * \param[in] source The intensity source image.
   * \param[in] label The label value to select voxels.
   * \param[out] val The sum of squared intensities for the selected voxels.
   */
  static void SqSumVoxelForLabel(mitk::Image::Pointer image, const mitk::Image::Pointer & source, unsigned int label, double & val );

  /**
   * \brief Compute the logical AND of two binary images.
   *
   * \param[in] image1 First binary image.
   * \param[in] image2 Second binary image.
   * \param[out] outimage The resulting AND image.
   */
  static void LogicalAndImages(const Image::Pointer &image1, const Image::Pointer &image2, Image::Pointer &outimage);

  /**
   * \brief Apply a Gaussian smoothing filter to an image.
   *
   * \param[in] image The input image.
   * \param[out] smoothed The smoothed output image.
   * \param[in] sigma Standard deviation of the Gaussian kernel.
   */
  static void GaussianFilter(mitk::Image::Pointer image, mitk::Image::Pointer & smoothed ,double sigma);

  /**
   * \brief Compute the Difference of Gaussians (DoG) of an image.
   *
   * The result is the difference between two Gaussian-smoothed images with sigmas sigma1 and sigma2.
   *
   * \param[in] image The input image.
   * \param[out] smoothed The DoG output image.
   * \param[in] sigma1 Standard deviation of the first Gaussian.
   * \param[in] sigma2 Standard deviation of the second Gaussian.
   */
  static void DifferenceOfGaussianFilter(mitk::Image::Pointer image, mitk::Image::Pointer & smoothed, double sigma1, double sigma2);

  /**
   * \brief Compute the Laplacian of Gaussian (LoG) of an image.
   *
   * \param[in] image The input image.
   * \param[out] smoothed The LoG output image.
   * \param[in] sigma1 Standard deviation of the Gaussian.
   */
  static void LaplacianOfGaussianFilter(mitk::Image::Pointer image, mitk::Image::Pointer & smoothed, double sigma1);

  /**
   * \brief Compute the Hessian of Gaussian for an image.
   *
   * Produces multiple output images corresponding to the Hessian matrix components.
   *
   * \param[in] image The input image.
   * \param[out] out Vector of output images for each Hessian component.
   * \param[in] sigma Standard deviation of the Gaussian.
   */
  static void HessianOfGaussianFilter(mitk::Image::Pointer image, std::vector<mitk::Image::Pointer> &out, double sigma);

  /**
   * \brief Compute local histograms for each voxel in an image.
   *
   * \param[in] image The input image.
   * \param[out] out Vector of output images, one per histogram bin.
   * \param[in] Bins Number of histogram bins.
   * \param[in] NeighbourhoodSize Size of the local neighbourhood.
   */
  static void LocalHistogram(mitk::Image::Pointer image, std::vector<mitk::Image::Pointer> &out, int Bins, int NeighbourhoodSize);

  /**
   * \brief Transform an Eigen matrix back to an MITK image using a mask.
   *
   * Non-zero voxels in the mask are filled with values from the matrix in order.
   *
   * \tparam TMatrixElementType Element type of the Eigen matrix.
   * \param[in] matrix The Eigen matrix of shape [n_masked_voxels, 1].
   * \param[in] mask The mask image defining which voxels to populate.
   * \return The resulting MITK image.
   */
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
  /**
   * \brief Transform an MITK image into an Eigen column matrix using a mask.
   *
   * Extracts intensity values from masked (non-zero) voxels and stores them
   * in a single-column Eigen matrix.
   *
   * \tparam TMatrixElementType Element type of the Eigen matrix.
   * \param img The input intensity image.
   * \param mask The mask image defining which voxels to extract.
   * \return An Eigen matrix of shape [n_masked_voxels, 1].
   */
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

  /**
   * \brief Dilate a binary image using a ball structuring element.
   * \param sourceImage The input binary image.
   * \param resultImage The dilated output image.
   * \param radius Radius of the structuring element in voxels.
   * \param d The anatomical dimension(s) for the operation.
   */
  static void DilateBinary(mitk::Image::Pointer & sourceImage, mitk::Image::Pointer& resultImage, int radius , MorphologicalDimensions d);

  /**
   * \brief Erode a binary image using a ball structuring element.
   * \param sourceImage The input binary image.
   * \param resultImage The eroded output image.
   * \param radius Radius of the structuring element in voxels.
   * \param d The anatomical dimension(s) for the operation.
   */
  static void ErodeBinary(mitk::Image::Pointer & sourceImage, mitk::Image::Pointer& resultImage, int radius, MorphologicalDimensions d);

  /**
   * \brief Perform morphological closing on a binary image.
   * \param sourceImage The input binary image.
   * \param resultImage The closed output image.
   * \param radius Radius of the structuring element in voxels.
   * \param d The anatomical dimension(s) for the operation.
   */
  static void ClosingBinary(mitk::Image::Pointer & sourceImage, mitk::Image::Pointer& resultImage, int radius, MorphologicalDimensions d);

  /**
   * \brief Merge labels in a label image according to a mapping.
   * \param img The label image to modify in place.
   * \param map Map of source label to target label. Each occurrence of a source label is replaced by its target.
   */
  static void MergeLabels(mitk::Image::Pointer & img, const std::map<unsigned int, unsigned int> & map);

  /**
   * \brief Compute connected components of a label image.
   * \param image The input label image.
   * \param mask Optional mask restricting the region. Can be nullptr.
   * \param outimage The output image with unique labels per connected component.
   * \param num_components The number of connected components found.
   */
  static void ConnectedComponentsImage(mitk::Image::Pointer & image, mitk::Image::Pointer& mask, mitk::Image::Pointer &outimage, unsigned int& num_components);

  /**
   * \brief Extract a single label from a label image into a binary mask.
   * \param image The input label image.
   * \param outimage The output binary image (1 where label matches, 0 otherwise).
   * \param label The label value to extract.
   */
  static void GrabLabel(mitk::Image::Pointer & image, mitk::Image::Pointer & outimage, unsigned int label);

  /**
   * \brief Insert a label value into an output image at positions defined by a mask.
   * \param image The output image to insert the label into (created if null).
   * \param maskImage The mask defining where to insert the label.
   * \param label The label value to insert.
   */
  static void InsertLabel(mitk::Image::Pointer & image, mitk::Image::Pointer & maskImage, unsigned int label);

  /**
   * \brief Perform grayscale erosion using a ball structuring element.
   * \param image The input image.
   * \param radius Radius of the structuring element in voxels.
   * \param d The anatomical dimension(s) for the operation.
   * \param outimage The eroded output image.
   */
  static void ErodeGrayscale(mitk::Image::Pointer & image, unsigned int radius, mitk::CLUtil::MorphologicalDimensions d, mitk::Image::Pointer & outimage );

  /**
   * \brief Perform grayscale dilation using a ball structuring element.
   * \param image The input image.
   * \param radius Radius of the structuring element in voxels.
   * \param d The anatomical dimension(s) for the operation.
   * \param outimage The dilated output image.
   */
  static void DilateGrayscale(mitk::Image::Pointer & image, unsigned int radius, mitk::CLUtil::MorphologicalDimensions d, mitk::Image::Pointer & outimage );

  /**
   * \brief Fill holes in a grayscale image.
   * \param image The input image.
   * \param outimage The output image with holes filled.
   */
  static void FillHoleGrayscale(mitk::Image::Pointer & image, mitk::Image::Pointer & outimage);

  /**
   * \brief Create a probability map based on Gaussian distribution parameters.
   * \param sourceImage The input intensity image.
   * \param mean The mean of the Gaussian distribution.
   * \param std_dev The standard deviation of the Gaussian distribution.
   * \param resultImage The output probability map image.
   */
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
