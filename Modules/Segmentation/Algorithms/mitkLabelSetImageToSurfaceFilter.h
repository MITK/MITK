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

#ifndef _mitkLabelSetImageToSurfaceFilter_H_
#define _mitkLabelSetImageToSurfaceFilter_H_

#include <mitkSurfaceSource.h>
#include "MitkExtExports.h"
#include "mitkTool.h"
#include "mitkSurface.h"

#include <vtkMatrix4x4.h>

#include "itkImage.h"

#include <map>

namespace mitk
{

/**
 * Calculates surfaces for labeled images.
 * If you want to calculate a surface representation only for one
 * specific label, you may call GenerateAllLabelsOff() and set the
 * desired label by SetLabel(label).
 */
class Segmentation_EXPORT LabelSetImageToSurfaceFilter : public SurfaceSource
{
public:

  mitkClassMacro( LabelSetImageToSurfaceFilter, SurfaceSource );

  itkNewMacro( Self );

  typedef mitk::Tool::DefaultSegmentationDataType LabelType;

  typedef std::map<LabelType, unsigned long> LabelMapType;

  typedef std::map<unsigned int, LabelType> IndexToLabelMapType;

  /**
  * Returns a const pointer to the labelset image set as input
  */
  const mitk::Image* GetInput(void);

  /**
  * Set the labelset image to create a surface from.
  */
  virtual void SetInput(const mitk::Image *image);

 //   virtual void SetObserver(mitk::ProcessObserver::Pointer observer);

  /**
   * Set whether you want to extract all labesl (true) or a single one.
   */
  itkSetMacro( GenerateAllLabels, bool );

  /**
   * @returns if all labels or only a specific label should be
   * extracted.
   */
  itkGetMacro( GenerateAllLabels, bool );
  itkBooleanMacro( GenerateAllLabels );

  /**
   * Set the label you want to extract. This method only has an effect,
   * if GenerateAllLabels() is set to false
   * @param _arg the label to extract, by default 1
   */
  itkSetMacro( RequestedLabel, int );

  /**
   * Returns the label you want to extract. This method only has an effect,
   * if GenerateAllLabels() is set to false
   * @returns _arg the label to extract, by default 1
   */
  itkGetMacro( RequestedLabel, int );

  /**
   * Set the label of the background. No surface will be generated for this label!
   * @param _arg the label of the background, by default 0
   */
  itkSetMacro( BackgroundLabel, int );

    /**
   * Set whether to use image smoothing before surface extraction
   */

  itkSetMacro( UseSmoothing, int );

  /**
   * Returns the label of the background. No surface will be generated for this label!
   * @returns the label of the background, by default 0
   */
  itkGetMacro( BackgroundLabel, int );

protected:

  LabelSetImageToSurfaceFilter();

  virtual ~LabelSetImageToSurfaceFilter();

   /**
   * Transforms a point by a 4x4 matrix
   */
   template <class T1, class T2, class T3>
   inline void mitkVtkLinearTransformPoint(T1 matrix[4][4], T2 in[3], T3 out[3])
   {
    T3 x = matrix[0][0]*in[0]+matrix[0][1]*in[1]+matrix[0][2]*in[2]+matrix[0][3];
    T3 y = matrix[1][0]*in[0]+matrix[1][1]*in[1]+matrix[1][2]*in[2]+matrix[1][3];
    T3 z = matrix[2][0]*in[0]+matrix[2][1]*in[1]+matrix[2][2]*in[2]+matrix[2][3];
    out[0] = x;
    out[1] = y;
    out[2] = z;
   }

  template < typename TPixel, unsigned int VImageDimension >
  void ITKProcessing( itk::Image<TPixel, VImageDimension>* input, mitk::Surface* surface );

  template < typename TPixel, unsigned int VImageDimension >
  void ITKProcessing2( itk::Image<TPixel, VImageDimension>* input, mitk::Surface* surface );

  template < typename TPixel, unsigned int VImageDimension >
  void GetAvailableLabelsInternal( itk::Image<TPixel, VImageDimension>* input, LabelMapType& availableLabels );

  bool m_GenerateAllLabels;

  int m_RequestedLabel;

  int m_BackgroundLabel;

  int m_UseSmoothing;

  LabelMapType m_AvailableLabels;

  IndexToLabelMapType m_IndexToLabels;

  mitk::Vector3D m_InputImageSpacing;

  virtual void GenerateData();

  virtual void GenerateOutputInformation();

  int m_LowerThreshold;
  int m_UpperThreshold;

//  mitk::ProcessObserver::Pointer m_Observer;

};

} // end of namespace mitk

#endif //_mitkLabelSetImageToSurfaceFilter_H_
