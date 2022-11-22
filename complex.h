// 
// Complex number operations
//
// 2022, Jonathan Tainer
//

#ifndef COMPLEX_H
#define COMPLEX_H

typedef struct Complex {
	float real;
	float imag;
} Complex;

Complex ComplexAdd(Complex a, Complex b);

Complex ComplexSubtract(Complex a, Complex b);

Complex ComplexMultiply(Complex a, Complex b);

Complex ComplexExp(Complex a, unsigned int n);

#endif
