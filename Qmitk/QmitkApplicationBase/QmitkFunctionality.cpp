#include "QmitkFunctionality.h"

#include <qlcdnumber.h> 
#include <qslider.h> 


QmitkFunctionality::QmitkFunctionality(QObject *parent, const char *name, mitk::DataTreeIterator * dataIt) : QObject(parent, name)
{
	available=false;
	m_DataTreeIterator = dataIt;
}

QmitkFunctionality::~QmitkFunctionality()
{

}

void QmitkFunctionality::activated()
{

}

void QmitkFunctionality::deactivated()
{

}

bool QmitkFunctionality::isAvailable()
{
	return available;
}

void QmitkFunctionality::setAvailability(bool available)
{
	this->available=available;
	emit availabilityChanged(this);
	emit availabilityChanged();
}

void QmitkFunctionality::setDataTree(mitk::DataTreeIterator * it) 
{
	m_DataTreeIterator = it;
}

mitk::DataTreeIterator * QmitkFunctionality::getDataTree()
{
	return m_DataTreeIterator;
}