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
 * If you have a labeled image containing 5 different labeled regions
 * plus the background, this class creates 5 surface representations
 * from the regions. Each surface is assigned to one output of the
 * filter. You can figure out, which label corresponds to which
 * output using the GetLabelForNthOutput() method.
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
    * Returns a const reference to the input image (e.g. the original input image that ist used to create the surface)
    */
    const mitk::Image *GetInput(void);

    /**
    * Set the source image to create a surface for this filter class. As input every mitk
    * 3D or 3D+t image can be used.
    */
    virtual void SetInput(const mitk::Image *image);

 //   virtual void SetObserver(mitk::ProcessObserver::Pointer observer);

  /**
   * Set whether you want to extract all (true) or only
   * a specific label (false)
   * @param _arg true by default
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
  itkSetMacro( RequestedLabel, LabelType );

  /**
   * Returns the label you want to extract. This method only has an effect,
   * if GenerateAllLabels() is set to false
   * @returns _arg the label to extract, by default 1
   */
  itkGetMacro( RequestedLabel, LabelType );

  /**
   * Set the label of the background. No surface will be generated for this label!
   * @param _arg the label of the background, by default 0
   */
  itkSetMacro( BackgroundLabel, LabelType );

  itkSetMacro( UseSmoothing, int );

  /**
   * Returns the label of the background. No surface will be generated for this label!
   * @returns the label of the background, by default 0
   */
  itkGetMacro( BackgroundLabel, LabelType );

  /**
   * Lets you retrieve the label which was used for generating the Nth output of this filter.
   * If GenerateAllLabels() is set to false, this filter only knows about the label provided
   * via SetRequestedLabel(). All other labels in the image are not known.
   * @param i the index of the Nth output.
   * @returns the label used for calculating the Nth output of the filter. If i is out of
   *          range, itk::NumericLimits<LabelType>::max() is returned.
   */
  LabelType GetLabelForNthOutput( const unsigned int& i );

  /**
   * Lets you retrieve the volume in milliliters of the region used to generate the Nth output.
   * @param i the index of the Nth output.
   * @returns the volume of the region used to generate the Nth output of the filter. If
   *          i is out of range, 0.0 is returned.
   */
  mitk::ScalarType GetVolumeForNthOutput( const unsigned int& i );

  /**
   * Lets you retrieve the volume in milliliters of the region with the given label. If
   * GenerateAllLabels is set to false, you may only call this method for the label provided
   * using the SetLabel() method.
   * @param label the label of the region you want to get the volume of
   * @returns the volume of the region with the given label. If
   *          label is invalid, 0.0 is returned.
   */
  mitk::ScalarType GetVolumeForLabel( const LabelType& label );

  bool GetDataIsAvailable();
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
  void GetAvailableLabelsInternal( itk::Image<TPixel, VImageDimension>* input, LabelMapType& availableLabels );

  bool m_GenerateAllLabels;

  LabelType m_RequestedLabel;

  LabelType m_BackgroundLabel;

  int m_UseSmoothing;

  LabelMapType m_AvailableLabels;

  IndexToLabelMapType m_IndexToLabels;

  mitk::Vector3D m_InputImageSpacing;

  virtual void GenerateData();

  virtual void GenerateOutputInformation();

  virtual LabelMapType GetAvailableLabels();

  int m_LowerThreshold;
  int m_UpperThreshold;
  bool m_DataIsAvailable;

//  mitk::ProcessObserver::Pointer m_Observer;

};

} // end of namespace mitk

#endif //_mitkLabelSetImageToSurfaceFilter_H_
