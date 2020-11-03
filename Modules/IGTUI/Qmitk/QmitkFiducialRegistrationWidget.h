/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _QmitkFiducialRegistrationWidget_H_INCLUDED
#define _QmitkFiducialRegistrationWidget_H_INCLUDED

#include "ui_QmitkFiducialRegistrationWidget.h"
#include "QmitkStdMultiWidget.h"
#include "MitkIGTUIExports.h"
#include "mitkNavigationData.h"
/*!
 * \brief IGT Fiducial Registration Widget
 *
 * Widget used to set fiducial landmarks in the image and to confirm the corresponding landmarks on the world object (patient/phantom).
 *
 * The widget can add tracker fiducials and perform the registration internally. To enable this functionaltity the
 * methods SetDataStorage(), setTrackerNavigationData() and setImageNode() needs to be called before.
 *
 * If Registration should be handled from outside the class the methods SetImageFiducialsNode() and SetTrackerFiducialsNode()
 * must be called, otherwise the QmitkPointListWidget can not work. If SetDataStorage() is not called the widget does nothing
 * internally.
 *
 * \sa IGT
*/
class MITKIGTUI_EXPORT QmitkFiducialRegistrationWidget : public QWidget
{
  Q_OBJECT // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
public:

  QmitkFiducialRegistrationWidget(QWidget* parent);
  ~QmitkFiducialRegistrationWidget() override;

  /** Adds the image node which is transformed after "register" is clicked. */
  void setImageNode(mitk::DataNode::Pointer i);

  /** Adds the tracker navigation data which is used when "add current instrument position" is clicked. */
  void setTrackerNavigationData(mitk::NavigationData::Pointer t);

  /** Sets the data storage. This is required is the widget shoul add tracker points and perform
   *  registrations internally. When not setting the data storage the widget can still be used
   *  by reacting on the signals and do custom actions outside.*/
  void setDataStorage(mitk::DataStorage::Pointer d);

  /*!
  \brief enumeration to specify the appearance of the widget.
  'FIDUCIALMODE' is likely to be used for (tracking) fiducial based registration purposes
  'LANDMARKMODE' can be used for any kind of landmark based registration (source landmarks -> target/reference landmarks)
  */
  enum WidgetAppearanceMode
  {
    FIDUCIALMODE,
    LANDMARKMODE
  };

  /*!
  \brief set the appearance mode of this widget
  'FIDUCIALMODE' adapts the widget for (tracking) fiducial based registration purposes
  'LANDMARKMODE' adapts the widget for landmark based registration (source landmarks -> target/reference landmarks)
  */
  void SetWidgetAppearanceMode(WidgetAppearanceMode widgetMode);

  void SetMultiWidget(QmitkStdMultiWidget* multiWidget); ///< (Deprecated method. Multiwidget is not required any more.) Set the default stdMultiWidget (needed for the PointListwidget)
  void AddSliceNavigationController(mitk::SliceNavigationController* snc); ///< add the slice navigation controller to be used to move the crosshair to the actual point position
  void SetImageFiducialsNode(mitk::DataNode::Pointer imageFiducialsNode); ///< specify data tree node for the image fiducials
  void SetTrackerFiducialsNode(mitk::DataNode::Pointer trackerFiducialsNode); ///< specify data tree node for the tracker fiducials
  mitk::DataNode::Pointer GetImageFiducialsNode(); ///< returns data tree node for the image fiducials
  mitk::DataNode::Pointer GetTrackerFiducialsNode(); ///< returns data tree node for the tracker fiducials
  void SetQualityDisplayText(QString text); ///< sets specific text on the UI (useful to display FRE/TRE...)

  /*!
  \brief Specify the name of the source landmarks. Will be used for label and button.

  Example: sourceLandmarkName="CT" will result in group box title "CT landmarks" and button text "Add CT landmark".
  */
  void SetSourceLandmarkName(QString sourceLandmarkName);

  /*!
  \brief Specify the name of the source landmarks. Will be used for label and button.

  Example: targetLandmarkName="CT" will result in group box title "CT landmarks" and button text "Add CT landmark".
  */
  void SetTargetLandmarkName(QString targetLandmarkName);

  bool UseICPIsChecked(); ///< returns true if automatic correspondences search is activated else false
  void HideStaticRegistrationRadioButton(bool on); ///< show or hide  "static Fiducial Registration" radio button in the UI
  void HideContinousRegistrationRadioButton(bool on); ///< show or hide  "hybrid continuous Fiducial Registration" radio button in the UI
  void HideFiducialRegistrationGroupBox(); ///< show or hide  "Fiducial Registration method" groupbox in the UI, depending on the visibility of the radio buttons
  void HideUseICPRegistrationCheckbox(bool on); ///< show or hide  "Find fiducial correspondences (needs 6+ fiducial pairs)" check box in the UI
  void HideImageFiducialButton(bool on); ///< show or hide  "Add image fiducial" button in the UI
  void HideTrackingFiducialButton(bool on); ///< show or hide  "Add tracking fiducial" button in the UI
  void AdjustButtonSpacing(); ///< Rearrange spacing when buttons are turned on or off

  signals:
    void AddedTrackingFiducial();      ///< signal if a world instrument position was added to a tracking space fiducial
    void AddedImageFiducial();      ///< signal if an image position was added to a image space fiducial
    void PerformFiducialRegistration();        ///< signal if all fiducial were added and registration can be performed
    void FindFiducialCorrespondences(bool on); ///< signal if automatic correspondences search is toggled

  protected slots:
    void DisableEditButtonRegistrationImagePoints(bool);///< Disables the edit button of the widget RegistrationImagePoints if the activated variable is true.
    void DisableEditButtonRegistrationTrackingPoints(bool);///< Disables the edit button of the widget RegistrationTrackingPoints if the activated variable is true.
    void AddTrackerPoint();
    void Register();

protected:

  void CreateQtPartControl(QWidget *parent);

  void CreateConnections();

  bool CheckRegistrationInitialization();

  Ui::QmitkFiducialRegistrationWidget* m_Controls;  ///< gui widget
  QmitkStdMultiWidget* m_MultiWidget;
  mitk::DataNode::Pointer m_ImageFiducialsNode;
  mitk::DataNode::Pointer m_TrackerFiducialsNode;
  mitk::DataStorage::Pointer m_DataStorage;
  mitk::NavigationData::Pointer m_TrackerNavigationData;
  mitk::DataNode::Pointer m_ImageNode;
  mitk::NavigationData::Pointer m_T_ObjectReg;

};
#endif // _QmitkFiducialRegistrationWidget_H_INCLUDED
