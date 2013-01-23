#ifndef MITKFIBERTRACKINGOBJECTFACTORY_H
#define MITKFIBERTRACKINGOBJECTFACTORY_H

#include "mitkCoreObjectFactory.h"
#include "FiberTrackingExports.h"

//modernized fiberbundle datastrucutre
#include "mitkFiberBundleX.h"
#include "mitkFiberBundleXIOFactory.h"
#include "mitkFiberBundleXWriterFactory.h"
#include "mitkFiberBundleXWriter.h"
#include "mitkFiberBundleXMapper3D.h"
#include "mitkFiberBundleXMapper2D.h"

#include "mitkFiberBundleXThreadMonitorMapper3D.h"
#include "mitkFiberBundleXThreadMonitor.h"



#include "mitkPlanarFigureMapper3D.h"

namespace mitk {

class FiberTracking_EXPORT FiberTrackingObjectFactory : public CoreObjectFactoryBase
{
  public:
    mitkClassMacro(FiberTrackingObjectFactory,CoreObjectFactoryBase);
    itkNewMacro(FiberTrackingObjectFactory);

    virtual Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId);
    virtual void SetDefaultProperties(mitk::DataNode* node);
    virtual const char* GetFileExtensions();
    virtual mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap();
    virtual const char* GetSaveFileExtensions();
    virtual mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap();
    void RegisterIOFactories();
  protected:
    FiberTrackingObjectFactory(bool registerSelf = true);
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
void FiberTracking_EXPORT RegisterFiberTrackingObjectFactory();


#endif // MITKFIBERTRACKINGOBJECTFACTORY_H
