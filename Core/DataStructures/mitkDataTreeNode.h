#ifndef DATATREENODE_H_HEADER_INCLUDED_C1E14338
#define DATATREENODE_H_HEADER_INCLUDED_C1E14338

#include "ImageSource.h"
#include "BaseData.h"
#include "Mapper.h"
#include "StateMachine.h"

#ifdef MBI_NO_STD_NAMESPACE
	#define MBI_STD
	#include <iostream.h>
	#include <fstream.h>
#else
	#define MBI_STD std
	#include <iostream>
	#include <fstream>
#endif

#include "PropertyList.h"

#include <map>
#include "LevelWindow.h"
#include "mitkColorProperty.h"

class vtkTransform;

namespace mitk {

class BaseRenderer;

//##ModelId=3E031E2C0143
//##Documentation
//## @brief Class for nodes of the DataTree
//## @ingroup DataTree
//## Contains the data (instance of BaseData), a list of mappers, which can
//## draw the data, a transform (vtkTransform) and a list of properties
//## (PropertyList).
class DataTreeNode : public BaseData
{
public:
    mitkClassMacro(DataTreeNode,BaseData);

	itkNewMacro(Self);  

	//##ModelId=3D6A0E8C02CC
	mitk::Mapper* GetMapper(MapperSlotId id) const;
	//##ModelId=3E32C49D00A8
    //##Documentation
    //## @brief Get the data object (instance of BaseData, e.g., an Image)
    //## managed by this DataTreeNode
	BaseData* GetData() const;
    //##ModelId=3ED91D050121
    //##Documentation
    //## @brief Get the transformation applied prior to displaying the data as
    //## a vtkTransform
    //## @sa m_VtkTransform
	vtkTransform* GetVtkTransform() const;
	//##Documentation
	//## @brief Get the Interactor 
    StateMachine::Pointer GetInteractor() const;

    //##ModelId=3E33F4E4025B
    //##Documentation
    //## @brief Set the data object (instance of BaseData, e.g., an Image)
    //## managed by this DataTreeNode
    virtual void SetData(mitk::BaseData* baseData);
	//##Documentation
	//## @brief Set the Interactor
	virtual void SetInteractor(mitk::StateMachine* interactor);

    //##ModelId=3E33F5D7032D
    mitk::DataTreeNode& operator=(const DataTreeNode& right);

    //##ModelId=3E34248B02E0
    mitk::DataTreeNode& operator=(BaseData* right);
    //##ModelId=3E69331903C9
    virtual void SetMapper(MapperSlotId id, mitk::Mapper* mapper);
    //##ModelId=3E860A5C0032
    virtual void UpdateOutputInformation();

    //##ModelId=3E860A5E011B
    virtual void SetRequestedRegionToLargestPossibleRegion();

    //##ModelId=3E860A5F03D9
    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();

    //##ModelId=3E860A620080
    virtual bool VerifyRequestedRegion();

    //##ModelId=3E860A640156
    virtual void SetRequestedRegion(itk::DataObject *data);

    //##ModelId=3E860A6601DB
    virtual void CopyInformation(const itk::DataObject *data);
    //##ModelId=3E3FE0420273
    //##Documentation
    //## @brief Get the PropertyList of the @a renderer. If @a renderer is @a
    //## NULL, the BaseRenderer-independent PropertyList of this DataTreeNode
    //## is returned. 
    //## @sa GetProperty
    //## @sa m_PropertyList
    //## @sa m_MapOfPropertyLists
    mitk::PropertyList::Pointer GetPropertyList(const mitk::BaseRenderer* renderer = NULL) const;

    //##ModelId=3EF189DB0111
    //##Documentation
    //## @brief Get the property (instance of BaseProperty) with key @a propertyKey from the PropertyList 
    //## of the @a renderer, if available there, otherwise use the BaseRenderer-independent PropertyList.
    //## 
    //## If @a renderer is @a NULL or the @a propertyKey cannot be found 
    //## in the PropertyList specific to @a renderer, the BaseRenderer-independent 
    //## PropertyList of this DataTreeNode is queried.
    //## @sa GetPropertyList
    //## @sa m_PropertyList
    //## @sa m_MapOfPropertyLists
    mitk::BaseProperty::Pointer GetProperty(const char *propertyKey, const mitk::BaseRenderer* renderer = NULL) const;
    
    void SetProperty(const char *propertyKey, BaseProperty* property, const mitk::BaseRenderer* renderer = NULL);	

    //##ModelId=3EF1941C011F
    //##Documentation
    //## @brief Convenience access method for color properties (instances of
    //## ColorProperty)
    //## @return @a true property was found
    bool GetColor(float rgb[3], mitk::BaseRenderer* renderer, const char* name = "color") const;

    //##ModelId=3EF1941E01D6
    //##Documentation
    //## @brief Convenience access method for visibility properties (instances
    //## of BoolProperty)
    //## @return @a true property was found
    //## @sa IsVisible
    bool GetVisibility(bool &visible, mitk::BaseRenderer* renderer, const char* name = "visible") const;

    //##ModelId=3EF19420016B
    //##Documentation
    //## @brief Convenience access method for opacity properties (instances of
    //## FloatProperty)
    //## @return @a true property was found
    bool GetOpacity(float &opacity, mitk::BaseRenderer* renderer, const char* name = "opacity") const;

    //##ModelId=3EF194220204
    //##Documentation
    //## @brief Convenience access method for color properties (instances of
    //## LevelWindoProperty)
    //## @return @a true property was found
    bool GetLevelWindow(mitk::LevelWindow &levelWindow, mitk::BaseRenderer* renderer, const char* name = "levelwindow") const;

    //##ModelId=3EF19424012B
    //##Documentation
    //## @brief Convenience access method for visibility properties (instances
    //## of BoolProperty). Return value is the visibility. Default is
    //## visible==true, i.e., true is returned even if the property (@a
    //## propertyKey) is not found.
    //## 
    //## Thus, the return value has a different meaning than in the
    //## GetVisibility method!
    //## @sa GetVisibility
    bool IsVisible(mitk::BaseRenderer* renderer, const char* name = "visible") const;

    //##Documentation
    //## @brief Convenience method for setting color properties (instances of
    //## ColorProperty)
    void SetColor(const mitk::Color &color, mitk::BaseRenderer* renderer = NULL, const char* name = "color");
    //##Documentation
    //## @brief Convenience method for setting color properties (instances of
    //## ColorProperty)
    void SetColor(float red, float green, float blue, mitk::BaseRenderer* renderer = NULL, const char* name = "color");
    //##ModelId=3EF196360303
    //##Documentation
    //## @brief Convenience method for setting color properties (instances of
    //## ColorProperty)
    void SetColor(const float rgb[3], mitk::BaseRenderer* renderer = NULL, const char* name = "color");

    //##ModelId=3EF1966703D6
    //##Documentation
    //## @brief Convenience method for setting visibility properties (instances
    //## of BoolProperty)
    void SetVisibility(bool visible, mitk::BaseRenderer* renderer, const char* name = "visible");

    //##ModelId=3EF196880095
    //##Documentation
    //## @brief Convenience method for setting opacity properties (instances of
    //## FloatProperty)
    void SetOpacity(float opacity, mitk::BaseRenderer* renderer, const char* name = "opacity");

    //##ModelId=3EF1969A0181
    //##Documentation
    //## @brief Convenience method for setting level-window properties
    //## (instances of LevelWindowProperty)
    void SetLevelWindow(mitk::LevelWindow levelWindow, mitk::BaseRenderer* renderer, const char* name = "levelwindow");

protected:
    //##ModelId=3E33F5D702AA
    DataTreeNode();

    //##ModelId=3E33F5D702D3
    virtual ~DataTreeNode();

	//##ModelId=3D6A0F8C0202
    //##Documentation
    //## @brief Mapper-slots
    //## @todo change to stl-vector
    mutable mitk::Mapper::Pointer mappers[10];

    //##ModelId=3ED91D0500D3
    //##Documentation
    //## @brief Transformation applied prior to displaying the data
    //## 
    //## The advantage of such a transformation is that you do not need
    //## to change the data itself if you just want to move or rotate it.
    //## The OpenGL-people do the same with the glScale and glTranslate
    //## functions.
    //## Mappers must take this transformation into account during rendering.
    //## For vtk-based renderers using a vtkProp3D, this transformation 
    //## will be forked into the user-transformation of the vtkProp3D.
    //## A vtkInteractorStyle will change the user-transformation of 
    //## vtkProp3D and therewith m_VtkTransform.
	mutable vtkTransform* m_VtkTransform;

	//##ModelId=3E32C49D0095
    //##Documentation
    //## @brief The data object (instance of BaseData, e.g., an Image) managed
    //## by this DataTreeNode
    BaseData::Pointer m_Data;

    //##ModelId=3E861B84033C
    //##Documentation
    //## @brief BaseRenderer-independent PropertyList
    //## 
    //## Properties herein can be overwritten specifically for each BaseRenderer
    //## by the BaseRenderer-specific properties defined in m_MapOfPropertyLists.
    PropertyList::Pointer m_PropertyList;

    //##ModelId=3EF16CFA010A
    //##Documentation
    //## @brief Map associating each BaseRenderer with its own PropertyList
    mutable std::map<const mitk::BaseRenderer*,mitk::PropertyList::Pointer> m_MapOfPropertyLists;

	//##Documentation
	//## @brief StateMachine, that handles the Interaction
    mitk::StateMachine::Pointer m_Interactor;
};


#if (_MSC_VER > 1200) || !defined(_MSC_VER)
MBI_STD::istream& operator>>( MBI_STD::istream& i, DataTreeNode::Pointer& dtn );

MBI_STD::ostream& operator<<( MBI_STD::ostream& o, DataTreeNode::Pointer& dtn);
#endif
} // namespace mitk

#if ((defined(_MSC_VER)) && (_MSC_VER <= 1200))
MBI_STD::istream& operator>>( MBI_STD::istream& i, mitk::DataTreeNode::Pointer& dtn );

MBI_STD::ostream& operator<<( MBI_STD::ostream& o, mitk::DataTreeNode::Pointer& dtn);
#endif


#endif /* DATATREENODE_H_HEADER_INCLUDED_C1E14338 */

