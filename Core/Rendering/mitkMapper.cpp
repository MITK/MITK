#include "mitkMapper.h"
#include "mitkDataTreeNode.h"

//##ModelId=3E3C320001E4
//void mitk::Mapper::SetDisplaySizeInDisplayUnits(unsigned int widthInDisplayUnits, unsigned int heightInDisplayUnits)
//{
//    m_DisplaySizeInDisplayUnits.set(widthInDisplayUnits, heightInDisplayUnits); 
//}
//
//##ModelId=3E3C337E0162
mitk::Mapper::Mapper() //: m_DisplaySizeInDisplayUnits(10.0, 10.0)
{
	// Modify superclass default values, can be overridden by subclasses
	this->SetNumberOfRequiredInputs(1);
}


//##ModelId=3E3C337E019E
mitk::Mapper::~Mapper()
{
}

void mitk::Mapper::SetInput(const mitk::DataTreeNode* data)
{
    this->ProcessObject::SetNthInput(0,
        const_cast< mitk::DataTreeNode * >( data ) );
}

//##ModelId=3E860B9A0378
mitk::BaseData* mitk::Mapper::GetData() const
{
    return static_cast<const mitk::DataTreeNode *> (const_cast<mitk::Mapper*>(this)->ProcessObject::GetInput(0))->GetData();
}

//##ModelId=3E86B0EA00B0
mitk::DataTreeNode* mitk::Mapper::GetDataTreeNode() const
{
    mitk::DataTreeNode* node= static_cast<mitk::DataTreeNode *> (const_cast<mitk::Mapper*>(this)->ProcessObject::GetInput(0));
    return node;
}

//##ModelId=3EF17276014B
bool mitk::Mapper::GetColor(float rgb[3], mitk::BaseRenderer* renderer, const char* name) const
{
    const mitk::DataTreeNode* node=GetDataTreeNode();
    if(node==NULL)
        return false;

    return node->GetColor(rgb, renderer, name);
}

//##ModelId=3EF17795006A
bool mitk::Mapper::GetVisibility(bool &visible, mitk::BaseRenderer* renderer, const char* name) const
{
    const mitk::DataTreeNode* node=GetDataTreeNode();
    if(node==NULL)
        return false;

    return node->GetVisibility(visible, renderer, name);
}

//##ModelId=3EF1781F0285
bool mitk::Mapper::GetOpacity(float &opacity, mitk::BaseRenderer* renderer, const char* name) const
{
    const mitk::DataTreeNode* node=GetDataTreeNode();
    if(node==NULL)
        return false;

    return node->GetOpacity(opacity, renderer, name);
}

//##ModelId=3EF179660018
bool mitk::Mapper::GetLevelWindow(mitk::LevelWindow& levelWindow, mitk::BaseRenderer* renderer, const char* name) const
{
    const mitk::DataTreeNode* node=GetDataTreeNode();
    if(node==NULL)
        return false;

    return node->GetLevelWindow(levelWindow, renderer, name);
}

//##ModelId=3EF18B340008
bool mitk::Mapper::IsVisible(mitk::BaseRenderer* renderer, const char* name) const
{
    bool visible=true;
    GetVisibility(visible, renderer, name);
    return visible;
}

//##ModelId=3EF1A43C01D9
void mitk::Mapper::StandardUpdate()
{
    Update();
}

