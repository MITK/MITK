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
#include "itkObject.h"
#include "mitkLevelWindow.h"
#include "itkWeakPointer.h"
#include "mitkXMLIO.h"

//Just included to get VTK version
#include <vtkConfigure.h>

class vtkWindow;

namespace mitk {

  class BaseRenderer;
  class BaseData;
  class DataTreeNode;

  //##Documentation
  //## @brief Base class of all mappers, 2D as well as 3D
  //##
  //## Base class of all Mappers, 2D as well as 3D. 
  //## Subclasses of mitk::Mapper control the creation of rendering primitives
  //## that interface to the graphics library (e.g., OpenGL, vtk). 
  //## @todo Should Mapper be a subclass of ImageSource?
  //## @ingroup Mapper
  class MITK_CORE_EXPORT Mapper : public itk::Object, public XMLIO
  {
  public:

    mitkClassMacro(Mapper, itk::Object);

    //##Documentation
    //## @brief Set the DataTreeNode containing the data to map
    itkSetObjectMacro(DataTreeNode, mitk::DataTreeNode);

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
    virtual bool GetColor(float rgb[3], mitk::BaseRenderer* renderer, const char* name = "color") const;

    //##Documentation
    //## @brief Convenience access method for visibility properties (instances
    //## of BoolProperty)
    //## @return @a true property was found
    //## @sa IsVisible
    virtual bool GetVisibility(bool &visible, mitk::BaseRenderer* renderer, const char* name = "visible") const;

    //##Documentation
    //## @brief Convenience access method for opacity properties (instances of
    //## FloatProperty)
    //## @return @a true property was found
    virtual bool GetOpacity(float &opacity, mitk::BaseRenderer* renderer, const char* name = "opacity") const;

    //##Documentation
    //## @brief Convenience access method for color properties (instances of
    //## LevelWindoProperty)
    //## @return @a true property was found
    virtual bool GetLevelWindow(mitk::LevelWindow &levelWindow, mitk::BaseRenderer* renderer, const char* name = "levelwindow") const;

    //##Documentation
    //## @brief Convenience access method for visibility properties (instances
    //## of BoolProperty). Return value is the visibility. Default is
    //## visible==true, i.e., true is returned even if the property (@a
    //## propertyKey) is not found.
    //## 
    //## Thus, the return value has a different meaning than in the
    //## GetVisibility method!
    //## @sa GetVisibility
    virtual bool IsVisible(mitk::BaseRenderer* renderer, const char* name = "visible") const;

    virtual void Update(mitk::BaseRenderer* renderer);

    virtual void MitkRenderOverlay(mitk::BaseRenderer* renderer) = 0;
    virtual void MitkRenderOpaqueGeometry(mitk::BaseRenderer* renderer) = 0;
    virtual void MitkRenderTranslucentGeometry(mitk::BaseRenderer* renderer) = 0;
    
    #if ( ( VTK_MAJOR_VERSION >= 5 ) && ( VTK_MINOR_VERSION>=2)  )
      virtual void MitkRenderVolumetricGeometry(mitk::BaseRenderer* renderer) = 0;
    #endif

    /** 
    * \brief Returns whether this is an vtk-based mapper
    */
    bool IsVtkBased() const
    {
      return m_VtkBased;
    }

    /** 
    * \brief Release vtk-based graphics resources. Must be overwritten in
    * subclasses if vtkProps are used.
    */
    virtual void ReleaseGraphicsResources(vtkWindow*) { };

    //##Documentation
    virtual const std::string& GetXMLNodeName() const;

    /** \brief Set default values of properties used by this mapper
    * to \a node
    *
    * \param node The node for which the properties are set
    * \param overwrite overwrite existing properties (default: \a false)
    * \param renderer defines which property list of node is used 
    * (default: \a NULL, i.e. default property list)
    */
    static void SetDefaultProperties(mitk::DataTreeNode* node, mitk::BaseRenderer* renderer = NULL, bool overwrite = false);

    /** \brief Returns the current time step as calculated from the renderer */
    const int GetTimestep() const {return m_TimeStep;}; 

    
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
    virtual void GenerateData(mitk::BaseRenderer *renderer);

    bool m_VtkBased;

    itk::WeakPointer<mitk::DataTreeNode> m_DataTreeNode;

    //##Documentation
    //## @brief timestamp of last update of stored data
    itk::TimeStamp m_LastUpdateTime;

    //##Documentation
    static const std::string XML_NODE_NAME;

  private:

    //## Updates the time step, which is sometimes needed in subclasses
    virtual void CalculateTimeStep( mitk::BaseRenderer *renderer );

    //## The current time step of the dataset to be rendered, for use in subclasses
    //## The momentary timestep can be accessed via the GetTimestep() method.
    int m_TimeStep;


  };

} // namespace mitk

#endif /* MAPPER_H_HEADER_INCLUDED_C1E6EA08 */
