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

#ifndef MITKHISTOGRAMMATCHING_H
#define MITKHISTOGRAMMATCHING_H

#include "itkHistogramMatchingImageFilter.h"
#include "MitkDeformableRegistrationExports.h"

#include "mitkRegistrationBase.h"
#include "mitkImageAccessByItk.h"

namespace mitk
{

  /*!
  \brief This class performes a histogram matching between the fixed image and moving image to improve the registration results.

  \ingroup DeformableRegistration

  \author Daniel Stein
  */

  class MITK_DEFORMABLEREGISTRATION_EXPORT HistogramMatching : public RegistrationBase
  {

  public:
    mitkClassMacro(HistogramMatching, RegistrationBase);

    /*!
    * \brief Method for creation through the object factory.
    */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /*!
    * \brief Sets the number of match points for the histogram generation.
    */
    void SetNumberOfMatchPoints(int matchPoints);

    /*!
    * \brief Sets the number of histogram levels for the histogram generation.
    */
    void SetNumberOfHistogramLevels(int histogrammLevels);

    /*!
    * \brief Sets whether a threshold at mean intensity should be used for the histogram generation.
    */
    void SetThresholdAtMeanIntensity(bool on);

    /*!
    * \brief Starts the histogram matching.
    */
    virtual void GenerateData()
    {
      if (this->GetInput())
      {
        AccessByItk(this->GetInput(), GenerateData2);
      }
    }

  protected:

    /*!
    * \brief Default constructor
    */
    HistogramMatching();

    /*!
    * \brief Default destructor
    */
    virtual ~HistogramMatching();

    /*!
    * \brief Template class to perform the histogram matching with any kind of image. Called by GenerateData().
    */
    template < typename TPixel, unsigned int VImageDimension >
    void GenerateData2( const itk::Image<TPixel, VImageDimension>* itkImage1);

    bool m_ThresholdAtMeanIntensity;
    int m_NumberOfHistogramLevels;
    int m_NumberOfMatchPoints;
  };

} // end namespace
#endif // MITKHISTOGRAMMATCHING_H
