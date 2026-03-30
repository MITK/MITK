/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkLineEditLevelWindowWidget_h
#define QmitkLineEditLevelWindowWidget_h

#include <MitkQtWidgetsExports.h>

// mitk core
#include <mitkLevelWindowManager.h>

// qt
#include <QWidget>

class QmitkLevelWindowWidgetContextMenu;
class QLineEdit;

/**
 * \ingroup QmitkModule
 * \brief Widget with two text input fields for direct numeric entry of level and window values.
 *
 * This widget provides a QLineEdit for the level (center) value and another for
 * the window (width) value of the current image's level/window. The user can type
 * values and press Enter to apply them. A context menu is available for additional
 * level/window operations.
 *
 * \sa QmitkSliderLevelWindowWidget
 * \sa QmitkLevelWindowWidget
 * \sa mitk::LevelWindowManager
 */
class MITKQTWIDGETS_EXPORT QmitkLineEditLevelWindowWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Constructs the line edit level/window widget.
   * \param[in] parent Optional parent widget.
   * \param[in] f      Optional window flags.
   */
  QmitkLineEditLevelWindowWidget(QWidget *parent = nullptr, Qt::WindowFlags f = {});

  ~QmitkLineEditLevelWindowWidget() override;

  /** \brief Input field for the level (center) value. */
  QLineEdit *m_LevelInput;

  /** \brief Input field for the window (width) value. */
  QLineEdit *m_WindowInput;

  /** \brief Stores the current level/window values manipulated by this widget. */
  mitk::LevelWindow m_LevelWindow;

  /** \brief The LevelWindowManager responsible for collecting and delivering level/window changes. */
  mitk::LevelWindowManager::Pointer m_Manager;

  /**
   * \brief Sets the LevelWindowManager for this widget.
   * \param[in] levelWindowManager The manager to use for level/window changes.
   */
  void SetLevelWindowManager(mitk::LevelWindowManager *levelWindowManager);

  /**
   * \brief Sets the DataStorage that holds all image nodes.
   * \param[in] ds The data storage to set.
   */
  void SetDataStorage(mitk::DataStorage *ds);

  /**
   * \brief Returns the LevelWindowManager used by this widget.
   * \return Pointer to the LevelWindowManager.
   */
  mitk::LevelWindowManager *GetManager();

private:
  /// creates the contextmenu for this widget from class QmitkLevelWindowWidgetContextMenu
  void contextMenuEvent(QContextMenuEvent *) override;

  /// change notifications from the mitkLevelWindowManager
  void OnPropertyModified(const itk::EventObject &e);

public Q_SLOTS:

  /**
   * \brief Reads the level input field and applies the value.
   *
   * Called when Enter is pressed in the level QLineEdit.
   */
  void SetLevelValue();

  /**
   * \brief Reads the window input field and applies the value.
   *
   * Called when Enter is pressed in the window QLineEdit.
   */
  void SetWindowValue();

protected:
  unsigned long m_ObserverTag;
  bool m_IsObserverTagSet;

  QmitkLevelWindowWidgetContextMenu *m_Contextmenu;
};

#endif
