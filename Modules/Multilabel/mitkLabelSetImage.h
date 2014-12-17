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
#include "MitkMultilabelExports.h"
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

class MitkMultilabel_EXPORT LabelSetImage : public Image
{

public:

  mitkClassMacro(LabelSetImage, Image)
  itkNewMacro(Self)

  typedef mitk::Label::PixelType PixelType;

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
   * @brief Initialize an empty mitk::LabelSetImage using the information
   *        of an mitk::Image
   * @param image the image which is used for initializing the mitk::LabelSetImage
   */
  virtual void Initialize(const mitk::Image* image);

  /**
    * \brief  */
  void Concatenate(mitk::LabelSetImage* image);

  /**
    * \brief  */
  void ClearBuffer();

  /**
    * \brief
  */
  void MergeLabels(std::vector<PixelType>& VectorOfLablePixelValues, PixelType index, unsigned int layer = 0);

  /**
    * \brief
  */
  void MergeLabel(PixelType targetPixelValue, unsigned int layer = 0);

  /**
    * \brief  */
  void UpdateCenterOfMass(PixelType pixelValue, unsigned int layer =0);

  /**
    * \brief  */
//  void CalculateLabelVolume(PixelType index, int layer = -1);

  /**
    * \brief  */
  void RemoveLabels(std::vector<PixelType>& VectorOfLabelPixelValues, unsigned int layer = 0);

  /**
    * \brief  */
  void EraseLabel(PixelType pixelValue, unsigned int layer = 0);

  /**
    * \brief  */
  void EraseLabels(std::vector<PixelType>& VectorOfLabelPixelValues, unsigned int layer = 0);

  /**
    * \brief  Returns true if the value exists in one of the labelsets*/
  bool ExistLabel(const PixelType pixelValue);


  /**
    * \brief  Returns true if the labelset exists*/
  bool ExistLabelSet(const unsigned int layer);

  /**
    * \brief  */
  mitk::Label* GetActiveLabel(unsigned int layer = 0);

  /**
    * \brief  */
  mitk::Label* GetLabel(PixelType pixelValue, unsigned int layer = 0) const;

  /**
    * \brief  */
  mitk::LabelSet* GetActiveLabelSet();

  /**
    * \brief  */
  mitk::LabelSet * GetLabelSet(unsigned int layer = 0);

  const mitk::LabelSet * GetLabelSet(unsigned int layer = 0) const;

  /**
  * \brief  */
  unsigned int GetActiveLayer() const;

  /**
    * \brief  */
  unsigned int GetNumberOfLabels(unsigned int layer = 0) const;

  /**
    * \brief  */
  unsigned int GetTotalNumberOfLabels() const;

  /**
    * \brief  */
  void SurfaceStamp(mitk::Surface* surface, bool forceOverwrite);

  /**
    * \brief  */
  mitk::Image::Pointer CreateLabelMask(PixelType index);

  /**
   * @brief Initialize a new mitk::LabelSetImage by an given image.
   * For all distinct pixel values of the parameter image new labels will
   * be created. If the number of distinct pixel values exceeds mitk::Label::MAX_LABEL_VALUE
   * a new layer will be created
   * @param image the image which is used for initialization
   */
  void InitializeByLabeledImage(mitk::Image::Pointer image);

  /**
    * \brief  */
  void MaskStamp(mitk::Image* mask, bool forceOverwrite);

  /**
    * \brief  */
  void SetActiveLayer(unsigned int layer);

  /**
    * \brief  */
  unsigned int GetNumberOfLayers() const;

  /**
    * \brief  */
  VectorImageType::Pointer GetVectorImage(bool forceUpdate) const;

  /**
    * \brief  */
  void SetVectorImage(VectorImageType::Pointer image );

  /**
    * \brief  */
  unsigned int AddLayer(mitk::LabelSet::Pointer=0);

  /**
    * \brief  */
  void RemoveLayer();

  /**
    * \brief  */
  mitk::Image* GetLayerImage(unsigned int layer);

  const mitk::Image* GetLayerImage(unsigned int layer) const;

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
  void LayerContainerToImageProcessing( ImageType* source, unsigned int layer);

  template < typename ImageType >
  void ImageToLayerContainerProcessing( ImageType* source, unsigned int layer) const;

  template < typename ImageType >
  void CalculateCenterOfMassProcessing( ImageType* input, PixelType index, unsigned int layer);

  template < typename ImageType >
  void ClearBufferProcessing( ImageType* input);

  template < typename ImageType >
  void EraseLabelProcessing( ImageType* input, PixelType index, unsigned int layer);

//  template < typename ImageType >
//  void ReorderLabelProcessing( ImageType* input, int index, int layer);

  template < typename ImageType >
  void MergeLabelProcessing( ImageType* input, PixelType pixelValue, PixelType index);

  template < typename ImageType >
  void ConcatenateProcessing( ImageType* input, mitk::LabelSetImage* other);

  template < typename ImageType >
  void MaskStampProcessing( ImageType* input, mitk::Image* mask, bool forceOverwrite);

  template < typename ImageType >
  void CreateLabelMaskProcessing( ImageType* input, mitk::Image* mask, PixelType index);

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
