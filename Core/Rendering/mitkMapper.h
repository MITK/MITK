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

namespace mitk {

class BaseRenderer;
class BaseData;
class DataTreeNode;

//##ModelId=3D6A0EE70237
//##Documentation
//## @brief Base class of all mappers, 2D as well as 3D
//##
//## Base class of all Mappers, 2D as well as 3D. 
//## Subclasses of mitk::Mapper control the creation of rendering primitives
//## that interface to the graphics library (e.g., OpenGL, vtk). 
//## @todo Should Mapper be a subclass of ImageSource?
//## @ingroup Mapper
class Mapper : public itk::Object, public XMLIO
{
public:
    mitkClassMacro(Mapper, itk::Object);


    //##ModelId=3E32E456028D
    //##Documentation
    //## @brief Set the DataTreeNode containing the data to map
    itkSetObjectMacro(DataTreeNode, mitk::DataTreeNode);

    //##ModelId=3E860B9A0378
    //##Documentation
    //## @brief Get the data to map
    //## 
    //## Returns the mitk::BaseData object associated with this mapper.
    //## @returns the mitk::BaseData associated with this mapper.
    BaseData* GetData() const;

    //##ModelId=3E86B0EA00B0
    //##Documentation
    //## @brief Get the DataTreeNode containing the data to map
    virtual DataTreeNode* GetDataTreeNode() const;
    
    //##ModelId=3EF17276014B
    //##Documentation
    //## @brief Convenience access method for color properties (instances of
    //## ColorProperty)
    //## @return @a true property was found
    virtual bool GetColor(float rgb[3], mitk::BaseRenderer* renderer, const char* name = "color") const;
    //##ModelId=3EF17795006A
    //##Documentation
    //## @brief Convenience access method for visibility properties (instances
    //## of BoolProperty)
    //## @return @a true property was found
    //## @sa IsVisible
    virtual bool GetVisibility(bool &visible, mitk::BaseRenderer* renderer, const char* name = "visible") const;
    //##ModelId=3EF1781F0285
    //##Documentation
    //## @brief Convenience access method for opacity properties (instances of
    //## FloatProperty)
    //## @return @a true property was found
    virtual bool GetOpacity(float &opacity, mitk::BaseRenderer* renderer, const char* name = "opacity") const;
    //##ModelId=3EF179660018
    //##Documentation
    //## @brief Convenience access method for color properties (instances of
    //## LevelWindoProperty)
    //## @return @a true property was found
    virtual bool GetLevelWindow(mitk::LevelWindow &levelWindow, mitk::BaseRenderer* renderer, const char* name = "levelwindow") const;
    //##ModelId=3EF18B340008
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

protected:
    //##ModelId=3E3C337E0162
    Mapper();

    //##ModelId=3E3C337E019E
    virtual ~Mapper();

    //##Documentation
    //## @brief Generate the data needed for rendering (independent of a specific renderer)
    virtual void GenerateData();
    //##Documentation
    //## @brief Generate the data needed for rendering into @a renderer
    virtual void GenerateData(mitk::BaseRenderer *renderer);

    //## Update time step, for use in subclasses
    virtual void CalculateTimeStep( mitk::BaseRenderer *renderer );

    //## The current time step of the dataset to be rendered, for use in subclasses
    int m_TimeStep;

    itk::WeakPointer<mitk::DataTreeNode> m_DataTreeNode;

    //##Documentation
    //## @brief timestamp of last update of stored data
    itk::TimeStamp m_LastUpdateTime;

    //##Documentation
    static const std::string XML_NODE_NAME;
};

} // namespace mitk

#endif /* MAPPER_H_HEADER_INCLUDED_C1E6EA08 */
