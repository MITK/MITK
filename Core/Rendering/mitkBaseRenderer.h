#ifndef BASERENDERER_H_HEADER_INCLUDED_C1E7600A
#define BASERENDERER_H_HEADER_INCLUDED_C1E7600A

#include "mitkCommon.h"
#include "DataTree.h"

namespace mitk {

//##ModelId=3C6E9AA90306
class BaseRenderer
{
  public:
    //##ModelId=3D6A1791038B
	  void setData(mitk::DataTreeIterator* iterator);

  private:
    //##ModelId=3D6A17780230
    int mapperID;

};

} // namespace mitk



#endif /* BASERENDERER_H_HEADER_INCLUDED_C1E7600A */
