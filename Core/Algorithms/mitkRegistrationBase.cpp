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

  void RegistrationBase::SetProgress(const itk::EventObject& event)
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
