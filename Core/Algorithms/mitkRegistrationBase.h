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

#ifndef MITKREGISTRATIONBASE_H
#define MITKREGISTRATIONBASE_H

#include "mitkImageToImageFilter.h"

namespace mitk {

  class MITK_CORE_EXPORT RegistrationBase : public ImageToImageFilter
  {
  public:
	  mitkClassMacro(RegistrationBase, ImageToImageFilter);
  
	  /** Method for creation through the object factory. */
	  itkNewMacro(Self);

    virtual void SetReferenceImage( Image::Pointer fixedImage);

  protected:
    RegistrationBase();
    virtual ~RegistrationBase();
    virtual void AddStepsToDo(int steps);
    virtual void SetProgress(const itk::EventObject&);
    virtual void SetRemainingProgress(int steps);
    Image::Pointer m_ReferenceImage;
  };

} // namespace mitk

#endif // MITKREGISTRATIONBASE_H
