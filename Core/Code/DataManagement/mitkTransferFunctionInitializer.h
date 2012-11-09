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

#include <MitkExports.h>

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
  class MITK_CORE_EXPORT TransferFunctionInitializer : public itk::Object
{
public:
  mitkClassMacro(TransferFunctionInitializer, itk::Object);

  itkNewMacro( TransferFunctionInitializer );
  mitkNewMacro1Param(TransferFunctionInitializer, TransferFunction::Pointer);

  static void GetPresetNames(std::vector<std::string>& presetNames);

  void SetTransferFunction(TransferFunction::Pointer transferFunction);
  mitk::TransferFunction::Pointer GetTransferFunction();
  void SetTransferFunctionMode(int mode);

  void InitTransferFunctionMode();

protected:
  TransferFunctionInitializer(TransferFunction::Pointer transferFunction = NULL);
  virtual ~TransferFunctionInitializer();

private:
  int m_Mode;
  mitk::TransferFunction::Pointer m_transferFunction;

  //Define Transfer Function
  enum TransferFunctionMode{
   TF_CT_DEFAULT,
   TF_CT_BLACK_WHITE,
   TF_CT_THORAX_LARGE,
   TF_CT_THORAX_SMALL,
   TF_CT_BONE,
   TF_CT_BONE_GRADIENT,
   TF_CT_CARDIAC,
   TF_MR_GENERIC
  };

  //remove all old points
  void RemoveAllPoints();
  void SetModified();
  void SetCtDefaultMode();
  void SetCtBlackWhiteMode();
  void SetCtThoraxLargeMode();
  void SetCtThoraxSmallMode();
  void SetCtBoneMode();
  void SetCtBoneGradientMode();
  void SetCtCardiacMode();
  void SetMrGenericMode();

};

}

#endif /* MITK_TRANSFER_FUNCTION_MODE_CREATOR_H_HEADER_INCLUDED */
