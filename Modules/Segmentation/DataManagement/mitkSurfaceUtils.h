#pragma once

#include "MitkSegmentationExports.h"

#include <mitkDataNode.h>
#include <mitkDataStorage.h>
#include <mitkEnumerationProperty.h>
#include <mitkImage.h>
#include <mitkSurface.h>

#include <vtkPolyData.h>

#include <itkProgressAccumulator.h>

namespace mitk
{
class MITKSEGMENTATION_EXPORT SurfaceCreator : public itk::ProcessObject
{

public:
  typedef SurfaceCreator                Self;
  typedef itk::ProcessObject            Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  itkNewMacro(Self);
  itkTypeMacro(SurfaceCreator, itk::ProcessObject);

  enum SurfaceCreationType {
    MITK,
    AGTK,
    ELASTIC_NET,
  };

  struct SurfaceCreationArgs {
    bool recreate = false;
    SurfaceCreationType creationType = SurfaceCreationType::MITK;
    float opacity = 1.f;
    bool decimation = false;
    float decimationRate = .2f;
    bool smooth = false;
    bool lightSmoothing = false; // Determines smoothing parameters if smoothing is enabled
    DataStorage::Pointer outputStorage = nullptr;
    bool overwrite = true;
    DataNode::Pointer removeOnComplete = nullptr; // Node will be removed after creating and adding new model
    int timestep = -1;
    int elasticIterations = 10;
    float elasticRelaxation = -1.f;
  };

  class SurfaceCreationTypeProperty : public EnumerationProperty
  {
  public:
    mitkClassMacro(SurfaceCreationTypeProperty, EnumerationProperty);

    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    mitkNewMacro1Param(SurfaceCreationTypeProperty, const IdType&);
    mitkNewMacro1Param(SurfaceCreationTypeProperty, const std::string&);

    virtual int GetSurfaceCreationType();

    using BaseProperty::operator=;

  protected:
    SurfaceCreationTypeProperty();
    SurfaceCreationTypeProperty(const IdType& value);
    SurfaceCreationTypeProperty(const std::string& value);

    virtual void AddSurfaceCreationTypes();

  private:
    SurfaceCreationTypeProperty& operator=(const SurfaceCreationTypeProperty&);
    virtual itk::LightObject::Pointer InternalClone() const override;
  };

  virtual void Update() override
  {
    UpdateProgress(0.f);

    try {
      UpdateOutputData(nullptr);
    }
    catch (itk::ProcessAborted&) {
      InvokeEvent(itk::AbortEvent());
      ResetPipeline();
      RestoreInputReleaseDataFlags();
    }
  }

  void setInput(DataNode::Pointer segmentation)
  {
    m_Input = segmentation;
  }

  DataNode::Pointer getOutput()
  {
    return m_Output;
  }

  void setArgs(SurfaceCreationArgs args)
  {
    m_Args = args;
  }

  // Deprecated methods for dumdums
  vtkSmartPointer<vtkPolyData> getPolyData();
  void populateCreationProperties(mitk::DataNode::Pointer segNode);

protected:
  SurfaceCreator();
  virtual ~SurfaceCreator() {}

  std::string generateLogStr();

  void GenerateData();

  DataNode::Pointer createModel();
  DataNode::Pointer recreateModel();

  static Image::Pointer extract3D(Image::Pointer multiDimImage, int targetTimeStep);

  DataNode::Pointer m_Input;
  DataNode::Pointer m_Output;

  SurfaceCreationArgs m_Args;

  vtkSmartPointer<vtkPolyData> createModelMitk(Image::Pointer segNode, SurfaceCreator::SurfaceCreationArgs args);
  vtkSmartPointer<vtkPolyData> createModelAgtk(Image::Pointer segNode, SurfaceCreator::SurfaceCreationArgs args);
  vtkSmartPointer<vtkPolyData> createModelElasticNet(Image::Pointer segNode, SurfaceCreator::SurfaceCreationArgs args);

  itk::ProgressAccumulator::Pointer m_ProgressAccumulator;

  Surface::Pointer m_4dRecreateSurface;

};
}

