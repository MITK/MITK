#ifndef MITKCONTOURMODELSETSERIALIZER_H
#define MITKCONTOURMODELSETSERIALIZER_H

#include <MitkContourModelExports.h>
#include <mitkBaseDataSerializer.h>

namespace mitk
{
  class MITKCONTOURMODEL_EXPORT ContourModelSetSerializer : public BaseDataSerializer
  {
  public:
    mitkClassMacro(ContourModelSetSerializer, BaseDataSerializer);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      std::string Serialize() override;

  protected:
    ContourModelSetSerializer();
    ~ContourModelSetSerializer() override;
  };
}

#endif // MITKCONTOURMODELSETSERIALIZER_H
