#pragma once

#include "MitkSegmentationExports.h"

#include <mitkDataNode.h>
#include <mitkDataStorage.h>
#include <mitkEnumerationProperty.h>

namespace mitk
{
class MITKSEGMENTATION_EXPORT SurfaceCreator {
public:
  enum SurfaceCreationType {
    MITK,
    AGTK,
  };

  struct SurfaceCreationArgs {
    SurfaceCreationType creationType = SurfaceCreationType::MITK;
    float opacity = 1.f;
    bool decimation = false;
    float decimationRate = .2f;
    bool smooth = false;
    bool lightSmoothing = false; // Determines smoothing parameters if smoothing is enabled
    DataStorage::Pointer outputStorage = nullptr;
    int progressSteps = -1;
    bool overwrite = true;
    DataNode::Pointer removeOnComplete = nullptr; // Node will be removed after creating and adding new model
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

  static DataNode::Pointer createModel(DataNode::Pointer segNode, SurfaceCreationArgs args);
  static DataNode::Pointer recreateModel(DataNode::Pointer segNode, DataStorage::Pointer storage, int progressSteps = -1);

private:
  static DataNode::Pointer createModelMitk(DataNode::Pointer segNode, SurfaceCreationArgs args);
  static DataNode::Pointer createModelAgtk(DataNode::Pointer segNode, SurfaceCreationArgs args);

  static float m_AgtkProgressWeight;
  static float m_AgtkLastProgress;
  static int m_AgtkProgressSteps;
  static int m_AgtkProgressLastStep;
  static void agtkOnProgress(vtkObject* caller, long unsigned int vtkNotUsed(eventId),
      void* vtkNotUsed(clientData), void* vtkNotUsed(callData));
  static void agtkOnEndProgress(vtkObject* caller, long unsigned int vtkNotUsed(eventId),
    void* vtkNotUsed(clientData), void* vtkNotUsed(callData));
};
}

