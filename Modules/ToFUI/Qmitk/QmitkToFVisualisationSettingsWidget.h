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

//QT headers
#include <QWidget>
#include <QTimer>
#include <QString>

//std headers

//itk headers

//mitk headers
#include "mitkToFImageGrabber.h"
#include <mitkDataNode.h>
#include <mitkTransferFunctionProperty.h>

//Qmitk headers
#include "QmitkRenderWindow.h"
#include "mitkToFVisualizationFilter.h"

#include <vtkColorTransferFunction.h>

class QmitkStdMultiWidget;

/** Documentation:
  *   \ingroup MBIIGTUI
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
    /* @brief This method is part of the widget an needs not to be called seperately. */
    virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
    /* @brief This method is part of the widget an needs not to be called seperately. */
    virtual void StdMultiWidgetNotAvailable();

    /* @brief initialize the Widget */
    void Initialize(mitk::DataStorage* dataStorage, QmitkStdMultiWidget* multiWidget);

    /* @brief set the required parameters
     *
     */
    void SetParameter(mitk::ToFVisualizationFilter* ToFVisualizationFilter);

    void InitializeTransferFunction(mitk::Image* distanceImage, mitk::Image* amplitudeImage, mitk::Image* intensityImage);

    void InitMember();

    void Reset();

    void SetDataNode(mitk::DataNode* node);

    mitk::ToFVisualizationFilter* GetToFVisualizationFilter();

    mitk::TransferFunction::Pointer GetTransferFunction();

    vtkColorTransferFunction* GetWidget1ColorTransferFunction();

    void SetWidget1ColorTransferFunction(vtkColorTransferFunction* colorTransferFunction);

    vtkColorTransferFunction* GetWidget2ColorTransferFunction();

    void SetWidget2ColorTransferFunction(vtkColorTransferFunction* colorTransferFunction);

    vtkColorTransferFunction* GetWidget3ColorTransferFunction();

    void SetWidget3ColorTransferFunction(vtkColorTransferFunction* colorTransferFunction);

    //vtkColorTransferFunction* GetWidget4ColorTransferFunction();

    //void SetWidget4ColorTransferFunction(vtkColorTransferFunction* colorTransferFunction);

    vtkColorTransferFunction* GetColorTransferFunctionByImageType(char* imageType);

    std::string GetWidget1ImageType();

    std::string GetWidget2ImageType();

    std::string GetWidget3ImageType();

    //QString GetWidget4ImageType();

  protected slots:
  
    /* @brief slot performing region growing and setting result for registration method  
     *
     */  
    void SetXValueColor();
    void OnUpdateCanvas();
    void UpdateRanges();
    void OnResetSlider();  
    void OnSpanChanged (int lower, int upper);
    void OnTransferFunctionReset();
    void OnWidgetSelected(int index);
    void OnImageTypeSelected(int index);
    void OnRangeSliderMaxChanged();
    void OnRangeSliderMinChanged();
    void OnTransferFunctionTypeSelected(int index);

  protected:

    Ui::QmitkToFVisualisationSettingsWidgetControls* m_Controls;
    QmitkStdMultiWidget* m_MultiWidget;
    mitk::DataStorage* m_DataStorage;

    mitk::ToFImageGrabber* m_ToFImageGrabber;

    int m_Channel;
    int m_Type;
    QString m_NewImageName;
    mitk::DataNode::Pointer m_CurrentImageNode;

    mitk::TransferFunctionProperty::Pointer m_TfpToChange;
    int m_RangeSliderMin;
    int m_RangeSliderMax;
    mitk::SimpleHistogramCache histogramCache;

    //vtkColorTransferFunction* m_Widget1ColorTransferFunction;
    //vtkColorTransferFunction* m_Widget2ColorTransferFunction;
    //vtkColorTransferFunction* m_Widget3ColorTransferFunction;
    //vtkColorTransferFunction* m_Widget4ColorTransferFunction;

    //QString m_Widget1ImageType;
    //QString m_Widget2ImageType;
    //QString m_Widget3ImageType;
    //QString m_Widget4ImageType;

    mitk::Image::Pointer m_MitkDistanceImage;
    mitk::Image::Pointer m_MitkAmplitudeImage;
    mitk::Image::Pointer m_MitkIntensityImage;

    //int m_Widget1TransferFunctionType;
    //int m_Widget2TransferFunctionType;
    //int m_Widget3TransferFunctionType;

    mitk::ToFVisualizationFilter::Pointer m_ToFVisualizationFilter;

  private:

    std::string GetImageType(int index);
    int GetImageTypeIndex(std::string imageType);
    void ResetTransferFunction(vtkColorTransferFunction* colorTransferFunction, int type, double min, double max);
    void ComputeMinMax(float* data, int num, int& min, int& max);

};

#endif // _QMITKNEEDLESHAPEREGISTRATIONWIDGET_H_INCLUDED