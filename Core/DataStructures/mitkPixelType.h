#ifndef PIXELTYPE_H_HEADER_INCLUDED_C1EBF565
#define PIXELTYPE_H_HEADER_INCLUDED_C1EBF565

#include "mitkCommon.h"

namespace mitk {

//##ModelId=3DF45FE9023C
class PixelType
{
  public:
	//##ModelId=3E1400C40198
    PixelType(const type_info& aTypeId);

    //##ModelId=3E1400150088
    PixelType(ipPicType_t type, int bpe);

    //##ModelId=3E1400060113
    PixelType(ipPicDescriptor* pic);
    //##ModelId=3E1412720060
    PixelType(const mitk::PixelType & aPixelType);
    //##ModelId=3E14131D0246
    inline const type_info * const GetTypeId() const
    {
		return m_TypeId;
    }


    //##ModelId=3E1413040237
    inline ipPicType_t GetType() const
    {
		return m_Type;
    }

  
    //##ModelId=3E1413110195
    inline int GetBpe() const
    {
		return m_Bpe;
    }

    //##ModelId=3E1550B401ED
	inline PixelType& operator=(const PixelType& aPixelType)
	{
		m_TypeId=aPixelType.GetTypeId();
		m_Type=aPixelType.GetType();
		m_Bpe=aPixelType.GetBpe();
		return *this;
	}
    //##ModelId=3E15F73502BB
    PixelType();


  protected:
    //##ModelId=3E140E4F00E9
    void Initialize(ipPicType_t type, int bpe);


  private:
    //##ModelId=3E13FFAD038C
    const type_info* m_TypeId;

    //##ModelId=3E1400610236
    ipPicType_t m_Type;

    //##ModelId=3E14006C02E6
    int m_Bpe;

};

} // namespace mitk



#endif /* PIXELTYPE_H_HEADER_INCLUDED_C1EBF565 */
