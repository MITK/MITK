#include "QmitkStepperAdapter.h"

QmitkStepperAdapter::QmitkStepperAdapter( QObject * navigator, mitk::Stepper * stepper, const char * name )
  : QObject( navigator, name ), m_Stepper(stepper)
{
  connect(this, SIGNAL(SendStepper(mitk::Stepper *)), navigator, SLOT(SetStepper(mitk::Stepper *)));
  connect(this, SIGNAL(Refetch()), navigator, SLOT(Refetch()));
  emit SendStepper(stepper);

  m_ItkEventListener = new ItkEventListener(this);
  m_Stepper->AddObserver(itk::ModifiedEvent(), m_ItkEventListener);

  emit Refetch();
}

QmitkStepperAdapter::~QmitkStepperAdapter()
{

}

