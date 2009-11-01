/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2008-02-07 17:17:57 +0100 (Do, 07 Feb 2008) $
Version:   $Revision: 11989 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef __mitkDiffusionImage__h
#define __mitkDiffusionImage__h

#include "mitkImage.h"
#include "itkVectorImage.h"
#include "itkVectorImageToImageAdaptor.h"

namespace mitk 
{

  /**
  * \brief this class encapsulates diffusion volumes (vectorimages not
  * yet supported by mitkImage)
  */
  template<class TPixelType>
  class DiffusionImage : public Image
  {

  public:

    typedef typename itk::VectorImage<TPixelType, 3> 
                                                ImageType;
    typedef vnl_vector_fixed< double, 3 >       GradientDirectionType;
    typedef itk::VectorContainer< unsigned int, 
      GradientDirectionType >                   GradientDirectionContainerType;
    typedef itk::VectorImageToImageAdaptor< TPixelType, 3 > 
                                                AdaptorType;

    mitkClassMacro( DiffusionImage, Image );
    itkNewMacro(Self);

    //void SetRequestedRegionToLargestPossibleRegion();
    //bool RequestedRegionIsOutsideOfTheBufferedRegion();
    //virtual bool VerifyRequestedRegion();
    //void SetRequestedRegion(itk::DataObject *data);

    void AverageRedundantGradients(double precision);

    typename ImageType::Pointer GetVectorImage()
    { return m_VectorImage; }
    void SetVectorImage(typename ImageType::Pointer image )
    { this->m_VectorImage = image; }

    void InitializeFromVectorImage();
    void SetDisplayIndexForRendering(int displayIndex);

    GradientDirectionContainerType::Pointer GetDirections()
    { return m_Directions; }
    void SetDirections( GradientDirectionContainerType::Pointer directions )
    { this->m_Directions = directions; }

    itkGetMacro(B_Value, float);
    itkSetMacro(B_Value, float);

    bool AreAlike(GradientDirectionType g1, GradientDirectionType g2, double precision);

  protected:
    DiffusionImage();
    virtual ~DiffusionImage();

    typename ImageType::Pointer               m_VectorImage;
    GradientDirectionContainerType::Pointer   m_Directions;
    float                                     m_B_Value;
    typename AdaptorType::Pointer             m_VectorImageAdaptor;

  };

} // namespace mitk

#include "mitkDiffusionImage.txx"

#endif /* __mitkDiffusionImage__h */
