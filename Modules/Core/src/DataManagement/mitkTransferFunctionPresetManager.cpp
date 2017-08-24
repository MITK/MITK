#include <iterator>
#include <sstream>

#include <TranslationUtilities.h>

#include "mitkTransferFunctionPresetManager.h"

namespace
{

const mitk::TransferFunctionPreset presetCtGeneric = {
  { // grayvalue->opacity
    std::make_tuple(132.108911, 0.000000),
    std::make_tuple(197.063492, 0.041333),
    std::make_tuple(1087.917989, 0.700000)
  },
  { // gradient at grayvalue->opacity
    std::make_tuple(560.695000, 1.000000)
  },
  { // grayvalue->color
    std::make_tuple(176.881890, 0.650980, 0.000000, 0.000000),
    std::make_tuple(239.427822, 0.933333, 0.000000, 0.000000),
    std::make_tuple(301.973753, 1.000000, 0.800000, 0.062745),
    std::make_tuple(495.866142, 1.000000, 0.905882, 0.666667),
    std::make_tuple(677.249344, 1.000000, 0.882353, 0.215686),
    std::make_tuple(808.595801, 1.000000, 1.000000, 1.000000)
  }
};

const mitk::TransferFunctionPreset presetCtBlackWhite = {
  { //Set Opacity
    std::make_tuple(135.063521, 0.0),
    std::make_tuple(948.137931, 1.0)
  },
  { //Set Gradient
    std::make_tuple(560.695000, 1)
  },
  { //Set Color
    std::make_tuple(122.088929, 0.352941, 0.352941, 0.352941),
    std::make_tuple(372.931034, 1.000000, 1.000000, 1.000000)
  }
};

const mitk::TransferFunctionPreset presetCtThoraxLarge = {
  { // grayvalue->opacity
    std::make_tuple(76.721239, 0.000000),
    std::make_tuple(139.524336, 0.000000),
    std::make_tuple(274.458333, 0.228650),
    std::make_tuple(638.420139, 0.721763)
  },
  { // gradient at grayvalue->opacity
    std::make_tuple(560.695000, 1.000000)
  },
  { // grayvalue->color
    std::make_tuple(85.382743, 0.478431, 0.000000, 0.000000),
    std::make_tuple(198.201327, 0.933333, 0.000000, 0.000000),
    std::make_tuple(254.610619, 1.000000, 0.800000, 0.062745),
    std::make_tuple(336.0907085, 1.000000, 0.905882, 0.666667),
    std::make_tuple(630.672566, 1.000000, 1.000000, 1.000000)
  }
};

const mitk::TransferFunctionPreset presetCtThoraxSmall = {
  { // grayvalue->opacity
    std::make_tuple(147.216912, 0.000000),
    std::make_tuple(274.458333, 0.228650),
    std::make_tuple(430.330882, 0.675532)
  },
  { // gradient at grayvalue->opacity
    std::make_tuple(560.695000, 1.000000)
  },
  { // grayvalue->color
    std::make_tuple(129.607774, 0.478431, 0.000000, 0.000000),
    std::make_tuple(213.812721, 0.933333, 0.000000, 0.000000),
    std::make_tuple(348.540636, 1.000000, 0.800000, 0.062745),
    std::make_tuple(500.419118, 1.000000, 0.898039, 0.776471),
    std::make_tuple(579.268382, 1.000000, 1.000000, 1.000000)
  }
};

const mitk::TransferFunctionPreset presetCtBone = {
  { // grayvalue->opacity
    std::make_tuple(126.413793, 0.000000),
    std::make_tuple(178.312160, 0.014663),
    std::make_tuple(247.509982, 0.000000),
    std::make_tuple(1013.010889, 1.000000)
  },
  { // gradient at grayvalue->opacity
    std::make_tuple(485.377495, 1.000000)
  },
  { // grayvalue->color
    std::make_tuple(312.382940, 1.000000, 0.564706, 0.274510),
    std::make_tuple(455.103448, 1.000000, 0.945098, 0.768627),
    std::make_tuple(623.773140, 1.000000, 0.800000, 0.333333),
    std::make_tuple(796.767695, 1.000000, 0.901961, 0.815686),
    std::make_tuple(930.838475, 1.000000, 1.000000, 1.000000),
    std::make_tuple(1073.558984, 1.000000, 0.839216, 0.423529),
    std::make_tuple(1220.604356, 1.000000, 0.772549, 0.490196)
  }
};

const mitk::TransferFunctionPreset presetCtBoneGradient = {
  { // grayvalue->opacity
    std::make_tuple(126.413793, 0.000000),
    std::make_tuple(186.961887, 0.146628),
    std::make_tuple(247.509982, 0.000000),
    std::make_tuple(1013.010889, 1.000000)
  },
  { // gradient at grayvalue->opacity
    std::make_tuple(22.617060, 0.000000),
    std::make_tuple(65.865699, 1.000000)
  },
  { // grayvalue->color
    std::make_tuple(312.382940, 1.000000, 0.564706, 0.274510),
    std::make_tuple(455.103448, 1.000000, 0.945098, 0.768627),
    std::make_tuple(623.773140, 1.000000, 0.800000, 0.333333),
    std::make_tuple(796.767695, 1.000000, 0.901961, 0.815686),
    std::make_tuple(930.838475, 1.000000, 1.000000, 1.000000),
    std::make_tuple(1073.558984, 1.000000, 0.839216, 0.423529),
    std::make_tuple(1220.604356, 1.000000, 0.772549, 0.490196)
  }
};

const mitk::TransferFunctionPreset presetCtCardiac = {
  { //Set Opacity
    std::make_tuple(150.246824, 0.000000),
    std::make_tuple(179.974592, 0.202346),
    std::make_tuple(276.589837, 0.000000),
    std::make_tuple(781.961887, 1.000000)
  },
  { //Set Gradient
    std::make_tuple(246.862069, 0.215827)
  },
  { //Set Color
    std::make_tuple(395.500907, 1.000000, 0.000000, 0.000000),
    std::make_tuple(410.364791, 1.000000, 0.749020, 0.000000),
    std::make_tuple(484.684211, 1.000000, 0.878431, 0.662745),
    std::make_tuple(588.731397, 1.000000, 0.784314, 0.482353)
  }
};

const mitk::TransferFunctionPreset presetMrGeneric = {
  { //Set Opacity
    std::make_tuple(0, 0),
    std::make_tuple(20, 0),
    std::make_tuple(40, 0.15),
    std::make_tuple(120, 0.3),
    std::make_tuple(220, 0.375),
    std::make_tuple(1024, 0.5)
  },
  { //Set Gradient
    std::make_tuple(0, 1),
    std::make_tuple(255, 1)
  },
  { //Set Color
    std::make_tuple(0, 0, 0, 0),
    std::make_tuple(20, 0.168627, 0, 0),
    std::make_tuple(40, 0.403922, 0.145098, 0.0784314),
    std::make_tuple(120, 0.780392, 0.607843, 0.380392),
    std::make_tuple(220, 0.847059, 0.835294, 0.788235),
    std::make_tuple(1024, 1, 1, 1)
  }
};
}

namespace mitk
{
TransferFunctionPresetManager& TransferFunctionPresetManager::GetInstance()
{
  static TransferFunctionPresetManager instance;
  return instance;
}

TransferFunctionPresetManager::TransferFunctionPresetManager()
  : m_presetNames(TranslationUtilities::getTranslatedTransferFunctionPresets())
  , m_presetPoints({ presetCtGeneric, presetCtBlackWhite, presetCtThoraxLarge, presetCtThoraxSmall,
    presetCtBone, presetCtBoneGradient, presetCtCardiac, presetMrGeneric })
{
}

TransferFunctionPresetManager::~TransferFunctionPresetManager()
{
}

void TransferFunctionPresetManager::GetPresetNames(std::vector<std::string>& presetNames) const
{
  presetNames.assign(m_presetNames.begin(), m_presetNames.end());
}

bool TransferFunctionPresetManager::ApplyPreset(int mode, mitk::TransferFunction& transferFunction) const
{
  if (mode < 0 || mode >= m_presetPoints.size()) {
    return false;
  }

  auto* scalarOpacityFunction = transferFunction.GetScalarOpacityFunction();
  auto* colorTransferFunction = transferFunction.GetColorTransferFunction();
  auto* gradientOpacityFunction = transferFunction.GetGradientOpacityFunction();

  scalarOpacityFunction->RemoveAllPoints();
  colorTransferFunction->RemoveAllPoints();
  gradientOpacityFunction->RemoveAllPoints();

  auto& preset = m_presetPoints[mode];

  for (auto& point : preset.m_scalarOpacityPoints) {
    scalarOpacityFunction->AddPoint(std::get<0>(point), std::get<1>(point));
  }
  for (auto& point : preset.m_colorTransferPoints) {
    colorTransferFunction->AddRGBPoint(std::get<0>(point), std::get<1>(point), std::get<2>(point), std::get<3>(point));
  }
  for (auto& point : preset.m_gradientOpacityPoints) {
    gradientOpacityFunction->AddPoint(std::get<0>(point), std::get<1>(point));
  }

  scalarOpacityFunction->Modified();
  colorTransferFunction->Modified();
  gradientOpacityFunction->Modified();

  return true;
}

int TransferFunctionPresetManager::SavePreset(mitk::TransferFunction& transferFunction, const std::string& name)
{
  auto* scalarOpacityFunction = transferFunction.GetScalarOpacityFunction();
  auto* colorTransferFunction = transferFunction.GetColorTransferFunction();
  auto* gradientOpacityFunction = transferFunction.GetGradientOpacityFunction();

  TransferFunctionPreset preset;

  const auto* scalarOpacityPoints = scalarOpacityFunction->GetDataPointer();
  const auto scalarOpacityPointsCount = scalarOpacityFunction->GetSize();
  for (auto i = 0; i < scalarOpacityPointsCount; ++i) {
    const auto j = i * 2;
    preset.m_scalarOpacityPoints.push_back(std::make_tuple(scalarOpacityPoints[j], scalarOpacityPoints[j + 1]));
  }

  const auto* gradientOpacityPoints = gradientOpacityFunction->GetDataPointer();
  const auto gradientOpacityPointsCount = gradientOpacityFunction->GetSize();
  for (auto i = 0; i < gradientOpacityPointsCount; ++i) {
    const auto j = i * 2;
    preset.m_gradientOpacityPoints.push_back(std::make_tuple(gradientOpacityPoints[j], gradientOpacityPoints[j + 1]));
  }

  const auto* colorTransferPoints = colorTransferFunction->GetDataPointer();
  const auto colorTransferPointsCount = colorTransferFunction->GetSize();
  for (auto i = 0; i < colorTransferPointsCount; ++i) {
    const auto j = i * 4;
    preset.m_colorTransferPoints.push_back(std::make_tuple(colorTransferPoints[j],
      colorTransferPoints[j + 1], colorTransferPoints[j + 2], colorTransferPoints[j + 3]));
  }

  return AddPreset(preset, name);
}

int TransferFunctionPresetManager::AddPreset(const TransferFunctionPreset& preset, const std::string& name)
{
  const auto index = m_presetPoints.size();
  if (name.empty()) {
    std::ostringstream out;
    out << TranslationUtilities::getDefaultUserPresetName() << (index - TF_MODE_COUNT);
    m_presetNames.push_back(out.str());
  } else {
    m_presetNames.push_back(name);
  }
  m_presetPoints.push_back(preset);
  return index;
}

bool TransferFunctionPresetManager::RemovePreset(int mode)
{
  if (TF_MODE_COUNT - 1 > mode || m_presetPoints.size() <= mode) {
    return false;
  }

  m_presetNames.erase(m_presetNames.begin() + mode + 1);
  m_presetPoints.erase(m_presetPoints.begin() + mode);

  return true;
}

void TransferFunctionPresetManager::RemoveUserPresets()
{
  m_presetNames.resize(TF_MODE_COUNT + 1);
  m_presetPoints.resize(TF_MODE_COUNT);
}

std::vector<std::pair<std::string, TransferFunctionPreset>> TransferFunctionPresetManager::GetUserPresets() const
{
  std::vector<std::pair<std::string, TransferFunctionPreset>> result;
  for (size_t i = TF_MODE_COUNT; i < m_presetPoints.size(); ++i) {
    result.push_back(std::make_pair(m_presetNames[i + 1], m_presetPoints[i]));
  }
  return result;
}
}