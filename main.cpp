#include<iostream>
#include"LR.h"
using namespace std;

int main()
{
	LR lr;
	lr.setGrammars({ 
		"S --> E |>> ;",
		"E --> E + E |>> ;",
		"E --> num |>> + ; )",
		"E --> (E) |>> + ;" });
	auto lrt = lr.getLR();
	
	lr.exportLRTable();
	return 0;
}