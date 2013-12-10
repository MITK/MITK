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
#include "SegmentationExports.h"
#include <mitkLabelSet.h>
#include <mitkSurface.h>

#include <itkImage.h>
#include <itkVectorImage.h>
#include <itkVectorImageToImageAdaptor.h>


namespace mitk
{

//##Documentation
//## @brief LabelSetImage class for handling labels and layers in a segmentation session.
//##
//## Handles operations for adding, removing, erasing and editing labels and layers.
//## @ingroup Data

class Segmentation_EXPORT LabelSetImage : public Image
{

public:

  mitkClassMacro(LabelSetImage, Image);
  itkNewMacro(Self);

  mitkNewMacro1Param(Self, LabelSetImage*);

  typedef unsigned char PixelType;

  typedef itk::Image< PixelType, 3 >                     LabelSetImageType;
  typedef itk::VariableLengthVector< PixelType >         VariableVectorType;
  typedef itk::VectorImage< PixelType, 3 >               VectorImageType;
  typedef itk::VectorImageToImageAdaptor< PixelType, 3 > ImageAdaptorType;

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
  Message1<int> ModifyLabelEvent;

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

  /**
    * \brief  */
  virtual void Initialize(const mitk::Image* image);

  PixelType* GetImageLayer(int layer);

  /**
    * \brief  */
  void Concatenate(mitk::LabelSetImage* image);

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
  void MergeLabels(std::vector<int>& indexes, int index, int layer = -1);

  /**
    * \brief
  */
  void MergeLabel(int index, int layer = -1);

  /**
    * \brief  */
  void EraseLabel(int index, bool reorder, int layer = -1);

  /**
    * \brief  */
  void CalculateLabelVolume(int index, int layer = -1);

  /**
    * \brief  */
  void SetLastModificationTime(const std::string& date);

  /**
    * \brief  */
  const std::string& GetLastModificationTime();

  /**
    * \brief  */
  void SetName(const std::string& date);

  /**
    * \brief  */
  const std::string& GetName();

  /**
    * \brief  */
  void SetLabelColor(int index, const mitk::Color &color, int layer = -1);

  /**
    * \brief  */
  const mitk::Color& GetLabelColor(int index, int layer = -1);

  /**
    * \brief  */
  void SetLabelOpacity(int index, float value, int layer = -1);

  /**
    * \brief  */
  void SetLabelVolume(int index, float value, int layer = -1);

  /**
    * \brief  */
  void RenameLabel(int index, const std::string& name, const mitk::Color& color, int layer = -1);

  /**
    * \brief  */
  void SetLabelName(int index, const std::string& name, int layer = -1);

  /**
    * \brief  */
  void SetAllLabelsLocked(bool sendEvent, int layer = -1);

  /**
    * \brief  */
  void SetAllLabelsVisible(bool sendEvent, int layer = -1);

  /**
    * \brief  */
  void SetLabelLocked(int index, bool value, int layer = -1);

  /**
    * \brief  */
  void SetLabelSelected(int index, bool value, int layer = -1);

  /**
    * \brief  */
  void SetLabelVisible(int index, bool value = false, int layer = -1);

  /**
    * \brief  */
  void RemoveLabel(int index, int layer = -1);

  /**
    * \brief  */
  void RemoveLabels(std::vector<int>& indexes, int layer = -1);

  /**
    * \brief  */
  void EraseLabels(std::vector<int>& indexes, int layer = -1);

  /**
    * \brief  */
  std::string GetLabelName(int index, int layer = -1) const;

  /**
    * \brief  */
  bool GetLabelSelected(int index, int layer = -1) const;

  /**
    * \brief  */
  bool GetLabelLocked(int index, int layer = -1) const;

  /**
    * \brief  */
  bool GetLabelVisible(int index, int layer = -1) const;

  /**
    * \brief  */
  int GetActiveLabelIndex(int layer = -1) const;

  /**
    * \brief  */
  const mitk::Color& GetActiveLabelColor(int layer = -1) const;

  /**
    * \brief  */
  const char* GetActiveLabelName(int layer = -1) const;

   /**
    * \brief  */
  double GetActiveLabelOpacity(int layer = -1) const;

  /**
    * \brief  */
  const mitk::Label* GetActiveLabel(int layer = -1) const;

  /**
    * \brief  */
  void SetActiveLabel(int index, int layer = -1);

  /**
  * \brief  */
  int GetActiveLayer() const;

  /**
    * \brief  */
  const mitk::Point3D& GetLabelCenterOfMassIndex(int index, bool forceUpdate = false, int layer = -1);

  /**
    * \brief  */
  const mitk::Point3D& GetLabelCenterOfMassCoordinates(int index, bool forceUpdate = false, int layer = -1);

  /**
    * \brief  */
  float GetLabelOpacity(int index, int layer = -1);

  /**
    * \brief  */
  float GetLabelVolume(int index, int layer = -1);

  /**
    * \brief  */
  int GetNumberOfLabels(int layer = -1) const;

  /**
    * \brief  */
  int GetTotalNumberOfLabels() const;

  /**
    * \brief  */
  void RemoveAllLabels(int layer = -1);

  /**
    * \brief  */
  mitk::LabelSet::ConstPointer GetLabelSet(int layer = -1) const;

  /**
    * \brief  */
  mitk::LabelSet* GetLabelSet(int layer = -1);

  /**
    * \brief  */
  const mitk::LookupTable* GetLookupTable(int layer = -1);

  /**
    * \brief  */
  void SetLabelSet(int layer, mitk::LabelSet* labelset);

  /**
    * \brief  */
//  bool IsLabelSelected(mitk::Label::Pointer label);

  /**
    * \brief  */
  void ResetLabels(int layer = -1);

  /**
    * \brief  */
  void SurfaceStamp(mitk::Surface* surface, bool forceOverwrite);

  /**
    * \brief  */
  mitk::Image::Pointer CreateLabelMask(int index);

  /**
    * \brief  */
  void InitializeByLabeledImage(mitk::Image::Pointer image);

  /**
    * \brief  */
  void MaskStamp(mitk::Image* mask, bool forceOverwrite);

  /**
    * \brief  */
  void SetActiveLayer(int layer);

  /**
    * \brief  */
  int GetNumberOfLayers();

  /**
    * \brief  */
  VectorImageType::Pointer GetVectorImage(bool forceUpdate);

  /**
    * \brief  */
  void SetVectorImage(VectorImageType::Pointer image );

  /**
    * \brief  */
  void AddLayer();

  /**
    * \brief  */
  void RemoveLayer();

  /**
    * \brief  */
  mitk::Image* GetLayerImage(int layer);

protected:
  LabelSetImage();
  LabelSetImage(mitk::LabelSetImage*);
  virtual ~LabelSetImage();

  LabelSet::Pointer CreateDefaultLabelSet();

  template < typename ImageType1, typename ImageType2 >
  void ChangeLayerProcessing( ImageType1* source, ImageType2* target );

  template < typename ImageType >
  void AddLayerProcessing( ImageType* input);

  template < typename ImageType >
  void LayerContainerToImageProcessing( ImageType* input, int layer);

  template < typename ImageType >
  void ImageToLayerContainerProcessing( ImageType* input, int layer);

  template < typename ImageType >
  void CalculateCenterOfMassProcessing( ImageType* input, int index, int layer);

  template < typename ImageType >
  void ClearBufferProcessing( ImageType* input);

  template < typename ImageType >
  void EraseLabelProcessing( ImageType* input, int index, int layer);

  template < typename ImageType >
  void ReorderLabelProcessing( ImageType* input, int index, int layer);

  template < typename ImageType >
  void MergeLabelProcessing( ImageType* input, int pixelValue, int index);

  template < typename ImageType >
  void ConcatenateProcessing( ImageType* input, mitk::LabelSetImage* other);

  template < typename ImageType >
  void MaskStampProcessing( ImageType* input, mitk::Image* mask, bool forceOverwrite);

  template < typename ImageType >
  void CreateLabelMaskProcessing( ImageType* input, mitk::Image* mask, int index);

  template < typename ImageType1, typename ImageType2 >
  void InitializeByLabeledImageProcessing( ImageType1* input, ImageType2* other);

  std::vector< LabelSet::Pointer > m_LabelSetContainer;
  std::vector< Image::Pointer > m_LayerContainer;

  int m_ActiveLayer;

  std::string m_LastModificationTime;

  std::string m_Name;

};

} // namespace mitk

#endif // __mitkLabelSetImage_H_
