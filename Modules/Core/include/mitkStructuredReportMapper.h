#pragma once

#include "mitkVtkMapper.h"
#include "mitkBaseRenderer.h"

namespace mitk {

class StructuredReport;

class MITKCORE_EXPORT StructuredReportMapper : public VtkMapper {

public:
  mitkClassMacro(StructuredReportMapper, VtkMapper);

  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

  virtual ~StructuredReportMapper();

  void* updateQtActorContext = nullptr;
  void(*updateQtActor)(StructuredReport* report, bool visible, void* context);
  void freeConnection();

  class MITKCORE_EXPORT LocalStorage : public Mapper::BaseLocalStorage {
  public:
    LocalStorage();

    itk::TimeStamp lastUpdateTime;
    vtkSmartPointer<vtkPropAssembly> actors;
  };

  vtkProp* GetVtkProp(BaseRenderer* renderer) override;

  void GenerateDataForRenderer(BaseRenderer* renderer) override;
  void Update(BaseRenderer* renderer) override;
private:
  LocalStorageHandler<LocalStorage> m_LSH;
};

}

