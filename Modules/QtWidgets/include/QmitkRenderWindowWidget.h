/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRenderWindowWidget_h
#define QmitkRenderWindowWidget_h

#include <MitkQtWidgetsExports.h>

// qt widgets module
#include <QmitkRenderWindow.h>

// mitk core
#include <mitkCrosshairManager.h>
#include <mitkDataStorage.h>
#include <mitkRenderWindow.h>
#include <QmitkRenderWindowUtilityWidget.h>

// qt
#include <QFrame>
#include <QMouseEvent>
#include <QVBoxLayout>

class vtkCornerAnnotation;

/**
* \brief The 'QmitkRenderWindowWidget' is a QFrame that holds a render window
*        and some associates properties, e.g. decorations.
*        Decorations are corner annotation (text and color), frame color or background color
*        and can be set using this class.
*        The 'QmitkRenderWindowWidget' is used inside a 'QmitkAbstractMultiWidget', where a map contains
*        several render window widgets to create the multi widget display.
*        This class uses a CrosshairManager, which allows to use plane geometries as crosshair.
*/
class MITKQTWIDGETS_EXPORT QmitkRenderWindowWidget : public QFrame
{
  Q_OBJECT

public:

  /**
  * \brief Construct a render-window widget.
  *
  * \param parent       Owning Qt parent. Must not be null.
  * \param widgetName   Qualified name registered with the rendering manager
  *                     (typically '<editorName>.<bareName>'). Must not be empty.
  * \param dataStorage  Data storage backing the render window. Must not be null;
  *                     the widget unconditionally talks to it during 'InitializeGUI'.
  *
  *   Defaults intentionally not provided: the widget is unusable without a
  *   non-null data storage and a non-empty name. Earlier versions defaulted
  *   all three parameters; that hid construction errors at the call site.
  */
  QmitkRenderWindowWidget(
    QWidget* parent,
    const QString& widgetName,
    mitk::DataStorage* dataStorage);

  ~QmitkRenderWindowWidget() override;

  void SetDataStorage(mitk::DataStorage* dataStorage);

  const QString& GetWidgetName() const { return m_WidgetName; };
  QmitkRenderWindow* GetRenderWindow() const { return m_RenderWindow; };

  /**
  * \brief Optional human-readable display label.
  *
  *   Persisted as the optional `name` field of the corresponding window leaf
  *   in the v2 layout document; empty when the layout omits that field. Pure
  *   metadata: not used for routing, addressing, persisted-state keying, or
  *   REST URL construction (those all use the bare widget id, derivable from
  *   `GetWidgetName` via the editor-prefix strip).
  */
  const QString& GetDisplayName() const { return m_DisplayName; };
  void SetDisplayName(const QString& displayName) { m_DisplayName = displayName; };

  mitk::SliceNavigationController* GetSliceNavigationController() const;

  void RequestUpdate();
  void ForceImmediateUpdate();

  void AddUtilityWidget(QWidget* utilityWidget);
  QmitkRenderWindowUtilityWidget* GetUtilityWidget();
  const QmitkRenderWindowUtilityWidget* GetUtilityWidget() const;

  void SetGradientBackgroundColors(const mitk::Color& upper, const mitk::Color& lower);
  void ShowGradientBackground(bool enable);
  std::pair<mitk::Color, mitk::Color> GetGradientBackgroundColors() const { return m_GradientBackgroundColors; };
  bool IsGradientBackgroundOn() const;

  void SetDecorationColor(const mitk::Color& color);
  mitk::Color GetDecorationColor() const { return m_DecorationColor; };

  void ShowColoredRectangle(bool show);
  bool IsColoredRectangleVisible() const;

  void ShowCornerAnnotation(bool show);
  bool IsCornerAnnotationVisible() const;
  void SetCornerAnnotationText(const std::string& cornerAnnotation);
  std::string GetCornerAnnotationText() const;

  bool IsRenderWindowMenuActivated() const;

  void SetCrosshairVisibility(bool visible);
  bool GetCrosshairVisibility();
  void SetCrosshairGap(unsigned int gapSize);

  void EnableCrosshair();
  void DisableCrosshair();

  void SetCrosshairPosition(const mitk::Point3D& newPosition);
  mitk::Point3D GetCrosshairPosition() const;

  void SetGeometry(const itk::EventObject& event);
  void SetGeometrySlice(const itk::EventObject& event);

public Q_SLOTS:

  void OnResetGeometry();

private:

  void InitializeGUI();
  void InitializeDecorations();
  void ResetGeometry(const mitk::TimeGeometry* referenceGeometry);

  QString m_WidgetName;
  QString m_DisplayName;
  QVBoxLayout* m_Layout;

  mitk::DataStorage* m_DataStorage;

  QmitkRenderWindow* m_RenderWindow;

  mitk::CrosshairManager::Pointer m_CrosshairManager;

  std::pair<mitk::Color, mitk::Color> m_GradientBackgroundColors;
  mitk::Color m_DecorationColor;
  vtkSmartPointer<vtkCornerAnnotation> m_CornerAnnotation;

};

#endif
