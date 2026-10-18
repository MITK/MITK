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
#include <optional>
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

    /** \brief The only time step that is remeshed, unless all of them are. */
    std::optional<mitk::TimeStepType> TimeStep;
  };

  void EnableWidgets(bool enable);
  int GetSubsampling() const;
  void UpdateMemoryEstimate();
  void SetUpHoverInfo(QWidget* parent);
  void OnTimeChanged();

  bool IsCurrentTimeStepOnly(const mitk::Surface* surface) const;

  /**
   * \brief The whole surface, or its current time step as a surface of its own.
   *
   * \param[in] surface The selected surface.
   * \param[in] copy Whether the poly data is copied, as the worker needs it.
   * \return The input of remeshing, or nullptr if the surface has no time step at the current time point.
   */
  mitk::Surface::ConstPointer GetRemeshingInput(const mitk::Surface* surface, bool copy) const;

  bool IsRemeshing() const;
  void CancelRemeshing();
  void OnRemeshingFinished();

  std::unique_ptr<Ui::QmitkRemeshingViewControls> m_Controls;
  std::optional<unsigned long> m_TimeObserverTag;

  /** \brief What the info label shows while the mouse is over a widget. */
  std::map<QObject*, QString> m_HoverInfo;
  QWidget* m_Parent;

  QFutureWatcher<RemeshingResult> m_Watcher;
  std::shared_ptr<RemeshingRun> m_Run;

  /** \brief The node of the surface being remeshed, the surface, and what the result takes from them. */
  mitk::WeakPointer<mitk::DataNode> m_Original;
  mitk::WeakPointer<mitk::BaseData> m_OriginalData;
  std::string m_OriginalName;
  std::array<float, 3> m_OriginalColor;
  bool m_OriginalWasPulsing;
  ResultOptions m_ResultOptions;
};

#endif
