#include "mitkSurfaceUtils.h"

#include <vtkCallbackCommand.h>

#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkImageCaster.h>
#include <mitkProgressBar.h>
#include "mitkShowSegmentationAsAgtkSurface.h"
#include "mitkShowSegmentationAsSurface.h"

#include <AutoplanLogging.h>

namespace mitk
{

SurfaceCreator::SurfaceCreationTypeProperty::SurfaceCreationTypeProperty()
{
  AddSurfaceCreationTypes();
  SetValue(static_cast<IdType>(SurfaceCreationType::MITK));
}

SurfaceCreator::SurfaceCreationTypeProperty::SurfaceCreationTypeProperty(const IdType& value)
{
  AddSurfaceCreationTypes();

  if (!IsValidEnumerationValue(value)) {
    SetValue(static_cast<IdType>(SurfaceCreationType::MITK));
    return;
  }

  SetValue(value);
}

SurfaceCreator::SurfaceCreationTypeProperty::SurfaceCreationTypeProperty(const std::string& value)
{
  AddSurfaceCreationTypes();

  if (!IsValidEnumerationValue(value)) {
    SetValue(static_cast<IdType>(SurfaceCreationType::MITK));
    return;
  }

  SetValue(value);
}

int SurfaceCreator::SurfaceCreationTypeProperty::GetSurfaceCreationType()
{
  return static_cast<int>(this->GetValueAsId());
}

void SurfaceCreator::SurfaceCreationTypeProperty::AddSurfaceCreationTypes()
{
  AddEnum("MITK", static_cast<IdType>(SurfaceCreationType::MITK));
  AddEnum("AGTK", static_cast<IdType>(SurfaceCreationType::AGTK));
}

itk::LightObject::Pointer SurfaceCreator::SurfaceCreationTypeProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}

DataNode::Pointer SurfaceCreator::createModel()
{
  m_Input->SetProperty("Surface Type", SurfaceCreationTypeProperty::New(m_Args.creationType));
  m_Input->SetProperty("Surface Type.Smooth", BoolProperty::New(m_Args.smooth));
  m_Input->SetProperty("Surface Type.Decimation", FloatProperty::New(m_Args.decimation * m_Args.decimationRate));
  m_Input->SetProperty("Surface Type.Light Smoothing", BoolProperty::New(m_Args.lightSmoothing));

  if (m_Args.outputStorage != nullptr && m_Args.overwrite) {
    auto childs = m_Args.outputStorage->GetDerivations(m_Input);
    for (const auto& children : *childs) {
      if (dynamic_cast<Surface*>(children->GetData()) != nullptr) {
        m_Args.outputStorage->Remove(children);
      }
    }
  }

  vtkSmartPointer<vtkPolyData> result;

  switch (m_Args.creationType) {
    default:
    case SurfaceCreationType::MITK:
      result = createModelMitk(m_Input, m_Args);
      break;
    case SurfaceCreationType::AGTK:
      result = createModelAgtk(m_Input, m_Args);
      break;
  }

  Surface::Pointer surface = Surface::New();
  surface->SetVtkPolyData(result, std::max(0, m_Args.timestep));

  DataNode::Pointer node = DataNode::New();

  float color[3];
  m_Input->GetColor(color);
  node->SetColor(color);
  node->SetOpacity(m_Args.opacity);

  node->SetData(surface);

  node->SetName(m_Input->GetName() + "_Model");

  if (m_Args.outputStorage != nullptr) {
    m_Args.outputStorage->Add(node, m_Input);

    if (m_Args.removeOnComplete != nullptr) {
      m_Args.outputStorage->Remove(m_Args.removeOnComplete);
    }
  }

  return node;
}

vtkSmartPointer<vtkPolyData> SurfaceCreator::getPolyData()
{
  if (m_Output == nullptr) {
    return nullptr;
  }

  mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>(m_Output->GetData());
  if (surface == nullptr) {
    return nullptr;
  }

  return surface->GetVtkPolyData();
}

void SurfaceCreator::populateCreationProperties(mitk::DataNode::Pointer segNode)
{
  if (m_Input == nullptr) {
    return;
  }

  segNode->SetProperty("Surface Type", SurfaceCreationTypeProperty::New(m_Args.creationType));
  segNode->SetProperty("Surface Type.Smooth", BoolProperty::New(m_Args.smooth));
  segNode->SetProperty("Surface Type.Decimation", FloatProperty::New(m_Args.decimation * m_Args.decimationRate));
  segNode->SetProperty("Surface Type.Light Smoothing", BoolProperty::New(m_Args.lightSmoothing));
}

SurfaceCreator::SurfaceCreator()
{
  m_Input = nullptr;
  m_Output = nullptr;

  m_ProgressAccumulator = itk::ProgressAccumulator::New();
}

std::string SurfaceCreator::generateLogStr()
{
  std::string result = "";

  result += "{";

  result += "\"algorithm\":";
  if (m_Args.creationType == SurfaceCreationType::MITK) {
    result += "\"MITK\"";
  } else if (m_Args.creationType == SurfaceCreationType::AGTK) {
    result += "\"AGTK\"";
  } else {
    result += "\"unknown\"";
  }
  result += ",";

  result += "\"smooth\":";
  result += m_Args.smooth ? "true" : "false";
  result += ",";

  result += "\"light_smoothing\":";
  result += m_Args.lightSmoothing ? "true" : "false";
  result += ",";

  result += "\"decimation\":";
  result += std::to_string((int)(m_Args.decimation * m_Args.decimationRate * 100));
  result += ",";

  result += "\"polygons\":";
  if (m_Output != nullptr) {
    mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>(m_Output->GetData());
    if (surface != nullptr) {
      result += std::to_string(surface->GetVtkPolyData(std::max(0, m_Args.timestep))->GetNumberOfPolys());
    } else {
      result += "-1";
    }
  } else {
    result += "-1";
  }

  result += "}";

  return result;
}

void SurfaceCreator::GenerateData()
{
  if (m_Input == nullptr) {
    MITK_INFO << "Can't create model. Input was not set.\n";
    return;
  }

  m_ProgressAccumulator->ResetProgress();
  m_ProgressAccumulator->SetMiniPipelineFilter(this);
  m_ProgressAccumulator->UnregisterAllFilters();

  if (m_Args.recreate) {
    m_Output = recreateModel();
  } else {
    m_Output = createModel();
  }

  // Log to server used method and amount of generated triangles
  Logger::Log::get().setAdditionalField("model_creation", generateLogStr().c_str());
  AUTOPLAN_INFO << "Model creation finished";
  Logger::Log::get().resetAdditionalField();

  m_ProgressAccumulator->SetMiniPipelineFilter(nullptr);
  m_ProgressAccumulator->UnregisterAllFilters();

  UpdateProgress(1.);
}

DataNode::Pointer SurfaceCreator::recreateModel()
{
  if (m_Args.outputStorage == nullptr) {
    MITK_ERROR << "Tried to recreate segmentation model without specifying output storage\n";
    return nullptr;
  }

  DataNode::Pointer previousModel = nullptr;

  auto childs = m_Args.outputStorage->GetDerivations(m_Input);
  for (const auto& children : *childs) {
    if (dynamic_cast<Surface*>(children->GetData()) != nullptr) {
      previousModel = children;
      break;
    }
  }

  if (previousModel == nullptr) {
    return nullptr;
  }

  BaseProperty* surfTypeProp = m_Input->GetProperty("Surface Type");

  if (surfTypeProp == nullptr || dynamic_cast<SurfaceCreationTypeProperty*>(surfTypeProp) == nullptr) {
    // Fallback for models generated the old way
    SurfaceCreationArgs args;
    args.creationType = SurfaceCreationType::AGTK;
    args.smooth = true;
    args.removeOnComplete = previousModel;
    args.outputStorage = m_Args.outputStorage;
    args.overwrite = false;
    args.timestep = m_Args.timestep;

    m_Args = args;
    return createModel();
  }

  SurfaceCreationArgs args;
  args.creationType = (SurfaceCreationType)dynamic_cast<SurfaceCreationTypeProperty*>(surfTypeProp)->GetSurfaceCreationType();
  previousModel->GetOpacity(args.opacity, nullptr);

  args.decimationRate = 0.f;
  m_Input->GetFloatProperty("Surface Type.Decimation", args.decimationRate);
  args.decimation = args.decimationRate > .0001f;
  args.smooth = true;
  m_Input->GetBoolProperty("Surface Type.Smooth", args.smooth);
  args.lightSmoothing = false;
  m_Input->GetBoolProperty("Surface Type.Light Smoothing", args.lightSmoothing);
  args.outputStorage = m_Args.outputStorage;
  args.overwrite = false;
  args.removeOnComplete = previousModel;
  args.timestep = m_Args.timestep;

  m_Args = args;

  return createModel();
}

Image::Pointer SurfaceCreator::extract3D(Image::Pointer multiDimImage, int targetTimeStep)
{
  Image::Pointer mitkImage3d = mitk::Image::New();
  AccessFixedDimensionByItk_n(multiDimImage, extract3Dfrom4DByItk, 4U, (mitkImage3d, targetTimeStep));
  return mitkImage3d;
}

vtkSmartPointer<vtkPolyData> SurfaceCreator::createModelMitk(DataNode::Pointer segNode, SurfaceCreator::SurfaceCreationArgs args)
{
  Image::Pointer image(dynamic_cast<Image*>(segNode->GetData()));
  if (image->GetDimension() > 3) {
    int targetTimeStep = args.timestep;
    if (targetTimeStep < 0) {
      MITK_WARN << "Timestep for 3d model was not selected. Fallbacking to first timestep.";
      targetTimeStep = 0;
    }
    image = extract3D(image, targetTimeStep);
  }

  ShowSegmentationAsSurface::Pointer surfaceFilter = ShowSegmentationAsSurface::New();

  ShowSegmentationAsSurface::InputImageType::Pointer itkImage;
  CastToItkImage(image, itkImage);

  surfaceFilter->SetInput(itkImage);
  ShowSegmentationAsSurface::SurfaceComputingParameters surfaceArgs;
  surfaceArgs.medianKernelSize = 3u;
  surfaceArgs.decimateMesh = args.decimation;
  surfaceArgs.decimationRate = args.decimationRate;
  surfaceArgs.applyMedian = false;
  surfaceArgs.smooth = args.smooth;

  // Can be uncommented for additional smoothnes at cost of the computation time
  /*if (args.smooth) {
    double smoothingCoef = std::numeric_limits<double>::lowest();
    Vector3D spacing = image->GetGeometry()->GetSpacing();

    for (Vector3D::Iterator iter = spacing.Begin(); iter != spacing.End(); ++iter) {
      smoothingCoef = std::max(smoothingCoef, *iter);
    }

    surfaceArgs.gaussianSD = (float)sqrt(smoothingCoef);
    surfaceArgs.applyMedian = true;
  }*/

  m_ProgressAccumulator->RegisterInternalFilter(surfaceFilter, 1.f);
  surfaceFilter->setArgs(surfaceArgs);

  surfaceFilter->Update();

  return surfaceFilter->GetOutput();
}

vtkSmartPointer<vtkPolyData> SurfaceCreator::createModelAgtk(DataNode::Pointer segNode, SurfaceCreationArgs args)
{
  Image::Pointer image(dynamic_cast<Image*>(segNode->GetData()));
  if (image->GetDimension() > 3) {
    int targetTimeStep = args.timestep;
    if (targetTimeStep < 0) {
      MITK_WARN << "Timestep for 3d model was not selected. Fallbacking to first timestep.";
      targetTimeStep = 0;
    }
    image = extract3D(image, targetTimeStep);
  }

  ShowSegmentationAsAgtkSurface::SurfaceComputingParameters surfaceParams;
  surfaceParams.blurSigma = .3f;
  if (args.lightSmoothing) {
    surfaceParams.smoothingIterations = 2;
    surfaceParams.smoothingRelaxation = .25f;
  } else {
    surfaceParams.smoothingIterations = 15;
    surfaceParams.smoothingRelaxation = .1f;
  }

  surfaceParams.decimationType = ShowSegmentationAsAgtkSurface::SurfaceDecimationType::None;
  surfaceParams.isResampling = false;

  if (args.decimation) {
    surfaceParams.decimationType = ShowSegmentationAsAgtkSurface::SurfaceDecimationType::DecimatePro;
    surfaceParams.reduction = args.decimationRate;
  } else {
    surfaceParams.decimationType = ShowSegmentationAsAgtkSurface::SurfaceDecimationType::None;
    surfaceParams.reduction = 0.f;
  }

  if (args.smooth) {
    if (args.lightSmoothing) {
      surfaceParams.smoothingType = ShowSegmentationAsAgtkSurface::SurfaceSmoothingType::Laplacian;
    } else {
      surfaceParams.smoothingType = ShowSegmentationAsAgtkSurface::SurfaceSmoothingType::WindowedSync;
    }
  } else {
    surfaceParams.isBlurImage = false;
    surfaceParams.levelValue = 0.f;
    surfaceParams.smoothingType = ShowSegmentationAsAgtkSurface::SurfaceSmoothingType::None;
    surfaceParams.decimationType = ShowSegmentationAsAgtkSurface::SurfaceDecimationType::None;
  }

  surfaceParams.spacing = image->GetGeometry()->GetSpacing();

  ShowSegmentationAsAgtkSurface::Pointer surfaceBuilder = ShowSegmentationAsAgtkSurface::New();
  surfaceBuilder->setArgs(args, surfaceParams);

  ShowSegmentationAsAgtkSurface::InputImageType::Pointer itkImage;
  CastToItkImage(image, itkImage);
  surfaceBuilder->SetInput(itkImage);

  m_ProgressAccumulator->RegisterInternalFilter(surfaceBuilder, 1.f);

  surfaceBuilder->Update();

  return surfaceBuilder->GetOutput();
}

}
