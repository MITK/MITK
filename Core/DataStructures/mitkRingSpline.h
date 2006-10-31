#ifndef MITKRingSpline_H_HEADER_INCLUDED
#define MITKRingSpline_H_HEADER_INCLUDED

#include <mitkClosedSpline.h>
#include <mitkCommon.h>
#include <mitkPlaneFit.h>

class vtkCardinalSpline;

namespace mitk {

class RingSpline: public ClosedSpline 
{
public:
  mitkClassMacro(RingSpline, ClosedSpline);

  itkNewMacro(Self);

protected:

  RingSpline();
  virtual ~RingSpline();

  //!@brief sorting points
  virtual void DoSortPoints();


private:

  PlaneFit::Pointer m_PlaneFit;
 
};

} // namespace mitk

#endif /* MITKRingSpline_H_HEADER_INCLUDED */
