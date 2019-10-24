#include "mitkSurfaceUtils.h"

#include <chrono>

#include <vtkCallbackCommand.h>

#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkImageCaster.h>
#include <mitkImageTimeSelector.h>
#include <mitkProgressBar.h>
#include "mitkShowSegmentationAsAgtkSurface.h"
#include "mitkShowSegmentationAsElasticNetSurface.h"
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
  AddEnum("ElasticNet", static_cast<IdType>(SurfaceCreationType::ELASTIC_NET));
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

  if (m_Args.creationType == SurfaceCreationType::ELASTIC_NET) {
    m_Input->SetProperty("Surface Type.Elastic Iterations", IntProperty::New(m_Args.elasticIterations));
    m_Input->SetProperty("Surface Type.Elastic Relaxation", FloatProperty::New(m_Args.elasticRelaxation));
  } else {
    m_Input->SetProperty("Surface Type.Smooth", BoolProperty::New(m_Args.smooth));
    m_Input->SetProperty("Surface Type.Decimation", FloatProperty::New(m_Args.decimation * m_Args.decimationRate));
    m_Input->SetProperty("Surface Type.Light Smoothing", BoolProperty::New(m_Args.lightSmoothing));
  }

  if (m_Args.outputStorage != nullptr && m_Args.overwrite) {
    auto childs = m_Args.outputStorage->GetDerivations(m_Input);
    for (const auto& children : *childs) {
      if (dynamic_cast<Surface*>(children->GetData()) != nullptr) {
        m_Args.outputStorage->Remove(children);
      }
    }
  }

  vtkSmartPointer<vtkPolyData> result;

  Image::Pointer image(dynamic_cast<Image*>(m_Input->GetData()));
  int minTimeStep = 0;
  int maxTimeStep = 1;
  if (image->GetDimension() > 3) {
    if (m_Args.timestep >= 0) {
      minTimeStep = m_Args.timestep;
      maxTimeStep = m_Args.timestep + 1;
    } else {
      minTimeStep = 0;
      maxTimeStep = image->GetDimension(3);
    }
  }

  Surface::Pointer surface;
  if (m_4dRecreateSurface == nullptr) {
    surface = Surface::New();
  } else {
    surface = m_4dRecreateSurface;
  }

  auto t1 = std::chrono::high_resolution_clock::now();
  for (int curTimeStep = minTimeStep; curTimeStep < maxTimeStep; curTimeStep++) {
    Image::Pointer image3d = extract3D(image, curTimeStep);

    switch (m_Args.creationType) {
      default:
      case SurfaceCreationType::MITK:
        result = createModelMitk(image3d, m_Args);
        break;
      case SurfaceCreationType::AGTK:
        result = createModelAgtk(image3d, m_Args);
        break;
      case SurfaceCreationType::ELASTIC_NET:
        result = createModelElasticNet(image3d, m_Args);
        break;
    }

    surface->SetVtkPolyData(result, curTimeStep);
  }
  auto t2 = std::chrono::high_resolution_clock::now();

  auto duration = std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count();
  MITK_INFO << "Model generation took " << duration << " seconds.";

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

  if (m_Args.creationType == SurfaceCreationType::ELASTIC_NET) {
    m_Input->SetProperty("Surface Type.Elastic Iterations", IntProperty::New(m_Args.elasticIterations));
    m_Input->SetProperty("Surface Type.Elastic Relaxation", FloatProperty::New(m_Args.elasticRelaxation));
  } else {
    segNode->SetProperty("Surface Type.Smooth", BoolProperty::New(m_Args.smooth));
    segNode->SetProperty("Surface Type.Decimation", FloatProperty::New(m_Args.decimation * m_Args.decimationRate));
    segNode->SetProperty("Surface Type.Light Smoothing", BoolProperty::New(m_Args.lightSmoothing));
  }
}

SurfaceCreator::SurfaceCreator()
{
  m_Input = nullptr;
  m_Output = nullptr;
  m_4dRecreateSurface = nullptr;

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
    Surface::Pointer surface = dynamic_cast<Surface*>(m_Output->GetData());
    Image::Pointer image     = dynamic_cast<Image*>(m_Input->GetData());
    if (surface != nullptr) {
      unsigned int polygonCount = 0;
      for (int timestep = 0; timestep < image->GetDimension(3); timestep++) {
        vtkPolyData* polyData = surface->GetVtkPolyData(timestep);
        if (polyData != nullptr) {
          polygonCount += polyData->GetNumberOfPolys();
        }
      }
      result += std::to_string(polygonCount);
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
  Surface::Pointer previousSurface = nullptr;

  auto childs = m_Args.outputStorage->GetDerivations(m_Input);
  for (const auto& children : *childs) {
    previousSurface = dynamic_cast<Surface*>(children->GetData());
    if (previousSurface != nullptr) {
      previousModel = children;
      break;
    }
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

  Image::Pointer image = dynamic_cast<Image*>(m_Input->GetData());
  // Case for recreating only one timestep from 3d model for 4d seg
  if (image && image->GetDimension() > 3 && m_Args.timestep >= 0) {
    m_4dRecreateSurface = previousSurface;
  }

  SurfaceCreationArgs args;
  args.creationType = (SurfaceCreationType)dynamic_cast<SurfaceCreationTypeProperty*>(surfTypeProp)->GetSurfaceCreationType();
  if (previousModel != nullptr) {
    previousModel->GetOpacity(args.opacity, nullptr);
  } else {
    args.opacity = 1.f;
  }

  args.removeOnComplete = previousModel;
  args.timestep = m_Args.timestep;
  args.overwrite = false;
  args.outputStorage = m_Args.outputStorage;

  if (args.creationType == SurfaceCreationType::ELASTIC_NET) {
    m_Input->GetIntProperty("Surface Type.Elastic Iterations", args.elasticIterations);
    m_Input->GetFloatProperty("Surface Type.Elastic Relaxation", args.elasticRelaxation);
  } else {
    args.decimationRate = 0.f;
    m_Input->GetFloatProperty("Surface Type.Decimation", args.decimationRate);
    args.decimation = args.decimationRate > .0001f;
    args.smooth = true;
    m_Input->GetBoolProperty("Surface Type.Smooth", args.smooth);
    args.lightSmoothing = false;
    m_Input->GetBoolProperty("Surface Type.Light Smoothing", args.lightSmoothing);
  }

  m_Args = args;

  return createModel();
}

Image::Pointer SurfaceCreator::extract3D(Image::Pointer multiDimImage, int targetTimeStep)
{
  if (multiDimImage->GetDimension() == 3) {
    return multiDimImage;
  }
  ImageTimeSelector::Pointer timeSelector = ImageTimeSelector::New();
  timeSelector->SetInput(multiDimImage);
  timeSelector->SetTimeNr(targetTimeStep);
  timeSelector->UpdateLargestPossibleRegion();
  Image::Pointer mitkImage3d = timeSelector->GetOutput();
  return mitkImage3d;
}

vtkSmartPointer<vtkPolyData> SurfaceCreator::createModelMitk(Image::Pointer image, SurfaceCreator::SurfaceCreationArgs args)
{
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

vtkSmartPointer<vtkPolyData> SurfaceCreator::createModelAgtk(Image::Pointer image, SurfaceCreationArgs args)
{

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

vtkSmartPointer<vtkPolyData> SurfaceCreator::createModelElasticNet(Image::Pointer image, SurfaceCreationArgs args)
{
  ShowSegmentationAsElasticNetSurface::SurfaceComputingParameters surfaceParams;
  surfaceParams.elasticNetIterations = args.elasticIterations;
  surfaceParams.elasticNetRelaxation = args.elasticRelaxation;
  ShowSegmentationAsElasticNetSurface::Pointer surfaceBuilder = ShowSegmentationAsElasticNetSurface::New();
  surfaceBuilder->setArgs(args, surfaceParams);

  ShowSegmentationAsElasticNetSurface::InputImageType::Pointer itkImage;
  CastToItkImage(image, itkImage);
  surfaceBuilder->SetInput(itkImage);

  m_ProgressAccumulator->RegisterInternalFilter(surfaceBuilder, 1.f);

  surfaceBuilder->Update();

  return surfaceBuilder->GetOutput();
}
}
