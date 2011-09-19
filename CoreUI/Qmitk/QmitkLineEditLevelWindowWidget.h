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

#ifndef QMITKLINEEDITLEVELWINDOWWIDGET
#define QMITKLINEEDITLEVELWINDOWWIDGET

#include <QmitkExports.h>

#include <QWidget>

#include <mitkLevelWindowManager.h>

class QmitkLevelWindowWidgetContextMenu;
class QLineEdit;
class QDoubleValidator;

/**
  \class QmitkLineEditLevelWindowWidget QmitkLineEditLevelWindowWidget.h QmitkLineEditLevelWindowWidget.h
  \ingroup Widgets

  \brief Provides a widget with two lineedit fields for controling the
  displayed intensity range of the current image.

  The widget has two operation mode. If the intensity range type is set to LevelWindow,
  the two fields set the level and window values, respectively. In WindowBounds mode,
  they set the upper and lower window bound, respectively. The default intensity range
  type is LevelWindow.
  */

class QMITK_EXPORT QmitkLineEditLevelWindowWidget : public QWidget {

  Q_OBJECT

public:

  enum IntensityRangeType {
    LevelWindow,
    WindowBounds
  };

  /// constructor
  QmitkLineEditLevelWindowWidget( QWidget * parent = 0, Qt::WindowFlags f = 0 );

  /// destructor
  ~QmitkLineEditLevelWindowWidget();
  
  /// input field for level value
  QLineEdit* m_LevelInput;

  /// input field for window value
  QLineEdit* m_WindowInput;

  /*!
  *  data structure which stores the values manipulated
  *  by a QmitkLineEditLevelWindowWidget
  */
  mitk::LevelWindow m_LevelWindow;

  char m_Format;
  int m_Precision;

  /// stores if the fields set the level/window or lower/upper window bound values
  IntensityRangeType m_IntensityRangeType;

  /// Validator for the level/lower bound value (depending on the intensity range type)
  QDoubleValidator* m_Validator1;

  /// Validator for the window/upper bound value (depending on the intensity range type)
  QDoubleValidator* m_Validator2;

  /// manager who is responsible to collect and deliver changes on Level/Window
  mitk::LevelWindowManager::Pointer m_Manager;
  
  /// sets the manager who is responsible to collect and deliver changes on Level/Window
  void setLevelWindowManager(mitk::LevelWindowManager* levelWindowManager);

  /// sets the DataStorage which holds all image-nodes
  void SetDataStorage( mitk::DataStorage* ds );

  /// returns the manager who is responsible to collect and deliver changes on Level/Window
  mitk::LevelWindowManager* GetManager();

private:

  /// creates the contextmenu for this widget from class QmitkLevelWindowWidgetContextMenu
  void contextMenuEvent ( QContextMenuEvent * );

  /// change notifications from the mitkLevelWindowManager
  void OnPropertyModified(const itk::EventObject& e);

  /// updates the input fields with the values read from the current LevelWindow
  void updateInputs();

  /*!
  * Tests if level <= maxRange - window/2, if not, level will be set to that limit.
  * Otherwise, tests if level >= minRange + window/2, if not, it will be set to that limit.
  * The window keeps its old value.
  * \return true if the level needed correction, otherwise false
  */
  bool correctLevel(double& level);

  /*!
  * Tests if window <= (maxRange - level) * 2, if not, window will be set to that limit.
  * Otherwise, tests if window >= (level - minRange) * 2, if not, it will be set to that limit.
  * The level keeps its old value.
  * \return true if the window needed correction, otherwise false
  */
  bool correctWindow(double& window);

  /*!
  * Tests if lower bound <= maxRange, if not, it will be set to maxRange.
  * Otherwise, tests if lower bound >= minRange, if not, it will be set to minRange.
  * The upper bound keeps its old value.
  * \return true if the lower bound needed correction, otherwise false
  */
  bool correctLowerBound(double& lowerBound);

  /*!
  * Tests if upper bound <= maxRange, if not, it will be set to maxRange.
  * Otherwise, tests if upper bound >= minRange, if not, it will be set to minRange.
  * The lower bound keeps its old value.
  * \return true if the upper bound needed correction, otherwise false
  */
  bool correctUpperBound(double& upperBound);

private slots:

  /// called when return is pressed in levelinput field
  void SetLevelValue();

  /// called when return is pressed in windowinput field
  void SetWindowValue();
  
public slots:

  // validator to accept only possible values for Level/Window in lineedits
  //void setValidator();

  /// if true, displays values in exponential format
  void SetExponentialFormat(bool value);

  /// displays and accepts values of the specified precision
  void SetPrecision(int precision);

  /// switches between level/window and window bounds operation mode
  void SetIntensityRangeType(IntensityRangeType mode);

protected:
  unsigned long m_ObserverTag;
  bool m_IsObserverTagSet;

  /*!
  *  data structure which creates the contextmenu for QmitkLineEditLevelWindowWidget
  */
  QmitkLevelWindowWidgetContextMenu* m_Contextmenu;

};
#endif // QMITKLINEEDITLEVELWINDOWWIDGET
