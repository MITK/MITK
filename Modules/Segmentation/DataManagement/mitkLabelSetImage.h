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

//class ImageVtkAccessor;

//##Documentation
//## @brief LabelSetImage class for storing labels in a segmentation session.
//##
//## Handles operations for editing labels.
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
  * \brief this constructor creates a labelset image out of the provided reference image.
  * @throw mitk::Exception Throws an exception if there is a problem while creating the labelset image.
  */
 // mitkNewMacro1Param(Self, Image::Pointer);

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
  void MergeLabels(int layer, std::vector<int>& indexes, int index);

  /**
    * \brief  */
  void EraseLabel(int layer, int index, bool reorder);

  /**
    * \brief  */
  void SmoothLabel(int layer, int index);

  /**
    * \brief  */
  void CalculateLabelVolume(int layer, int index);

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
  void SetLabelColor(int layer, int index, const mitk::Color &color);

  /**
    * \brief  */
  const mitk::Color& GetLabelColor(int layer, int index);

  /**
    * \brief  */
  void SetLabelOpacity(int layer, int index, float value);

  /**
    * \brief  */
  void SetLabelVolume(int layer, int index, float value);

  /**
    * \brief  */
  void RenameLabel(int layer, int index, const std::string& name, const mitk::Color& color);

  /**
    * \brief  */
  void SetLabelName(int layer, int index, const std::string& name);

  /**
    * \brief  */
  std::string GetLabelName(int layer, int index);

  /**
    * \brief  */
  void SetAllLabelsLocked(int layer, bool);

  /**
    * \brief  */
  void SetAllLabelsVisible(int layer, bool);

  /**
    * \brief  */
  void SetLabelLocked(int layer, int index, bool value);

  /**
    * \brief  */
  void SetLabelSelected(int layer, int index, bool value);

  /**
    * \brief  */
  void SetLabelVisible(int layer, int index, bool value);

  /**
    * \brief  */
  void RemoveLabel(int layer, int index);

  /**
    * \brief  */
  void RemoveLabels(int layer, std::vector<int>& indexes);

  /**
    * \brief  */
  void EraseLabels(int layer, std::vector<int>& indexes);

/**
    * \brief  */
  void SmoothLabels(int layer, std::vector<int>& indexes);

  /**
    * \brief  */
  bool GetLabelSelected(int layer, int index) const;

  /**
    * \brief  */
  bool GetLabelLocked(int layer, int index) const;

  /**
    * \brief  */
  bool GetLabelVisible(int layer, int index) const;

  /**
    * \brief  */
  int GetActiveLabelIndex(int layer) const;

  /**
    * \brief  */
  const mitk::Color& GetActiveLabelColor(int layer) const;

   /**
    * \brief  */
  double GetActiveLabelOpacity(int layer) const;

  /**
    * \brief  */
  const mitk::Label* GetActiveLabel(int layer) const;

  /**
  * \brief  */
  int GetActiveLayer() const;

  /**
    * \brief  */
  void SetActiveLabel(int layer, int index, bool sendEvent);

  /**
    * \brief  */
  const mitk::Point3D& GetLabelCenterOfMassIndex(int layer, int index, bool forceUpdate);

  /**
    * \brief  */
  const mitk::Point3D& GetLabelCenterOfMassCoordinates(int layer, int index, bool forceUpdate);

  /**
    * \brief  */
  float GetLabelOpacity(int layer, int index);

  /**
    * \brief  */
  float GetLabelVolume(int layer, int index);

  /**
    * \brief  */
  int GetNumberOfLabels(int layer) const;

  /**
    * \brief  */
  int GetTotalNumberOfLabels() const;

  /**
    * \brief  */
  void RemoveAllLabels(int layer);

  /**
    * \brief  */
  mitk::LabelSet::ConstPointer GetLabelSet(int layer) const;

  /**
    * \brief  */
  mitk::LabelSet* GetLabelSet(int layer);

  /**
    * \brief  */
  const mitk::LookupTable* GetLookupTable(int layer);

  /**
    * \brief  */
  void SetLabelSet(int layer, mitk::LabelSet* labelset);

  /**
    * \brief  */
//  bool IsLabelSelected(mitk::Label::Pointer label);

  /**
    * \brief  */
  void ResetLabels(int layer);

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

  VectorImageType::Pointer GetVectorImage(bool forceUpdate);

  void SetVectorImage(VectorImageType::Pointer image );

  /**
    * \brief  */
  void AddLayer();

  /**
    * \brief  */
  void RemoveLayer();

  /**
    * \brief  */
  PixelType* GetLayerBufferPointer(int layer);

protected:
  LabelSetImage();
  LabelSetImage(mitk::LabelSetImage*);
  virtual ~LabelSetImage();

  void CreateDefaultLabelSet(int layer);

  //mutable ImageVtkAccessor* m_VtkImageData;

  template < typename ImageType1, typename ImageType2 >
  void ChangeLayerProcessing( ImageType1* source, ImageType2* target );

  template < typename ImageType >
  void AddLayerProcessing( ImageType* input);

  template < typename ImageType >
  void VectorToImageProcessing( ImageType* input, int layer);

  template < typename ImageType >
  void ImageToVectorProcessing( ImageType* input, int layer);

  template < typename ImageType >
  void CalculateCenterOfMassProcessing( ImageType* input, int index);

  template < typename ImageType >
  void ClearBufferProcessing( ImageType* input);

  template < typename ImageType >
  void EraseLabelProcessing( ImageType* input, int index, bool reorder);

  template < typename ImageType >
  void SmoothLabelProcessing( ImageType* input, int layer, int index);

  template < typename ImageType >
  void MergeLabelsProcessing( ImageType* input, int index);

  template < typename ImageType >
  void ConcatenateProcessing( ImageType* input, mitk::LabelSetImage* other);

  template < typename ImageType >
  void MaskStampProcessing( ImageType* input, mitk::Image* mask, bool forceOverwrite);

  template < typename ImageType >
  void CreateLabelMaskProcessing( ImageType* input, mitk::Image* mask, int index);

  template < typename ImageType1, typename ImageType2 >
  void InitializeByLabeledImageProcessing( ImageType1* input, ImageType2* other);

  std::vector< LabelSet::Pointer > m_LabelSetContainer;

  int m_ActiveLayer;

  std::string m_LastModificationTime;

  std::string m_Name;

  VectorImageType::Pointer  m_VectorImage;
  ImageAdaptorType::Pointer m_ImageToVectorAdaptor;
};

} // namespace mitk

#endif // __mitkLabelSetImage_H_
