#ifndef PICFILEREADER_H_HEADER_INCLUDED_C1F48A22
#define PICFILEREADER_H_HEADER_INCLUDED_C1F48A22

#include "mitkCommon.h"
#include "Reader.h"
#include "FileReader.h"

namespace mitk {

//##ModelId=3D7B112F00B0
class PicFileReader : public FileReader
{
public:
	/** Standard class typedefs. */
    //##ModelId=3E18763D0029
	typedef PicFileReader      Self;
    //##ModelId=3E18763D0051
	typedef FileReader         Superclass;
    //##ModelId=3E18763D007A
	typedef itk::SmartPointer<Self>  Pointer;

	/** Method for creation through the object factory. */
	itkNewMacro(Self);

	/** Run-time type information (and related methods). */
	itkTypeMacro(PicFileReader, FileReader);

protected:
	//##ModelId=3E187255016C
	virtual void GenerateData();

    //##ModelId=3E1878400265
    virtual void GenerateOutputInformation();

	//##ModelId=3E1874D202D0
	PicFileReader();

	//##ModelId=3E1874E50179
	~PicFileReader();

	/** Time when Header was last read. */
    //##ModelId=3E19EA120063
	itk::TimeStamp m_ReadHeaderTime;
};

} // namespace mitk



#endif /* PICFILEREADER_H_HEADER_INCLUDED_C1F48A22 */
