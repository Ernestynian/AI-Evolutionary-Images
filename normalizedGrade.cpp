
#include "normalizedGrade.h"


void NormalizedGrade::set(double grade, int id) {
	this->grade = grade;
	this->accumulated = grade;
	this->id = id;
}


void NormalizedGrade::addAccumulated(double acc) {
	this->accumulated += acc;
}


double NormalizedGrade::getAccumulated() {
	return this->accumulated;
}


int NormalizedGrade::getID() {
	return this->id;
}


double NormalizedGrade::NormalizedGrade::getGrade() const {
	return this->grade;
}


bool NormalizedGrade::descending(const NormalizedGrade& ng1, const NormalizedGrade& ng2) {
	return ng1.getGrade() > ng2.getGrade();
}