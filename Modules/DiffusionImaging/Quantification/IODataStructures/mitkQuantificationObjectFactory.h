#ifndef MITKQUANTIFICATIONOBJECTFACTORY_H
#define MITKQUANTIFICATIONOBJECTFACTORY_H


#include "mitkCoreObjectFactory.h"
#include <QuantificationExports.h>

namespace mitk {

class Quantification_EXPORT QuantificationObjectFactory : public CoreObjectFactoryBase
{
  public:
    mitkClassMacro(QuantificationObjectFactory,CoreObjectFactoryBase);
    itkNewMacro(QuantificationObjectFactory);

    virtual Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId);
    virtual void SetDefaultProperties(mitk::DataNode* node);
    virtual const char* GetFileExtensions();
    virtual mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap();
    virtual const char* GetSaveFileExtensions();
    virtual mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap();
    void RegisterIOFactories();
  protected:
    QuantificationObjectFactory(bool registerSelf = true);
  private:
    void CreateFileExtensionsMap();
    std::string m_ExternalFileExtensions;
    std::string m_InternalFileExtensions;
    std::string m_SaveFileExtensions;
    MultimapType m_FileExtensionsMap;
    MultimapType m_SaveFileExtensionsMap;
};

}
// global declaration for simple call by
// applications
void Quantification_EXPORT RegisterQuantificationObjectFactory();

#endif // MITKQUANTIFICATIONOBJECTFACTORY_H
