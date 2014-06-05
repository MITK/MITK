#ifndef MITKCONTOURMODELSETSERIALIZER_H
#define MITKCONTOURMODELSETSERIALIZER_H

#include <mitkBaseDataSerializer.h>
#include <MitkContourModelExports.h>

namespace mitk
{

class MitkContourModel_EXPORT ContourModelSetSerializer : public BaseDataSerializer
{
public:
  mitkClassMacro(ContourModelSetSerializer, BaseDataSerializer);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  virtual std::string Serialize();

protected:

  ContourModelSetSerializer();
  virtual ~ContourModelSetSerializer();
};

}

#endif // MITKCONTOURMODELSETSERIALIZER_H
