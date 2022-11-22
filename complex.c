// 
// Complex number math functions
//
// 2022, Jonathan Tainer
//

#include "complex.h"

Complex ComplexAdd(Complex a, Complex b) {
	return (Complex) { a.real + b.real, a.imag + b.imag };
}

Complex ComplexSubtract(Complex a, Complex b) {
	return (Complex) { a.real - b.real, a.imag - b.imag };
}

Complex ComplexMultiply(Complex a, Complex b) {
	return (Complex) { (a.real * b.real) - (a.imag * b.imag), (a.real * b.imag) + (a.imag * b.real) };
}

Complex ComplexExp(Complex a, unsigned int n) {
	Complex b = { 1, 0 };
	for (int i = 0; i < n; i++) {
		b = ComplexMultiply(a, b);
	}
	return b;
}
