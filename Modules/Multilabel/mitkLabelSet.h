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

#include "MitkMultilabelExports.h"
#include <mitkLookupTable.h>
#include <mitkMessage.h>

#include <itkObject.h>
#include <itkObjectFactory.h>

#include <mitkLabel.h>

namespace mitk
{
  //
  // Documentation
  // @brief LabelSet containing the labels corresponding to a segmentation session.
  // @ingroup Data
  //

  class MITKMULTILABEL_EXPORT LabelSet : public itk::Object
  {
  public:
    mitkClassMacroItkParent(LabelSet, itk::Object);
    itkNewMacro(Self);

    typedef mitk::Label::PixelType PixelType;

    typedef std::map<PixelType, Label::Pointer> LabelContainerType;
    typedef LabelContainerType::const_iterator LabelContainerConstIteratorType;
    typedef LabelContainerType::iterator LabelContainerIteratorType;

    /**
    * \brief AddLabelEvent is emitted whenever a new label has been added to the LabelSet.
    *
    * Observers should register to this event by calling myLabelSet->AddLabelEvent.AddListener(myObject,
    * MyObject::MyMethod).
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
    * Observers should register to this event by calling myLabelSet->RemoveLabelEvent.AddListener(myObject,
    * MyObject::MyMethod).
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
    * Observers should register to this event by calling myLabelSet->ModifyLabelEvent.AddListener(myObject,
    * MyObject::MyMethod).
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
    Message1<PixelType> ActiveLabelEvent;

    /**
    * \brief AllLabelsModifiedEvent is emitted whenever a new label has been removed from the LabelSet.
    *
    * Observers should register to this event by calling myLabelSet->AllLabelsModifiedEvent.AddListener(myObject,
    * MyObject::MyMethod).
    * After registering, myObject->MyMethod() will be called every time a new label has been removed from the LabelSet.
    * Observers should unregister by calling myLabelSet->AllLabelsModifiedEvent.RemoveListener(myObject,
    * MyObject::MyMethod).
    *
    * member variable is not needed to be locked in multi-threaded scenarios since the LabelSetEvent is a typedef for
    * a Message object which is thread safe
    */
    Message<> AllLabelsModifiedEvent;

    /** \brief Returns a const iterator poiting to the begining of the container.
    */
    LabelContainerConstIteratorType IteratorConstBegin() const;

    /** \brief Returns a const iterator pointing to the end of the container.
    */
    LabelContainerConstIteratorType IteratorConstEnd() const;

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
    void SetLayer(unsigned int);

    /** \brief
    */
    void SetActiveLabel(PixelType);

    /** \brief
    */
    void RemoveLabel(PixelType);

    /** \brief
    */
    bool ExistLabel(PixelType);

    /** \brief
    */
    void AddLabel(mitk::Label *label);

    /** \brief
    */
    void AddLabel(const std::string &name, const Color &color);

    /** \brief
    */
    void RenameLabel(PixelType, const std::string &, const Color &);

    /** \brief
    */
    unsigned int GetNumberOfLabels() const;

    /** \brief
    */
    void SetAllLabelsVisible(bool);

    /** \brief
    */
    void SetAllLabelsLocked(bool);

    /** \brief
    */
    void RemoveAllLabels();

    void SetNextActiveLabel();

    /** \brief
    */
    Label *GetActiveLabel() { return GetLabel(m_ActiveLabelValue); }
    /** \brief
    */
    const Label *GetActiveLabel() const { return GetLabel(m_ActiveLabelValue); }
    /** \brief
    */
    Label *GetLabel(PixelType pixelValue);

    /** \brief
    */
    const Label *GetLabel(PixelType pixelValue) const;

    itkGetMacro(Layer, int) itkGetConstMacro(Layer, int) itkGetModifiableObjectMacro(LookupTable, mitk::LookupTable)

      /** \brief
      */
      void SetLookupTable(LookupTable *lut);

    /** \brief
    */
    void UpdateLookupTable(PixelType pixelValue);

  protected:
    LabelSet();
    LabelSet(const LabelSet &);

    mitkCloneMacro(Self)

      ~LabelSet() override;

    void PrintSelf(std::ostream &os, itk::Indent indent) const override;

    LabelContainerType m_LabelContainer;

    LookupTable::Pointer m_LookupTable;

    PixelType m_ActiveLabelValue;

    unsigned int m_Layer;
  };

  /**
  * @brief Equal A function comparing two label sets  for beeing equal in data
  *
  * @ingroup MITKTestingAPI
  *
  * Following aspects are tested for equality:
  *  - LabelSetmembers
  *  - Label container (map)
  *
  * @param rightHandSide An image to be compared
  * @param leftHandSide An image to be compared
  * @param eps Tolarence for comparison. You can use mitk::eps in most cases.
  * @param verbose Flag indicating if the user wants detailed console output or not.
  * @return true, if all subsequent comparisons are true, false otherwise
  */
  MITKMULTILABEL_EXPORT bool Equal(const mitk::LabelSet &leftHandSide,
                                   const mitk::LabelSet &rightHandSide,
                                   ScalarType eps,
                                   bool verbose);

} // namespace mitk

#endif // __mitkLabelSet_H_
