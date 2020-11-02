#include "mitkStructuredReport.h"

namespace mitk {

std::string StructuredReport::GetReportText()
{
  std::ostringstream ss;
  reportData.renderHTML(ss);
  return ss.str();
}

std::string StructuredReport::GetPatientName()
{
  std::string res;
  reportData.getPatientName(res);
  return res;
}

void StructuredReport::Expand(unsigned int timeSteps) {
}

void StructuredReport::SetRequestedRegionToLargestPossibleRegion() {
}

bool StructuredReport::RequestedRegionIsOutsideOfTheBufferedRegion() {
  return false;
}

bool StructuredReport::VerifyRequestedRegion() {
  return false;
}

bool StructuredReport::IsInitialized() const {
  return false;
}

void StructuredReport::Clear() {
}

bool StructuredReport::IsEmptyTimeStep(unsigned int t) const {
  if (t == 0) {
    return IsEmpty();
  } else {
    return false;
  }
}

bool StructuredReport::IsEmpty() const {
  return false;
}

void StructuredReport::SetRequestedRegion(const itk::DataObject *data) {
}

void StructuredReport::SetGeometry(BaseGeometry* aGeometry3D) {
}

void StructuredReport::SetTimeGeometry(TimeGeometry* geometry) {
}

void StructuredReport::SetClonedGeometry(const BaseGeometry* aGeometry3D) {
}

void StructuredReport::SetClonedTimeGeometry(const TimeGeometry* geometry) {
}

void StructuredReport::SetClonedGeometry(const BaseGeometry* aGeometry3D, unsigned int time) {
}

void StructuredReport::SetOrigin(const Point3D& origin) {
}

unsigned long StructuredReport::GetMTime() const {
  return 0L;
}

void StructuredReport::Graft(const DataObject*) {
}

void StructuredReport::InitializeTimeGeometry(unsigned int timeSteps) {
}

void StructuredReport::ClearData() {
}

void StructuredReport::PrintSelf(std::ostream& os, itk::Indent indent) const {
}


}

