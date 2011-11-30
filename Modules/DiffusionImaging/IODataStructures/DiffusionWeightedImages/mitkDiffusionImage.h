/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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
    typedef TPixelType PixelType;
    typedef typename itk::VectorImage<TPixelType, 3>
                                                ImageType;
    typedef vnl_vector_fixed< double, 3 >       GradientDirectionType;
    typedef itk::VectorContainer< unsigned int,
      GradientDirectionType >                   GradientDirectionContainerType;
    typedef itk::VectorImageToImageAdaptor< TPixelType, 3 >
                                                AdaptorType;
    typedef vnl_matrix_fixed< double, 3, 3 >      MeasurementFrameType;

    mitkClassMacro( DiffusionImage, Image );
    itkNewMacro(Self);

    //void SetRequestedRegionToLargestPossibleRegion();
    //bool RequestedRegionIsOutsideOfTheBufferedRegion();
    //virtual bool VerifyRequestedRegion();
    //void SetRequestedRegion(itk::DataObject *data);

    void AverageRedundantGradients(double precision);

    GradientDirectionContainerType::Pointer CalcAveragedDirectionSet(double precision, GradientDirectionContainerType::Pointer directions);

    void CorrectDKFZBrokenGradientScheme(double precision);

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
    void SetDirections(const std::vector<itk::Vector<double,3> > directions)
    {
      m_Directions = GradientDirectionContainerType::New();
      for(unsigned int i=0; i<directions.size(); i++)
      {
        m_Directions->InsertElement( i, directions[i].Get_vnl_vector() );
      }
    }
    GradientDirectionContainerType::Pointer GetOriginalDirections()
    { return m_OriginalDirections; }
    void SetOriginalDirections( GradientDirectionContainerType::Pointer directions )
    { this->m_OriginalDirections = directions; this->ApplyMeasurementFrame(); }
    void SetOriginalDirections(const std::vector<itk::Vector<double,3> > directions)
    {
      m_OriginalDirections = GradientDirectionContainerType::New();
      for(unsigned int i=0; i<directions.size(); i++)
      {
        m_OriginalDirections->InsertElement( i, directions[i].Get_vnl_vector() );
      }
      this->ApplyMeasurementFrame();
    }

    MeasurementFrameType GetMeasurementFrame()
    { return m_MeasurementFrame; }
    void SetMeasurementFrame( MeasurementFrameType mFrame )
    { this->m_MeasurementFrame = mFrame; this->ApplyMeasurementFrame(); }

    itkGetMacro(B_Value, float);
    itkSetMacro(B_Value, float);

    float GetB_Value(int i)
    {
      if(i > m_Directions->Size()-1)
        return -1;

      if(m_Directions->ElementAt(i).one_norm() <= 0.0)
      {
        return 0;
      }
      else
      {
        double twonorm = m_Directions->ElementAt(i).two_norm();
        return m_B_Value*twonorm*twonorm ;
      }
    }

    bool AreAlike(GradientDirectionType g1, GradientDirectionType g2, double precision);

    int GetNumDirections();
    int GetNumB0();
    std::vector<int> GetB0Indices();
    bool IsMultiBval();

  protected:
    DiffusionImage();
    virtual ~DiffusionImage();

    void ApplyMeasurementFrame();

    typename ImageType::Pointer               m_VectorImage;
    GradientDirectionContainerType::Pointer   m_Directions;
    GradientDirectionContainerType::Pointer   m_OriginalDirections;
    float                                     m_B_Value;
    typename AdaptorType::Pointer             m_VectorImageAdaptor;
    int                                       m_DisplayIndex;
    MeasurementFrameType                      m_MeasurementFrame;
  };

} // namespace mitk

#include "mitkDiffusionImage.txx"

#endif /* __mitkDiffusionImage__h */
