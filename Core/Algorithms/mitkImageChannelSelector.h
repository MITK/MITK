#ifndef IMAGECHANNELSELECTOR_H_HEADER_INCLUDED_C1E4F4E7
#define IMAGECHANNELSELECTOR_H_HEADER_INCLUDED_C1E4F4E7

#include "mitkCommon.h"
#include "ImageToImageFilter.h"
#include "SubImageSelector.h"

namespace mitk {

//##ModelId=3E0B46B200FD
class ImageChannelSelector : public SubImageSelector
{
public:
	/** Standard class typedefs. */
	//##ModelId=3E18A01301FC
	typedef ImageChannelSelector     Self;
	//##ModelId=3E18A0130224
	typedef SubImageSelector         Superclass;
	//##ModelId=3E18A013024C
	typedef itk::SmartPointer<Self>  Pointer;
	//##ModelId=3E18A0130274
	typedef itk::SmartPointer<const Self>  ConstPointer;

	/** Run-time type information (and related methods). */
	itkTypeMacro(ImageChannelSelector,SubImageSelector);

	itkNewMacro(Self);  

	itkGetConstMacro(ChannelNr,int);
	itkSetMacro(ChannelNr,int);

	//##ModelId=3E1B1980039C
	ImageChannelSelector();
	//##ModelId=3E1B198003B0
	virtual ~ImageChannelSelector();

protected:
	//##ModelId=3E1B1A0C005E
	int m_ChannelNr;
};

} // namespace mitk



#endif /* IMAGECHANNELSELECTOR_H_HEADER_INCLUDED_C1E4F4E7 */
