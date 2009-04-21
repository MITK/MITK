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

#include "mitkRegistrationBase.h"
#include "mitkImageAccessByItk.h"

namespace mitk
{

  class MITK_CORE_EXPORT SymmetricForcesDemonsRegistration : public RegistrationBase
  {

  public:
    
    mitkClassMacro(SymmetricForcesDemonsRegistration, RegistrationBase);

    itkNewMacro(Self);

    void SetNumberOfIterations(int iterations);
    void SetStandardDeviation(float deviation);
    void SetSaveDeformationField(bool saveField);
    void SetDeformationFieldFileName(const char* fieldName);
    void SetSaveResult(bool saveResult);
    void SetResultFileName(const char* resultName);
    itk::Image<class itk::Vector<float, 3>,3>::Pointer GetDeformationField();

    virtual void GenerateData()
    {
      if (this->GetInput())
      {
        AccessByItk(this->GetInput(), GenerateData2);
      }
    }    

  protected:
    SymmetricForcesDemonsRegistration();
    virtual ~SymmetricForcesDemonsRegistration();
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
