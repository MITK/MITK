#ifndef QMITKINTERACTIVESEGMENTATIONVIEW_H_
#define QMITKINTERACTIVESEGMENTATIONVIEW_H_

#include <cherryIPartListener.h>

#include <QmitkFunctionality.h>
#include <QmitkStandardViews.h>
#include <QmitkStdMultiWidgetEditor.h>

#include "ui_QmitkInteractiveSegmentationControls.h"

/**
 * \ingroup org_mitk_gui_qt_interactivesegmentation_internal
 */
class QmitkInteractiveSegmentationView : public QObject
  , public QmitkFunctionality
{
  Q_OBJECT
public:

  void SetFocus();

  QmitkInteractiveSegmentationView();
  virtual ~QmitkInteractiveSegmentationView();

protected:

  void CreateQtPartControl(QWidget* parent);

// QmitkInteractiveSegmentation
  public:

    virtual void CreateConnections();

    virtual void Activated();

    virtual void Deactivated();

    void OnNodePropertiesChanged();
    void OnNewNodesGenerated();

    ///
    /// Called when a StdMultiWidget is available.
    ///
    virtual void StdMultiWidgetAvailable(QmitkStdMultiWidget& stdMultiWidget);
    ///
    /// Called when no StdMultiWidget is available.
    ///
    virtual void StdMultiWidgetNotAvailable();

    ///
    /// Called when no StdMultiWidget is getting closed.
    ///
    virtual void StdMultiWidgetClosed(QmitkStdMultiWidget& stdMultiWidget);

  protected slots:

    void DataStorageChanged( const mitk::DataTreeNode* node );

    void ReinitializeToImage();
    void CreateNewSegmentation();
    void LoadSegmentation();
    void DeleteSegmentation();
    void SaveSegmentation();
    void SetReferenceImagePixelSmoothing(bool on);

    void OnToolSelected(int id);
    void OnReferenceNodeSelected(const mitk::DataTreeNode*);

  protected:
    void HandleException( const char* str, QWidget* parent, bool showDialog ) const;
    void CheckImageAlignment(mitk::Image* image);

    QmitkStdMultiWidget * m_MultiWidget;

    QWidget* m_Parent;

    Ui::QmitkInteractiveSegmentationControls * m_Controls;
    unsigned long m_ObserverTag;
    mitk::DataStorage::Pointer m_DataStorage;

  /// from QmitkInteractiveSegmentation

};

#endif /*QMITKINTERACTIVESEGMENTATIONVIEW_H_*/
