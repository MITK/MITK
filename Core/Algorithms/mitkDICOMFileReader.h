#ifndef MITKDICOMFILEREADER_H_HEADER_INCLUDED_C1F48A22
#define MITKDICOMFILEREADER_H_HEADER_INCLUDED_C1F48A22

#include "mitkCommon.h"
#include "mitkFileReader.h"
#include "mitkImageSource.h"

namespace mitk {
//##Documentation
//## @brief Reader to read files in DICOM-format
//## @ingroup Process
class DICOMFileReader : public ImageSource, public FileReader
{
public:
    mitkClassMacro(DICOMFileReader, FileReader);

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    itkSetStringMacro(FileName);
    itkGetStringMacro(FileName);

    itkSetStringMacro(FilePrefix);
    itkGetStringMacro(FilePrefix);

    itkSetStringMacro(FilePattern);
    itkGetStringMacro(FilePattern);

protected:
    //##ModelId=3E187255016C
    virtual void GenerateData();

    //##ModelId=3E1878400265
    virtual void GenerateOutputInformation();

    //##ModelId=3E1874D202D0
    DICOMFileReader();

    //##ModelId=3E1874E50179
    ~DICOMFileReader();

    //##ModelId=3E19EA120063
    //##Description 
    //## @brief Time when Header was last read
    itk::TimeStamp m_ReadHeaderTime;

    //##ModelId=3EA6ADB80138
    int m_StartFileIndex;
  protected:
    //##ModelId=3E186FC80253
    std::string m_FileName;

    //##ModelId=3E1873B3030A
    std::string m_FilePrefix;

    //##ModelId=3E1873B30346
    std::string m_FilePattern;
private:
	std::string outputfile_name;
	ipPicDescriptor *pic;

};

} // namespace mitk

#endif /* MITKDICOMFILEREADER_H_HEADER_INCLUDED_C1F48A22 */

