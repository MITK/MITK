#ifndef __mitkAutoCropImageFilter_h_
#define __mitkAutoCropImageFilter_h_

#include "mitkCommon.h"
#include "mitkImageToImageFilter.h"
#include "mitkSubImageSelector.h"
#include "itkImageRegion.h"

namespace mitk {

/** 
 *
 * @brief This filter determines the bounding box of all pixels different 
 * from the background, crops the input image and returns it.
 * @ingroup Process
 * 
 */
class AutoCropImageFilter : public SubImageSelector
{
public:

  typedef itk::ImageRegion<3> RegionType;

	mitkClassMacro(AutoCropImageFilter,SubImageSelector);

	itkNewMacro(Self);  

	itkGetConstMacro(BackgroundValue,float);
	itkSetMacro(BackgroundValue,float);

	itkGetConstMacro(MarginFactor,float);
	itkSetMacro(MarginFactor,float);

protected:

  void Crop3DImage(mitk::ImageToImageFilter::InputImageConstPointer img);

protected:
	//##ModelId=3E1B1975031E
	AutoCropImageFilter();

	//##ModelId=3E1B1975033C
	virtual ~AutoCropImageFilter();

	//##ModelId=3E3BD0CC0232
    virtual void GenerateOutputInformation();

  virtual void GenerateInputRequestedRegion();

	//##ModelId=3E3BD0CE0194
    virtual void GenerateData();

  //##ModelId=3E1B1A0501C7
	float m_BackgroundValue;

  RegionType m_CroppingRegion;

  float m_MarginFactor;
};

} // namespace mitk



#endif /* AutoCropImageFilter_H_HEADER_INCLUDED_C1E4861D */
