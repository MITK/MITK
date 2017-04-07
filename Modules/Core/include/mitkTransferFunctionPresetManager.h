#ifndef MITK_TRANSFER_FUNCTION_PRESET_MANAGER_H_HEADER_INCLUDED
#define MITK_TRANSFER_FUNCTION_PRESET_MANAGER_H_HEADER_INCLUDED

#include <vector>
#include <string>
#include <tuple>

#include <itkObject.h>
#include <itkObjectFactory.h>

#include <MitkCoreExports.h>

#include "mitkTransferFunction.h"

namespace mitk
{
struct TransferFunctionPreset
{
  std::vector<std::tuple<double, double>> m_scalarOpacityPoints;
  std::vector<std::tuple<double, double>> m_gradientOpacityPoints;
  std::vector<std::tuple<double, double, double, double>> m_colorTransferPoints;
};

class MITKCORE_EXPORT TransferFunctionPresetManager : public itk::Object
{
public:
  mitkClassMacroItkParent(TransferFunctionPresetManager, itk::Object);

  //Define Transfer Function
  enum TransferFunctionMode
  {
    TF_CT_DEFAULT,
    TF_CT_BLACK_WHITE,
    TF_CT_THORAX_LARGE,
    TF_CT_THORAX_SMALL,
    TF_CT_BONE,
    TF_CT_BONE_GRADIENT,
    TF_CT_CARDIAC,
    TF_MR_GENERIC,
    TF_MODE_COUNT
  };

  static TransferFunctionPresetManager& GetInstance();

  void GetPresetNames(std::vector<std::string>& presetNames) const;

  bool ApplyPreset(int mode, mitk::TransferFunction& transferFunction) const;

  int SavePreset(mitk::TransferFunction& transferFunction, const std::string& name = std::string());
  int AddPreset(const TransferFunctionPreset& preset, const std::string& name = std::string());

  bool RemovePreset(int mode);
  void RemoveUserPresets();

  std::vector<std::pair<std::string, TransferFunctionPreset>> GetUserPresets() const;

private:
  TransferFunctionPresetManager();
  ~TransferFunctionPresetManager();

  TransferFunctionPresetManager(const TransferFunctionPresetManager&) = delete;
  TransferFunctionPresetManager& operator=(const TransferFunctionPresetManager&) = delete;

  std::vector<std::string> m_presetNames;
  std::vector<TransferFunctionPreset> m_presetPoints;
};
}

#endif /* MITK_TRANSFER_FUNCTION_PRESET_MANAGER_H_HEADER_INCLUDED */
