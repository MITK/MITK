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

#ifndef _QMITKTOFCOMPOSITEFILTERWIDGET_H_INCLUDED
#define _QMITKTOFCOMPOSITEFILTERWIDGET_H_INCLUDED

#include <MitkToFUIExports.h>
#include "ui_QmitkToFCompositeFilterWidgetControls.h"

//mitk headers
#include <mitkToFCompositeFilter.h>

/**
* @brief Widget for controlling the ToFCompositeFilter (located in module ToFProcessing)
*
* The widget allows to enable/disable the filters internally used in the ToFCompositeFilter
* and to set the individual filter parameters using GUI elements
*
* @ingroup ToFUI
*/
class MITKTOFUI_EXPORT QmitkToFCompositeFilterWidget :public QWidget
{
  //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkToFCompositeFilterWidget(QWidget* p = 0, Qt::WindowFlags f1 = 0);
    virtual ~QmitkToFCompositeFilterWidget();

    /* @brief This method is part of the widget an needs not to be called seperately. */
    virtual void CreateQtPartControl(QWidget *parent);
    /* @brief This method is part of the widget an needs not to be called seperately. (Creation of the connections of main and control widget.)*/
    virtual void CreateConnections();

    /*!
    \brief Sets the ToFCompositeFilter used by this widget
    \param tofCompositeFilter pointer to the internally used ToFCompositeFilter
    */
    void SetToFCompositeFilter(mitk::ToFCompositeFilter* toFCompositeFilter);
    /*!
    \brief Returns the ToFCompositeFilter used by this widget
    \return tofCompositeFilter pointer to the internally used ToFCompositeFilter
    */
    mitk::ToFCompositeFilter* GetToFCompositeFilter();
    /*!
    \brief update parameters of ToFCompositeFilter according to current GUI setting
    */
    void UpdateFilterParameter();
    /*!
    \brief set the configuration of the widget specifying which filter is enabled on start
    */
    void SetWidgetConfiguration(bool threshold, bool mask, bool tempMedian, bool tempAverage, bool median, bool bilateral );
    /*!
    \brief sets the standard parameters used for the bilateral filter to the ComboBoxes used in the GUI
    \param domainSigma Parameter controlling the smoothing effect of the bilateral filter. Default value: 2
    \param rangeSigma Parameter controlling the edge preserving effect of the bilateral filter. Default value: 60
    \param kernelRadius radius of the filter mask of the bilateral filter
    */
    void SetStandardParametersBilateralFilter(double domainSigma, double rangeSigma, int kernelRadius=0);
    /*!
    \brief set the standard parameters for the threshold filter to the combo boxes in the GUI
    \param min lower threshold of the threshold filter
    \param max upper threshold of the threshold filter
    */
    void SetStandardParametersThresholdFilter(int min, int max);
    /*!
    \brief set the standard value for the number of images to be averaged to the combo box in the GUI
    */
    void SetStandardParameterTemporalAveraging(int nImages);

    void SetDataStorage(mitk::DataStorage::Pointer dataStorage);

signals:

  protected slots:
    /**
     * @brief OnShowAdvancedOptionsCheckboxChecked Show/hide advanced options.
     * @param checked show/hide
     */
    void OnShowAdvancedOptionsCheckboxChecked(bool checked);
    /*!
    \brief slot en-/disabling temporal median filter in internal ToFCompositeFilter
    */
    void OnTemporalMedianFilterCheckBoxChecked(bool checked);
    /*!
    \brief slot en-/disabling average filter in internal ToFCompositeFilter
    */
    void OnAverageFilterCheckBoxChecked(bool checked);
    /*!
    \brief slot en-/disabling threshold filter in internal ToFCompositeFilter
    */
    void OnThresholdFilterCheckBoxChecked(bool checked);
    /*!
    \brief slot en-/disabling the mask segmentation in internal ToFCompositeFilter
    */
    void OnMaskSegmentationCheckBoxChecked(bool checked);
    /*!
    \brief slot en-/disabling median filter in internal ToFCompositeFilter
    */
    void OnMedianFilterCheckBoxChecked(bool checked);
    /*!
    \brief slot en-/disabling bilateral filter in internal ToFCompositeFilter
    */
    void OnBilateralFilterCheckBoxChecked(bool checked);

    /*!
    \brief slot updating threshold spin boxes according to slider position
    */
    void OnSpanChanged(int lower, int upper);
    /*!
    \brief slot resetting threshold range slider to default values (min: 1, max: 7000)
    */
    void OnResetThresholdFilterRangeSlider();
    /*!
    \brief slot updating the parameter "number of frames" of the temporal median filter in the ToFCompositeFilter
    */
    void OnTemporalMedianFilterNumOfFramesSpinBoxValueChanged(int value);
    /*!
    \brief slot updating the parameter "domain sigma" of the bilateral filter in the ToFCompositeFilter
    */
    void OnBilateralFilterDomainSigmaSpinBoxValueChanged(double value);
    /*!
    \brief slot updating the paramter "range sigma" of the bilateral filter in the ToFCompositeFilter
    */
    void OnBilateralFilterRangeSigmaSpinBoxValueChanged(double value);
    /*!
    \brief slot updating the paramter "kernel radius" of the bilateral filter in the ToFCompositeFilter
    */
    void OnBilateralFilterKernelRadiusSpinBoxValueChanged(int value);
    /*!
    \brief slot updating the paramter "minimal threshold" of the threshold filter in the ToFCompositeFilter
    */
    void OnThresholdFilterMinValueChanged(int value);
    /*!
    \brief slot updating the paramter "maximal threshold" of the threshold filter in the ToFCompositeFilter
    */
    void OnThresholdFilterMaxValueChanged(int value);

  protected:

    Ui::QmitkToFCompositeFilterWidgetControls* m_Controls; ///< member holding the UI elements of this widget

    mitk::ToFCompositeFilter::Pointer m_ToFCompositeFilter; ///< member holding the internally used ToFCompositeFilter

  private:

    /*!
    \brief method updating the parameters of the bilateral filter in the ToFCompositeFilter
    */
    void SetBilateralFilterParameter();
    /*!
    \brief method updating the parameters of the threshold filter in the ToFCompositeFilter
    */
    void SetThresholdFilterParameter();

    mitk::DataStorage::Pointer m_DataStorage;
};

#endif // _QMITKTOFCOMPOSITEFILTERWIDGET_H_INCLUDED
