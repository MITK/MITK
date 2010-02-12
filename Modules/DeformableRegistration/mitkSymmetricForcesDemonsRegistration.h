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

#ifndef MITKSYMMETRICFORCESDEMONSREGISTRATION_H
#define MITKSYMMETRICFORCESDEMONSREGISTRATION_H

#include "itkSymmetricForcesDemonsRegistrationFilter.h"
#include "MitkDeformableRegistrationExports.h"

#include "mitkRegistrationBase.h"
#include "mitkImageAccessByItk.h"

namespace mitk
{

  /*!
  \brief This class performes a symmetric forces demons registration between two images.

  \ingroup DeformableRegistration

  \author Daniel Stein
  */

  class MITK_DEFORMABLEREGISTRATION_EXPORT SymmetricForcesDemonsRegistration : public RegistrationBase
  {

  public:
    
    mitkClassMacro(SymmetricForcesDemonsRegistration, RegistrationBase);

    /*!  
    * \brief Method for creation through the object factory. 
    */
    itkNewMacro(Self);

    /*!  
    * \brief Sets the number of iterations which will be performed during the registration process. 
    */
    void SetNumberOfIterations(int iterations);

    /*!  
    * \brief Sets the standard deviation used by the symmetric forces demons registration. 
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
    itk::Image<class itk::Vector<float, 3>,3>::Pointer GetDeformationField();

    /*!  
    * \brief Starts the symmetric forces demons registration. 
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
    SymmetricForcesDemonsRegistration();

    /*!  
    * \brief Default destructor
    */
    virtual ~SymmetricForcesDemonsRegistration();

    /*!  
    * \brief Template class to perform the symmetric forces demons registration with any kind of image. Called by GenerateData().
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

#endif // MITKSYMMETRICFORCESDEMONSREGISTRATION_H
