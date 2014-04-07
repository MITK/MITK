/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef __itkNonLocalMeansDenoisingFilter_h_
#define __itkNonLocalMeansDenoisingFilter_h_

#include "itkImageToImageFilter.h"
#include "itkVectorImage.h"
#include <mitkDiffusionImage.h>

//#include <itkStatisticsImageFilter.h>


namespace itk{
  /** \class NonLocalMeansDenoisingFilter
   * \brief This class denoises a vectorimage according to the non local means procedure.
   *
   * This Filter needs as an input a diffusion weigthed image.
   * An input mask is optional to denoise only inside the mask range. All other voxels will be set to 0.
  */

  template< class TPixelType >
  class NonLocalMeansDenoisingFilter :
    public ImageToImageFilter< VectorImage < TPixelType, 3 >, VectorImage < TPixelType, 3 > >
  {
  public:

    /** Typedefs */
    typedef NonLocalMeansDenoisingFilter                                                          Self;
    typedef SmartPointer<Self>                                                                    Pointer;
    typedef SmartPointer<const Self>                                                              ConstPointer;
    typedef ImageToImageFilter< VectorImage < TPixelType, 3 >, VectorImage < TPixelType, 3 > >    Superclass;
    typedef typename Superclass::InputImageType                                                   InputImageType;
    typedef typename Superclass::OutputImageType                                                  OutputImageType;
    typedef typename Superclass::OutputImageRegionType                                            OutputImageRegionType;
    typedef Image <TPixelType, 3>                                                                 MaskImageType;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(NonLocalMeansDenoisingFilter, ImageToImageFilter)

    /**
     * \brief Set flag to use joint information
     */
    itkSetMacro(UseJointInformation, bool)
    /**
     * \brief Set the searchradius
     *
     * The searchradius generates a neighborhood of size (2 * searchradius + 1)³.
     * Default is 4.
     */
    itkSetMacro(SearchRadius, int)
    /**
     * \brief Set the comparisonradius
     *
     * The comparisonradius generates neighborhoods of size (2 * comparisonradius +1)³.
     * Default is 1.
     */
    itkSetMacro(ComparisonRadius, int)
    /**
     * \brief Set the variance of the noise
     *
     * The variance of the noise needs to be estimated tu use this filter properly.
     * Default is 1.
     */
    itkSetMacro(Variance, double)
    /**
     * \brief Set flag to use a rician adaption
     *
     * If this flag is true the filter uses a method which is optimized for Rician distributed noise.
     */
    itkSetMacro(UseRicianAdaption, bool)
    /**
     * \brief Get the amount of calculated Voxels
     *
     * Returns the number of calculated Voxels until yet, useful for the use of a progressbars.
     */
    itkGetMacro(CurrentVoxelCount, unsigned int)


    /** \brief Set the input image. **/
    void SetInputImage(const InputImageType* image);
    /**
     * \brief Set a denoising mask
     *
     * optional
     *
     * Set a mask to denoise only the masked area, all voxel outside this area will be set to 0.
     */
    void SetInputMask(MaskImageType* mask);

  protected:
    NonLocalMeansDenoisingFilter();
    ~NonLocalMeansDenoisingFilter() {}

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType);



  private:

    int m_SearchRadius;
    int m_ComparisonRadius;
    bool m_UseJointInformation;
    bool m_UseRicianAdaption;
    unsigned int m_CurrentVoxelCount;
    double m_Variance;
    typename MaskImageType::Pointer m_Mask;
  };
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkNonLocalMeansDenoisingFilter.txx"
#endif

#endif //__itkNonLocalMeansDenoisingFilter_h_
