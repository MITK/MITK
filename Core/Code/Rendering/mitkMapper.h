/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MAPPER_H_HEADER_INCLUDED_C1E6EA08
#define MAPPER_H_HEADER_INCLUDED_C1E6EA08

#include "mitkCommon.h"
#include "mitkBaseRenderer.h"
#include "mitkLevelWindow.h"

#include <itkObject.h>
#include <itkWeakPointer.h>

//Just included to get VTK version
#include <vtkConfigure.h>

class vtkWindow;
class vtkProp;

namespace mitk {

  class BaseRenderer;
  class BaseData;
  class DataTreeNode;
  

  /** \brief Interface for accessing (templated) LocalStorageHandler instances.
   */
  class BaseLocalStorageHandler
  {
    public:
      virtual void ClearLocalStorage(mitk::BaseRenderer *renderer,bool unregisterFromBaseRenderer=true )=0;
  };


  //##Documentation
  //## @brief Base class of all mappers, 2D as well as 3D
  //##
  //## Base class of all Mappers, 2D as well as 3D. 
  //## Subclasses of mitk::Mapper control the creation of rendering primitives
  //## that interface to the graphics library (e.g., OpenGL, vtk). 
  //## @todo Should Mapper be a subclass of ImageSource?
  //## @ingroup Mapper
  class MITK_CORE_EXPORT Mapper : public itk::Object
  {
  public:

    mitkClassMacro(Mapper, itk::Object);

    //##Documentation
    //## @brief Set the DataTreeNode containing the data to map
    itkSetObjectMacro(DataTreeNode, DataTreeNode);

    //##Documentation
    //## @brief Get the data to map
    //## 
    //## Returns the mitk::BaseData object associated with this mapper.
    //## @returns the mitk::BaseData associated with this mapper.
    BaseData* GetData() const;

    //##Documentation
    //## @brief Get the DataTreeNode containing the data to map
    virtual DataTreeNode* GetDataTreeNode() const;

    //##Documentation
    //## @brief Convenience access method for color properties (instances of
    //## ColorProperty)
    //## @return @a true property was found
    virtual bool GetColor(float rgb[3], BaseRenderer* renderer, const char* name = "color") const;

    //##Documentation
    //## @brief Convenience access method for visibility properties (instances
    //## of BoolProperty)
    //## @return @a true property was found
    //## @sa IsVisible
    virtual bool GetVisibility(bool &visible, BaseRenderer* renderer, const char* name = "visible") const;

    //##Documentation
    //## @brief Convenience access method for opacity properties (instances of
    //## FloatProperty)
    //## @return @a true property was found
    virtual bool GetOpacity(float &opacity, BaseRenderer* renderer, const char* name = "opacity") const;

    //##Documentation
    //## @brief Convenience access method for color properties (instances of
    //## LevelWindoProperty)
    //## @return @a true property was found
    virtual bool GetLevelWindow(LevelWindow &levelWindow, BaseRenderer* renderer, const char* name = "levelwindow") const;

    //##Documentation
    //## @brief Convenience access method for visibility properties (instances
    //## of BoolProperty). Return value is the visibility. Default is
    //## visible==true, i.e., true is returned even if the property (@a
    //## propertyKey) is not found.
    //## 
    //## Thus, the return value has a different meaning than in the
    //## GetVisibility method!
    //## @sa GetVisibility
    virtual bool IsVisible(BaseRenderer* renderer, const char* name = "visible") const;

    virtual void Update(BaseRenderer* renderer);

    virtual void MitkRenderOverlay(BaseRenderer* renderer) = 0;
    virtual void MitkRenderOpaqueGeometry(BaseRenderer* renderer) = 0;
    virtual void MitkRenderTranslucentGeometry(BaseRenderer* renderer) = 0;
    
    #if ( ( VTK_MAJOR_VERSION >= 5 ) && ( VTK_MINOR_VERSION>=2)  )
      virtual void MitkRenderVolumetricGeometry(BaseRenderer* renderer) = 0;
    #endif

    /** 
    * \brief Returns whether this is an vtk-based mapper
    */
    bool IsVtkBased() const
    {
      return m_VtkBased;
    }

    /** \brief Returns true if this mapper owns the specified vtkProp for
     * the given BaseRenderer.
     *
     * Note: returns false by default; should be implemented for VTK-based
     * Mapper subclasses. */
    virtual bool HasVtkProp( const vtkProp* /*prop*/, BaseRenderer* /*renderer*/ )
    { 
      return false; 
    }

    /** 
    * \brief Release vtk-based graphics resources. Must be overwritten in
    * subclasses if vtkProps are used.
    */
    virtual void ReleaseGraphicsResources(vtkWindow*) { };

    /** \brief Set default values of properties used by this mapper
    * to \a node
    *
    * \param node The node for which the properties are set
    * \param overwrite overwrite existing properties (default: \a false)
    * \param renderer defines which property list of node is used 
    * (default: \a NULL, i.e. default property list)
    */
    static void SetDefaultProperties(DataTreeNode* node, BaseRenderer* renderer = NULL, bool overwrite = false);

    /** \brief Returns the current time step as calculated from the renderer */
    int GetTimestep() const {return m_TimeStep;}; 

    
    /** Returns true if this Mapper currently allows for Level-of-Detail rendering.
     * This reflects whether this Mapper currently invokes StartEvent, EndEvent, and
     * ProgressEvent on BaseRenderer. */
    virtual bool IsLODEnabled( BaseRenderer * /*renderer*/ ) const { return false; }

  protected:

    Mapper();

    virtual ~Mapper();

    //##Documentation
    //## @brief Generate the data needed for rendering (independent of a specific renderer)
    virtual void GenerateData();
    //##Documentation
    //## @brief Generate the data needed for rendering into @a renderer
    virtual void GenerateData(BaseRenderer *renderer);

    bool m_VtkBased;

    itk::WeakPointer<DataTreeNode> m_DataTreeNode;

    //##Documentation
    //## @brief timestamp of last update of stored data
    itk::TimeStamp m_LastUpdateTime;

  private:

    //## Updates the time step, which is sometimes needed in subclasses
    virtual void CalculateTimeStep( BaseRenderer *renderer );

    //## The current time step of the dataset to be rendered, for use in subclasses
    //## The momentary timestep can be accessed via the GetTimestep() method.
    int m_TimeStep;


  public:

    /** \brief Base class for mapper specific rendering ressources.
     */
    class BaseLocalStorage
    {
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
         * Should be used by mappers in GenerateData() and ApplyProperties()
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




  };

} // namespace mitk

#endif /* MAPPER_H_HEADER_INCLUDED_C1E6EA08 */
