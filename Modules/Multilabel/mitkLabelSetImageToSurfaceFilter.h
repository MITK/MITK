/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _mitkLabelSetImageToSurfaceFilter_H_
#define _mitkLabelSetImageToSurfaceFilter_H_

#include "MitkMultilabelExports.h"
#include "mitkLabelSetImage.h"
#include "mitkSurface.h"
#include <mitkSurfaceSource.h>

#include <vtkMatrix4x4.h>

#include <itkImage.h>

#include <map>

namespace mitk
{
  /**
   * Generates surface meshes from a labelset image.
   * If you want to calculate a surface representation for all available labels,
   * you may call GenerateAllLabelsOn().
   */
  class MITKMULTILABEL_EXPORT LabelSetImageToSurfaceFilter : public SurfaceSource
  {
  public:
    mitkClassMacro(LabelSetImageToSurfaceFilter, SurfaceSource);

    itkNewMacro(Self);

    typedef LabelSetImage::PixelType LabelType;

    typedef std::map<LabelType, unsigned long> LabelMapType;

    typedef std::map<unsigned int, LabelType> IndexToLabelMapType;

    /**
    * Returns a const pointer to the labelset image set as input
    */
    const mitk::Image *GetInput(void);

    /**
    * Set the labelset image to create a surface from.
    */
    using ProcessObject::SetInput;
    virtual void SetInput(const mitk::Image *image);

    //   virtual void SetObserver(mitk::ProcessObserver::Pointer observer);

    /**
     * Set whether you want to extract all labesl (true) or a single one.
     */
    itkSetMacro(GenerateAllLabels, bool);

    /**
     * @returns if all labels or only a specific label should be
     * extracted.
     */
    itkGetMacro(GenerateAllLabels, bool);
    itkBooleanMacro(GenerateAllLabels);

    /**
     * Set the label you want to extract. This method only has an effect,
     * if GenerateAllLabels() is set to false
     * @param _arg the label to extract, by default 1
     */
    itkSetMacro(RequestedLabel, int);

    /**
     * Returns the label you want to extract. This method only has an effect,
     * if GenerateAllLabels() is set to false
     * @return the label to extract, by default 1
     */
    itkGetMacro(RequestedLabel, int);

    /**
     * Sets the label value of the background. No surface will be generated for this label.
     * @param _arg the label of the background, by default 0
     */
    itkSetMacro(BackgroundLabel, int);

    /**
     * Gets the label value of the background. No surface will be generated for this label.
     * @return the label of the background, by default 0
     */
    itkGetMacro(BackgroundLabel, int);

    /**
     * Sets whether to provide a smoothed surface
     */
    itkSetMacro(UseSmoothing, int);

    /**
     * Sets the Sigma used in the gaussian smoothing
     */
    itkSetMacro(Sigma, float);

  protected:
    LabelSetImageToSurfaceFilter();

    ~LabelSetImageToSurfaceFilter() override;

    /**
    * Transforms a point by a 4x4 matrix
    */
    template <class T1, class T2, class T3>
    inline void mitkVtkLinearTransformPoint(T1 matrix[4][4], T2 in[3], T3 out[3])
    {
      T3 x = matrix[0][0] * in[0] + matrix[0][1] * in[1] + matrix[0][2] * in[2] + matrix[0][3];
      T3 y = matrix[1][0] * in[0] + matrix[1][1] * in[1] + matrix[1][2] * in[2] + matrix[1][3];
      T3 z = matrix[2][0] * in[0] + matrix[2][1] * in[1] + matrix[2][2] * in[2] + matrix[2][3];
      out[0] = x;
      out[1] = y;
      out[2] = z;
    }

    mitk::Image::Pointer m_ResultImage;

    template <typename TPixel, unsigned int VImageDimension>
    void InternalProcessing(const itk::Image<TPixel, VImageDimension> *input, mitk::Surface *surface);

    bool m_GenerateAllLabels;

    int m_RequestedLabel;

    int m_BackgroundLabel;

    int m_UseSmoothing;

    float m_Sigma;

    LabelMapType m_AvailableLabels;

    IndexToLabelMapType m_IndexToLabels;

    mitk::Vector3D m_InputImageSpacing;

    void GenerateData() override;

    void GenerateOutputInformation() override;

    //  mitk::ProcessObserver::Pointer m_Observer;
  };

} // end of namespace mitk

#endif //_mitkLabelSetImageToSurfaceFilter_H_
