#ifndef SUBIMAGESELECTOR_H_HEADER_INCLUDED_C1E4F463
#define SUBIMAGESELECTOR_H_HEADER_INCLUDED_C1E4F463

#include "mitkCommon.h"
#include "ImageToImageFilter.h"
#include "ImageDataItem.h"
#include "BaseData.h"

namespace mitk {

//##ModelId=3DCBFE1800BB
class SubImageSelector : public ImageToImageFilter
{
public:
	/** Standard class typedefs. */
	//##ModelId=3E18A01302C4
	typedef SubImageSelector            Self;
	//##ModelId=3E18A013030A
	typedef ImageToImageFilter       Superclass;
	//##ModelId=3E18A0130350
	typedef itk::SmartPointer<Self>  Pointer;
	//##ModelId=3E18A01303A1
	typedef itk::SmartPointer<const Self>  ConstPointer;

	/** Run-time type information (and related methods). */
	itkTypeMacro(SubImageSelector,ImageToImageFilter);

	itkNewMacro(Self);  

	//##ModelId=3E0B4E070066
	virtual itk::DataObject* GetDataObject();

	//##ModelId=3E0B4E0B0080
	virtual void* GetData();

	//##ModelId=3E0B4E0E0048
	virtual vtkImageData* GetVtkImageData();

	//##ModelId=3E0B4E1403D6
	virtual ipPicDescriptor* GetPic();

    //##ModelId=3E0B4663009F
	virtual void SetPosNr(int p);

    //##ModelId=3E1B19AD02A6
    SubImageSelector();

    //##ModelId=3E1B19AD02CE
    virtual ~SubImageSelector();

protected:
	//##ModelId=3E1A0FC903A5
	mitk::ImageDataItem::Pointer GetSliceData(int s = 0, int t = 0, int n = 0);

	//##ModelId=3E1A123E0396
	void SetDataItem(mitk::ImageDataItem::Pointer dataItem, int n = 0);
};

} // namespace mitk



#endif /* SUBIMAGESELECTOR_H_HEADER_INCLUDED_C1E4F463 */
