#include <iostream>
#include <TComplex.h>


void TestComplex(){

	TComplex z(0.,TMath::Pi()/2.);

	double x = z.Rho();

	std::cout << TComplex::Exp(z) << std::endl;
	std::cout << x << std::endl;

}
