#include "BaseRenderer.h"
#include "Geometry3D.h"

//##ModelId=3D6A1791038B
void mitk::BaseRenderer::setData(mitk::DataTreeIterator* iterator)
{
  while(iterator->hasNext()) {
    LevelWindow lw;
    unsigned int dummy[] = {10,10,10};
    Geometry3D geometry(3,dummy);
    iterator->next()->GetMapper(defaultMapper)->Paint(geometry,0,0,lw);
  } 
}

