#ifndef itkNeighborhoodFunctorImageFilter_h
#define itkNeighborhoodFunctorImageFilter_h

#include "itkImageToImageFilter.h"
#include "itkZeroFluxNeumannBoundaryCondition.h"
#include "itkConstNeighborhoodIterator.h"
#include "itkImage.h"
#include "stdio.h"

#include <itkHistogramToTextureFeaturesFilter.h>
#include "itkHistogram.h"

namespace itk
{

template<typename TInputImageType, typename TFeatureImageType , class FunctorType>
class NeighborhoodFunctorImageFilter : public ImageToImageFilter< TInputImageType, TFeatureImageType>
{
public:
    typedef NeighborhoodFunctorImageFilter                          Self;
    typedef ImageToImageFilter<TInputImageType, TFeatureImageType>  Superclass;
    typedef SmartPointer< Self >                                    Pointer;
    typedef SmartPointer< const Self >                              ConstPointer;

    itkNewMacro(Self);

    itkTypeMacro(NeighborhoodFunctorImageFilter, ImageToImageFilter);

    /** Extract some information from the image types.  Dimensionality
   * of the two images is assumed to be the same. */
    itkStaticConstMacro(ImageDimension, unsigned int,
                        TFeatureImageType::ImageDimension);
    itkStaticConstMacro(InputImageDimension, unsigned int,
                        TInputImageType::ImageDimension);

    typedef TInputImageType                                         InputImageType;
    typedef typename TInputImageType::PixelType                     InputImagePixelType;
    typedef itk::Image<short, InputImageDimension>          MaskImageType;
    typedef typename MaskImageType::PixelType                       MaskImagePixelType;
    typedef TFeatureImageType                                       FeatureImageType;
    typedef typename FeatureImageType::PixelType                    FeaturePixelType;
    typedef itk::Size<InputImageDimension>                          SizeType;

    /** Typedef for generic boundary condition pointer. */
    typedef ImageBoundaryCondition< InputImageType > * ImageBoundaryConditionPointerType;

    /** Typedef for the default boundary condition */
    typedef ZeroFluxNeumannBoundaryCondition< InputImageType > DefaultBoundaryCondition;

    /** Superclass typedefs. */
    typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

    typedef Neighborhood< InputImagePixelType, InputImageDimension > NeighborhoodType;

    /** Allows a user to override the internal boundary condition. Care should be
   * be taken to ensure that the overriding boundary condition is a persistent
   * object during the time it is referenced.  The overriding condition
   * can be of a different type than the default type as long as it is
   * a subclass of ImageBoundaryCondition. */
    void OverrideBoundaryCondition(const ImageBoundaryConditionPointerType i)
    { m_BoundsCondition = i; }

    /** Get the boundary condition specified */
    ImageBoundaryConditionPointerType GetBoundaryCondition()
    { return m_BoundsCondition; }

    void SetNeighborhoodSize(SizeType size){m_Size = size;}
    void SetNeighborhoodSize(unsigned int size){m_Size.Fill(size);}
    SizeType GetNeighborhoodSize(){return m_Size;}

    void SetMask(const typename MaskImageType::Pointer & ptr){m_MaskImage = ptr;}

    const FunctorType & GetFunctorReference() const
    {
      return m_Functor;
    }

    FunctorType & GetFunctorReference()
    {
      return m_Functor;
    }

    void SetFunctor(const FunctorType & func)
    {
      m_Functor = func;
    }

protected:

    NeighborhoodFunctorImageFilter()
    {
        m_Size.Fill(0);
        m_MaskImage = NULL;
        m_BoundsCondition = static_cast< ImageBoundaryConditionPointerType >( &m_DefaultBoundaryCondition );
        this->SetNumberOfIndexedOutputs(FunctorType::OutputCount);
    }
    ~NeighborhoodFunctorImageFilter(){}

    void BeforeThreadedGenerateData();

    void ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread,
                              ThreadIdType threadId);


    /** NeighborhoodFunctorImageFilter needs a larger input requested
       * region than the output requested region.  As such,
       * NeighborhoodOperatorImageFilter needs to provide an implementation for
       * GenerateInputRequestedRegion() in order to inform the pipeline
       * execution model.
       *
       * \sa ProcessObject::GenerateInputRequestedRegion() */
    virtual void GenerateInputRequestedRegion() throw ( InvalidRequestedRegionError );


private:
    NeighborhoodFunctorImageFilter(const Self &); // purposely not implemented
    void operator=(const Self &); // purposely not implemented

    /** Pointer to a persistent boundary condition object used
   * for the image iterator. */
    ImageBoundaryConditionPointerType m_BoundsCondition;

    /** Default boundary condition */
    DefaultBoundaryCondition m_DefaultBoundaryCondition;

    /** Internal operator used to filter the image. */
    FunctorType m_Functor;

    itk::Size<InputImageDimension> m_Size;

    typename MaskImageType::Pointer m_MaskImage;

};
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "../src/Features/itkNeighborhoodFunctorImageFilter.cpp"
#endif

#endif // itkFeatureImageFilter_h
