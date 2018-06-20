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

#ifndef SpectralUnmixing_h
#define SpectralUnmixing_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>
#include <mitkPASpectralUnmixingFilterBase.h>
#include <mitkPASpectralUnmixingSO2.h>

#include "ui_SpectralUnmixingControls.h"

/**
* \brief The spectral unmixing plugin provides a GUI tool to perform spectral unmixing of multispectral MITK images.
* It was designed to unmix beamformed photoacoustic imgaes. The outputs are as well MITK images for every chosen absorber
* (endmember). Furthermore it is possible to calculate the oxygen saturation of the multispectral input if the endmembers
* oxy- and deoxyhemoglobin are selected in the GUI.
*
* For further information look at the documentation of the mitkPASpectralUnmixingFilterBase.h
*/
class SpectralUnmixing : public QmitkAbstractView
{
   // this is needed for all Qt objects that should have a Qt meta-object
   // (everything that derives from QObject and wants to have signal/slots)
   Q_OBJECT

  public:
    static const std::string VIEW_ID;
   
  protected:
    virtual void CreateQtPartControl(QWidget *parent) override;
    virtual void SetFocus() override;
  
    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
                                   const QList<mitk::DataNode::Pointer> &nodes) override;

    /**
    * \brief Called when the user clicks the GUI button. Checks if the selected data is an image. Then performs spectral
    * unmixing via the GenerateOutput method based on the spectral unmixing filter base and its subclasses.
    * @exception if nothing is selected. Inform the user and return
    */
    void DoImageProcessing();
  
    Ui::SpectralUnmixingControls m_Controls;

    /**
    * \brief passes the algorithm information from the GUI on to the spectral unmixing filter base subclass method
    * "SetAlgortihm" and initializes the subclassFilter::Pointer.
    * @param algorithm has to be a string which can be assigned to the mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType
    * @thorw if the algorithm string doesn't match to an implemented algorithm
    */
    mitk::pa::SpectralUnmixingFilterBase::Pointer GetFilterInstance(std::string algorithm);

    bool PluignVerbose = true;

  private:

    /*
    * \brief passes the wavelength information from the GUI on to the spectral unmixing filter base method "AddWavelength".
    * @param m_SpectralUnmixingFilter is a pointer of the spectral unmixing filter base
    */
    virtual void SetWavlength(mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter);

    /*
    * \brief passes the chromophore information from the GUI on to the spectral unmixing filter base method "AddChromophore".
    * @param m_SpectralUnmixingFilter is a pointer of the spectral unmixing filter base
    * @param boolVec is a vector which contains the information which chromophore was checked in the GUI
    * @param chromophoreNameVec contains the names of all chromophores as strings
    * @throw "PRESS 'IGNORE' AND CHOOSE A CHROMOPHORE!" if no chromophore was chosen
    */
    virtual void SetChromophore(mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter, std::vector<bool> boolVec, std::vector<std::string> chromophoreNameVec);

    /*
    * \brief The method takes a image pointer and a file name which then will get to the data storage.
    * @param m_Image is a mitk_::Image::Pointer pointing at the output which one wants to get stored
    * @param name has to be a string and will be the file name
    */
    virtual void WriteOutputToDataStorage(mitk::Image::Pointer m_Image, std::string name);

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
    * @throw if oxy- or deoxyhemoglobin was not selected in the GUI
    */
    virtual void CalculateSO2(mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter, std::vector<bool> boolVec);

    /**
    * \brief takes an MITK image as input and performs spectral unmixing based on the spectral unmixing filter base and its subclasses.
    * @param image has to be an MITK image (pointer). For the request on the image look at the docu of the mitkPASpectralUnmixngFilterBase.h
    */
    virtual void GenerateOutput(mitk::Image::Pointer image);
};

#endif // SpectralUnmixing_h
