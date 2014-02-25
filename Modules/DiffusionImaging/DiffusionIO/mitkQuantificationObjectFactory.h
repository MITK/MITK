#ifndef MITKQUANTIFICATIONOBJECTFACTORY_H
#define MITKQUANTIFICATIONOBJECTFACTORY_H


#include "mitkCoreObjectFactory.h"

namespace mitk {

class QuantificationObjectFactory : public CoreObjectFactoryBase
{
  public:
    mitkClassMacro(QuantificationObjectFactory,CoreObjectFactoryBase)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    ~QuantificationObjectFactory();

    virtual Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId);

    virtual void SetDefaultProperties(mitk::DataNode* node);

    virtual const char* GetFileExtensions();

    virtual mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap();

    virtual const char* GetSaveFileExtensions();

    virtual mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap();

protected:
    QuantificationObjectFactory();
  private:
    void CreateFileExtensionsMap();
    std::string m_ExternalFileExtensions;
    std::string m_InternalFileExtensions;
    std::string m_SaveFileExtensions;
    MultimapType m_FileExtensionsMap;
    MultimapType m_SaveFileExtensionsMap;

    itk::ObjectFactoryBase::Pointer m_NrrdTbssImageIOFactory;
    itk::ObjectFactoryBase::Pointer m_NrrdTbssRoiImageIOFactory;

    itk::ObjectFactoryBase::Pointer m_NrrdTbssImageWriterFactory;
    itk::ObjectFactoryBase::Pointer m_NrrdTbssRoiImageWriterFactory;
};

}

#endif // MITKQUANTIFICATIONOBJECTFACTORY_H
