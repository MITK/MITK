/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef SpectralUnmixing_h
#define SpectralUnmixing_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>
#include <mitkPASpectralUnmixingFilterBase.h>
#include <mitkPASpectralUnmixingSO2.h>

#include <chrono>
#include "ui_SpectralUnmixingControls.h"

/**
* \brief The spectral unmixing plugin provides a GUI tool to perform spectral unmixing of multispectral MITK images.
* It was designed to unmix beamformed photoacoustic imgaes. The outputs are as well MITK images for every chosen absorber
* (endmember). Furthermore it is possible to calculate the oxygen saturation of the multispectral input if the endmembers
* oxy- and deoxyhemoglobin are selected in the GUI.
*
* For further information look at the documentation of the mitkPASpectralUnmixingFilterBase.h
*
* @exeption if the GenerateOutput method throws a exception the plugin will show a QMessageBox with the exception
* message at the GUI
*/
class SpectralUnmixing : public QmitkAbstractView
{
   // this is needed for all Qt objects that should have a Qt meta-object
   // (everything that derives from QObject and wants to have signal/slots)
   Q_OBJECT

  public:
    static const std::string VIEW_ID;

  protected:
    void CreateQtPartControl(QWidget *parent) override;
    void SetFocus() override;

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    void OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
                                   const QList<mitk::DataNode::Pointer> &nodes) override;


    /**
    * \brief Called when the user clicks the GUI button. Checks if the selected data is an image. Then passen on the GUI
    * information using the Settings method. Afterwards it performs spectral unmixing via the WorkingThreadUpdateFilter
    * method in a own thread. The spectral unmixing is based on the spectral unmixing filter base and its subclasses.
    * @exception if nothing is selected. Inform the user and return
    * @exception if settings fails. Informs with the mitkthorw information of the filter as QMessageBox
    */
    void DoImageProcessing();

    /**
    * \brief slots are there to show/hide input tables for weights-, relative error and SO2 settings ig they are not needed
    */
  public slots:
    void EnableGUIWeight();
    void EnableGUISO2();
    void EnableGUIError();

    /**
    * \brief slot waits for finishSignal of the working thread and starts storeOutputs
    */
  public slots:
    /**
    * \brief slot does the image post processing
    * - GetOutput from m_SpectralUnmixingFilter
    * - calles WriteOutputToDataStorage
    * - if (true) calls CalculateSO2
    * - does the rendering
    * - if (true) shows the chrono result
    * - switches the GUI back on
    */
    void storeOutputs();
  signals:
    void finishSignal();

  public slots:
  void EnableGUIControl();
  signals:
    void enableSignal();

    /**
    * \brief slot waits for crashSignal and if neccessary ends working thread and shows QMessageBox with the error message
    */
    public slots:
    void crashInfo();
  signals:
    void crashSignal();

  protected:
    Ui::SpectralUnmixingControls m_Controls;

    /**
    * \brief passes the algorithm information from the GUI on to the spectral unmixing filter base subclass method
    * "SetAlgortihm" and initializes the subclassFilter::Pointer.
    * @param algorithm has to be a string which can be assigned to the mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType
    * @throws if the algorithm string doesn't match to an implemented algorithm
    */
    mitk::pa::SpectralUnmixingFilterBase::Pointer GetFilterInstance(std::string algorithm);

    bool PluginVerbose = true;
    mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter;
    std::vector<std::string> outputNameVec;
    std::vector<bool> boolVec;
    std::string Algorithm;
    bool sO2Bool;
    mitk::Image *image;
    std::chrono::steady_clock::time_point _start;
    std::string errorMessage;

  private:
    /**
    * \brief thread
    * - disables GUI
    * - tries Filter->Update() method
    *   - gives finishSignal which calls storeOutputs
    * - cathes by enables GUI and gives crashSignal
    */
    void WorkingThreadUpdateFilter(mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter);

    /**
    * \brief takes an MITK image as input and performs spectral unmixing based on the spectral unmixing filter base and its subclasses.
    * Therefor it first passes all information from the GUI into the filter by using the "set"methods of the plugin, which then are calling
    * the "add" methods of the filter(base).
    * @param image has to be an MITK image (pointer). For the request on the image look at the docu of the mitkPASpectralUnmixngFilterBase.h
    */
    virtual void Settings(mitk::Image::Pointer image);

    /**
    * \brief The method takes a image pointer and a file name which then will get to the data storage.
    * @param m_Image is a mitk_::Image::Pointer pointing at the output which one wants to get stored
    * @param name has to be a string and will be the file name
    */
    virtual void WriteOutputToDataStorage(mitk::Image::Pointer m_Image, std::string name);

    /**
    * \brief passes the algorithm information if verbose mode is requested from the GUI on to the spectral unmixing filter
    * @param m_SpectralUnmixingFilter is a pointer of the spectral unmixing filter base
    * @param PluginVerbose is the GUI information bool
    */
    virtual void SetVerboseMode(mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter, bool PluginVerbose);

    /**
    * \brief passes the wavelength information from the GUI on to the spectral unmixing filter base method "AddWavelength".
    * @param m_SpectralUnmixingFilter is a pointer of the spectral unmixing filter base
    */
    virtual void SetWavlength(mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter);

    /**
    * \brief passes the chromophore information from the GUI on to the spectral unmixing filter base method "AddChromophore".
    * @param m_SpectralUnmixingFilter is a pointer of the spectral unmixing filter base
    * @param boolVec is a vector which contains the information which chromophore was checked in the GUI
    * @param chromophoreNameVec contains the names of all chromophores as strings
    * @throws "PRESS 'IGNORE' AND CHOOSE A CHROMOPHORE!" if no chromophore was chosen
    */
    virtual void SetChromophore(mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter, std::vector<bool> boolVec, std::vector<std::string> chromophoreNameVec);

    /**
    * \brief passes the SetRelativeErrorSettings information from the GUI on to the spectral unmixing filter base method "AddRelativeErrorSettings".
    * @param m_SpectralUnmixingFilter is a pointer of the spectral unmixing filter base#
    */
    virtual void SetRelativeErrorSettings(mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter);

    /**
    * \brief passes the SetSO2Settings information from the GUI on to the spectral unmixing SO2 filter method "AddSO2Settings".
    * @param m_sO2 is a pointer of the spectral unmixing SO2 filter
    */
    virtual void SetSO2Settings(mitk::pa::SpectralUnmixingSO2::Pointer m_sO2);

    /**
    * \brief calcultes out of two identical sized MITK images the oxygen saturation and stores the result in an image. Herein the two
    * input images are the output for oxy- and deoxyhemoglobin from the GenerateOutput method (spectral unmixing filter results).
    * @param m_SpectralUnmixingFilter is a pointer of the spectral unmixing filter base to get the filter output images as sO2 input
    * @param boolVec is a vector which contains the information which chromophore was checked in the GUI
    * @throws if oxy- or deoxyhemoglobin was not selected in the GUI
    */
    virtual void CalculateSO2(mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter, std::vector<bool> boolVec);

    /**
    * \brief enables/disables GUI
    * @param change true means GUI buttons enabled, false disabled respectively
    */
    virtual void SwitchGUIControl(bool change);
};

#endif // SpectralUnmixing_h
