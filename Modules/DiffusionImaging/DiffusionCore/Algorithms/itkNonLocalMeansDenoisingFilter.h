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

/*===================================================================

This file is based heavily on a corresponding ITK filter.

===================================================================*/
#ifndef __itkNonLocalMeansDenoisingFilter_h_
#define __itkNonLocalMeansDenoisingFilter_h_

#include "itkImageToImageFilter.h"
#include "itkVectorImage.h"
#include <mitkDiffusionImage.h>
#include <itkNeighborhoodIterator.h>
#include <itkVectorImageToImageFilter.h>
#include <itkChangeInformationImageFilter.h>
#include <itkExtractImageFilter.h>
#include <itkLabelStatisticsImageFilter.h>


namespace itk{
  /** \class NonLocalMeansDenoisingFilter
  */

  template< class TPixelType >
  class NonLocalMeansDenoisingFilter :
    public ImageToImageFilter< VectorImage < TPixelType, 3 >, VectorImage < TPixelType, 3 > >
  {
  public:

    typedef NonLocalMeansDenoisingFilter Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter< VectorImage < TPixelType, 3 >, VectorImage < TPixelType, 3 > >  Superclass;

    /** Method for creation through the object factory. */
    itkNewMacro(Self)

    /** Runtime information support. */
    itkTypeMacro(NonLocalMeansDenoisingFilter, ImageToImageFilter)

    typedef typename Superclass::InputImageType InputImageType;
    typedef typename Superclass::OutputImageType OutputImageType;
    typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

    typedef Image <TPixelType, 3> MaskImageType;
    typedef VectorImageToImageFilter < TPixelType > ImageExtractorType;
    typedef ChangeInformationImageFilter < MaskImageType > ChangeInformationType;
    typedef ExtractImageFilter < MaskImageType, MaskImageType > ExtractImageFilterType;
    typedef LabelStatisticsImageFilter < MaskImageType, MaskImageType > LabelStatisticsFilterType;

    itkSetMacro(UseJointInformation, bool)

    void SetNRadius(unsigned int n);
    void SetVRadius(unsigned int v);
    void SetInputImage(const InputImageType* image);
    void SetInputMask(const MaskImageType* mask);
    //void SetProgressbar(

  protected:
    NonLocalMeansDenoisingFilter();
    ~NonLocalMeansDenoisingFilter() {}
    void PrintSelf(std::ostream& os, Indent indent) const;

    void BeforeThreadedGenerateData();
    void ThreadedGenerateData( const OutputImageRegionType &outputRegionForThread, ThreadIdType);

    typename NeighborhoodIterator< VectorImage< TPixelType, 3 > >::RadiusType m_V_Radius;
    typename NeighborhoodIterator< VectorImage< TPixelType, 3 > >::RadiusType m_N_Radius;
    VariableLengthVector< double > m_Deviations;
    bool m_UseJointInformation;

    unsigned long m_Pixels;

  private:
    void IndividualDenoising(const OutputImageRegionType &outputRegionForThread);
    void JointInformationDenoising(const OutputImageRegionType &outputRegionForThread);
  };
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkNonLocalMeansDenoisingFilter.cpp"
#endif

#endif //__itkNonLocalMeansDenoisingFilter_h_
