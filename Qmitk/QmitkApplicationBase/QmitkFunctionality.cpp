#include "QmitkFunctionality.h"

#include <qlcdnumber.h> 
#include <qslider.h> 

QmitkFunctionality::QmitkFunctionality(QObject *parent, const char *name) : QObject(parent, name)
{
	available=false;
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
