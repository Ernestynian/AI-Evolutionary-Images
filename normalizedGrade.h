#ifndef NORMALIZEDGRADE_H
#define NORMALIZEDGRADE_H

class NormalizedGrade {
public:
	void set(double grade, int id);

	void addAccumulated(double acc);
	double getAccumulated();
	
	int getID();
	double getGrade() const;

	static bool descending(const NormalizedGrade& ng1, const NormalizedGrade& ng2);
	
private:
	double grade;
	int id;
	
	double accumulated;
};

#endif /* NORMALIZEDGRADE_H */

