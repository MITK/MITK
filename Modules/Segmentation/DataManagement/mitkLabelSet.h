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


#ifndef __mitkLabelSet_H_
#define __mitkLabelSet_H_

#include "MitkSegmentationExports.h"
#include <mitkMessage.h>
#include <mitkLookupTable.h>

#include <itkObject.h>
#include <itkObjectFactory.h>

#include <mitkLabel.h>

namespace mitk
{
//
//Documentation
// @brief LabelSet containing the labels corresponding to a segmentation session.
// @ingroup Data
//

class MitkSegmentation_EXPORT LabelSet : public itk::Object
{
public:

    mitkClassMacro( LabelSet, itk::Object );
    itkNewMacro(Self);

    typedef std::map <unsigned int, Label::Pointer>             LabelContainerType;
    typedef LabelContainerType::const_iterator    LabelContainerConstIteratorType;
    typedef LabelContainerType::iterator          LabelContainerIteratorType;

    /**
    * \brief AddLabelEvent is emitted whenever a new label has been added to the LabelSet.
    *
    * Observers should register to this event by calling myLabelSet->AddLabelEvent.AddListener(myObject, MyObject::MyMethod).
    * After registering, myObject->MyMethod() will be called every time a new label has been added to the LabelSet.
    * Observers should unregister by calling myLabelSet->AddLabelEvent.RemoveListener(myObject, MyObject::MyMethod).
    *
    * member variable is not needed to be locked in multi-threaded scenarios since the LabelSetEvent is a typedef for
    * a Message1 object which is thread safe
    */
    Message<> AddLabelEvent;

    /**
    * \brief RemoveLabelEvent is emitted whenever a new label has been removed from the LabelSet.
    *
    * Observers should register to this event by calling myLabelSet->RemoveLabelEvent.AddListener(myObject, MyObject::MyMethod).
    * After registering, myObject->MyMethod() will be called every time a new label has been removed from the LabelSet.
    * Observers should unregister by calling myLabelSet->RemoveLabelEvent.RemoveListener(myObject, MyObject::MyMethod).
    *
    * member variable is not needed to be locked in multi-threaded scenarios since the LabelSetEvent is a typedef for
    * a Message object which is thread safe
    */
    Message<> RemoveLabelEvent;

    /**
    * \brief ModifyLabelEvent is emitted whenever a label has been modified from the LabelSet.
    *
    * Observers should register to this event by calling myLabelSet->ModifyLabelEvent.AddListener(myObject, MyObject::MyMethod).
    * After registering, myObject->MyMethod() will be called every time a new label has been removed from the LabelSet.
    * Observers should unregister by calling myLabelSet->ModifyLabelEvent.RemoveListener(myObject, MyObject::MyMethod).
    *
    * member variable is not needed to be locked in multi-threaded scenarios since the LabelSetEvent is a typedef for
    * a Message object which is thread safe
    */
    Message<> ModifyLabelEvent;

    /**
    * \brief ActiveLabelEvent is emitted whenever a label has been set as active in the LabelSet.
    */
    Message1<int> ActiveLabelEvent;

    /**
    * \brief AllLabelsModifiedEvent is emitted whenever a new label has been removed from the LabelSet.
    *
    * Observers should register to this event by calling myLabelSet->AllLabelsModifiedEvent.AddListener(myObject, MyObject::MyMethod).
    * After registering, myObject->MyMethod() will be called every time a new label has been removed from the LabelSet.
    * Observers should unregister by calling myLabelSet->AllLabelsModifiedEvent.RemoveListener(myObject, MyObject::MyMethod).
    *
    * member variable is not needed to be locked in multi-threaded scenarios since the LabelSetEvent is a typedef for
    * a Message object which is thread safe
    */
    Message<> AllLabelsModifiedEvent;

    void Initialize(LabelSet *other);

    /** \brief Returns a const iterator poiting to the begining of the container.
    */
    LabelContainerConstIteratorType IteratorConstBegin();

    /** \brief Returns a const iterator pointing to the end of the container.
    */
    LabelContainerConstIteratorType IteratorConstEnd();

    /** \brief Returns a iterator poiting to the begining of the container.
    */
    LabelContainerIteratorType IteratorBegin();

    /** \brief Returns a iterator pointing to the end of the container.
    */
    LabelContainerIteratorType IteratorEnd();

    /** \brief
     * Recall itk::Object::Modified event from a label and send a ModifyLabelEvent
    */
    void OnLabelModified();

    /** \brief
    */
    void SetLayer(int);

    /** \brief
    */
    void SetActiveLabel(int);

    /** \brief
    */
    void RemoveLabel(int);

    /** \brief
    */
    bool ExistLabel(int pixelValue);

    /** \brief
    */
    void AddLabel(mitk::Label * label);

    /** \brief
    */
    void AddLabel(const std::string& name, const Color& color);

    /** \brief
    */
    void RenameLabel(int, const std::string&, const Color&);

    /** \brief
    */
    int GetNumberOfLabels() const;

    /** \brief
    */
    void SetAllLabelsVisible(bool);

    /** \brief
    */
    void SetAllLabelsLocked(bool);

    /** \brief
    */
    void RemoveAllLabels();

    /** \brief
    */
    Label* GetActiveLabel() { return m_ActiveLabel; }

    /** \brief
    */
    const Label* GetActiveLabel() const { return m_ActiveLabel; }

    /** \brief
    */
    Label* GetLabel(int pixelValue);

    /** \brief
    */
    const Label* GetLabel(int pixelValue) const;

    /** \brief
    */
    int GetLayer() { return m_Layer; }

    /** \brief
    */
    LookupTable* GetLookupTable();

    /** \brief
    */
    void SetLookupTable( LookupTable* lut );

    /** \brief
    */
    void UpdateLookupTable(int pixelValue);

protected:

    LabelSet();

    virtual ~LabelSet();

    void PrintSelf(std::ostream &os, itk::Indent indent) const;

    LabelContainerType m_LabelContainer;

    LookupTable::Pointer m_LookupTable;

    Label* m_ActiveLabel;

    int m_Layer;
};

} // namespace mitk

#endif // __mitkLabelSet_H_
