#ifndef _PIC_VOLUME_TIME_SERIES_READER__H_
#define _PIC_VOLUME_TIME_SERIES_READER__H_

#include "mitkCommon.h"
#include "mitkFileSeriesReader.h"
#include "mitkImageSource.h"
#include <vector>

namespace mitk
{
//##Documentation
//## @brief Reader to read a series of volume files in DKFZ-pic-format
//## @ingroup Process
class PicVolumeTimeSeriesReader : public ImageSource, public FileSeriesReader
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

    PicVolumeTimeSeriesReader();

    ~PicVolumeTimeSeriesReader();

    //##Description
    //## @brief Time when Header was last read
    itk::TimeStamp m_ReadHeaderTime;

};

} // namespace mitk

#endif 

