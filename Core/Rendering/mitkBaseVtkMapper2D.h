#ifndef BASEVTKMAPPER2D_H_HEADER_INCLUDED_C1C517D5
#define BASEVTKMAPPER2D_H_HEADER_INCLUDED_C1C517D5

#include "mitkCommon.h"
#include "Mapper2D.h"
#include <vtkProp.h>

namespace mitk {

//##ModelId=3E32C60F00E6
//##Documentation
//## @brief Base class of all vtk-based 2D-Mappers
//## @ingroup Mapper
//## Those must implement the abstract
//## method vtkProp* GetProp().
class BaseVtkMapper2D : public Mapper2D
{
  public:
    //##ModelId=3E3AA40002D4
    virtual vtkProp* GetProp() = 0;

  protected:
    //##ModelId=3E3AE0060159
    BaseVtkMapper2D();

    //##ModelId=3E3AE0060177
    virtual ~BaseVtkMapper2D();



};

} // namespace mitk



#endif /* BASEVTKMAPPER2D_H_HEADER_INCLUDED_C1C517D5 */
