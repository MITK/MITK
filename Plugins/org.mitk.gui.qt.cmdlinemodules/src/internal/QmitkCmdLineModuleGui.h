/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QmitkCmdLineModuleGui_h
#define QmitkCmdLineModuleGui_h

#include <QBuffer>
#include <QUiLoader>

#include <ctkCmdLineModuleReference.h>
#include <ctkCmdLineModuleFrontendQtGui.h>

namespace mitk
{
  class DataStorage;
}
struct QmitkCmdLineModuleGuiPrivate;
class QWidget;

/**
 * \class QmitkCmdLineModuleGui
 * \brief Derived from ctkCmdLineModuleQtGui to implement an MITK specific command line module,
 * that has access to the mitk::DataStorage, and also instantiates QmitkDataStorageComboBox
 * for any "imageInputWidget" type, and also provides QmitkDataStorageComboBox.xsl to override
 * the standard CTK xslt transformation.
 * \author Matt Clarkson (m.clarkson@ucl.ac.uk)
 * \ingroup org_mitk_gui_qt_cmdlinemodules_internal
 * \sa QmitkCmdLineModuleFactoryGui
 * \sa ctkCmdLineModuleFrontendQtGui
 */
class QmitkCmdLineModuleGui : public ctkCmdLineModuleFrontendQtGui
{
  Q_OBJECT

public:
  QmitkCmdLineModuleGui(const mitk::DataStorage* dataStorage, const ctkCmdLineModuleReference& moduleRef);
  virtual ~QmitkCmdLineModuleGui();

  /**
   * \brief Returns the top level widget containing the whole GUI, and
   * should be used in preference to ctkCmdLineModuleFrontend::guiHandle.
   */
  QWidget* getGui();

  /**
   * \brief Copies the visible parameters from another QmitkCmdLineModuleGui;
   * \param another a QmitkCmdLineModuleGui frontend.
   */
  void copyParameters(QmitkCmdLineModuleGui& another);

  /**
   * \brief A custom method to enable access to a mitk::DataNode::Pointer for input images.
   * \param parameter The name of the parameter as specified in XML.
   * \param role The role, \see ctkCmdLineModuleFrontend.
   * \return QVariant
   *
   * If role==UserRole and the parameter specified by parameter name is an
   * input image, will return a mitk::DataNode::Pointer wrapped in a QVariant.
   *
   * If role==UserRole and the parameter specified is not an input image,
   * returns an Empty QVariant.
   *
   * For any other scenario, calls base class.
   *
   * \sa ctkCmdLineModuleFrontend::value
   */
  virtual QVariant value(const QString &parameter, int role) const;

  /**
   * \brief A custom method to enable the setting of mitk::DataNode::Pointer for input images.
   * \param parameter The name of the parameter as specified in XML.
   * \param value QVariant containing a mitk::DataNode::Pointer
   * \param role The role, \see ctkCmdLineModuleFrontend.
   *
   * If role==UserRole and the parameter specified by parameter name is an
   * input image, will set the value for that parameter.
   *
   *
   * For any other scenario, calls base class.
   *
   * \sa ctkCmdLineModuleFrontend::setValue
   */
  virtual void setValue(const QString& parameter, const QVariant& value, int role = DisplayRole);

protected:

  /**
   * \brief Virtual getter.
   * \see ctkCmdLineModuleFrontendQtGui::uiLoader()
   */
  virtual QUiLoader* uiLoader() const;

  /**
   * \brief Virtual getter.
   * \see ctkCmdLineModuleFrontendQtGui::xslTransform()
   */
  virtual ctkCmdLineModuleXslTransform* xslTransform() const;

private:

  QScopedPointer<QmitkCmdLineModuleGuiPrivate> d;

}; // end class

#endif // QmitkCmdLineModuleGui_h
