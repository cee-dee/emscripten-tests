#include <iostream>
using namespace std;

#include "test.h"

MyClass::MyClass(int a)
{
  x = a;
}

void MyClass::incrementX()
{
  ++x;
}

int MyClass::getX()
{
  return x;
}
