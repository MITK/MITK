/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef LOCALSTORAGEHANDLER_H_HEADER_INCLUDED_C1E6EA08
#define LOCALSTORAGEHANDLER_H_HEADER_INCLUDED_C1E6EA08

#include <MitkExports.h>
#include "mitkBaseRenderer.h"
#include "mitkVtkPropRenderer.h"
#include "mitkLevelWindow.h"
#include "mitkCommon.h"

#include <itkObject.h>
#include <itkWeakPointer.h>

//Just included to get VTK version
#include <vtkConfigure.h>

class vtkWindow;
class vtkProp;

namespace mitk {


  /** \brief Interface for accessing (templated) LocalStorageHandler instances.
   */
  class BaseLocalStorageHandler
  {
    public:
      virtual ~BaseLocalStorageHandler() {}
      virtual void ClearLocalStorage(mitk::BaseRenderer *renderer,bool unregisterFromBaseRenderer=true )=0;
  };


    /** \brief Templated class for management of LocalStorage implementations in Mappers.
     *
     * The LocalStorageHandler is responsible for providing a LocalStorage to a
     * concrete mitk::Mapper subclass. Each RenderWindow / mitk::BaseRenderer is
     * assigned its own LocalStorage instance so that all contained ressources
     * (actors, shaders, textures, ...) are provided individually per window.
     *
     */
    template<class L> class LocalStorageHandler : public mitk::BaseLocalStorageHandler
    {
      protected:

        std::map<mitk::BaseRenderer *,L*> m_BaseRenderer2LS;

      public:


        /** \brief deallocates a local storage for a specifc BaseRenderer (if the
         * BaseRenderer is itself deallocating it in its destructor, it has to set
         * unregisterFromBaseRenderer=false)
         */
        virtual void ClearLocalStorage(mitk::BaseRenderer *renderer,bool unregisterFromBaseRenderer=true )
        {
          //MITK_INFO << "deleting a localstorage on a mapper request";
          if(unregisterFromBaseRenderer)
            renderer->UnregisterLocalStorageHandler( this );
          L *l = m_BaseRenderer2LS[renderer];
          m_BaseRenderer2LS.erase( renderer );
          delete l;
        }

        /** \brief Retrieves a LocalStorage for a specific BaseRenderer.
         *
         * Should be used by mappers in GenerateDataForRenderer()
         */
        L *GetLocalStorage(mitk::BaseRenderer *forRenderer)
        {
          L *l = m_BaseRenderer2LS[ forRenderer ];
          if(!l)
          {
            //MITK_INFO << "creating new localstorage";
            l = new L;
            m_BaseRenderer2LS[ forRenderer ] = l;
            forRenderer->RegisterLocalStorageHandler( this );
          }
          return l;
        }

        ~LocalStorageHandler()
        {
          typename std::map<mitk::BaseRenderer *,L*>::iterator it;

          for ( it=m_BaseRenderer2LS.begin() ; it != m_BaseRenderer2LS.end(); it++ )
          {
            (*it).first->UnregisterLocalStorageHandler(this);
            delete (*it).second;
          }

          m_BaseRenderer2LS.clear();
        }

    };


} // namespace mitk

#endif /* LOCALSTORAGEHANDLER_H_HEADER_INCLUDED_C1E6EA08 */
