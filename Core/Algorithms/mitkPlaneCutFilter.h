#ifndef PLANECUTFILTER_H_HEADER_INCLUDED_C1F48A22
#define PLANECUTFILTER_H_HEADER_INCLUDED_C1F48A22

#include "mitkCommon.h"
#include "mitkImageToImageFilter.h"

namespace mitk {

//##Documentation
//## @brief Filter to cut an image with planes. Everything in 
//## the direction of the normal of the planes will be set to a 
//## specified value.
//## @todo only one plane supported yet, add 4D support
//## @warning first version, not yet tested.
//## @ingroup Process
class PlaneCutFilter : public ImageToImageFilter
{
public:
    mitkClassMacro(PlaneCutFilter, ImageToImageFilter);

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

	typedef enum {Fill, FillInverse} CreationMode;

    //##Documentation
    //## @brief Set background grey level
	itkSetMacro(BackgroundLevel, float);
	itkGetMacro(BackgroundLevel, float);

	itkSetMacro(CreationMode, CreationMode);
	itkGetMacro(CreationMode, CreationMode);

	itkSetMacro(Planes, PlaneView*);
	itkGetMacro(Planes, const PlaneView*);
protected:
    virtual void GenerateData();

    PlaneCutFilter();

    ~PlaneCutFilter();

	float m_BackgroundLevel;

	const PlaneView *m_Planes;

	CreationMode m_CreationMode;
};

} // namespace mitk

#endif /* PLANECUTFILTER_H_HEADER_INCLUDED_C1F48A22 */

