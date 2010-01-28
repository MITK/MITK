/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkImageToImageFilter.h,v $
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKDEMONSREGISTRATION_H
#define MITKDEMONSREGISTRATION_H

#include "itkDemonsRegistrationFilter.h"
#include "MitkExtExports.h"

#include "mitkRegistrationBase.h"
#include "mitkImageAccessByItk.h"

namespace mitk
{

  /*!
  \brief This class performes a demons registration between two images.

  \ingroup DeformableRegistration

  \author Daniel Stein
  */

  class MitkExt_EXPORT DemonsRegistration : public RegistrationBase
  {

  public:
    
    mitkClassMacro(DemonsRegistration, RegistrationBase);

    /*!  
    * \brief Method for creation through the object factory. 
    */
    itkNewMacro(Self);

    /*!  
    * \brief Sets the number of iterations which will be performed during the registration process. 
    */
    void SetNumberOfIterations(int iterations);

    /*!  
    * \brief Sets the standard deviation used by the demons registration. 
    */
    void SetStandardDeviation(float deviation);

    /*!  
    * \brief Sets whether the resulting deformation field should be saved or not. 
    */
    void SetSaveDeformationField(bool saveField);

    /*!  
    * \brief Sets the filename for the resulting deformation field. 
    */
    void SetDeformationFieldFileName(const char* fieldName);

    /*!  
    * \brief Sets whether the result should be saved or not. 
    */
    void SetSaveResult(bool saveResult);

    /*!  
    * \brief Sets the filename for the resulting deformed image. 
    */
    void SetResultFileName(const char* resultName);

    /*!  
    * \brief Returns the deformation field, which results by the registration. 
    */
    itk::Image<itk::Vector<float, 3>,3>::Pointer GetDeformationField();
    
    /*!  
    * \brief Starts the demons registration. 
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
    DemonsRegistration();

    /*!  
    * \brief Default destructor
    */
    virtual ~DemonsRegistration();

    /*!  
    * \brief Template class to perform the demons registration with any kind of image. Called by GenerateData().
    */  
    template < typename TPixel, unsigned int VImageDimension >
      void GenerateData2( itk::Image<TPixel, VImageDimension>* itkImage1);

    int m_Iterations;
    float m_StandardDeviation;
    const char* m_FieldName;
    const char* m_ResultName;
    bool m_SaveField;
    bool m_SaveResult;
    itk::Image<class itk::Vector<float, 3>,3>::Pointer m_DeformationField;
  };
}

#endif // MITKDEMONSREGISTRATION_H
