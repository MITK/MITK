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

  /**
  * \brief this constructor creates a labelset image identical to the recieved mitkImage. The Metadata are set to default.
  *  The image data is shared, so don't continue to manipulate the original image.
  * @throw mitk::Exception Throws an exception if there is a problem with access to the data while constructing the image.
  */
  mitkNewMacro1Param(Self, Image::Pointer);

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
  * \brief ModifyLabelEvent is emitted whenever a new label has been removed from the LabelSet.
  *
  * Observers should register to this event by calling myLabelSet->ModifyLabelEvent.AddListener(myObject, MyObject::MyMethod).
  * After registering, myObject->MyMethod() will be called every time a new label has been removed from the LabelSet.
  * Observers should unregister by calling myLabelSet->ModifyLabelEvent.RemoveListener(myObject, MyObject::MyMethod).
  *
  * member variable is not needed to be locked in multi-threaded scenarios since the LabelSetEvent is a typedef for
  * a Message object which is thread safe
  */
  Message1<int> ModifyLabelEvent;

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

  /**
    * \brief  */
  bool Concatenate(mitk::LabelSetImage* image);

  /**
    * \brief  */
  void ClearBuffer();

  /**
    * \brief   */
  void AddLabel(const std::string& name, const mitk::Color& color);

  /**
    * \brief    */
  void AddLabel(const mitk::Label& label);

  /**
    * \brief
  */
  void MergeLabels(int begin, int count, int index);

  /**
    * \brief  */
  void EraseLabel(int index, bool reorder);

  /**
    * \brief  */
  void CalculateLabelVolume(int index);

  /**
    * \brief  */
  void SetName(const std::string& name);

  /**
    * \brief  */
  std::string GetLabelSetName();

  /**
    * \brief  */
  void SetLabelSetLastModified(const std::string& date);

  /**
    * \brief  */
  std::string GetLabelSetLastModified();

  /**
    * \brief  */
  void SetLabelColor(int index, const mitk::Color &color);

  /**
    * \brief  */
  const mitk::Color& GetLabelColor(int index);

  /**
    * \brief  */
  void SetLabelOpacity(int index, float value);

  /**
    * \brief  */
  void SetLabelVolume(int index, float value);

  /**
    * \brief  */
  void RenameLabel(int index, const std::string& name, const mitk::Color& color);

  /**
    * \brief  */
  void SetLabelName(int index, const std::string& name);

  /**
    * \brief  */
  std::string GetLabelName(int index);

  /**
    * \brief  */
  void SetAllLabelsLocked(bool);

  /**
    * \brief  */
  void SetAllLabelsVisible(bool);

  /**
    * \brief  */
  void SetLabelLocked(int index, bool value);

  /**
    * \brief  */
  void SetLabelSelected(int index, bool value);

  /**
    * \brief  */
  void SetLabelVisible(int index, bool value);

  /**
    * \brief  */
  void RemoveLabel(int index);

  /**
    * \brief  */
  void RemoveLabels(std::vector<int>& indexes);

  /**
    * \brief  */
  void EraseLabels(std::vector<int>& indexes);

//  void RemoveActiveLabel();

  /**
    * \brief  */
  bool GetLabelSelected(int index) const;

  /**
    * \brief  */
  bool GetLabelLocked(int index) const;

  /**
    * \brief  */
  bool GetLabelVisible(int index);

  /**
    * \brief  */
  int GetActiveLabelIndex() const;

  /**
    * \brief  */
  const mitk::Color& GetActiveLabelColor() const;

  const mitk::Label* GetActiveLabel() const;

  /**
    * \brief  */
  void SetActiveLabel(int index, bool sendEvent);

  /**
    * \brief  */
  const mitk::Point3D& GetLabelCenterOfMassIndex(int index, bool update);

  /**
    * \brief  */
  const mitk::Point3D& GetLabelCenterOfMassCoordinates(int index, bool update);

  /**
    * \brief  */
  float GetLabelOpacity(int index);

  /**
    * \brief  */
  float GetLabelVolume(int index);

  /**
    * \brief  */
  int GetNumberOfLabels();

  /**
    * \brief  */
  void RemoveAllLabels();

  /**
    * \brief  */
  mitk::LabelSet::ConstPointer GetConstLabelSet() const;

  /**
    * \brief  */
  mitk::LabelSet* GetLabelSet();

  /**
    * \brief  */
  void SetLabelSet(mitk::LabelSet* labelset);

  /**
    * \brief  */
  bool IsLabelSelected(mitk::Label::Pointer label);

  /**
    * \brief  */
  void ResetLabels();

protected:
  LabelSetImage();
  virtual ~LabelSetImage();

  virtual void CreateDefaultLabelSet();

  /**
    * \brief this constructor creates a labelset Image identical to the recieved mitkImage. The Metadata are set to default.
    *  The image data is shared, so don't continue to manipulate the original image.
  */
  LabelSetImage(mitk::Image::Pointer image);

  template < typename LabelSetImageType >
  void CalculateCenterOfMassProcessing(LabelSetImageType * itkImage,int index);

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
