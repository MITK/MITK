#ifndef _PIC_VOLUME_TIME_SERIES_READER__H_
#define _PIC_VOLUME_TIME_SERIES_READER__H_

#include "mitkCommon.h"
#include "mitkFileReader.h"
#include "mitkImageSource.h"
#include <vector>

namespace mitk
{
//##Documentation
//## @brief Reader to read a series of volume files in DKFZ-pic-format
//## @ingroup Process
class PicVolumeTimeSeriesReader : public ImageSource, public FileReader
{
public:
    mitkClassMacro( PicVolumeTimeSeriesReader, FileReader );

    /** Method for creation through the object factory. */
    itkNewMacro( Self );

    itkSetStringMacro( FileName );
    itkGetStringMacro( FileName );

    itkSetStringMacro( FilePrefix );
    itkGetStringMacro( FilePrefix );

    itkSetStringMacro( FilePattern );
    itkGetStringMacro( FilePattern );    
    
protected:
    virtual void GenerateData();

    virtual void GenerateOutputInformation();

    virtual bool GenerateFileList();
    
    PicVolumeTimeSeriesReader();

    ~PicVolumeTimeSeriesReader();

    //##Description
    //## @brief Time when Header was last read
    itk::TimeStamp m_ReadHeaderTime;

    int m_StartFileIndex;

    std::string m_FileName;

    std::string m_FilePrefix;

    std::string m_FilePattern;

    std::vector< std::string > m_MatchedFiles;
};

} // namespace mitk

#endif 

