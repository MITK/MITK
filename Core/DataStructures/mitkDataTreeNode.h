#ifndef DATATREENODE_H_HEADER_INCLUDED_C1E14338
#define DATATREENODE_H_HEADER_INCLUDED_C1E14338

#include "mitkCommon.h"
#include "Mapper.h"

namespace mitk {

//##ModelId=3E031E2C0143
class DataTreeNode
{
  public:
    //##ModelId=3D6A0E8C02CC
    mitk::Mapper* GetMapper();

  protected:
    //##ModelId=3D6A0F8C0202
    Mapper mappers;

};

} // namespace mitk



#endif /* DATATREENODE_H_HEADER_INCLUDED_C1E14338 */
