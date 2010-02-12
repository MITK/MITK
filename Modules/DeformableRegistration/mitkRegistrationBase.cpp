/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include "mitkRegistrationBase.h"
#include "mitkProgressBar.h"

namespace mitk {

  RegistrationBase::RegistrationBase()
  {
    m_ReferenceImage = Image::New();
  }

  RegistrationBase::~RegistrationBase()
  {
  }

  void RegistrationBase::SetReferenceImage(Image::Pointer fixedImage)
  {
    m_ReferenceImage = fixedImage;
    SetNthInput(1, m_ReferenceImage);
    Modified();
  }

  void RegistrationBase::SetProgress(const itk::EventObject& )
  {
    ProgressBar::GetInstance()->AddStepsToDo(1);
    ProgressBar::GetInstance()->Progress();
  }

  void RegistrationBase::AddStepsToDo(int steps)
  {
    ProgressBar::GetInstance()->AddStepsToDo(steps);
  }

  void RegistrationBase::SetRemainingProgress(int steps)
  {
    ProgressBar::GetInstance()->Progress(steps);
  }
}
