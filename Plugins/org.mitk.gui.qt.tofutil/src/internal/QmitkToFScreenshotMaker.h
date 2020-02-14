/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkToFScreenshotMaker_h
#define QmitkToFScreenshotMaker_h


#include <ui_QmitkToFScreenshotMakerControls.h>
#include <QmitkAbstractView.h>
#include <QStringList>
#include <ui_QmitkToFUtilViewControls.h>


/*!
  \brief QmitkToFScreenshotMaker Select a ToF image source in the GUI to make a screenshot of the provided data.
  If a camera is active, the Make Screenshot button will become enabled. Select the data including format you
  want to save at the given path. To activate a camera, you can for example use the ToF Util view. Note you can
  only select data which is provided by the device. Screenshots will be saved at the respective path with a
  counter indicating the order.

  \ingroup ToFUtil
*/
class QmitkToFScreenshotMaker : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkToFScreenshotMaker();
  ~QmitkToFScreenshotMaker() override;

  void SetFocus() override;

  void CreateQtPartControl(QWidget *parent) override;


protected slots:

  /**
    * @brief OnMakeScreenshotClicked Slot called when the "Make screenshot" button is pressed.
    */
  void OnMakeScreenshotClicked();

  /**
    * @brief OnSelectCamera Slot called to update the GUI according to the selected image source.
    */
  void OnSelectCamera();

protected:

  Ui::QmitkToFScreenshotMakerControls m_Controls;

private:

  /**
     * @brief UpdateGUIElements Internal helper method to update the GUI.
     * @param device The device of the selected image source.
     * @param ToFImageType Type of the image (e.g. depth, RGB, intensity, etc.)
     * @param saveCheckBox Checkbox indicating whether the type should be saved.
     * @param saveTypeComboBox Combobox to chose in which format the data should be saved (e.g. nrrd)
     * @param fileExentions Other possible file extensions.
     * @param preferredFormat Default format for this type (e.g. png for RGB).
     */
  void UpdateGUIElements(mitk::ToFCameraDevice* device, const char *ToFImageType, QCheckBox *saveCheckBox,
                         QComboBox *saveTypeComboBox, QStringList fileExentions, const char *preferredFormat);

  /**
     * @brief SaveImage Saves a ToF image.
     * @param image The image to save.
     * @param saveImage Should it be saved?
     * @param path Path where to save the image.
     * @param name Name of the image.
     * @param extension Type extension (e.g. .nrrd).
     */
  void SaveImage(mitk::Image::Pointer image, bool saveImage, std::string path, std::string name, std::string extension);

  /**
   * @brief m_SavingCounter Internal counter for saving images with higher number.
   */
  int m_SavingCounter;
};

#endif // QmitkToFScreenshotMaker_h
