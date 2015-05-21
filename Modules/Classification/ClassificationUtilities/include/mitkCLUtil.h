#ifndef mitkCLUtil_h
#define mitkCLUtil_h

#include <mitkImage.h>
#include <Eigen/Dense>
#include <MitkClassificationUtilitiesExports.h>
namespace mitk
{

struct MITKCLASSIFICATIONUTILITIES_EXPORT CLUtil
{


  ///
  /// \brief CountSamplesPerLabels
  /// \param image
  /// \param map
  ///
  template<class TImageType>
  static void  CountSamplesPerLabels( TImageType * image, std::map<unsigned int, unsigned int> & map);


  ///
  /// \brief CountVoxelForLabel
  /// \param image
  /// \param label
  /// \param count
  ///
  template <class TImageType>
  static void CountVoxelForLabel(TImageType* image, typename TImageType::PixelType label, unsigned int & count );


  ///
  /// \brief SumVoxelForLabel
  /// \param image
  /// \param source
  /// \param label
  /// \param val
  ///
  template <class TImageType>
  static void SumVoxelForLabel(TImageType* image, const mitk::Image::Pointer & source , typename TImageType::PixelType label, double & val );


  ///
  /// \brief SqSumVoxelForLabel
  /// \param image
  /// \param source
  /// \param label
  /// \param val
  ///
  template <class TImageType>
  static void SqSumVoxelForLabel(TImageType* image, const mitk::Image::Pointer & source, typename TImageType::PixelType label, double & val );

  ///
  /// \brief SampleLabel
  /// \param image
  /// \param output
  /// \param acceptrate
  /// \param label
  ///
  template <class TImageType1, class TImageType2>
  static void SampleLabel(TImageType1* image, TImageType2* output, double acceptrate, unsigned int label);


  ///
  /// \brief countLabledVoxels
  /// \param mask
  /// \param n_numSamples
  ///
  template<typename TImageType>
  static inline void CountLabledVoxels(TImageType * mask, unsigned int & n_numSamples);

  ///
  /// \brief transform
  /// \param matrix
  /// \param mask
  /// \param outimage
  ///
  template<typename TMatrixElementType>
  static mitk::Image::Pointer Transform(const Eigen::Matrix<TMatrixElementType, Eigen::Dynamic, Eigen::Dynamic> & matrix, const mitk::Image::Pointer & mask);

  ///
  /// \brief TransformImageToMatrix
  /// \param in_img
  /// \param mask
  /// \param out_matrix
  ///
  template<typename TMatrixElementType>
  static Eigen::Matrix<TMatrixElementType, Eigen::Dynamic, Eigen::Dynamic> Transform(const mitk::Image::Pointer & img, const mitk::Image::Pointer & mask);

  ///
  /// \brief itkDilateBinary
  /// \param sourceImage
  /// \param resultImage
  /// \param factor
  ///
  template<typename TImageType>
  static void itkDilateBinary(TImageType * sourceImage, mitk::Image::Pointer& resultImage, int factor);

  ///
  /// \brief itkErodeBinary
  /// \param sourceImage
  /// \param resultImage
  /// \param factor
  ///
  template<typename TImageType>
  static void itkErodeBinary(TImageType * sourceImage, mitk::Image::Pointer& resultImage, int factor);

  /// \brief itkFillHolesBinary
  /// \param sourceImage
  /// \param resultImage
  ///
  template<typename TPixel, unsigned int VDimension>
  static void itkFillHolesBinary(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer& resultImage);

  ///
  /// \brief itkLogicalAndImages voxel wise comparison if a voxel of image1 or voxel of image imag2 is zero set
  /// in image1 and image2 the current voxel to zero
  /// \param image1
  /// \param image2
  ///
  template<typename TImageType1, typename TImageType2>
  static void itkLogicalAndImages(TImageType1 * image1, TImageType2 * image2);

  ///
  /// \brief GaussianFilter
  /// \param image
  /// \param smoothed
  /// \param sigma
  ///
  template<class TImageType>
  static void GaussianFilter(TImageType * image, mitk::Image::Pointer & smoothed ,double sigma);


  };

} //namespace MITK

#include "../src/mitkCLUtil.hxx"

#endif
