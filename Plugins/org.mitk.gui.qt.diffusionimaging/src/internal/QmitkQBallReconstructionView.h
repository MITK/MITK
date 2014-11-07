/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef _QMITKQBALLRECONSTRUCTIONVIEW_H_INCLUDED
#define _QMITKQBALLRECONSTRUCTIONVIEW_H_INCLUDED

#include <QmitkFunctionality.h>

#include <string>

#include "ui_QmitkQBallReconstructionViewControls.h"

#include "mitkDiffusionImage.h"

#include <berryIPartListener.h>
#include <berryISelectionListener.h>
#include <berryIStructuredSelection.h>

typedef short DiffusionPixelType;

struct QbrSelListener;

struct QbrShellSelection;

/*!
 * \ingroup org_mitk_gui_qt_qballreconstruction_internal
 *
 * \brief QmitkQBallReconstructionView
 *
 * Document your class here.
 *
 * \sa QmitkFunctionality
 */
class QmitkQBallReconstructionView : public QmitkFunctionality
{

  friend struct QbrSelListener;

  friend struct QbrShellSelection;

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;

  QmitkQBallReconstructionView();
  virtual ~QmitkQBallReconstructionView();

  virtual void CreateQtPartControl(QWidget *parent);

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

  /// \brief Called when the functionality is activated
  virtual void Activated();

  virtual void Deactivated();

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();

  static const int nrconvkernels;

protected slots:

  void ReconstructStandard();
  void AdvancedCheckboxClicked();
  void MethodChoosen(int method);
  void Reconstruct(int method, int normalization);

  void NumericalQBallReconstruction(mitk::DataStorage::SetOfObjects::Pointer inImages, int normalization);
  void AnalyticalQBallReconstruction(mitk::DataStorage::SetOfObjects::Pointer inImages, int normalization);
  void MultiQBallReconstruction(mitk::DataStorage::SetOfObjects::Pointer inImages);


  /**
   * @brief PreviewThreshold Generates a preview of the values that are cut off by the thresholds
   * @param threshold
   */
  void PreviewThreshold(int threshold);

protected:

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

  Ui::QmitkQBallReconstructionViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;

  template<int L>
  void TemplatedAnalyticalQBallReconstruction(mitk::DataNode* dataNodePointer, float lambda, int normalization);

  template<int L>
  void TemplatedMultiQBallReconstruction(float lambda, mitk::DataNode*);

  void SetDefaultNodeProperties(mitk::DataNode::Pointer node, std::string name);

  //void Create

  berry::ISelectionListener::Pointer m_SelListener;
  berry::IStructuredSelection::ConstPointer m_CurrentSelection;

  mitk::DataStorage::SetOfObjects::Pointer m_DiffusionImages;

private:

  std::map< const mitk::DataNode *, QbrShellSelection * > m_ShellSelectorMap;
  void GenerateShellSelectionUI(mitk::DataStorage::SetOfObjects::Pointer set);
};




#endif // _QMITKQBALLRECONSTRUCTIONVIEW_H_INCLUDED

