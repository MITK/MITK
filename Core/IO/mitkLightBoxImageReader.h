#ifndef PICFILEREADER_H_HEADER_INCLUDED_C1F48A22
#define PICFILEREADER_H_HEADER_INCLUDED_C1F48A22

#include "mitkCommon.h"
#include "Reader.h"
#include <include/plugin.h>

namespace mitk {

class LightBoxImageReader : public Reader
{
public:
    /** Standard class typedefs. */
    mitkClassMacro(LightBoxImageReader, Reader);

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    void SetLightBox(lightbox_t lightbox);
    lightbox_t GetLightBox();

protected:
    virtual void GenerateData();

    virtual void GenerateOutputInformation();

    LightBoxImageReader();

    ~LightBoxImageReader();

    /** Time when Header was last read. */
    itk::TimeStamp m_ReadHeaderTime;

    lightbox_t m_LightBox;
};

} // namespace mitk



#endif /* PICFILEREADER_H_HEADER_INCLUDED_C1F48A22 */
