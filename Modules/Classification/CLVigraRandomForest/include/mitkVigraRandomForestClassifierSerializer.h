#ifndef MITKVIGRARANDOMFORESTCLASSIFIERSERIALIZER_H
#define MITKVIGRARANDOMFORESTCLASSIFIERSERIALIZER_H

#include <mitkBaseDataSerializer.h>
#include <MitkCLVigraRandomForestExports.h>

namespace mitk
{

class MITKCLVIGRARANDOMFOREST_EXPORT VigraRandomForestClassifierSerializer : public BaseDataSerializer
{
public:
  mitkClassMacro(VigraRandomForestClassifierSerializer, BaseDataSerializer);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  std::string Serialize() override;

protected:

  VigraRandomForestClassifierSerializer();
  ~VigraRandomForestClassifierSerializer() override;
};

}

#endif // MITKCONTOURMODELSETSERIALIZER_H
