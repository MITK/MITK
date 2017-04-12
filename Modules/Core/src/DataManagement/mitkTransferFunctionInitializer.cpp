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

#include "mitkTransferFunctionInitializer.h"

#include "mitkTransferFunctionPresetManager.h"

namespace mitk
{

  TransferFunctionInitializer::TransferFunctionInitializer(mitk::TransferFunction::Pointer transferFunction)
{
  if (transferFunction.IsNotNull())
  {
    m_transferFunction = transferFunction;
  }
}

TransferFunctionInitializer::~TransferFunctionInitializer()
{
}

void TransferFunctionInitializer::GetPresetNames(std::vector<std::string>& names)
{
  TransferFunctionPresetManager::GetInstance().GetPresetNames(names);
}

void TransferFunctionInitializer::SetTransferFunction(mitk::TransferFunction::Pointer transferFunction)
{
  m_transferFunction = transferFunction;
}

void TransferFunctionInitializer::SetTransferFunctionMode( int mode )
{
  if(m_transferFunction)
  {
    m_Mode = mode;
    this->InitTransferFunctionMode();
  }
}

void TransferFunctionInitializer::InitTransferFunctionMode()
{
  if(m_transferFunction)
  {
    if (!TransferFunctionPresetManager::GetInstance().ApplyPreset(m_Mode, *m_transferFunction)) {
      itkExceptionMacro(<< "No Mode set!");
    }
  } else {
    itkExceptionMacro(<< "No Transferfunction set!");
  }
}

mitk::TransferFunction::Pointer TransferFunctionInitializer::GetTransferFunction()
{
  return m_transferFunction;
}
} // namespace
