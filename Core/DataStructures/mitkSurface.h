#ifndef MITKSURFACEDATA_H_HEADER_INCLUDED_C19085D7
#define MITKSURFACEDATA_H_HEADER_INCLUDED_C19085D7

#include "mitkCommon.h"
#include "mitkBaseData.h"
#include "mitkTimeSlicedGeometry.h"
#include <vector>

class vtkPolyData;

namespace mitk {

//##ModelId=3E70F66001B9
//##Documentation
//## @brief Class for storing surfaces (vtkPolyData)
//## @ingroup Data
class Surface : public BaseData
{
public:
  mitkClassMacro(Surface, BaseData);
  
  itkNewMacro(Self);
  
  //##ModelId=3E70F661009A
  virtual void SetVtkPolyData(vtkPolyData* polydata, unsigned int t = 0);
  
  //##ModelId=3E70F66100A5
  virtual vtkPolyData* GetVtkPolyData(unsigned int t = 0);
  
  //##ModelId=3E70F66100AE
  virtual void UpdateOutputInformation();
  
  //##ModelId=3E70F66100B0
  virtual void SetRequestedRegionToLargestPossibleRegion();
  
  //##ModelId=3E70F66100B6
  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
  
  //##ModelId=3E70F66100B8
  virtual bool VerifyRequestedRegion();
  
  //##ModelId=3E70F66100BA
  virtual void SetRequestedRegion(itk::DataObject *data);
  
  //##ModelId=3E70F66100C1
  virtual void CopyInformation(const itk::DataObject *data);
  
  virtual void Update();
  
  virtual void Resize( unsigned int timeSteps );
  
  virtual TimeSlicedGeometry* GetTimeSlicedGeometry();

  virtual void Initialize(unsigned int timeSteps=1);

protected:
 
  typedef std::vector< vtkPolyData* > VTKPolyDataSeries;

  //##ModelId=3E70F66100C4
  Surface();
  
  //##ModelId=3E70F66100CA
  virtual ~Surface();
  
  //##ModelId=3E70F6610099
  VTKPolyDataSeries m_PolyDataSeries;
  
  bool m_CalculateBoundingBox;
};

} // namespace mitk



#endif /* MITKSURFACEDATA_H_HEADER_INCLUDED_C19085D7 */
