#ifndef LIGHTBOXIMAGEREADER_H_HEADER_INCLUDED_C1F48A22
#define LIGHTBOXIMAGEREADER_H_HEADER_INCLUDED_C1F48A22
#define NO_pVersion
#include "mitkImageSource.h"
#include "mitkCommon.h"
#include <chili/qclightbox.h>


namespace mitk {

class LightBoxImageReader : public ImageSource 
{
public:
    /** Standard class typedefs. */
    mitkClassMacro(LightBoxImageReader, ImageSource);

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    mitk::Vector3D GetSpacingFromLB();

    void SetLightBox(QcLightbox* lightbox);
    QcLightbox* GetLightBox();

protected:
    virtual void GenerateData();

    virtual void GenerateOutputInformation();

    LightBoxImageReader();

    ~LightBoxImageReader();

    /** Time when Header was last read. */
    itk::TimeStamp m_ReadHeaderTime;
    //double ConvertTime(ipPicDescriptor*  pic);
    QcLightbox* m_LightBox;
};

} // namespace mitk


#endif /* LIGHTBOXIMAGEREADER_H_HEADER_INCLUDED_C1F48A22 */
