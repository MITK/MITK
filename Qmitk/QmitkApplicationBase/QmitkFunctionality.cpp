#include "QmitkFunctionality.h"
#include "mitkPlaneGeometry.h"

#include <qlcdnumber.h>
#include <qslider.h>


QmitkFunctionality::QmitkFunctionality(QObject *parent, const char *name, mitk::DataTreeIterator * dataIt) : 
  QObject(parent, name), m_Available(false), m_Activated(false), m_DataTreeIterator(dataIt)

{

}

QmitkFunctionality::~QmitkFunctionality()
{

}

void QmitkFunctionality::activated()
{
  m_Activated = true;
}

void QmitkFunctionality::deactivated()
{
  m_Activated = false;
}

bool QmitkFunctionality::isActivated()
{
	return m_Activated;
}

bool QmitkFunctionality::isAvailable()
{
  return m_Available;
}

void QmitkFunctionality::setAvailability(bool available)
{
	this->m_Available=available;
	emit availabilityChanged(this);
	emit availabilityChanged();
}

void QmitkFunctionality::initWidget(mitk::DataTreeIterator* it,
				   QmitkSelectableGLWidget* widget,
				   const Vector3f& origin,
				   const Vector3f& right,
				   const Vector3f& bottom)
{
    mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();

    //ohne den Pointer-Umweg meckert gcc
    mitk::PlaneView* view = new mitk::PlaneView(origin,right,bottom);
    plane->SetPlaneView(*view);
    delete view;

    widget->GetRenderer()->SetData(it);
    widget->GetRenderer()->SetWorldGeometry(plane);
    widget->GetRenderer()->GetDisplayGeometry()->Fit();

    widget->update();
}

void QmitkFunctionality::setDataTree(mitk::DataTreeIterator * it)
{
	m_DataTreeIterator = it;
}

mitk::DataTreeIterator * QmitkFunctionality::getDataTree()
{
	return m_DataTreeIterator;
}
