#ifndef PLANECUTFILTER_H_HEADER_INCLUDED_C1F48A22
#define PLANECUTFILTER_H_HEADER_INCLUDED_C1F48A22

#include "mitkCommon.h"
#include "ImageToImageFilter.h"

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

	typedef enum {NewVolume, Set, SetInverse} CreationMode;

    //##Documentation
    //## @brief Set background grey level
	itkSetMacro(BackgroundLevel, float);
	itkGetMacro(BackgroundLevel, float);

	ipSetMacro(CreationMode, CreationMode);
	ipGetMacro(CreationMode, CreationMode);

//	ipSetMacro(Planes, PlaneView*);
//	ipGetMacro(Planes, PlaneView*);
protected:
    virtual void GenerateData();

    PlaneCutFilter();

    ~PlaneCutFilter();

	float m_BackgroundLevel;

	PlaneView *m_Planes;

	CreationMode mode;
};

} // namespace mitk

#endif /* PLANECUTFILTER_H_HEADER_INCLUDED_C1F48A22 */

