#ifndef __itkConnectednessFilter_h
#define __itkConnectednessFilter_h

#include "itkImageToImageFilter.h"
#include "itkImageRegionIterator.h"
#include <itkDiffusionTensor3D.h>

namespace itk
{

/** ConnectednessFilter - Propagate the border voxels of seed region, also calculates distance maps
 *
 * Propagates the values of the boder-region of the seed mask. This is done by looking for the shortes path
 * w.r.t. some distance metric that can be chosen.
 *
 * Options are
 * FeatureSimilarity - shortest path is determined by looking at feature differences only
 * VectorAgreement - cost is determined by path agreement with supplied vector field
 * FeatureVectorAgreement - cost is a combination of vector agreement and feature similarity
 *
 */

template <class TFeatureImage,class TSeedImage, typename TTensorImagePixelType = double >
class ConnectednessFilter : public ImageToImageFilter< TFeatureImage,TFeatureImage >
{
public:
  /** Standard class typedefs. */
  typedef ConnectednessFilter Self;
  typedef ImageToImageFilter< TFeatureImage, TFeatureImage> Superclass;
  typedef SmartPointer< Self > Pointer;

  typedef typename itk::Image< itk::DiffusionTensor3D<TTensorImagePixelType>,3> TTensorImage;

  typedef typename TSeedImage::PixelType SeedPixelType;
  typedef typename TFeatureImage::PixelType FeaturePixelType;
  typedef typename TTensorImage::PixelType TensorPixelType;

  typedef typename TFeatureImage::IndexType IndexType;
  typedef typename TSeedImage::IndexType IndexSeedType;
  typedef typename TTensorImage::IndexType IndexTensorType;

  typedef typename itk::ImageRegionIterator< TSeedImage > SeedIteratorType;
  typedef typename itk::ImageRegionIterator< TFeatureImage > FeatureIteratorType;


  /** Method for creation through the object factory. */
  itkNewMacro(Self)

  /** Run-time type information (and related methods). */
  itkTypeMacro(PropagateLocalFeaturesFilter, ImageToImageFilter)

  /**
   * @brief The DistanceMode enum Available distance metrics
   */
  enum DistanceMode { FeatureSimilarity , VectorAgreement , FeatureVectorAgreement };


  void SetInputImage(const TFeatureImage* image);

  /**
   * @brief SetInputSeed - seed area from which features are to be propagated
   * @param mask
   */
  void SetInputSeed(const TSeedImage* mask);

  /**
   * @brief SetInputMask - Filter only operates in masked area
   * @param mask
   */
  void SetInputMask(const TSeedImage* mask);

  /**
   * @brief SetInputVectorField
   * @param vecs
   */
  void SetInputVectorField(const TTensorImage* vecs);

  /**
   * @brief SetInputVectorFieldConfidenceMap - Map that assigned a weight/confidence to each vector
   * @param conf
   */
  void SetInputVectorFieldConfidenceMap(const TFeatureImage* conf);


  /**
   * @brief SetPropagationImage - OPTIONAL. Set image which values are propagated, it this is not supplied the InputImage is chosen as default.
   * @param prop
   */
  void SetPropagationImage(const TFeatureImage* prop);

  /**
   * @brief GetDistanceImage - Return the distance image using the specified metric
   * @return
   */
  typename TFeatureImage::Pointer GetDistanceImage();

  /**
   * @brief GetEuclideanDistanceImage - Return distance image that provides distance of shortest path for each voxel w.r.t to euclidean space
   * @return
   */

  typename TFeatureImage::Pointer GetEuclideanDistanceImage();

  void SetMode(DistanceMode mode = FeatureSimilarity)
  {
    m_Mode = mode;
  }

  /**
   * @brief SetApplyRankFilter - if true the values propagated are determined by calculating the median within the mask within a 3x3x3 neighborhood
   * @param applyFilter
   */
  void SetApplyRankFilter(bool applyFilter)
  {
    m_ApplyRankFilter = applyFilter;
  }

protected:
  ConnectednessFilter(){
    m_ApplyRankFilter = false;
  }
  ~ConnectednessFilter(){}

  /** Does the real work. */
  virtual void GenerateData();

  virtual double GetDistanceValue(IndexType idxStart, IndexType idxEnd);


private:
  ConnectednessFilter(const Self &); //purposely not implemented
  void operator=(const Self &);  //purposely not implemented


  typename TFeatureImage::Pointer m_InputImage;
  typename TTensorImage::Pointer m_TensorImage;
  typename TFeatureImage::Pointer m_ConfidenceImage;
  // Distance image w.r.t. to metric
  typename TFeatureImage::Pointer m_DistanceImage;
  // Euclidean distance, that keeps track of distance of shortest path
  typename TFeatureImage::Pointer m_EuclideanDistance;

  DistanceMode m_Mode;

  bool m_ApplyRankFilter;

};
} //namespace ITK


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkConnectednessFilter.cxx"
#endif


#endif // itkConnectednessFilter_h
