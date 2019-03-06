#include "mitkSurfaceUtils.h"

#include <vtkCallbackCommand.h>

#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkProgressBar.h>
#include "mitkShowSegmentationAsAgtkSurface.h"
#include "mitkShowSegmentationAsSurface.h"

namespace mitk
{

float SurfaceCreator::m_AgtkProgressWeight = 1.f;
float SurfaceCreator::m_AgtkLastProgress = 0;
int SurfaceCreator::m_AgtkProgressSteps = 0;
int SurfaceCreator::m_AgtkProgressLastStep = 0;

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

DataNode::Pointer SurfaceCreator::createModel(DataNode::Pointer segNode, SurfaceCreationArgs args)
{
  if (args.outputStorage == nullptr) {
    MITK_ERROR << "Tried to create segmentation without specifying output storage\n";
    return nullptr;
  }

  segNode->SetProperty("Surface Type", SurfaceCreationTypeProperty::New(args.creationType));
  segNode->SetProperty("Surface Type.Smooth", BoolProperty::New(args.smooth));
  segNode->SetProperty("Surface Type.Decimation", FloatProperty::New(args.decimation * args.decimationRate));
  segNode->SetProperty("Surface Type.Light Smoothing", BoolProperty::New(args.lightSmoothing));

  if (args.overwrite) {
    auto childs = args.outputStorage->GetDerivations(segNode);
    for (const auto& children : *childs) {
      if (dynamic_cast<Surface*>(children->GetData()) != nullptr) {
        args.outputStorage->Remove(children);
      }
    }
  }

  switch (args.creationType) {
    default:
    case SurfaceCreationType::MITK:
      return createModelMitk(segNode, args);
    case SurfaceCreationType::AGTK:
      return createModelAgtk(segNode, args);
  }

  return nullptr;
}

DataNode::Pointer SurfaceCreator::recreateModel(DataNode::Pointer segNode, DataStorage::Pointer storage, int progressSteps)
{
  BaseProperty* surfTypeProp = segNode->GetProperty("Surface Type");
  if (surfTypeProp == nullptr) {
    return nullptr;
  }

  DataNode::Pointer previousModel = nullptr;

  auto childs = storage->GetDerivations(segNode);
  for (const auto& children : *childs) {
    if (dynamic_cast<Surface*>(children->GetData()) != nullptr) {
      previousModel = children;
      break;
    }
  }

  if (previousModel == nullptr) {
    return nullptr;
  }

  SurfaceCreationArgs args;
  args.creationType = (SurfaceCreationType)dynamic_cast<SurfaceCreationTypeProperty*>(surfTypeProp)->GetSurfaceCreationType();
  previousModel->GetOpacity(args.opacity, nullptr);

  args.decimationRate = 0.f;
  segNode->GetFloatProperty("Surface Type.Decimation", args.decimationRate);
  args.decimation = args.decimationRate > .0001f;
  args.smooth = true;
  segNode->GetBoolProperty("Surface Type.Smooth", args.smooth);
  args.lightSmoothing = false;
  segNode->GetBoolProperty("Surface Type.Light Smoothing", args.lightSmoothing);
  args.outputStorage = storage;
  args.progressSteps = progressSteps;
  if (progressSteps > 0 && args.creationType == SurfaceCreationType::AGTK) {
    ProgressBar::GetInstance()->AddStepsToDo(progressSteps);
  }
  args.overwrite = false;
  args.removeOnComplete = previousModel;

  return createModel(segNode, args);
}

template<typename TPixel, unsigned int VImageDimension>
void AccessItkPadFilter(const itk::Image<TPixel, VImageDimension>* itkImage, Image::Pointer& output)
{
  typedef itk::Image<TPixel, VImageDimension> itkImageType;

  typedef itk::ConstantPadImageFilter<itkImageType, itkImageType> PadFilterType;
  typename PadFilterType::Pointer padFilter = PadFilterType::New();

  itkImageType::SizeType paddingSize;
  paddingSize.Fill(1);

  padFilter->SetInput(itkImage);
  padFilter->SetPadBound(paddingSize);
  padFilter->SetConstant(0);
  padFilter->Update();

  CastToMitkImage<itkImageType>(padFilter->GetOutput(), output);

  // Move back by slice to adjust position by padding
  Point3D point = output->GetGeometry()->GetOrigin();
  auto spacing = output->GetGeometry()->GetSpacing();
  auto directionMatrix = itkImage->GetDirection();
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      point[i] -= spacing[i] * directionMatrix[i][j];
    }
  }
  output->SetOrigin(point);
}

DataNode::Pointer SurfaceCreator::createModelMitk(DataNode::Pointer segNode, SurfaceCreationArgs args)
{
  Image::Pointer image(dynamic_cast<Image*>(segNode->GetData()));

  // Add padding to image
  Image::Pointer imagePadded;
  AccessByItk_n(image, AccessItkPadFilter, (imagePadded));

  ShowSegmentationAsSurface::Pointer surfaceFilter = ShowSegmentationAsSurface::New();
  surfaceFilter->SetDataStorage(*args.outputStorage);
  surfaceFilter->SetPointerParameter("Input", imagePadded);
  surfaceFilter->SetPointerParameter("Group node", segNode);
  surfaceFilter->SetParameter("Show result", true);
  surfaceFilter->SetParameter("Sync visibility", false);
  surfaceFilter->SetParameter("Median kernel size", 3u);
  surfaceFilter->SetParameter("Decimate mesh", args.decimation);
  surfaceFilter->SetParameter("Decimation rate", args.decimationRate);
  surfaceFilter->SetParameter("Creation type", 1u);
  surfaceFilter->SetParameter("Model Opacity", args.opacity);
  surfaceFilter->SetParameter("Apply median", false);
  surfaceFilter->SetParameter("Smooth", args.smooth);
  surfaceFilter->SetPointerParameter("Remove Node on Complete", args.removeOnComplete);

  // Can be uncommented for additional smoothnes at cost of the computation time
  /*if (args.smooth) {
    double smoothingCoef = std::numeric_limits<double>::lowest();
    Vector3D spacing = image->GetGeometry()->GetSpacing();

    for (Vector3D::Iterator iter = spacing.Begin(); iter != spacing.End(); ++iter) {
      smoothingCoef = std::max(smoothingCoef, *iter);
    }

    surfaceFilter->SetParameter("Gaussian SD", (float)sqrt(smoothingCoef));
    surfaceFilter->SetParameter("Apply median", true);
  }*/

  surfaceFilter->StartAlgorithm();

  return surfaceFilter->getCreatedNode();
}


DataNode::Pointer SurfaceCreator::createModelAgtk(DataNode::Pointer segNode, SurfaceCreationArgs args)
{
  Image::Pointer image(dynamic_cast<Image*>(segNode->GetData()));


  vtkSmartPointer<vtkCallbackCommand> progressCallback = vtkSmartPointer<vtkCallbackCommand>::New();
  vtkSmartPointer<vtkCallbackCommand> endProgressCallback = vtkSmartPointer<vtkCallbackCommand>::New();
  if (args.progressSteps > 0) {
    progressCallback->SetCallback(SurfaceCreator::agtkOnProgress);
    endProgressCallback->SetCallback(SurfaceCreator::agtkOnEndProgress);
  }

  ShowSegmentationAsAgtkSurface::SurfaceComputingParameters surfaceParams;
  surfaceParams.blurSigma = .3f;
  if (args.lightSmoothing) {
    surfaceParams.smoothingIterations = 1;
    surfaceParams.smoothingRelaxation = .25f;
  } else {
    surfaceParams.smoothingIterations = 15;
    surfaceParams.smoothingRelaxation = .1f;
  }

  surfaceParams.smoothingType = ShowSegmentationAsAgtkSurface::SurfaceSmoothingType::WindowedSync;
  surfaceParams.decimationType = ShowSegmentationAsAgtkSurface::SurfaceDecimationType::None;
  surfaceParams.isResampling = true;
  surfaceParams.vtkProgressCallback = progressCallback;
  surfaceParams.vtkEndProgressCallback = endProgressCallback;

  if (args.decimation) {
    surfaceParams.decimationType = ShowSegmentationAsAgtkSurface::SurfaceDecimationType::DecimatePro;
    surfaceParams.reduction = args.decimationRate;
  } else {
    surfaceParams.decimationType = ShowSegmentationAsAgtkSurface::SurfaceDecimationType::None;
    surfaceParams.reduction = 0.f;
  }

  if (args.smooth) {
    surfaceParams.smoothingType = ShowSegmentationAsAgtkSurface::SurfaceSmoothingType::WindowedSync;
  } else {
    surfaceParams.isBlurImage = false;
    surfaceParams.levelValue = 0.f;
    surfaceParams.smoothingType = ShowSegmentationAsAgtkSurface::SurfaceSmoothingType::None;
    surfaceParams.decimationType = ShowSegmentationAsAgtkSurface::SurfaceDecimationType::None;
  }

  m_AgtkProgressWeight = 1.f / (1 + (float)args.smooth
      + (float)(surfaceParams.smoothingType != ShowSegmentationAsAgtkSurface::SurfaceSmoothingType::None)
      + (float)(surfaceParams.decimationType != ShowSegmentationAsAgtkSurface::SurfaceDecimationType::None));
  m_AgtkLastProgress = 0.;
  m_AgtkProgressSteps = args.progressSteps;
  m_AgtkProgressLastStep = 0;

  Surface::Pointer surface;
  AccessByItk_n(image, ShowSegmentationAsAgtkSurface::AccessItkAgtkSurfaceFilter, (args, surfaceParams, surface));

  DataNode::Pointer node = DataNode::New();

  float color[3];
  segNode->GetColor(color);
  node->SetColor(color);
  node->SetOpacity(args.opacity);

  node->SetData(surface);

  node->SetName(segNode->GetName() + "_Model");

  args.outputStorage->Add(node, segNode);

  if (args.removeOnComplete != nullptr) {
    args.outputStorage->Remove(args.removeOnComplete);
  }

  return node;
}

void SurfaceCreator::agtkOnProgress(vtkObject* caller, long unsigned int vtkNotUsed(eventId),
    void* vtkNotUsed(clientData), void* vtkNotUsed(callData))
{
  vtkPolyDataAlgorithm* filter = static_cast<vtkPolyDataAlgorithm*>(caller);
  float nextProgress = m_AgtkLastProgress + filter->GetProgress() * m_AgtkProgressWeight * m_AgtkProgressSteps;
  int stepDiff = (int)nextProgress - m_AgtkProgressLastStep;
  m_AgtkProgressLastStep = (int)nextProgress;
  if (stepDiff > 0) {
    ProgressBar::GetInstance()->Progress(stepDiff);
  }
}

void SurfaceCreator::agtkOnEndProgress(vtkObject* caller, long unsigned int vtkNotUsed(eventId),
  void* vtkNotUsed(clientData), void* vtkNotUsed(callData))
{
  vtkPolyDataAlgorithm* filter = static_cast<vtkPolyDataAlgorithm*>(caller);
  float nextProgress = m_AgtkLastProgress + filter->GetProgress() * m_AgtkProgressWeight * m_AgtkProgressSteps;
  m_AgtkLastProgress = nextProgress;
}

}
