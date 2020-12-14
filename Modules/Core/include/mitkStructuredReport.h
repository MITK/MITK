#pragma once

#include "MitkCoreExports.h"

#include "mitkBaseData.h"
#include "mitkCommon.h"

class DSRDocument;

namespace mitk {

class MITK_EXPORT StructuredReport : public BaseData {
public:
  mitkClassMacro(StructuredReport, BaseData);

  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

  std::shared_ptr<DSRDocument> reportData;

  std::string GetReportText();
  std::string GetPatientName();

  void Expand(unsigned int timeSteps) override;
  void SetRequestedRegionToLargestPossibleRegion() override;
  bool RequestedRegionIsOutsideOfTheBufferedRegion() override;
  bool VerifyRequestedRegion() override;
  bool IsInitialized() const override;
  void Clear() override;
  bool IsEmptyTimeStep(unsigned int t) const override;
  bool IsEmpty() const override;
  void SetRequestedRegion(const itk::DataObject *data) override;
  void SetGeometry(BaseGeometry* aGeometry3D) override;
  void SetTimeGeometry(TimeGeometry* geometry) override;
  void SetClonedGeometry(const BaseGeometry* aGeometry3D) override;
  void SetClonedTimeGeometry(const TimeGeometry* geometry) override;
  void SetClonedGeometry(const BaseGeometry* aGeometry3D, unsigned int time) override;
  void SetOrigin(const Point3D& origin) override;
  unsigned long GetMTime() const override;
  void Graft(const DataObject*) override;
  void InitializeTimeGeometry(unsigned int timeSteps = 1) override;
  void ClearData() override;
  void PrintSelf(std::ostream& os, itk::Indent indent) const override;

protected:
  StructuredReport();
  virtual ~StructuredReport();
};

};

