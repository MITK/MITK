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


#ifndef MAPPER_H_HEADER_INCLUDED_C1E6EA08
#define MAPPER_H_HEADER_INCLUDED_C1E6EA08

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

  class BaseRenderer;
  class BaseData;
  class DataNode;


  /** \brief Interface for accessing (templated) LocalStorageHandler instances.
   */
  class BaseLocalStorageHandler
  {
    public:
      virtual ~BaseLocalStorageHandler() {}
      virtual void ClearLocalStorage(mitk::BaseRenderer *renderer,bool unregisterFromBaseRenderer=true )=0;
  };


  /** \brief Base class of all mappers, Vtk as well as OpenGL mappers
  *
  * By the help of mappers, the input data is transformed to tangible primitives,
  * such as surfaces, points, lines, etc.
  * This is the base class of all mappers, Vtk as well as OpenGL mappers.
  * Subclasses of mitk::Mapper control the creation of rendering primitives
  * that interface to the graphics library (e.g., OpenGL, vtk).
  *
  * \todo Should Mapper be a subclass of ImageSource?
  * \ingroup Mapper
  */
  class MITK_CORE_EXPORT Mapper : public itk::Object
  {
  public:

    mitkClassMacro(Mapper, itk::Object);

    /** \brief Set the DataNode containing the data to map */
    itkSetObjectMacro(DataNode, DataNode);


    /** \brief Get the DataNode containing the data to map */
    virtual DataNode* GetDataNode() const;


    /**\brief Get the data to map
    *
    * Returns the mitk::BaseData object associated with this mapper.
    * \return the mitk::BaseData associated with this mapper.
    * @deprecated Use GetDataNode()->GetData() instead to access the data
    */
    DEPRECATED(BaseData* GetData() const);

    /** \brief Convenience access method for color properties (instances of
    * ColorProperty)
    * \return \a true property was found
    * @deprecated Use GetDataNode()->GetColor(...) instead to get the color
    */
    DEPRECATED(virtual bool GetColor(float rgb[3], BaseRenderer* renderer, const char* name = "color") const);


    /** \brief Convenience access method for visibility properties (instances
    * of BoolProperty)
    * \return \a true property was found
    * \sa IsVisible
    * @deprecated Use GetDataNode()->GetVisibility(...) instead to get the visibility
    */
    DEPRECATED(virtual bool GetVisibility(bool &visible, BaseRenderer* renderer, const char* name = "visible") const);


    /** \brief Convenience access method for opacity properties (instances of
    * FloatProperty)
    * \return \a true property was found
    * @deprecated Use GetDataNode()->GetOpacity(...) instead to get the opacity
    */
    DEPRECATED(virtual bool GetOpacity(float &opacity, BaseRenderer* renderer, const char* name = "opacity") const);


    /** \brief Convenience access method for color properties (instances of
    * LevelWindoProperty)
    * \return \a true property was found
    * @deprecated Use GetDataNode->GetLevelWindow(...) instead to get the levelwindow
    */
    DEPRECATED(virtual bool GetLevelWindow(LevelWindow &levelWindow, BaseRenderer* renderer, const char* name = "levelwindow") const);


    /** \brief Convenience access method for visibility properties (instances
    * of BoolProperty). Return value is the visibility. Default is
    * visible==true, i.e., true is returned even if the property (\a
    * propertyKey) is not found.
    *
    * Thus, the return value has a different meaning than in the
    * GetVisibility method!
    * \sa GetVisibility
    * @deprecated Use GetDataNode()->GetVisibility(...) instead
    */
    DEPRECATED(virtual bool IsVisible(BaseRenderer* renderer, const char* name = "visible") const);


     /** \brief Returns whether this is an vtk-based mapper
    * @deprecated All mappers of superclass VTKMapper are vtk based, use a dynamic_cast instead
    */
    virtual bool IsVtkBased() const = 0;


    /** \brief Calls the time step of the input data for the specified renderer and checks
    * whether the time step is valid and calls method GenerateDataForRenderer()
    */
    virtual void Update(BaseRenderer* renderer);

    /** \brief Responsible for calling the appropriate render functions.
    *   To be implemented in sub-classes.
    */
    virtual void MitkRender(mitk::BaseRenderer* renderer, mitk::VtkPropRenderer::RenderType type) = 0;

    /**
    * \brief Apply specific color and opacity properties read from the PropertyList.
    * Reimplemented in GLmapper (does not use the actor) and the VtkMapper class.
    * The function is called by the individual mapper (mostly in the ApplyProperties() or ApplyAllProperties()
    * method).
    */
    virtual void ApplyColorAndOpacityProperties(mitk::BaseRenderer* renderer, vtkActor* actor = NULL) = 0;


    /** \brief Set default values of properties used by this mapper
    * to \a node
    *
    * \param node The node for which the properties are set
    * \param overwrite overwrite existing properties (default: \a false)
    * \param renderer defines which property list of node is used
    * (default: \a NULL, i.e. default property list)
    */
    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = NULL, bool overwrite = false);

    /** \brief Returns the current time step as calculated from the renderer */
    int GetTimestep() const { return m_TimeStep; };

    /** Returns true if this Mapper currently allows for Level-of-Detail rendering.
     * This reflects whether this Mapper currently invokes StartEvent, EndEvent, and
     * ProgressEvent on BaseRenderer. */
    virtual bool IsLODEnabled( BaseRenderer * /*renderer*/ ) const { return false; }

  protected:

    /** \brief explicit constructor which disallows implicit conversions */
    explicit Mapper();

    /** \brief virtual destructor in order to derive from this class */
    virtual ~Mapper();


    /** \brief Generate the data needed for rendering (independent of a specific renderer) */
    // @deprecated Use GenerateDataForRenderer(BaseRenderer* renderer) instead.
    DEPRECATED( virtual void GenerateData() { });

    /** \brief Generate the data needed for rendering into \a renderer */
    virtual void GenerateDataForRenderer(BaseRenderer* /* renderer */) { };

    /** \brief Updates the time step, which is sometimes needed in subclasses */
    virtual void CalculateTimeStep( BaseRenderer* renderer );

    /** \brief Reset the mapper (i.e., make sure that nothing is displayed) if no
    * valid data is present. In most cases the reimplemented function
    * disables the according actors (toggling visibility off)
    *
    * To be implemented in sub-classes.
    */
    virtual void ResetMapper( BaseRenderer* /*renderer*/ ) { };

    //\brief not thread-safe
    itk::WeakPointer<DataNode> m_DataNode;

    /** \brief timestamp of last update of stored data */
    itk::TimeStamp m_LastUpdateTime;

  private:

    /** \brief The current time step of the dataset to be rendered,
    * for use in subclasses.
    * The current timestep can be accessed via the GetTimestep() method.
    */
    int m_TimeStep;

    /** \brief copy constructor */
    Mapper( const Mapper &);

    /** \brief assignment operator */
    Mapper &operator=(const Mapper &);


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

  };

} // namespace mitk

#endif /* MAPPER_H_HEADER_INCLUDED_C1E6EA08 */
