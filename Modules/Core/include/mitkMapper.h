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

#include "mitkBaseRenderer.h"
#include "mitkCommon.h"
#include "mitkLevelWindow.h"
#include "mitkLocalStorageHandler.h"
#include "mitkVtkPropRenderer.h"
#include <MitkCoreExports.h>

#include <itkObject.h>
#include <itkWeakPointer.h>

// Just included to get VTK version
#include <vtkConfigure.h>

class vtkWindow;
class vtkProp;

namespace mitk
{
  class BaseRenderer;
  class BaseData;
  class DataNode;

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
  class MITKCORE_EXPORT Mapper : public itk::Object
  {
  public:
    mitkClassMacroItkParent(Mapper, itk::Object);

    /** \brief Set the DataNode containing the data to map */
    itkSetObjectMacro(DataNode, DataNode);

    /** \brief Get the DataNode containing the data to map.
    * Method only returns valid DataNode Pointer if the mapper belongs to a data node.
    * Otherwise, the returned DataNode Pointer might be invalid. */
    virtual DataNode *GetDataNode() const;

    /**\brief Get the data to map
    *
    * Returns the mitk::BaseData object associated with this mapper.
    * \return the mitk::BaseData associated with this mapper.
    * \deprecatedSince{2013_03} Use GetDataNode()->GetData() instead to access the data
    */
    DEPRECATED(BaseData *GetData() const);

    /** \brief Convenience access method for color properties (instances of
    * ColorProperty)
    * \return \a true property was found
    * \deprecatedSince{2013_03} Use GetDataNode()->GetColor(...) instead to get the color
    */
    DEPRECATED(virtual bool GetColor(float rgb[3], BaseRenderer *renderer, const char *name = "color") const);

    /** \brief Convenience access method for visibility properties (instances
    * of BoolProperty)
    * \return \a true property was found
    * \sa IsVisible
    * \deprecatedSince{2013_03} Use GetDataNode()->GetVisibility(...) instead to get the visibility
    */
    DEPRECATED(virtual bool GetVisibility(bool &visible, BaseRenderer *renderer, const char *name = "visible") const);

    /** \brief Convenience access method for opacity properties (instances of
    * FloatProperty)
    * \return \a true property was found
    * \deprecatedSince{2013_03} Use GetDataNode()->GetOpacity(...) instead to get the opacity
    */
    DEPRECATED(virtual bool GetOpacity(float &opacity, BaseRenderer *renderer, const char *name = "opacity") const);

    /** \brief Convenience access method for color properties (instances of
    * LevelWindoProperty)
    * \return \a true property was found
    * \deprecatedSince{2013_03} Use GetDataNode->GetLevelWindow(...) instead to get the levelwindow
    */
    DEPRECATED(virtual bool GetLevelWindow(LevelWindow &levelWindow,
                                           BaseRenderer *renderer,
                                           const char *name = "levelwindow") const);

    /** \brief Convenience access method for visibility properties (instances
    * of BoolProperty). Return value is the visibility. Default is
    * visible==true, i.e., true is returned even if the property (\a
    * propertyKey) is not found.
    *
    * Thus, the return value has a different meaning than in the
    * GetVisibility method!
    * \sa GetVisibility
    * \deprecatedSince{2013_03} Use GetDataNode()->GetVisibility(...) instead
    */
    DEPRECATED(virtual bool IsVisible(BaseRenderer *renderer, const char *name = "visible") const);

    /** \brief Returns whether this is an vtk-based mapper
   * \deprecatedSince{2013_03} All mappers of superclass VTKMapper are vtk based, use a dynamic_cast instead
   */
    virtual bool IsVtkBased() const { return true; }

    /** \brief Calls the time step of the input data for the specified renderer and checks
    * whether the time step is valid and calls method GenerateDataForRenderer()
    */
    virtual void Update(BaseRenderer *renderer);

    /** \brief Responsible for calling the appropriate render functions.
    *   To be implemented in sub-classes.
    */
    virtual void MitkRender(mitk::BaseRenderer *renderer, mitk::VtkPropRenderer::RenderType type) = 0;

    /**
    * \brief Apply specific color and opacity properties read from the PropertyList.
    * Reimplemented in GLmapper (does not use the actor) and the VtkMapper class.
    * The function is called by the individual mapper (mostly in the ApplyProperties() or ApplyAllProperties()
    * method).
    */
    virtual void ApplyColorAndOpacityProperties(mitk::BaseRenderer *renderer, vtkActor *actor = nullptr) = 0;

    /** \brief Set default values of properties used by this mapper
    * to \a node
    *
    * \param node The node for which the properties are set
    * \param overwrite overwrite existing properties (default: \a false)
    * \param renderer defines which property list of node is used
    * (default: \a nullptr, i.e. default property list)
    */
    static void SetDefaultProperties(DataNode *node, BaseRenderer *renderer = nullptr, bool overwrite = false);

    /** \brief Returns the current time step as calculated from the renderer */
    int GetTimestep() const { return m_TimeStep; }
    /** Returns true if this Mapper currently allows for Level-of-Detail rendering.
     * This reflects whether this Mapper currently invokes StartEvent, EndEvent, and
     * ProgressEvent on BaseRenderer. */
    virtual bool IsLODEnabled(BaseRenderer * /*renderer*/) const { return false; }
  protected:
    /** \brief explicit constructor which disallows implicit conversions */
    explicit Mapper();

    /** \brief virtual destructor in order to derive from this class */
    ~Mapper() override;

    /** \brief Generate the data needed for rendering (independent of a specific renderer)
     *  \deprecatedSince{2013_03} Use GenerateDataForRenderer(BaseRenderer* renderer) instead.
     */
    DEPRECATED(virtual void GenerateData()) {}
    /** \brief Generate the data needed for rendering into \a renderer */
    virtual void GenerateDataForRenderer(BaseRenderer * /* renderer */) {}
    /** \brief Updates the time step, which is sometimes needed in subclasses */
    virtual void CalculateTimeStep(BaseRenderer *renderer);

    /** \brief Reset the mapper (i.e., make sure that nothing is displayed) if no
    * valid data is present. In most cases the reimplemented function
    * disables the according actors (toggling visibility off)
    *
    * To be implemented in sub-classes.
    */
    virtual void ResetMapper(BaseRenderer * /*renderer*/) {}
    mitk::DataNode *m_DataNode;

  private:
    /** \brief The current time step of the dataset to be rendered,
    * for use in subclasses.
    * The current timestep can be accessed via the GetTimestep() method.
    */
    int m_TimeStep;

    /** \brief copy constructor */
    Mapper(const Mapper &);

    /** \brief assignment operator */
    Mapper &operator=(const Mapper &);

  public:
    /** \brief Base class for mapper specific rendering ressources.
     */
    class MITKCORE_EXPORT BaseLocalStorage
    {
    public:
      BaseLocalStorage() = default;
      virtual ~BaseLocalStorage() = default;

      BaseLocalStorage(const BaseLocalStorage &) = delete;
      BaseLocalStorage & operator=(const BaseLocalStorage &) = delete;

      bool IsGenerateDataRequired(mitk::BaseRenderer *renderer, mitk::Mapper *mapper, mitk::DataNode *dataNode) const;

      inline void UpdateGenerateDataTime() { m_LastGenerateDataTime.Modified(); }
      inline itk::TimeStamp &GetLastGenerateDataTime() { return m_LastGenerateDataTime; }
    protected:
      /** \brief timestamp of last update of stored data */
      itk::TimeStamp m_LastGenerateDataTime;
    };
  };

} // namespace mitk

#endif /* MAPPER_H_HEADER_INCLUDED_C1E6EA08 */
