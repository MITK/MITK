#ifndef MITKDOPPLERTOSTRAINRATEFILTER_H_HEADER_INCLUDED_C1F48A22
#define MITKDOPPLERTOSTRAINRATEFILTER_H_HEADER_INCLUDED_C1F48A22

#include "mitkCommon.h"
#include "ImageToImageFilter.h"
#include <vecmath.h>


#define M_PI       3.14159265358979323846
//#endif

namespace mitk {
//##Documentation
//## @brief 
//## @ingroup Process

class DopplerToStrainRateFilter : public ImageToImageFilter
{
public:
  mitkClassMacro(DopplerToStrainRateFilter, ImageToImageFilter);

	itkNewMacro(Self);

  itkSetMacro(Distance, int);
	itkGetMacro(Distance, int);

  itkSetMacro(NoStrainInterval, int);
	itkGetMacro(NoStrainInterval, int);

//  itkSetMacro(Origin, Point3<int>);
//	itkGetMacro(Origin, Point3<int>);
	
  void SetOrigin(Point3<int> origin){m_Origin = origin;};
  Point3<int> GetOrigin(){return m_Origin;};
  
protected:

  //##Description 
  //## @brief Time when Header was last initialized
  itk::TimeStamp m_TimeOfHeaderInitialization;
 
 
protected:
  virtual void GenerateData();

	virtual void GenerateOutputInformation();

	virtual void GenerateInputRequestedRegion();

  
	DopplerToStrainRateFilter();

    ~DopplerToStrainRateFilter();


private:

  int m_Distance;
  int m_NoStrainInterval;
  Point3<int> m_Origin;  
    
};

} // namespace mitk

#endif /* MITKDOPPLERTOSTRAINRATE_H_HEADER_INCLUDED_C1F48A22 */

