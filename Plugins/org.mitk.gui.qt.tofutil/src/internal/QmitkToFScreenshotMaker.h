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

#ifndef QmitkToFScreenshotMaker_h
#define QmitkToFScreenshotMaker_h


#include <ui_QmitkToFScreenshotMakerControls.h>
#include <QmitkAbstractView.h>

#include <QStringList>

#include <ui_QmitkToFUtilViewControls.h>


/*!
  \brief QmitkToFScreenshotMaker

  \sa QmitkFunctionality
  \ingroup Functionalities
*/
class QmitkToFScreenshotMaker : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkToFScreenshotMaker();
    ~QmitkToFScreenshotMaker();

    void SetFocus();

    virtual void CreateQtPartControl(QWidget *parent);


protected slots:

    /*!
    \brief Slot called when the "Make screenshot" button is pressed.
    */
    void OnMakeScreenshotClicked();

    void OnSelectCamera();

  protected:

    Ui::QmitkToFScreenshotMakerControls m_Controls;

  private:

    /**
     * @brief UpdateFileExtensionComboBox
     * @param active
     * @param box
     * @param fileExentions
     * @param preferredFormat
     */
    void UpdateFileExtensionComboBox(bool active, QComboBox* box, QStringList fileExentions, const char *preferredFormat);

    /**
     * @brief SaveImage
     * @param image
     * @param saveImage
     * @param path
     * @param name
     * @param extension
     */
    void SaveImage(mitk::Image::Pointer image, bool saveImage, std::string path, std::string name, std::string extension);

    int m_SavingCounter;
    mitk::ToFImageGrabber::Pointer m_ToFImageGrabber;
};

#endif // _QmitkToFScreenshotMaker_H_INCLUDED
