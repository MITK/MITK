/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
#include "mitkImageSource.h"
#include "mitkLevelWindow.h"

namespace mitk {

class DataTreeNode;
class BaseData;
class BaseRenderer;

//##ModelId=3D6A0EE70237
//##Documentation
//## @brief Base class of all mappers, 2D as well as 3D
//## @ingroup Mapper
//## Base class of all Mappers, 2D as well as 3D. 
//## Subclasses of mitk::Mapper control the creation of rendering primitives
//## that interface to the graphics library (e.g., OpenGL, vtk). 
//## @todo Should Mapper be a subclass of ImageSource?
class Mapper : public ImageSource
{
public:
    mitkClassMacro(Mapper, ImageSource);

	/** Method for creation through the object factory. */
	//   itkNewMacro(Self);  

	//##ModelId=3E32E456028D
    //##Documentation
    //## @brief Set the DataTreeNode containing the data to map
	virtual void SetInput(const mitk::DataTreeNode* data);

    //##ModelId=3E860B9A0378
    //##Documentation
    //## @brief Get the data to map
    BaseData* GetData() const;

    //##ModelId=3E86B0EA00B0
    //##Documentation
    //## @brief Get the DataTreeNode containing the data to map
    mitk::DataTreeNode* GetDataTreeNode() const;
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

  protected:
    //##ModelId=3E3C337E0162
    Mapper();

    //##ModelId=3E3C337E019E
    virtual ~Mapper();
    //##ModelId=3EF1A43C01D9
    //##Documentation
    //## @brief Calls Update(). MSVC 7 seems to have a bug: says Update does
    //## not take 0 parameters in subclasses of BaseVtkMapper3D, which is only
    //## true for one of the Update-methods
    //## (BaseVtkMapper3D::Update(mitk::BaseRenderer* renderer), but not for
    //## the Update inherited from itk::ProcessObject.
    void StandardUpdate();

};

} // namespace mitk

#endif /* MAPPER_H_HEADER_INCLUDED_C1E6EA08 */
