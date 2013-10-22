#include "mitkUSAbstractControlInterface.h"
#include "mitkUSDevice.h"

mitk::USAbstractControlInterface::USAbstractControlInterface(itk::SmartPointer<USDevice> device)
  : m_Device(device)
{
}

mitk::USAbstractControlInterface::~USAbstractControlInterface()
{
}