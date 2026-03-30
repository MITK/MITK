/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRegEvaluationObject_h
#define mitkRegEvaluationObject_h

//MITK
#include <mitkImage.h>
#include <mitkDataNode.h>

//MatchPoint
#include <mapRegistrationBase.h>
#include <mapRegistration.h>
#include <mapExceptionObjectMacros.h>
#include <mapContinuousElements.h>

//MITK
#include <mitkMAPRegistrationWrapper.h>
#include <MitkMatchPointRegistrationExports.h>

namespace mitk
{
  /**
   * \brief Data container holding all information needed for visual evaluation of a registration.
   *
   * RegEvaluationObject bundles a registration together with rescaled target and moving images
   * (converted to unsigned char pixel type) for use by the RegEvaluationMapper2D. It also
   * stores references to the original data nodes for property lookup during rendering.
   *
   * \sa mitk::RegEvaluationMapper2D, mitk::RegEvalStyleProperty, mitk::MAPRegistrationWrapper
   */
  class MITKMATCHPOINTREGISTRATION_EXPORT RegEvaluationObject: public mitk::BaseData
  {
  public:

    mitkClassMacro( RegEvaluationObject, BaseData );

    itkNewMacro( Self );

    /**
     * \brief Passes through to the target image that defines the region.
     */
    void SetRequestedRegionToLargestPossibleRegion() override;

    /**
     * \brief Passes through to the target image that defines the region.
     * \return True if the requested region is outside the buffered region.
     */
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;

    /**
     * \brief Passes through to the target image that defines the region.
     * \return True if the requested region is valid.
     */
    bool VerifyRequestedRegion() override;

    /**
     * \brief Passes through to the target image that defines the region.
     * \param[in] data The data object from which to copy the requested region.
     */
    void SetRequestedRegion(const itk::DataObject* data) override;

    itkSetObjectMacro(Registration, mitk::MAPRegistrationWrapper);

    /**
     * \brief Sets the target image for evaluation.
     *
     * The input image is rescaled and converted to unsigned char pixel type for visualization.
     *
     * \param[in] tImg Pointer to the target image.
     */
    void SetTargetImage(const mitk::Image* tImg);

    /**
     * \brief Sets the moving image for evaluation.
     *
     * The input image is rescaled and converted to unsigned char pixel type for visualization.
     *
     * \param[in] mImg Pointer to the moving image.
     */
    void SetMovingImage(const mitk::Image* mImg);

    itkGetObjectMacro(Registration, mitk::MAPRegistrationWrapper);
    itkGetObjectMacro(TargetImage, mitk::Image);
    itkGetObjectMacro(MovingImage, mitk::Image);

    itkGetConstObjectMacro(Registration, mitk::MAPRegistrationWrapper);
    itkGetConstObjectMacro(TargetImage, mitk::Image);
    itkGetConstObjectMacro(MovingImage, mitk::Image);

    /**
     * \brief Sets the target data node reference for property access during rendering.
     * \param[in] tNode Pointer to the target data node.
     */
    void SetTargetNode(const mitk::DataNode* tNode);

    /**
     * \brief Sets the moving data node reference for property access during rendering.
     * \param[in] mNode Pointer to the moving data node.
     */
    void SetMovingNode(const mitk::DataNode* mNode);

    itkGetConstObjectMacro(TargetNode, mitk::DataNode);
    itkGetConstObjectMacro(MovingNode, mitk::DataNode);

  protected:
    typedef ::itk::Image<unsigned char, 3> InternalImageType;

    template <typename TPixelType, unsigned int VImageDimension >
    void doConversion(const ::itk::Image<TPixelType,VImageDimension>* input, mitk::Image::Pointer& result) const;

    void PrintSelf (std::ostream &os, itk::Indent indent) const override;

    RegEvaluationObject();
    ~RegEvaluationObject() override;

    mitk::MAPRegistrationWrapper::Pointer m_Registration;
    mitk::Image::Pointer m_TargetImage;
    mitk::Image::Pointer m_MovingImage;
    mitk::DataNode::ConstPointer m_TargetNode;
    mitk::DataNode::ConstPointer m_MovingNode;

  private:
    RegEvaluationObject& operator = (const RegEvaluationObject&);
    RegEvaluationObject(const RegEvaluationObject&);
  };

}

#endif
