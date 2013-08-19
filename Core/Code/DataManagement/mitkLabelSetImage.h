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


#ifndef __mitkLabelSetImage_H_
#define __mitkLabelSetImage_H_

#include "mitkImage.h"
#include "MitkExports.h"
#include <mitkLabelSet.h>

#include <itkImage.h>

namespace mitk
{


//##Documentation
//## @brief LabelSetImage class for storing labels in a segmentation session.
//##
//## Handles operations for editing labels.
//## @ingroup Data

class MITK_CORE_EXPORT LabelSetImage : public Image
{

public:

  mitkClassMacro(LabelSetImage, Image);
  itkNewMacro(Self);

    // @brief AddLabelEvent is emitted whenever a new label has been added to the LabelSet.
    //
    // Observers should register to this event by calling myLabelSet->AddLabelEvent.AddListener(myObject, MyObject::MyMethod).
    // After registering, myObject->MyMethod() will be called every time a new label has been added to the LabelSet.
    // Observers should unregister by calling myLabelSet->AddLabelEvent.RemoveListener(myObject, MyObject::MyMethod).
    //
    // member variable is not needed to be locked in multi-threaded scenarios since the LabelSetEvent is a typedef for
    // a Message1 object which is thread safe
    Message<> AddLabelEvent;

    //Documentation
    // @brief RemoveLabelEvent is emitted whenever a new label has been removed from the LabelSet.
    //
    // Observers should register to this event by calling myLabelSet->RemoveLabelEvent.AddListener(myObject, MyObject::MyMethod).
    // After registering, myObject->MyMethod() will be called every time a new label has been removed from the LabelSet.
    // Observers should unregister by calling myLabelSet->RemoveLabelEvent.RemoveListener(myObject, MyObject::MyMethod).
    //
    // member variable is not needed to be locked in multi-threaded scenarios since the LabelSetEvent is a typedef for
    // a Message object which is thread safe
    Message<> RemoveLabelEvent;

    //Documentation
    // @brief ModifyLabelEvent is emitted whenever a new label is modified in the LabelSet.
    //
    // Observers should register to this event by calling myLabelSet->ModifyLabelEvent.AddListener(myObject, MyObject::MyMethod).
    // After registering, myObject->MyMethod() will be called every time a label has been modified in the LabelSet.
    // Observers should unregister by calling myLabelSet->ModifyLabelEvent.RemoveListener(myObject, MyObject::MyMethod).

    // member variable is not needed to be locked in multi-threaded scenarios since the LabelSetEvent is a typedef for
    // a Message1 object which is thread safe
    Message1<int> ModifyLabelEvent;

    //Documentation
    // @brief AllLabelsModifiedEvent is emitted whenever all label are modified in the LabelSet.
    //
    // Observers should register to this event by calling myLabelSet->ModifyLabelEvent.AddListener(myObject, MyObject::MyMethod).
    // After registering, myObject->MyMethod() will be called every time a label has been modified in the LabelSet.
    // Observers should unregister by calling myLabelSet->ModifyLabelEvent.RemoveListener(myObject, MyObject::MyMethod).

    // member variable is not needed to be locked in multi-threaded scenarios since the LabelSetEvent is a typedef for
    // a Message1 object which is thread safe
    Message<> AllLabelsModifiedEvent;

  /// handles operations
  virtual void ExecuteOperation(Operation* operation);

  bool Concatenate(mitk::LabelSetImage* image);

  void ClearBuffer();

  void AddLabel(const std::string& name, const mitk::Color& color);

  void AddLabel(const mitk::Label& label);

  void MergeLabels(int begin, int count, int index);

  void EraseLabel(int index, bool reorder);

  void CalculateLabelVolume(int index);

  void SetLabelSetName(const std::string& name);

  std::string GetLabelSetName();

  void SetLabelSetLastModified(const std::string& date);

  std::string GetLabelSetLastModified();

  virtual void SetLabelColor(int index, const mitk::Color &color);

  virtual const mitk::Color& GetLabelColor(int index);

  virtual void SetLabelOpacity(int index, float value);

  virtual void SetLabelVolume(int index, float value);

  virtual void RenameLabel(int index, const std::string& name, const mitk::Color& color);

  virtual void SetLabelName(int index, const std::string& name);

  virtual std::string GetLabelName(int index);

  virtual void SetAllLabelsLocked(bool);

  virtual void SetAllLabelsVisible(bool);

  virtual void SetLabelLocked(int index, bool value);

  virtual void SetLabelSelected(int index, bool value);

  virtual void SetLabelVisible(int index, bool value);

  virtual void RemoveLabel(int index);

  virtual void RemoveLabels(std::vector<int>& indexes);

  virtual void EraseLabels(std::vector<int>& indexes);

//  virtual void RemoveActiveLabel();

  virtual bool GetLabelSelected(int index) const;

  virtual bool GetLabelLocked(int index) const;

  virtual bool GetLabelVisible(int index);

  virtual int GetActiveLabelIndex() const;

  virtual const mitk::Color& GetActiveLabelColor() const;

  //virtual const mitk::Label* GetActiveLabel() const;

  virtual void SetActiveLabel(int index, bool sendEvent);

  virtual mitk::Point3D GetActiveLabelCenterOfMass();

  virtual float GetLabelOpacity(int index);

  virtual float GetLabelVolume(int index);

  virtual int GetNumberOfLabels();

  virtual void RemoveAllLabels();

  mitk::LabelSet::ConstPointer GetLabelSet() const
    {return m_LabelSet.GetPointer(); };

  void SetLabelSet(const mitk::LabelSet& labelset);

  bool IsLabelSelected(mitk::Label::Pointer label);

  virtual void ResetLabels();

protected:
  LabelSetImage();
  virtual ~LabelSetImage();

  template < typename LabelSetImageType >
  void CenterOfMassProcessing(LabelSetImageType * itkImage, mitk::Point3D& pos);

  template < typename LabelSetImageType >
  void ClearBufferProcessing(LabelSetImageType * itkImage);

  template < typename LabelSetImageType >
  void EraseLabelProcessing(LabelSetImageType * itkImage, int index, bool reorder);

  template < typename LabelSetImageType >
  void MergeLabelsProcessing(LabelSetImageType * itkImage, int index);

  template < typename LabelSetImageType >
  void ConcatenateProcessing(LabelSetImageType * itkImage1, mitk::LabelSetImage* other);

  mitk::LabelSet::Pointer m_LabelSet;
};

} // namespace mitk

#endif // __mitkLabelSetImage_H_
