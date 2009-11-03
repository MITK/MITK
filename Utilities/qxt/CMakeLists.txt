
IF(MITK_USE_QT) # only if MITK is built with Qt

PROJECT(QXT)

SET(QXT_MOC_HEADERS
  include/qxtnamespace.h
  
  include/qxtbasespinbox.h
  include/qxtcheckcombobox.h
  include/qxtcheckcombobox_p.h
  include/qxtconfigdialog.h
  include/qxtconfigdialog_p.h
  include/qxtconfirmationmessage.h
  include/qxtcrumbview.h
  include/qxtcrumbview_p.h
  include/qxtdockwidget.h
  include/qxtflowview.h
  include/qxtflowview_p.h
  include/qxtgroupbox.h
  include/qxtheaderview.h
  include/qxtitemdelegate.h
  include/qxtitemdelegate_p.h
  include/qxtlabel.h
  include/qxtlanguagecombobox.h
  include/qxtlanguagecombobox_p.h
  include/qxtletterboxwidget.h
  include/qxtletterboxwidget_p.h
  include/qxtlistwidget.h
  include/qxtlistwidget_p.h
  include/qxtprogresslabel.h
  include/qxtpushbutton.h
  include/qxtscheduleheaderwidget.h
  include/qxtscheduleitemdelegate.h
  include/qxtscheduleview.h
  include/qxtscheduleviewheadermodel_p.h
  include/qxtscheduleview_p.h
  include/qxtsortfilterproxymodel.h
  include/qxtspanslider.h
  include/qxtspanslider_p.h
  include/qxtstars.h
  include/qxtstringspinbox.h
  include/qxtstringvalidator.h
  include/qxttabbar.h
  include/qxttablewidget.h
  include/qxttablewidget_p.h
  include/qxttabwidget.h
  include/qxttabwidget_p.h
  include/qxttooltip_p.h
  include/qxttreewidget.h
  include/qxttreewidget_p.h
)

SET(QXT_HEADERS
  include/qxtglobal.h
  include/qxtconfig.h
  include/qxtpimpl.h
  
  include/qxtgui.h  
  include/qxtitemeditorcreator.h
  include/qxtitemeditorcreatorbase.h
  include/qxtlistwidgetitem.h
  include/qxtnativeeventfilter.h
  include/qxtproxystyle.h
  include/qxtstandarditemeditorcreator.h
  include/qxtstringvalidator_p.h  
  include/qxtstyleoptionscheduleviewitem.h 
  include/qxttablewidgetitem.h
  include/qxttooltip.h
  include/qxttreewidgetitem.h
)

SET(QXT_SOURCES
  src/qxtglobal.cpp
  
  src/qxtbasespinbox.cpp
  src/qxtcheckcombobox.cpp
  src/qxtconfigdialog.cpp
  src/qxtconfirmationmessage.cpp
  src/qxtcrumbview.cpp
  src/qxtdockwidget.cpp
  src/qxtflowview.cpp
  src/qxtflowview_p.cpp
  src/qxtgroupbox.cpp
  src/qxtheaderview.cpp
  src/qxtitemdelegate.cpp
  src/qxtlabel.cpp
  src/qxtlanguagecombobox.cpp
  src/qxtletterboxwidget.cpp
  src/qxtlistwidget.cpp
  src/qxtlistwidgetitem.cpp
  src/qxtprogresslabel.cpp
  src/qxtproxystyle.cpp
  src/qxtpushbutton.cpp
  src/qxtscheduleheaderwidget.cpp
  src/qxtscheduleitemdelegate.cpp
  src/qxtscheduleview.cpp
  src/qxtscheduleviewheadermodel_p.cpp
  src/qxtscheduleview_p.cpp
  src/qxtsortfilterproxymodel.cpp
  src/qxtspanslider.cpp
  src/qxtstars.cpp
  src/qxtstringspinbox.cpp
  src/qxtstringvalidator.cpp
  src/qxtstyleoptionscheduleviewitem.cpp
  src/qxttabbar.cpp
  src/qxttablewidget.cpp
  src/qxttablewidgetitem.cpp
  src/qxttabwidget.cpp
  src/qxttooltip.cpp
  src/qxttreewidget.cpp
  src/qxttreewidgetitem.cpp
)

SET(QT_MT_REQUIRED TRUE)
SET(QXT_INCLUDE_DIR ${QXT_SOURCE_DIR}/include CACHE PATH "MITK internal qxt include directory")
INCLUDE_DIRECTORIES(${QXT_INCLUDE_DIR})

FIND_PACKAGE(Qt4 REQUIRED)
SET(QT_USE_QTSVG 1)
INCLUDE(${QT_USE_FILE})
QT4_WRAP_CPP(QXT_SOURCES ${QXT_MOC_HEADERS})

ADD_DEFINITIONS(-DBUILD_QXT_GUI)
ADD_DEFINITIONS(-DBUILD_QXT_CORE)

ADD_LIBRARY(qxt ${QXT_SOURCES})
TARGET_LINK_LIBRARIES(qxt ${QT_LIBRARIES})

# install only if shared lib (for now)
MITK_INSTALL_TARGETS(qxt)

MITK_CREATE_MODULE_CONF(qxt
  INCLUDE_DIRS include
)

ENDIF(MITK_USE_QT) # only if MITK is built with Qt