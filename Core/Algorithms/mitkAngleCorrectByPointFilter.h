#ifndef MITKANGLECORRECTBYPOINTFILTER_H_HEADER_INCLUDED_C1F48A22
#define MITKANGLECORRECTBYPOINTFILTER_H_HEADER_INCLUDED_C1F48A22

#include "mitkCommon.h"
#include "mitkImageToImageFilter.h"
#include "mitkVector.h"

namespace mitk {

//##Documentation
//## @brief 
//## @ingroup Process
class AngleCorrectByPointFilter : public ImageToImageFilter
{
public:
  mitkClassMacro(AngleCorrectByPointFilter, ImageToImageFilter);

	itkNewMacro(Self);
  
  itkSetMacro(Center, Point3D);
	itkGetConstReferenceMacro(Center, Point3D);

  itkSetMacro(TransducerPosition, Point3D);
	itkGetConstReferenceMacro(TransducerPosition, Point3D);

  itkSetMacro(PreferTransducerPositionFromProperty, bool);
	itkGetMacro(PreferTransducerPositionFromProperty, bool);

protected:

  //##Description 
  //## @brief Time when Header was last initialized
  itk::TimeStamp m_TimeOfHeaderInitialization;
 
protected:
	AngleCorrectByPointFilter();

  ~AngleCorrectByPointFilter();

  virtual void GenerateData();

	virtual void GenerateOutputInformation();

	virtual void GenerateInputRequestedRegion();

  Point3D m_TransducerPosition;
  Point3D m_Center;

  bool m_PreferTransducerPositionFromProperty;
};

} // namespace mitk

#endif /* MITKANGLECORRECTBYPOINTFILTER_H_HEADER_INCLUDED_C1F48A22 */

