#pragma once

class CMatrix
{
private:
	double** array;
	int n_rows;
	int n_cols;
public:
	CMatrix();
	CMatrix(int, int);
	CMatrix(int);
	CMatrix(const CMatrix&);
	~CMatrix();
	double& operator()(int, int);
	double& operator()(int);
	CMatrix operator-();
	CMatrix operator=(const CMatrix&);
	CMatrix operator*(CMatrix&);
	CMatrix operator+(CMatrix&);
	CMatrix operator-(CMatrix&);
	CMatrix operator+(double);
	CMatrix operator-(double);
	int rows()const { return n_rows; };
	int cols()const { return n_cols; };
	CMatrix Transp();
	CMatrix GetRow(int);
	CMatrix GetRow(int, int, int);
	CMatrix GetCol(int);
	CMatrix GetCol(int, int, int);
	CMatrix RedimMatrix(int, int);
	CMatrix RedimData(int, int);
	CMatrix RedimMatrix(int);
	CMatrix RedimData(int);
	double MaxElement();
	double MinElement();
};