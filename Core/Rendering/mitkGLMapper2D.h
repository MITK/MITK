#ifndef MITKGLMAPPER2D_H_HEADER_INCLUDED_C197C872
#define MITKGLMAPPER2D_H_HEADER_INCLUDED_C197C872

#include "mitkCommon.h"
#include "mitkMapper2D.h"
#include "mitkBaseRenderer.h"

namespace mitk {

//##ModelId=3E6814230160
//##Documentation
//## @brief Base class of all OpenGL-based 2D-Mappers
//## @ingroup Mapper
//## Those must implement the abstract method Paint(BaseRenderer).
class GLMapper2D : public Mapper2D
{
  public:
    //##ModelId=3E67E9390346
    //##Documentation
    //## @brief Do the painting into the @a renderer
    virtual void Paint(mitk::BaseRenderer *renderer) = 0;
    //##ModelId=3EF17AA10120
    //##Documentation
    //## @brief Apply color and opacity read from the PropertyList
    virtual void ApplyProperties(mitk::BaseRenderer* renderer);

  protected:
    //##ModelId=3E681470037E
    GLMapper2D();
    //##ModelId=3E681470039C
    virtual ~GLMapper2D();
};

} // namespace mitk



#endif /* MITKGLMAPPER2D_H_HEADER_INCLUDED_C197C872 */
