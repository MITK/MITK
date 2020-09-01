/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef itkStitchImageFilter_h
#define itkStitchImageFilter_h

#include "itkFixedArray.h"
#include "itkTransform.h"
#include "itkImageRegionIterator.h"
#include "itkImageToImageFilter.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkSize.h"
#include "itkDefaultConvertPixelTraits.h"
#include "itkDataObjectDecorator.h"


namespace itk
{
  enum class StitchStrategy
  {
    Mean = 0, //use the mean value of all inputs that can provide a pixel vaule
    BorderDistance = 1 //use the value that is largest minimal distance to its image borders (use e.g. if vaules tend to be not reliable at borders)
  };

  std::ostream& operator<< (std::ostream& os, const itk::StitchStrategy& strategy)
  {
    if (itk::StitchStrategy::Mean == strategy)
      os << "Mean";
    else if (itk::StitchStrategy::BorderDistance == strategy)
      os << "BorderDistance";
    else
      os << "unkown";

    return os;
  };

  /** \class StitchImageFilter
 * \brief ITK filter that resamples/stitches multiple images into a given reference geometry.
 *
 * StitchImageFilter is similar to itk's ResampleImageFilter, but in difference to the last
 * mentioned StitchImageFilter is able to resample multiple input images at once (with a transform
 * for each input image). If multiple input images cover the output region the behavior depends on
 * the StitchStragy:
 * - Mean: a weighted sum of all voxels mapped input pixel values will be calculated.
 * - BorderDistance: the voxels will be choosen that have the largest minimal distance to its own image borders.
 *
 * All other behaviors are similar to itk::ResampleImageFilter. See the filter's description for
 * more details.
 */
template< typename TInputImage,
          typename TOutputImage,
          typename TInterpolatorPrecisionType = double,
          typename TTransformPrecisionType = TInterpolatorPrecisionType>
class StitchImageFilter :
  public ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef StitchImageFilter                             Self;
  typedef ImageToImageFilter< TInputImage, TOutputImage > Superclass;
  typedef SmartPointer< Self >                            Pointer;
  typedef SmartPointer< const Self >                      ConstPointer;

  typedef TInputImage                           InputImageType;
  typedef TOutputImage                          OutputImageType;
  typedef typename InputImageType::Pointer      InputImagePointer;
  typedef typename InputImageType::ConstPointer InputImageConstPointer;
  typedef typename OutputImageType::Pointer     OutputImagePointer;
  typedef typename InputImageType::RegionType   InputImageRegionType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(StitchImageFilter, ImageToImageFilter);

  /** Number of dimensions. */
  itkStaticConstMacro(ImageDimension, unsigned int,
                      TOutputImage::ImageDimension);
  itkStaticConstMacro(InputImageDimension, unsigned int,
                      TInputImage::ImageDimension);

  /** base type for images of the current ImageDimension */
  typedef ImageBase< itkGetStaticConstMacro(ImageDimension) > ImageBaseType;

  /**
   *  Transform typedef.
   */
  typedef Transform< TTransformPrecisionType,
                     itkGetStaticConstMacro(ImageDimension),
                     itkGetStaticConstMacro(ImageDimension) >   TransformType;
  typedef typename TransformType::ConstPointer                  TransformPointerType;
  typedef DataObjectDecorator<TransformType>                    DecoratedTransformType;
  typedef typename DecoratedTransformType::Pointer              DecoratedTransformPointer;


  /** Interpolator typedef. */
  typedef InterpolateImageFunction< InputImageType,
                                    TInterpolatorPrecisionType >     InterpolatorType;
  typedef typename InterpolatorType::Pointer InterpolatorPointerType;

  typedef typename InterpolatorType::OutputType InterpolatorOutputType;

  typedef DefaultConvertPixelTraits< InterpolatorOutputType > InterpolatorConvertType;

  typedef typename InterpolatorConvertType::ComponentType ComponentType;

  typedef LinearInterpolateImageFunction< InputImageType,
                                          TInterpolatorPrecisionType >   LinearInterpolatorType;
  typedef typename LinearInterpolatorType::Pointer
  LinearInterpolatorPointerType;

  /** Image size typedef. */
  typedef Size< itkGetStaticConstMacro(ImageDimension) > SizeType;

  /** Image index typedef. */
  typedef typename TOutputImage::IndexType IndexType;

  /** Image point typedef. */
  typedef typename InterpolatorType::PointType PointType;
  //typedef typename TOutputImage::PointType            PointType;

  /** Image pixel value typedef. */
  typedef typename TOutputImage::PixelType PixelType;
  typedef typename TInputImage::PixelType  InputPixelType;

  typedef DefaultConvertPixelTraits<PixelType> PixelConvertType;

  typedef typename PixelConvertType::ComponentType PixelComponentType;

  /** Input pixel continuous index typdef */
  typedef ContinuousIndex< TTransformPrecisionType, ImageDimension >
  ContinuousInputIndexType;

  /** Typedef to describe the output image region type. */
  typedef typename TOutputImage::RegionType OutputImageRegionType;

  /** Image spacing,origin and direction typedef */
  typedef typename TOutputImage::SpacingType   SpacingType;
  typedef typename TOutputImage::PointType     OriginPointType;
  typedef typename TOutputImage::DirectionType DirectionType;

  using Superclass::GetInput;

  /** Typedef the reference image type to be the ImageBase of the OutputImageType */
  typedef ImageBase<ImageDimension> ReferenceImageBaseType;

  using Superclass::SetInput;
  void SetInput(const InputImageType* image) override;
  void SetInput(unsigned int index, const InputImageType* image) override;
  /** Convinience methods that allows setting of input image and its transform in
   one call.*/
  virtual void SetInput(unsigned int index, const InputImageType* image, const TransformType* transform);
  virtual void SetInput(unsigned int index, const InputImageType* image, const TransformType* transform, InterpolatorType* interpolator);

  const TransformType* GetTransform(unsigned int index) const;

  const InterpolatorType* GetInterpolator(unsigned int index) const;

  /** Get/Set the size of the output image. */
  itkSetMacro(Size, SizeType);
  itkGetConstReferenceMacro(Size, SizeType);

  /** Get/Set the pixel value when a transformed pixel is outside of the
   * image.  The default default pixel value is 0. */
  itkSetMacro(DefaultPixelValue, PixelType);
  itkGetConstReferenceMacro(DefaultPixelValue, PixelType);

  /** Set the output image spacing. */
  itkSetMacro(OutputSpacing, SpacingType);
  virtual void SetOutputSpacing(const double *values);

  /** Get the output image spacing. */
  itkGetConstReferenceMacro(OutputSpacing, SpacingType);

  /** Set the output image origin. */
  itkSetMacro(OutputOrigin, OriginPointType);
  virtual void SetOutputOrigin(const double *values);

  /** Get the output image origin. */
  itkGetConstReferenceMacro(OutputOrigin, OriginPointType);

  /** Set the output direciton cosine matrix. */
  itkSetMacro(OutputDirection, DirectionType);
  itkGetConstReferenceMacro(OutputDirection, DirectionType);

  /** Helper method to set the output parameters based on this image. */
  void SetOutputParametersFromImage(const ImageBaseType *image);

  /** Set the start index of the output largest possible region.
   * The default is an index of all zeros. */
  itkSetMacro(OutputStartIndex, IndexType);

  /** Get the start index of the output largest possible region. */
  itkGetConstReferenceMacro(OutputStartIndex, IndexType);

   /** Set a reference image to use to define the output information.
    *  By default, output information is specificed through the
    *  SetOutputSpacing, Origin, and Direction methods.  Alternatively,
    *  this method can be used to specify an image from which to
    *  copy the information. UseReferenceImageOn must be set to utilize the
    *  reference image. */
  itkSetInputMacro(ReferenceImage, ReferenceImageBaseType);

  /** Get the reference image that is defining the output information. */
  itkGetInputMacro(ReferenceImage, ReferenceImageBaseType);

  /** Turn on/off whether a specified reference image should be used to define
   *  the output information. */
  itkSetMacro(UseReferenceImage, bool);
  itkBooleanMacro(UseReferenceImage);
  itkGetConstMacro(UseReferenceImage, bool);

  itkSetMacro(StitchStrategy, StitchStrategy);
  itkGetConstMacro(StitchStrategy, StitchStrategy);

  /** StitchImageFilter produces an image which is a different size
   * than its input.  As such, it needs to provide an implementation
   * for GenerateOutputInformation() in order to inform the pipeline
   * execution model.  The original documentation of this method is
   * below. \sa ProcessObject::GenerateOutputInformaton() */
  virtual void GenerateOutputInformation() ITK_OVERRIDE;

  /** StitchImageFilter needs a different input requested region than
   * the output requested region.  As such, StitchImageFilter needs
   * to provide an implementation for GenerateInputRequestedRegion()
   * in order to inform the pipeline execution model.
   * \sa ProcessObject::GenerateInputRequestedRegion() */
  virtual void GenerateInputRequestedRegion() ITK_OVERRIDE;

  /** Set up state of filter before multi-threading.
   * InterpolatorType::SetInputImage is not thread-safe and hence
   * has to be set up before ThreadedGenerateData */
  virtual void BeforeThreadedGenerateData() ITK_OVERRIDE;

  /** Set the state of the filter after multi-threading. */
  virtual void AfterThreadedGenerateData() ITK_OVERRIDE;

  /** Compute the Modified Time based on the changed components. */
  ModifiedTimeType GetMTime(void) const ITK_OVERRIDE;

#ifdef ITK_USE_CONCEPT_CHECKING
  // Begin concept checking
  itkConceptMacro( OutputHasNumericTraitsCheck,
                   ( Concept::HasNumericTraits< PixelComponentType > ) );
  // End concept checking
#endif

protected:
  StitchImageFilter();
  ~StitchImageFilter() ITK_OVERRIDE {}
  void PrintSelf(std::ostream & os, Indent indent) const ITK_OVERRIDE;

  /** Override VeriyInputInformation() since this filter's inputs do
   * not need to occoupy the same physical space.
   *
   * \sa ProcessObject::VerifyInputInformation
   */
  virtual void VerifyInputInformation() ITK_OVERRIDE { }

  /** StitchImageFilter can be implemented as a multithreaded filter.
   * Therefore, this implementation provides a ThreadedGenerateData()
   * routine which is called for each processing thread. The output
   * image data is allocated automatically by the superclass prior
   * to calling ThreadedGenerateData().
   * ThreadedGenerateData can only write to the portion of the output image
   * specified by the parameter "outputRegionForThread"
   * \sa ImageToImageFilter::ThreadedGenerateData(),
   *     ImageToImageFilter::GenerateData() */
  virtual void ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread,
                                    ThreadIdType threadId) ITK_OVERRIDE;

  /** Cast pixel from interpolator output to PixelType. */
  virtual PixelType CastPixelWithBoundsChecking( const InterpolatorOutputType value,
                                                 const ComponentType minComponent,
                                                 const ComponentType maxComponent) const;

  void SetTransform(unsigned int index, const TransformType* transform);

  /** Helper that ensures that a transform is specified for every input image.
   If a input image has no specified transforms, an identity transform will
   be created and set as default.*/
  void EnsureTransforms();

  /** Helper that ensures that an interpolator is specified for every input image.
   If a input image has no specified interpolator, a linear interpolator will
   be created and set as default.*/
  void EnsureInterpolators();

  static std::string GetTransformInputName(unsigned int index);

private:
  ITK_DISALLOW_COPY_AND_ASSIGN(StitchImageFilter);

  typedef std::vector<const InputImageType*> InputImageVectorType;
  typedef std::map<const InputImageType*, typename TransformType::ConstPointer> TransformMapType;
  typedef std::map<const InputImageType*, InterpolatorPointerType> InterpolatorMapType;

  InputImageVectorType GetInputs();
  TransformMapType GetTransforms();

  InterpolatorMapType m_Interpolators;   // Image function for
                                          // interpolation
  PixelType m_DefaultPixelValue;          // default pixel value
                                          // if the point is
                                          // outside the image
  SizeType        m_Size;                 // Size of the output image
  SpacingType     m_OutputSpacing;        // output image spacing
  OriginPointType m_OutputOrigin;         // output image origin
  DirectionType   m_OutputDirection;      // output image direction cosines
  IndexType       m_OutputStartIndex;     // output image start index
  bool            m_UseReferenceImage;
  StitchStrategy  m_StitchStrategy;
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkStitchImageFilter.tpp"
#endif

#endif
