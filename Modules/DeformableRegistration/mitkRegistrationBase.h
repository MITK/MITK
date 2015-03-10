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

#ifndef MITKREGISTRATIONBASE_H
#define MITKREGISTRATIONBASE_H

#include "mitkImageToImageFilter.h"
#include "MitkDeformableRegistrationExports.h"

namespace mitk {

  /*!
  \brief This class handles the images for the registration as well as taking care of the progress bar during the registration process.
  It is the base class for the registration classes.

  \ingroup DeformableRegistration

  \author Daniel Stein
  */

  class MITKDEFORMABLEREGISTRATION_EXPORT RegistrationBase : public ImageToImageFilter
  {
  public:
    mitkClassMacro(RegistrationBase, ImageToImageFilter);

    /*!
    * \brief Method for creation through the object factory.
    */
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /*!
    * \brief Sets the reference image (fixed image) for the registration.
    */
    virtual void SetReferenceImage( Image::Pointer fixedImage);

  protected:

    /*!
    * \brief Default constructor
    */
    RegistrationBase();

    /*!
    * \brief Default destructor
    */
    virtual ~RegistrationBase();

    /*!
    * \brief Adds steps to the progress bar, which will be done with AddStepsToDo(int steps) and SetRemainingProgress(int steps)
    */
    virtual void AddStepsToDo(int steps);

    /*!
    * \brief Sets one step of progress to the progress bar
    */
    virtual void SetProgress(const itk::EventObject&);

    /*!
    * \brief Sets the remaining progress to the progress bar
    */
    virtual void SetRemainingProgress(int steps);

    Image::Pointer m_ReferenceImage;
  };

} // namespace mitk

#endif // MITKREGISTRATIONBASE_H
