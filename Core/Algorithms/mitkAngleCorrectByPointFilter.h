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
  
  itkSetMacro(Center, ITKPoint3D);
	itkGetConstReferenceMacro(Center, ITKPoint3D);

  itkSetMacro(TransducerPosition, ITKPoint3D);
	itkGetConstReferenceMacro(TransducerPosition, ITKPoint3D);

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

  ITKPoint3D m_TransducerPosition;
  ITKPoint3D m_Center;

  bool m_PreferTransducerPositionFromProperty;
};

} // namespace mitk

#endif /* MITKANGLECORRECTBYPOINTFILTER_H_HEADER_INCLUDED_C1F48A22 */

