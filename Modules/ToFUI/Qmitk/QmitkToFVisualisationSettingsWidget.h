/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-20 13:35:09 +0200 (Mi, 20 Mai 2009) $
Version:   $Revision: 17332 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _QMITKTOFVISUALISATIONSETTINGSWIDGET_H_INCLUDED
#define _QMITKTOFVISUALISATIONSETTINGSWIDGET_H_INCLUDED

#include "mitkTOFUIExports.h"
#include "ui_QmitkToFVisualisationSettingsWidgetControls.h"

// QT headers
#include <QWidget>
// vtk includes
#include <vtkColorTransferFunction.h>

class QmitkStdMultiWidget;

/** Documentation:
  * Widget controlling the visualization of Time-of-Flight image data. A color transfer function can be configured for
  * a given distance, amplitude and intensity image. The pre-configured vtkColorTransferFunctions can be accessed as
  * an output of the widget.
  *
  * \ingroup ToFUI
  */
class mitkTOFUI_EXPORT QmitkToFVisualisationSettingsWidget :public QWidget
{

  //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkToFVisualisationSettingsWidget (QWidget* p = 0, Qt::WindowFlags f1 = 0);
    virtual ~QmitkToFVisualisationSettingsWidget ();

    /* @brief This method is part of the widget an needs not to be called seperately. */
    virtual void CreateQtPartControl(QWidget *parent);
    /* @brief This method is part of the widget an needs not to be called seperately. (Creation of the connections of main and control widget.)*/
    virtual void CreateConnections();
    /*!
    \brief initialize the widget with the images to be shown
    \param distanceImage image holding the range image of a ToF camera
    \param amplitudeImage image holding the amplitude image of a ToF camera
    \param intensityImage image holding the intensity image of a ToF camera
    */
    void Initialize(mitk::Image* distanceImage=NULL, mitk::Image* amplitudeImage=NULL, mitk::Image* intensityImage=NULL);

    /*!
    \brief Access the color transfer function of widget 1 (distance image)
    \return vtkColorTransferFunction that can be used to define a TransferFunctionProperty
    */
    vtkColorTransferFunction* GetWidget1ColorTransferFunction();
    /*!
    \brief Access the color transfer function of widget 2 (distance image)
    \return vtkColorTransferFunction that can be used to define a TransferFunctionProperty
    */
    vtkColorTransferFunction* GetWidget2ColorTransferFunction();
    /*!
    \brief Access the color transfer function of widget 3 (distance image)
    \return vtkColorTransferFunction that can be used to define a TransferFunctionProperty
    */
    vtkColorTransferFunction* GetWidget3ColorTransferFunction();
    /*!
    \brief Access the color transfer of the currently selected widget
    \return vtkColorTransferFunction that can be used to define a TransferFunctionProperty
    */
    vtkColorTransferFunction* GetSelectedColorTransferFunction();
    /*!
    \brief Return the index of the selected image: 0 = Distance, 1 = Amplitude, 2 = Intensity
    */
    int GetSelectedImageIndex();

  protected slots:
  
    void OnSetXValueColor();
    /*!
    \brief Slot invoking a reset of the RangeSlider to the minimal and maximal values of the according image
    */
    void OnResetSlider();  
    /*!
    \brief Slot called when the range span has changed. 
    */
    void OnSpanChanged (int lower, int upper);
    /*!
    \brief Resets the transfer function according to the currently selected widget / image
    */
    void OnTransferFunctionReset();
    /*!
    \brief Updates the GUI according to the widget / image selection
    */
    void OnWidgetSelected(int index);
    /*!
    \brief Slot called when the line edit of the maximal value of the range slider has changed. Leads to an update of the range slider.
    */
    void OnRangeSliderMaxChanged();
    /*!
    \brief Slot called when the line edit of the minimal value of the range slider has changed. Leads to an update of the range slider.
    */
    void OnRangeSliderMinChanged();
    /*!
    \brief Sets the TransferFunctionType members according to the selection of the widget and the transfer type.
    */
    void OnTransferFunctionTypeSelected(int index);

  protected:

    /*!
    \brief Invokes an update of the ColorTransferFunctionCanvas. Called when the ColorTransferFunction has changed
    */
    void UpdateCanvas();
    /*!
    \brief Resets the ColorTransferFunctionCanvas according to the lower and upper value of the RangeSlider
    */
    void UpdateRanges();

    Ui::QmitkToFVisualisationSettingsWidgetControls* m_Controls;

    int m_RangeSliderMin; ///< Minimal value of the transfer function range. Initialized to the minimal value of the corresponding image.
    int m_RangeSliderMax; ///< Maximal value of the transfer function range. Initialized to the maximal value of the corresponding image.

    mitk::Image::Pointer m_MitkDistanceImage; ///< Range image of the ToF camera as set by Initialize()
    mitk::Image::Pointer m_MitkAmplitudeImage; ///< Amplitud image of the ToF camera as set by Initialize()
    mitk::Image::Pointer m_MitkIntensityImage; ///< Intensity image of the ToF camera as set by Initialize()

    vtkColorTransferFunction* m_Widget1ColorTransferFunction; ///< vtkColorTransferFunction of widget 1 (distance) that can be used to define a TransferFunctionProperty
    vtkColorTransferFunction* m_Widget2ColorTransferFunction; ///< vtkColorTransferFunction of widget 2 (amplitude) that can be used to define a TransferFunctionProperty
    vtkColorTransferFunction* m_Widget3ColorTransferFunction; ///< vtkColorTransferFunction of widget 3 (intensity) that can be used to define a TransferFunctionProperty

    int m_Widget1TransferFunctionType; ///< member holding the type of the transfer function applied to the image shown in widget 1 (distance image): 0 = gray scale, 1 = color
    int m_Widget2TransferFunctionType; ///< member holding the type of the transfer function applied to the image shown in widget 2 (amplitude image): 0 = gray scale, 1 = color
    int m_Widget3TransferFunctionType; ///< member holding the type of the transfer function applied to the image shown in widget 3 (intensity image): 0 = gray scale, 1 = color

  private:

    /*!
    \brief Reset the color transfer function to the given type and range
    \param colorTransferFunction vtkColorTransferfunction to be resetted
    \param type type of the transfer function: 0 = gray scale, 1 = color
    \param min minimal value to be set to the transfer function
    \param max maximal value to be set to the transfer function
    */
    void ResetTransferFunction(vtkColorTransferFunction* colorTransferFunction, int type, double min, double max);
};

#endif // _QMITKTOFVISUALISATIONSETTINGSWIDGET_H_INCLUDED
