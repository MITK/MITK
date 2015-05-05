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

class MITKMULTILABEL_EXPORT LabelSetImage : public Image
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
  using mitk::Image::Initialize;
  virtual void Initialize(const mitk::Image* image) override;

  /**
    * \brief  */
  void Concatenate(mitk::LabelSetImage* image);

  /**
    * \brief  */
  void ClearBuffer();


  /**
   * @brief Merges the mitk::Label with a given target value with the active label
   * @param targetPixelValue the value of the mitk::Label that should be merged with the active one
   * @param layer the layer in which the merge should be performed
   */
  void MergeLabel(PixelType targetPixelValue, unsigned int layer = 0);

  /**
   * @brief Merges a list of mitk::Labels with the mitk::Label that has a specific value
   * @param VectorOfLablePixelValues the list of labels that should be merge with the specified one
   * @param index the value of the label into which the other should be merged
   * @param layer the layer in which the merge should be performed
   */
  void MergeLabels(std::vector<PixelType>& VectorOfLablePixelValues, PixelType index, unsigned int layer = 0);

  /**
    * \brief  */
  void UpdateCenterOfMass(PixelType pixelValue, unsigned int layer =0);


  /**
   * @brief Removes labels from the mitk::LabelSet of given layer.
   *        Calls mitk::LabelSetImage::EraseLabels() which also removes the labels from within the image.
   * @param VectorOfLabelPixelValues a list of labels to be removed
   * @param layer the layer in which the labels should be removed
   */
  void RemoveLabels(std::vector<PixelType>& VectorOfLabelPixelValues, unsigned int layer = 0);

  /**
   * @brief Erases the label with the given value in the given layer from the underlying image.
   *        The label itself will not be erased from the respective mitk::LabelSet. In order to
   *        remove the label itself use mitk::LabelSetImage::RemoveLabels()
   * @param pixelValue the label which will be remove from the image
   * @param layer the layer in which the label should be removed
   */
  void EraseLabel(PixelType pixelValue, unsigned int layer = 0);

  /**
   * @brief Similar to mitk::LabelSetImage::EraseLabel() this funtion erase a list of labels from the image
   * @param VectorOfLabelPixelValues the list of labels that should be remove
   * @param layer the layer for which the labels should be removed
   */
  void EraseLabels(std::vector<PixelType>& VectorOfLabelPixelValues, unsigned int layer = 0);

  /**
    * \brief  Returns true if the value exists in one of the labelsets*/
  bool ExistLabel(PixelType pixelValue) const;

  /**
   * @brief Checks if a label exists in a certain layer
   * @param pixelValue the label value
   * @param layer the layer in which should be searched for the label
   * @return true if the label exists otherwise false
   */
  bool ExistLabel(PixelType pixelValue, unsigned int layer) const;

  /**
    * \brief  Returns true if the labelset exists*/
  bool ExistLabelSet(unsigned int layer) const;

  /**
   * @brief Returns the active label of a specific layer
   * @param layer the layer ID for which the active label should be returned
   * @return the active label of the specified layer
   */
  mitk::Label* GetActiveLabel(unsigned int layer = 0);

  /**
   * @brief Returns the mitk::Label with the given pixelValue and for the given layer
   * @param pixelValue the pixel value of the label
   * @param layer the layer in which the labels should be located
   * @return the mitk::Label if available otherwise NULL
   */
  mitk::Label* GetLabel(PixelType pixelValue, unsigned int layer = 0) const;

  /**
   * @brief Returns the currently active mitk::LabelSet
   * @return the mitk::LabelSet of the active layer or NULL if non is present
   */
  mitk::LabelSet* GetActiveLabelSet();

  /**
   * @brief Gets the mitk::LabelSet for the given layer
   * @param layer the layer for which the mitk::LabelSet should be retrieved
   * @return the respective mitk::LabelSet or NULL if non exists for the given layer
   */
  mitk::LabelSet * GetLabelSet(unsigned int layer = 0);
  const mitk::LabelSet * GetLabelSet(unsigned int layer = 0) const;

  /**
   * @brief Gets the ID of the currently active layer
   * @return the ID of the active layer
   */
  unsigned int GetActiveLayer() const;

  /**
   * @brief Get the number of all existing mitk::Labels for a given layer
   * @param layer the layer ID for which the active mitk::Labels should be retrieved
   * @return the number of all existing mitk::Labels for the given layer
   */
  unsigned int GetNumberOfLabels(unsigned int layer = 0) const;

  /**
   * @brief Returns the number of all labels summed up across all layers
   * @return the overall number of labels across all layers
   */
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
   * @brief Adds a new layer to the LabelSetImage. The new layer will be set as the active one
   * @param layer a mitk::LabelSet which will be set as new layer.
   * @return the layer ID of the new layer
   */
  unsigned int AddLayer(mitk::LabelSet::Pointer layer =0);

  /**
   * @brief Removes the active layer and the respective mitk::LabelSet and image information.
   *        The new active layer is the one below, if exists
   */
  void RemoveLayer();

  /**
    * \brief  */
  mitk::Image* GetLayerImage(unsigned int layer);

  const mitk::Image* GetLayerImage(unsigned int layer) const;

  void OnLabelSetModified();

  /**
   * @brief Sets the label which is used as default exterior label when creating a new layer
   * @param label the label which will be used as new exterior label
   */
  void SetExteriorLabel(mitk::Label * label);

  /**
   * @brief Gets the mitk::Label which is used as default exterior label
   * @return the exterior mitk::Label
   */
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
MITKMULTILABEL_EXPORT bool Equal( const mitk::LabelSetImage& leftHandSide, const mitk::LabelSetImage& rightHandSide, ScalarType eps, bool verbose );


} // namespace mitk

#endif // __mitkLabelSetImage_H_
