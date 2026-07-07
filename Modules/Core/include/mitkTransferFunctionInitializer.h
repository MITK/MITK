/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTransferFunctionInitializer_h
#define mitkTransferFunctionInitializer_h

#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkSmartPointer.h>

#include <mitkTransferFunction.h>

#include <itkObject.h>
#include <itkObjectFactory.h>

#include <MitkCoreExports.h>

namespace mitk
{
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

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self)
      mitkNewMacro1Param(TransferFunctionInitializer, TransferFunction::Pointer);

    /**
     * \brief Populate a vector with the names of all available transfer function presets.
     * \param[out] presetNames Vector to be filled with preset name strings.
     */
    static void GetPresetNames(std::vector<std::string> &presetNames);

    /**
     * \brief Set the transfer function to be initialized.
     * \param[in] transferFunction The transfer function to configure.
     */
    void SetTransferFunction(TransferFunction::Pointer transferFunction);

    /**
     * \brief Return the currently held transfer function.
     * \return The transfer function, or nullptr if none has been set.
     */
    mitk::TransferFunction::Pointer GetTransferFunction();

    /**
     * \brief Set the transfer function preset mode and immediately apply it.
     *
     * Valid mode values correspond to the internal TransferFunctionMode enum
     * (e.g., 0 for CT Default, 7 for MR Generic).
     *
     * \param[in] mode The preset index to apply.
     */
    void SetTransferFunctionMode(int mode);

    /**
     * \brief Initialize the transfer function according to the currently set mode.
     * \pre A transfer function must have been set via SetTransferFunction() or the constructor.
     * \throw itk::ExceptionObject if no transfer function has been set.
     */
    void InitTransferFunctionMode();

  protected:
    TransferFunctionInitializer(TransferFunction::Pointer transferFunction = nullptr);
    ~TransferFunctionInitializer() override;

  private:
    int m_Mode;
    mitk::TransferFunction::Pointer m_transferFunction;

    // Define Transfer Function
    enum TransferFunctionMode
    {
      TF_CT_DEFAULT,
      TF_CT_BLACK_WHITE,
      TF_CT_THORAX_LARGE,
      TF_CT_THORAX_SMALL,
      TF_CT_BONE,
      TF_CT_BONE_GRADIENT,
      TF_CT_CARDIAC,
      TF_MR_GENERIC
    };

    // remove all old points
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

#endif
