#pragma once

#include <qobject.h>

#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateProperty.h>
#include <QmitkRenderWindow.h>

enum class CrosshairMode { PLANE, POINT, NONE };

class MITKQTWIDGETS_EXPORT CrosshairModeController : public QObject {
  Q_OBJECT
public:
  static CrosshairModeController* instance;
  static CrosshairModeController* getInstance()
  {
    if (instance == nullptr) {
      instance = new CrosshairModeController();
    }
    return instance;
  }
CrosshairMode getMode() { return m_Mode; }
void setMode(CrosshairMode mode)
{
  if (m_Mode == mode) {
    return;
  }
  m_Mode = mode;
  emit crosshairModeChanged(m_Mode);
}
signals:
  void crosshairModeChanged(CrosshairMode mode);
private:
  CrosshairModeController() : m_Mode(CrosshairMode::PLANE) {};
  CrosshairMode m_Mode;
};

class MITKQTWIDGETS_EXPORT mitkCrosshairManager : public QObject {
  Q_OBJECT
public:
  mitkCrosshairManager(const QString& parentWidget);
  ~mitkCrosshairManager();

  // Changes crosshair mode for selected windows
  void setCrosshairMode(CrosshairMode mode);
  CrosshairMode getCrosshairMode();

  // Adds window to managed windows
  void addWindow(QmitkRenderWindow* window);

  // Removes window from managed windows
  void removeWindow(QmitkRenderWindow* window);
  void removeAll();

  // Updates crosshair for window
  void updateWindow(QmitkRenderWindow* window);
  void updateAllWindows();

  // Selects window for show selected window only flag
  void selectWindow(QmitkRenderWindow* window);

  // In this mode there is no crosshair in selected window
  // All other windows are showing selected window position
  void setShowSelectedOnly(bool show);

  // Enable/Disable planes in 3D view
  void setShowPlanesIn3d(bool show);

  // Enable/Disable planes in 3D view when cursor is disabled
  void setShowPlanesIn3dWithoutCursor(bool show);

  // Set crosshair gap
  void setCrosshairGap(int gap);

  // Use crosshair gap in 2d windows
  void setUseCrosshairGap(bool use);

  // Set window colors in order windows were added to manager
  void setWindowsColors(std::vector<mitk::Color> colors);

  // Use separate windows colors instead of default colors
  void setUseWindowsColors(bool use);

  // Apply changes only to first window, cause they all share crosshair
  void setSingleDataStorage(bool single);

  // Returns QToolButton which can be used to control crosshair mode
  static QToolButton* createUiModeController();

  // Updates crosshair position
  // Only updates point crosshair, cause planes changes dinamicaly
  void updateCrosshairsPositions();

  // Updates crosshair colors for swivel mode. Hack cause there is only one place where this needed
  void updateSwivelColors(QmitkRenderWindow* window);

  std::function<bool(QmitkRenderWindow*, QmitkRenderWindow*)>* checkWindowsShareCrosshair;

  bool getShowPlanesIn3D();

signals:
  void crosshairModeChanged(CrosshairMode mode);
  void savePlaneVisibilityIn3D(bool visibility);

private:
  bool hasWindow(QmitkRenderWindow* window);

  void addCrosshair(QmitkRenderWindow* window);
  void removeCrosshair(QmitkRenderWindow* window);
  void recreateCrosshair(QmitkRenderWindow* window);

  void addPlaneCrosshair(QmitkRenderWindow* window, bool render2d = false, bool render3d = false);
  void addPointCrosshair(QmitkRenderWindow* window);

  mitk::NodePredicateProperty::Pointer m_CrosshairPredicate;

  void setDefaultProperties(mitk::DataNode::Pointer crosshair);

  CrosshairMode m_CrosshairMode;
  std::vector<QmitkRenderWindow*> m_ManagedWindows;

  QmitkRenderWindow* m_Selected;
  bool m_ShowSelected;

  bool m_ShowPlanesIn3d;
  bool m_ShowPlanesIn3dWithoutCursor;
  int m_CrosshairGap;
  bool m_UseCrosshairGap;

  std::vector<mitk::Color> m_WindowsColors;
  bool m_UseWindowsColors;

  bool m_SingleDataStorage;

  mitk::Color m_SwivelColor;
  mitk::Color m_SwivelSelectedColor;

  QString m_ParentWidget;
};
