/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkXnatCreateObjectDialog_h
#define QmitkXnatCreateObjectDialog_h

#include <MitkXNATExports.h>

#include <QDialog>

class ctkXnatObject;

/**
 * \brief Dialog for creating new XNAT objects (subjects or experiments).
 *
 * QmitkXnatCreateObjectDialog presents a form that allows the user to fill in the
 * properties needed to create a new XNAT subject or experiment. The dialog embeds
 * the appropriate property widget (QmitkXnatSubjectWidget or QmitkXnatExperimentWidget)
 * in CREATE mode, making all input fields editable. It validates that mandatory fields
 * (such as label/ID and, for experiments, the XSI type) are filled before accepting.
 *
 * \sa QmitkXnatSubjectWidget, QmitkXnatExperimentWidget, ctkXnatObject
 */
class MITKXNAT_EXPORT QmitkXnatCreateObjectDialog : public QDialog
{
  Q_OBJECT

public:
  /**
   * \brief Enumeration of XNAT object types that can be created.
   */
  enum SpecificType
  {
    // PROJECT,  ///< Project creation (not yet implemented).
    SUBJECT,    ///< Create a new XNAT subject.
    EXPERIMENT  ///< Create a new XNAT experiment.
  };

  /**
   * \brief Construct the create-object dialog for the given XNAT type.
   *
   * Creates the appropriate property widget (QmitkXnatSubjectWidget or
   * QmitkXnatExperimentWidget) in CREATE mode and embeds it into the dialog layout
   * along with Create and Cancel buttons.
   *
   * \param[in] type The type of XNAT object to create (SUBJECT or EXPERIMENT).
   * \param[in] parent Optional parent widget.
   */
  QmitkXnatCreateObjectDialog(SpecificType type, QWidget *parent = nullptr);

  /**
   * \brief Destructor. Cleans up the embedded property widget.
   */
  ~QmitkXnatCreateObjectDialog() override;

  /**
   * \brief Retrieve the XNAT object populated with user-entered properties.
   *
   * The returned object is a ctkXnatSubject or ctkXnatExperiment depending on the
   * SpecificType passed to the constructor. Properties are set from the widget fields
   * when the user clicks Create.
   *
   * \return Pointer to the created ctkXnatObject. Ownership is transferred to the caller.
   * \pre The dialog should have been accepted for the object properties to be fully populated.
   */
  ctkXnatObject *GetXnatObject();

protected slots:

  /**
   * \brief Slot called when the Create button is clicked.
   *
   * Validates mandatory fields (label for subjects; label and xsiType for experiments).
   * If validation fails, a warning message box is shown and the dialog remains open.
   * Otherwise the dialog is accepted.
   */
  void OnAcceptClicked();

  /**
   * \brief Slot called when the Cancel button is clicked. Rejects the dialog.
   */
  void OnCancelClicked();

private:
  SpecificType m_Type;
  ctkXnatObject *m_Object;
  QWidget *m_Widget;
};

#endif
