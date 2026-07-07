/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkXnatExperimentWidget_h
#define QmitkXnatExperimentWidget_h

// XNATUI
#include <MitkXNATExports.h>

// Qt
#include <QWidget>
#include <memory>

// CTK XNAT Core
class ctkXnatExperiment;

namespace Ui { class QmitkXnatExperimentWidgetControls; }

/**
 * \brief Widget for displaying or editing XNAT experiment properties.
 *
 * QmitkXnatExperimentWidget provides a form-based user interface for viewing or creating
 * XNAT experiments. In INFO mode, the widget displays read-only experiment properties
 * (label, modality, date, time, scanner) along with a breadcrumb showing the parent
 * project and subject. In CREATE mode, the modality field becomes an XSI type selector
 * with auto-completion for all standard XNAT experiment session types, and all fields
 * are editable.
 *
 * \sa QmitkXnatCreateObjectDialog, QmitkXnatSubjectWidget, QmitkXnatProjectWidget, ctkXnatExperiment
 */
class MITKXNAT_EXPORT QmitkXnatExperimentWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Operation mode for the experiment widget.
   */
  enum Mode
  {
    INFO,   ///< Display experiment properties in read-only mode.
    CREATE  ///< Allow editing of experiment properties for creation.
  };

  /**
   * \brief Construct the widget in INFO mode.
   *
   * \param[in] parent Optional parent widget.
   */
  QmitkXnatExperimentWidget(QWidget *parent = nullptr);

  /**
   * \brief Construct the widget in the specified mode.
   *
   * In CREATE mode, the modality field is replaced by an XSI type field with
   * auto-completion for all supported XNAT experiment session types.
   *
   * \param[in] mode The operation mode (INFO or CREATE).
   * \param[in] parent Optional parent widget.
   */
  QmitkXnatExperimentWidget(Mode mode, QWidget *parent = nullptr);

  /**
   * \brief Destructor.
   */
  ~QmitkXnatExperimentWidget() override;

  /**
   * \brief Populate the widget fields from the given experiment object.
   *
   * Sets the breadcrumb label, ID/label, modality, date, time, and scanner
   * fields from the experiment's properties.
   *
   * \param[in] experiment The XNAT experiment whose properties should be displayed.
   * \pre \p experiment must not be \c nullptr.
   */
  void SetExperiment(ctkXnatExperiment *experiment);

  /**
   * \brief Retrieve the experiment object, optionally updated with user-entered values.
   *
   * In CREATE mode, the returned experiment has its label, xsiType, date, time, and
   * scanner properties updated from the widget's input fields. In INFO mode, the
   * experiment is returned unchanged.
   *
   * \return Pointer to the ctkXnatExperiment with current widget values applied.
   */
  ctkXnatExperiment *GetExperiment() const;

protected:
  std::unique_ptr<Ui::QmitkXnatExperimentWidgetControls> m_Controls; ///< UI controls generated from the .ui form.

private:
  void Init();
  Mode m_Mode;
  ctkXnatExperiment *m_Experiment;
};

#endif
