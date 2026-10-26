#include "pch.h"
#include <fstream>
#ifndef CMATRIXH
# define CMATRIXH 1

using namespace std;

class CMatrix
{
	double** array;
	int n_rows;
	int n_cols;
public:
	CMatrix();
	CMatrix(int, int);
	CMatrix(int);
	CMatrix(const CMatrix&);

	CMatrix(ifstream& file)
	{
		int r = 0;
		int c = 0;
		file >> r;
		file >> c;

		n_rows = r;
		n_cols = c;

		array = new double* [n_rows];
		for (int i = 0; i < n_rows; i++)
			array[i] = new double[n_cols];

		for (int i = 0; i < n_rows; i++)
			for (int j = 0; j < n_cols; j++)
				file >> array[i][j];
	}

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

#endif