#ifndef QMITKsegmentationVIEW_H_
#define QMITKsegmentationVIEW_H_

#include <cherryIPartListener.h>

#include <QmitkFunctionality.h>
#include <QmitkStandardViews.h>
#include <QmitkStdMultiWidgetEditor.h>

#include "ui_QmitkSegmentationControls.h"
#include <cherryISelectionListener.h>
#include <cherryIStructuredSelection.h>

/**
 * \ingroup org_mitk_gui_qt_segmentation_internal
 */
class QmitkSegmentationView : public QObject
  , public QmitkFunctionality
{
  Q_OBJECT
public:

  void SetFocus();

  QmitkSegmentationView();
  virtual ~QmitkSegmentationView();
    
  void OnSurfaceCalculationDone();

protected:

  void CreateQtPartControl(QWidget* parent);

// QmitkSegmentation
  public:

    virtual void CreateConnections();

    virtual void Activated();

    virtual void Deactivated();

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

    void SelectionChanged(cherry::IWorkbenchPart::Pointer sourcepart, cherry::ISelection::ConstPointer selection);

    void CreateNewSegmentation();

    void OnToolSelected(int id);
    void OnReferenceNodeSelected(const mitk::DataTreeNode*);
    void OnWorkingDataSelectionChanged(const mitk::DataTreeNode*);

    void PartHidden(cherry::IWorkbenchPartReference::Pointer);
    void PartVisible(cherry::IWorkbenchPartReference::Pointer);

    void SendSelectedEvent( mitk::DataTreeNode* referenceNode, mitk::DataTreeNode* workingNode );

    void CreateSurface(bool);

  protected:

    typedef std::vector<mitk::DataTreeNode*> NodeList;
    NodeList GetSelectedNodes() const;

    void CheckImageAlignment();

    QmitkStdMultiWidget * m_MultiWidget;

    QWidget* m_Parent;

    Ui::QmitkSegmentationControls * m_Controls;
    unsigned long m_ObserverTag;
    mitk::DataStorage::Pointer m_DataStorage;

    cherry::IStructuredSelection::ConstPointer m_CurrentSelection;
    cherry::ISelectionListener::Pointer m_SelectionListener;
    friend class cherry::SelectionChangedAdapter<QmitkSegmentationView>;
  
    QAction* m_CreateSurfaceAction;

  /// from QmitkSegmentation

};

#endif /*QMITKsegmentationVIEW_H_*/
