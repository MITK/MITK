#include "DataTreeNode.h"
#include "MapperFactory.h"

//##ModelId=3D6A0E8C02CC
mitk::Mapper* mitk::DataTreeNode::GetMapper(MapperSlotId id) {

// TODO
   if (mappers[id] == NULL) {
     mappers[id] = MapperFactory::CreateMapper(GetData(),id);
   }
   // mappers[id]->AddInput(GetData());
   return mappers[id];
  
   //  return NULL;
}
mitk::BaseData* mitk::DataTreeNode::GetData() {
  return data;
}

