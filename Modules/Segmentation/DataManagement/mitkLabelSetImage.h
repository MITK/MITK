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
#include "MitkSegmentationExports.h"
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

class MitkSegmentation_EXPORT LabelSetImage : public Image
{

public:

  mitkClassMacro(LabelSetImage, Image)
  itkNewMacro(Self)

  typedef unsigned char PixelType;

  typedef itk::Image< PixelType, 3 >                     LabelSetImageType;
  typedef itk::VariableLengthVector< PixelType >         VariableVectorType;
  typedef itk::VectorImage< PixelType, 3 >               VectorImageType;
  typedef itk::VectorImageToImageAdaptor< PixelType, 3 > ImageAdaptorType;

  /**
  * \brief BeforeChangeLayerEvent (e.g. used for GUI integration)
  * As soon as active labelset should be changed, the signal emits.
  * Emitted by SetActiveLayer(int layer);
  */
  Message<> BeforeChangeLayerEvent;

  /**
  * \brief AfterchangeLayerEvent (e.g. used for GUI integration)
  * As soon as active labelset was changed, the signal emits.
  * Emitted by SetActiveLayer(int layer);
  */
  Message<> AfterchangeLayerEvent;

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
    * \brief
  */
  void MergeLabels(std::vector<int>& VectorOfLablePixelValues, int index, int layer = -1);

  /**
    * \brief
  */
  void MergeLabel(int targetPixelValue, int layer = -1);

  /**
    * \brief  */
  void UpdateCenterOfMass(int pixelValue, int layer = -1);

  /**
    * \brief  */
  void CalculateLabelVolume(int index, int layer = -1);

  /**
    * \brief  */
  void RemoveLabels(std::vector<int>& VectorOfLabelPixelValues, int layer = -1);

  /**
    * \brief  */
  void EraseLabel(int pixelValue, int layer = -1);

  /**
    * \brief  */
  void EraseLabels(std::vector<int>& VectorOfLabelPixelValues, int layer = -1);

  /**
    * \brief  Returns true if the value exists in one of the labelsets*/
  bool ExistLabel(const int pixelValue);


  /**
    * \brief  Returns true if the labelset exists*/
  bool ExistLabelSet(const int layer);

  /**
    * \brief  */
  mitk::Label* GetActiveLabel(int layer = -1);

  /**
    * \brief  */
  mitk::Label* GetLabel(int pixelValue, int layer = -1);

  /**
    * \brief  */
  mitk::LabelSet* GetActiveLabelSet();

  /**
    * \brief  */
  mitk::LabelSet * GetLabelSet(int layer = -1);

  const mitk::LabelSet * GetLabelSet(int layer = -1) const;

  /**
  * \brief  */
  int GetActiveLayer() const;

  /**
    * \brief  */
  int GetNumberOfLabels(int layer = -1) const;

  /**
    * \brief  */
  int GetTotalNumberOfLabels() const;

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
  int GetNumberOfLayers() const;

  /**
    * \brief  */
  VectorImageType::Pointer GetVectorImage(bool forceUpdate);

  /**
    * \brief  */
  void SetVectorImage(VectorImageType::Pointer image );

  /**
    * \brief  */
  int AddLayer();

  /**
    * \brief  */
  void RemoveLayer();

  /**
    * \brief  */
  mitk::Image* GetLayerImage(int layer);

  const mitk::Image* GetLayerImage(int layer) const;

  void OnLabelSetModified();

  void SetExteriorLabel(mitk::Label * label);

  mitk::Label* GetExteriorLabel();

  const mitk::Label* GetExteriorLabel() const;

protected:

  mitkCloneMacro(Self)

  LabelSetImage();
  LabelSetImage(const LabelSetImage & other);
  virtual ~LabelSetImage();

  template < typename ImageType1, typename ImageType2 >
  void ChangeLayerProcessing( ImageType1* source, ImageType2* target );

  template < typename ImageType >
  void LayerContainerToImageProcessing( ImageType* source, int layer);

  template < typename ImageType >
  void ImageToLayerContainerProcessing( ImageType* source, int layer);

  template < typename ImageType >
  void CalculateCenterOfMassProcessing( ImageType* input, int index, int layer);

  template < typename ImageType >
  void ClearBufferProcessing( ImageType* input);

  template < typename ImageType >
  void EraseLabelProcessing( ImageType* input, int index, int layer);

//  template < typename ImageType >
//  void ReorderLabelProcessing( ImageType* input, int index, int layer);

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

  mitk::Label::Pointer m_ExteriorLabel;

};

/**
* @brief Equal A function comparing two label set images for beeing equal in meta- and imagedata
*
* @ingroup MITKTestingAPI
*
* Following aspects are tested for equality:
*  - LabelSetImage members
*  - working image data
*  - layer image data
*  - labels in label set
*
* @param rightHandSide An image to be compared
* @param leftHandSide An image to be compared
* @param eps Tolarence for comparison. You can use mitk::eps in most cases.
* @param verbose Flag indicating if the user wants detailed console output or not.
* @return true, if all subsequent comparisons are true, false otherwise
*/
MITK_CORE_EXPORT bool Equal( const mitk::LabelSetImage& leftHandSide, const mitk::LabelSetImage& rightHandSide, ScalarType eps, bool verbose );


} // namespace mitk

#endif // __mitkLabelSetImage_H_
