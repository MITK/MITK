/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRemeshingView_h
#define QmitkRemeshingView_h

#include <QmitkAbstractView.h>
#include <QmitkSingleNodeSelectionWidget.h>

#include <mitkSurface.h>
#include <mitkWeakPointer.h>

#include <QFutureWatcher>

#include <array>
#include <exception>
#include <map>
#include <memory>
#include <string>

namespace Ui
{
  class QmitkRemeshingViewControls;
}

class QmitkRemeshingView : public QmitkAbstractView
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkRemeshingView();
  ~QmitkRemeshingView() override;

  void CreateQtPartControl(QWidget* parent) override;
  void SetFocus() override;

protected:
  bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
  void OnSurfaceChanged(const QmitkSingleNodeSelectionWidget::NodeList& nodes);
  void OnDensityChanged(int numVertices);
  void OnRemeshButtonClicked();

private:
  struct RemeshingResult
  {
    mitk::Surface::Pointer Surface;
    bool Canceled = false;
    std::exception_ptr Error;
  };

  /** \brief What the view and the worker of a running remeshing share. */
  struct RemeshingRun;

  /** \brief The options as they were when remeshing started. */
  struct ResultOptions
  {
    bool ReplaceOriginal = false;
    bool HideOriginal = true;
    bool Wireframe = true;
  };

  void EnableWidgets(bool enable);
  int GetNumberOfVertices(int density) const;
  int GetSubsampling() const;
  void UpdateMemoryEstimate();
  void SetUpHoverInfo(QWidget* parent);

  bool IsRemeshing() const;
  void CancelRemeshing();
  void OnRemeshingFinished();

  std::unique_ptr<Ui::QmitkRemeshingViewControls> m_Controls;
  int m_MaxNumberOfVertices;

  /** \brief What the info label shows while the mouse is over a widget. */
  std::map<QObject*, QString> m_HoverInfo;
  QWidget* m_Parent;

  QFutureWatcher<RemeshingResult> m_Watcher;
  std::shared_ptr<RemeshingRun> m_Run;

  /** \brief The node of the surface being remeshed, and what the result takes from it. */
  mitk::WeakPointer<mitk::DataNode> m_Original;
  std::string m_OriginalName;
  std::array<float, 3> m_OriginalColor;
  bool m_OriginalWasPulsing;
  ResultOptions m_ResultOptions;
};

#endif
