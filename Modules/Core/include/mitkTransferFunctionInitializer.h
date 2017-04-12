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

#ifndef MITK_TRANSFER_FUNCTION_MODE_CREATOR_H_HEADER_INCLUDED
#define MITK_TRANSFER_FUNCTION_MODE_CREATOR_H_HEADER_INCLUDED

#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkSmartPointer.h>

#include "mitkTransferFunction.h"

#include <itkObject.h>
#include <itkObjectFactory.h>

#include <MitkCoreExports.h>

namespace mitk {

/**
 * \brief Wrapper class for VTK scalar opacity, gradient opacity, and color
 * transfer functions.
 *
 * Holds a copy of each of the three standard VTK transfer functions (scalar
 * opacity, gradient opacity, color) and provides an interface for manipulating
 * their control points. Each original function can be retrieved by a Get()
 * method.
 *
 * NOTE: Currently, transfer function initialization based on histograms or
 * computed-tomography-presets is also provided by this class, but will likely
 * be separated into a specific initializer class.
 */
  class MITKCORE_EXPORT TransferFunctionInitializer : public itk::Object
{
public:
  mitkClassMacroItkParent(TransferFunctionInitializer, itk::Object);

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
  mitkNewMacro1Param(TransferFunctionInitializer, TransferFunction::Pointer);

  static void GetPresetNames(std::vector<std::string>& presetNames);

  void SetTransferFunction(TransferFunction::Pointer transferFunction);
  mitk::TransferFunction::Pointer GetTransferFunction();
  void SetTransferFunctionMode(int mode);

  void InitTransferFunctionMode();

protected:
  TransferFunctionInitializer(TransferFunction::Pointer transferFunction = nullptr);
  virtual ~TransferFunctionInitializer();

private:
  int m_Mode;
  mitk::TransferFunction::Pointer m_transferFunction;
};

}

#endif /* MITK_TRANSFER_FUNCTION_MODE_CREATOR_H_HEADER_INCLUDED */

