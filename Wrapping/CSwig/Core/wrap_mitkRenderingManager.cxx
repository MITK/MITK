#include "mitkRenderingManager.h"
#include "mitkCSwigMacros.h"
#include "vtkRenderWindow.h"
#include "mitkDataStorage.h"
#include "mitkGlobalInteraction.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="RenderingManager";
     namespace wrappers
     {
         MITK_WRAP_OBJECT(RenderingManager)
         typedef std::vector< vtkRenderWindow* > RenderWindowVector;
         typedef std::vector< float > FloatVector;
         typedef std::vector< bool > BoolVector;
         typedef itk::SmartPointer< mitk::DataStorage > DataStoragePointer;
         typedef itk::SmartPointer< mitk::GlobalInteraction > GlobalInteractionPointer;
         typedef mitk::RenderingManager::RequestType RenderingManager_RequestType;
     }
}

#endif

