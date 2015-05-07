#ifndef mitkCLUtil_h
#define mitkCLUtil_h

#include <itkImageRegionIterator.h>
#include <mitkImage.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>

#include <Eigen/Dense>

namespace mitk
{

struct CLUtil
{


  ///
  /// \brief CountSamplesPerLabels
  /// \param image
  /// \param map
  ///
  template<class TImageType>
  static void CountSamplesPerLabels( TImageType * image, std::map<unsigned int, unsigned int> & map)
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

  ///
  /// \brief CountVoxelForLabel
  /// \param image
  /// \param label
  /// \param count
  ///
  template <class TImageType>
  static void CountVoxelForLabel(TImageType* image, typename TImageType::PixelType label, unsigned int & count )
  {
    itk::ImageRegionConstIterator<TImageType> inputIter(image, image->GetLargestPossibleRegion());
    while(!inputIter.IsAtEnd())
    {
      if(inputIter.Value() == label) ++count;
      ++inputIter;
    }
  }

  ///
  /// \brief SampleLabel
  /// \param image
  /// \param output
  /// \param acceptrate
  /// \param label
  ///
  template <class TImageType1, class TImageType2>
  static void SampleLabel(TImageType1* image, TImageType2* output, double acceptrate, unsigned int label)
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


  ///
  /// \brief countLabledVoxels
  /// \param mask
  /// \param n_numSamples
  ///
  template<typename TImageType>
  static void CountLabledVoxels(TImageType * mask, unsigned int & n_numSamples)
  {
    auto mit = itk::ImageRegionConstIterator<TImageType>(mask, mask->GetLargestPossibleRegion());
    while (!mit.IsAtEnd())
    {
      if(mit.Value() > 0)
        n_numSamples++;
      ++mit;
    }
  }


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

    mitk::Image::Pointer out_img;
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
    mitk::CLUtil::CountLabledVoxels(current_mask.GetPointer(),n_numSamples);

    typename itk::Image<TMatrixElementType, 3>::Pointer current_img;
    mitk::CastToItkImage(mask,current_img);

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

};
} //namespace MITK

#endif
