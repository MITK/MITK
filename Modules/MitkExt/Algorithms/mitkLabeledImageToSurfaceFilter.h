/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef _mitk_labeled_image_to_surface_filter__h_
#define _mitk_labeled_image_to_surface_filter__h_

#include <mitkImageToSurfaceFilter.h>
#include <vtkImageData.h>
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
class MITKEXT_CORE_EXPORT LabeledImageToSurfaceFilter : public ImageToSurfaceFilter
{
public:
    
  mitkClassMacro( LabeledImageToSurfaceFilter, ImageToSurfaceFilter );

  itkNewMacro( Self );
  
  typedef int LabelType;
  
  typedef std::map<LabelType, unsigned long> LabelMapType;
  
  typedef std::map<unsigned int, LabelType> IdxToLabelMapType;
  
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
  itkSetMacro( Label, LabelType );
  
  /**
   * Returns the label you want to extract. This method only has an effect,
   * if GenerateAllLabels() is set to false
   * @returns _arg the label to extract, by default 1
   */
  itkGetMacro( Label, LabelType );
  
  /**
   * Set the label of the background. No surface will be generated for this label!
   * @param _arg the label of the background, by default 0
   */
  itkSetMacro( BackgroundLabel, LabelType );
  
  /**
   * Returns the label of the background. No surface will be generated for this label!
   * @returns the label of the background, by default 0
   */
  itkGetMacro( BackgroundLabel, LabelType );
  
  /**
   * Set standard deviation for Gaussian Filter.
   * @param _arg by default 1.5
   */
  itkSetMacro( GaussianStandardDeviation, double );
      
  /**
   * Returns the standard deviation of the Gaussian filter which will be
   * used when filter is enabled.
   */
  itkGetMacro( GaussianStandardDeviation, double );
  
  /**
   * Lets you retrieve the label which was used for generating the Nth output of this filter.
   * If GenerateAllLabels() is set to false, this filter only knows about the label provided
   * via SetLabel(). All other labels in the image are not known. 
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

protected:

  double m_GaussianStandardDeviation;

  bool m_GenerateAllLabels;

  LabelType m_Label;
  
  LabelType m_BackgroundLabel;
  
  LabelMapType m_AvailableLabels;
  
  IdxToLabelMapType m_IdxToLabels;
  
  virtual void GenerateData();
  
  virtual void GenerateOutputInformation();
        
  virtual void CreateSurface( int time, vtkImageData *vtkimage, mitk::Surface * surface, LabelType label );
  
  virtual LabelMapType GetAvailableLabels();
  
  LabeledImageToSurfaceFilter();
  
  virtual ~LabeledImageToSurfaceFilter();
  
private:
    
  itkSetMacro(Threshold, ScalarType);

  itkGetConstMacro(Threshold, ScalarType);
  
  virtual void CreateSurface(int time, vtkImageData *vtkimage, mitk::Surface * surface, const ScalarType threshold);
};

} // end of namespace mitk

#endif



