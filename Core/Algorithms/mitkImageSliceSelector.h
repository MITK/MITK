#ifndef IMAGESLICESELECTOR_H_HEADER_INCLUDED_C1E4BE7B
#define IMAGESLICESELECTOR_H_HEADER_INCLUDED_C1E4BE7B

#include "mitkCommon.h"
#include "ImageToImageFilter.h"
#include "SubImageSelector.h"

namespace mitk {

//##ModelId=3DCBFE35015D
//##Documentation
//## Provides access to a slice of the input image. If the input is generated
//## by a ProcessObject, only the required data is requested.
class ImageSliceSelector : public SubImageSelector
{
  public:
	/** Standard class typedefs. */
    //##ModelId=3E18A0140027
	typedef ImageSliceSelector        Self;
    //##ModelId=3E18A014006D
	typedef SubImageSelector	     Superclass;
    //##ModelId=3E18A01400B3
	typedef itk::SmartPointer<Self>  Pointer;
    //##ModelId=3E18A01400F9
	typedef itk::SmartPointer<const Self>  ConstPointer;

	/** Run-time type information (and related methods). */
	itkTypeMacro(ImageSliceSelector,SubImageSelector);

    itkNewMacro(Self);  

	itkGetConstMacro(SliceNr,int);
	itkSetMacro(SliceNr,int);

	itkGetConstMacro(TimeNr,int);
	itkSetMacro(TimeNr,int);

	itkGetConstMacro(ChannelNr,int);
	itkSetMacro(ChannelNr,int);

protected:
    //##ModelId=3E1A0A2E027F
    virtual void GenerateOutputInformation();

    //##ModelId=3E1A0A320090
    virtual void GenerateData();
    //##ModelId=3E1B196A0188
    ImageSliceSelector();
    //##ModelId=3E1B196A01C4
    virtual ~ImageSliceSelector();

    //##ModelId=3E1A0B390283
    int m_SliceNr;

    //##ModelId=3E1A0B57011E
    int m_TimeNr;

    //##ModelId=3E1A0B5A0281
    int m_ChannelNr;

//    ImageDataItem::Pointer m_CurrentSliceData;
};

} // namespace mitk



#endif /* IMAGESLICESELECTOR_H_HEADER_INCLUDED_C1E4BE7B */
