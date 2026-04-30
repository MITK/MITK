/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLabeledImageToSurfaceFilter_h
#define mitkLabeledImageToSurfaceFilter_h

#include <MitkAlgorithmsExtExports.h>
#include <map>
#include <mitkImageToSurfaceFilter.h>
#include <vtkImageData.h>

namespace mitk
{
  /**
   * \brief Generates surface representations from labeled (segmented) images.
   *
   * For a labeled image containing N distinct label regions (plus background),
   * this filter creates N surface representations, one per labeled region.
   * Each surface is assigned to a separate output of the filter. The mapping
   * between output index and label can be queried via GetLabelForNthOutput().
   *
   * Alternatively, surface generation can be restricted to a single specific
   * label by calling GenerateAllLabelsOff() and then SetLabel(label).
   *
   * The filter also computes the volume (in mm^3) of each labeled region,
   * accessible via GetVolumeForNthOutput() or GetVolumeForLabel().
   *
   * \sa ImageToSurfaceFilter
   * \ingroup Process
   */
  class MITKALGORITHMSEXT_EXPORT LabeledImageToSurfaceFilter : public ImageToSurfaceFilter
  {
  public:
    mitkClassMacro(LabeledImageToSurfaceFilter, ImageToSurfaceFilter);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      typedef int LabelType;

    typedef std::map<LabelType, unsigned long> LabelMapType;

    typedef std::map<unsigned int, LabelType> IdxToLabelMapType;

    /**
     * Set whether you want to extract all (true) or only
     * a specific label (false)
     * \param _arg true by default
     */
    itkSetMacro(GenerateAllLabels, bool);

    /**
     * \return if all labels or only a specific label should be
     * extracted.
     */
    itkGetMacro(GenerateAllLabels, bool);

    itkBooleanMacro(GenerateAllLabels);

    /**
     * Set the label you want to extract. This method only has an effect,
     * if GenerateAllLabels() is set to false
     * \param _arg the label to extract, by default 1
     */
    itkSetMacro(Label, LabelType);

    /**
     * Returns the label you want to extract. This method only has an effect,
     * if GenerateAllLabels() is set to false
     * \return _arg the label to extract, by default 1
     */
    itkGetMacro(Label, LabelType);

    /**
     * Set the label of the background. No surface will be generated for this label!
     * \param _arg the label of the background, by default 0
     */
    itkSetMacro(BackgroundLabel, LabelType);

    /**
     * Returns the label of the background. No surface will be generated for this label!
     * \return the label of the background, by default 0
     */
    itkGetMacro(BackgroundLabel, LabelType);

    /**
     * Set standard deviation for Gaussian Filter.
     * \param _arg by default 1.5
     */
    itkSetMacro(GaussianStandardDeviation, double);

    /**
     * Returns the standard deviation of the Gaussian filter which will be
     * used when filter is enabled.
     */
    itkGetMacro(GaussianStandardDeviation, double);

    /**
     * Lets you retrieve the label which was used for generating the Nth output of this filter.
     * If GenerateAllLabels() is set to false, this filter only knows about the label provided
     * via SetLabel(). All other labels in the image are not known.
     * \param i the index of the Nth output.
     * \return the label used for calculating the Nth output of the filter. If i is out of
     *          range, itk::NumericLimits<LabelType>::max() is returned.
     */
    LabelType GetLabelForNthOutput(const unsigned int &i);

    /**
     * Lets you retrieve the volume in milliliters of the region used to generate the Nth output.
     * \param i the index of the Nth output.
     * \return the volume of the region used to generate the Nth output of the filter. If
     *          i is out of range, 0.0 is returned.
     */
    mitk::ScalarType GetVolumeForNthOutput(const unsigned int &i);

    /**
     * Lets you retrieve the volume in milliliters of the region with the given label. If
     * GenerateAllLabels is set to false, you may only call this method for the label provided
     * using the SetLabel() method.
     * \param label the label of the region you want to get the volume of
     * \return the volume of the region with the given label. If
     *          label is invalid, 0.0 is returned.
     */
    mitk::ScalarType GetVolumeForLabel(const LabelType &label);

  protected:
    double m_GaussianStandardDeviation;

    bool m_GenerateAllLabels;

    LabelType m_Label;

    LabelType m_BackgroundLabel;

    LabelMapType m_AvailableLabels;

    IdxToLabelMapType m_IdxToLabels;

    void GenerateData() override;

    void GenerateOutputInformation() override;

    virtual void CreateSurface(int time, vtkImageData *vtkimage, mitk::Surface *surface, LabelType label);

    virtual LabelMapType GetAvailableLabels();

    LabeledImageToSurfaceFilter();

    ~LabeledImageToSurfaceFilter() override;

  private:
    virtual void CreateSurface(int time, vtkImageData *vtkimage, mitk::Surface *surface, const ScalarType threshold);
  };

} // end of namespace mitk

#endif
