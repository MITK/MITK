#ifndef mitkCLUtil_h
#define mitkCLUtil_h

#include <Eigen/Dense>
#include <MitkClassificationUtilitiesExports.h>
#include <itkImageRegionIterator.h>


#include <mitkImage.h>
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>

namespace mitk
{

class MITKCLASSIFICATIONUTILITIES_EXPORT CLUtil
{
public:
  ///
  /// \brief The MorphologicalDimensions enum
  ///
  enum MorphologicalDimensions
  {
    Axial,Coronal,Sagial,All
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
  static void LogicalAndImages(Image::Pointer &image1, Image::Pointer &image2);

  ///
  /// \brief GaussianFilter
  /// \param image
  /// \param smoothed
  /// \param sigma
  ///
  static void GaussianFilter(mitk::Image::Pointer image, mitk::Image::Pointer & smoothed ,double sigma);

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
  /// \param sourceImage
  /// \param resultImage
  /// \param factor
  /// \param d
  ///
  static void DilateBinary(mitk::Image::Pointer & sourceImage, mitk::Image::Pointer& resultImage, int factor , MorphologicalDimensions d);

  ///
  /// \brief ErodeBinary
  /// \param sourceImage
  /// \param resultImage
  /// \param factor
  /// \param d
  ///
  static void ErodeBinary(mitk::Image::Pointer & sourceImage, mitk::Image::Pointer& resultImage, int factor, MorphologicalDimensions d);

  ///
  /// \brief ClosingBinary
  /// \param sourceImage
  /// \param resultImage
  /// \param factor
  /// \param d
  ///
  static void ClosingBinary(mitk::Image::Pointer & sourceImage, mitk::Image::Pointer& resultImage, int factor, MorphologicalDimensions d);


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

private:
  template< typename TImageType >
  static void itkCreateCheckerboardMask(TImageType * image, mitk::Image::Pointer & outimage);

  template< typename TImageType >
  static void itkInterpolateCheckerboardPrediction(TImageType * checkerboard_prediction, mitk::Image::Pointer & checkerboard_mask, mitk::Image::Pointer & outimage);

  template <class TImageType>
  static void itkSumVoxelForLabel(TImageType* image, const mitk::Image::Pointer & source , typename TImageType::PixelType label, double & val );

  template <class TImageType>
  static void itkSqSumVoxelForLabel(TImageType* image, const mitk::Image::Pointer & source, typename TImageType::PixelType label, double & val );

  template<typename TStructuringElement>
  static void itkFitStructuringElement(TStructuringElement & se, MorphologicalDimensions d, int factor);

  template<typename TImageType>
  static void itkDilateBinary(TImageType * sourceImage, mitk::Image::Pointer& resultImage, int factor , MorphologicalDimensions d);

  template<typename TImageType>
  static void itkErodeBinary(TImageType * sourceImage, mitk::Image::Pointer& resultImage, int factor, MorphologicalDimensions d);

  template<typename TImageType>
  static void itkClosingBinary(TImageType * sourceImage, mitk::Image::Pointer& resultImage, int factor, MorphologicalDimensions d);

  template<typename TPixel, unsigned int VDimension>
  static void itkFillHolesBinary(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer& resultImage);

  template<typename TImageType1, typename TImageType2>
  static void itkLogicalAndImages(TImageType1 * image1, TImageType2 * image2);

  template<class TImageType>
  static void itkGaussianFilter(TImageType * image, mitk::Image::Pointer & smoothed ,double sigma);


};

} //namespace MITK

#endif
