#include "mitkStructuredReportMapper.h"

#include <vtkTextActor.h>

#include "mitkStructuredReport.h"

namespace mitk {

StructuredReportMapper::~StructuredReportMapper()
{
  if (updateQtActorContext != nullptr) {
    updateQtActor(dynamic_cast<StructuredReport*>(m_DataNode->GetData()), false, updateQtActorContext);
  }
}

vtkProp* StructuredReportMapper::GetVtkProp(BaseRenderer* renderer)
{
  return m_LSH.GetLocalStorage(renderer)->actors;
}

void StructuredReportMapper::GenerateDataForRenderer(BaseRenderer* renderer)
{
  LocalStorage* localStorage = m_LSH.GetLocalStorage(renderer);
  
  DataNode* node = GetDataNode();
  StructuredReport* report = dynamic_cast<StructuredReport*>(node->GetData());

  if (report == nullptr) {
    return;
  }

  bool visible = false;
  node->GetVisibility(visible, renderer);

  localStorage->lastUpdateTime.Modified();
  if (updateQtActorContext != nullptr) {
    updateQtActor(report, visible, updateQtActorContext);
  }
}

void StructuredReportMapper::freeConnection()
{
  if (updateQtActorContext != nullptr) {
    DataNode* node = GetDataNode();
    StructuredReport* report = dynamic_cast<StructuredReport*>(node->GetData());

    if (report == nullptr) {
      return;
    }

    updateQtActor(report, false, updateQtActorContext);
  }
  updateQtActorContext = nullptr;
}

void StructuredReportMapper::Update(BaseRenderer* renderer)
{
  const DataNode* node = GetDataNode();
  LocalStorage* localStorage = m_LSH.GetLocalStorage(renderer);

  if (localStorage->lastUpdateTime < node->GetMTime() ||
      localStorage->lastUpdateTime < node->GetPropertyList()->GetMTime() ||
      localStorage->lastUpdateTime < node->GetPropertyList(renderer)->GetMTime() ||
      localStorage->lastUpdateTime < node->GetData()->GetPropertyList()->GetMTime() ||
      localStorage->lastUpdateTime < node->GetProperty("visible")->GetMTime() ||
      localStorage->lastUpdateTime < node->GetProperty("visible", renderer)->GetMTime()) {
    GenerateDataForRenderer(renderer);
  }
}

StructuredReportMapper::LocalStorage::LocalStorage()
{
  actors = vtkSmartPointer<vtkPropAssembly>::New();
}

}
