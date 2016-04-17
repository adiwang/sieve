#include "pyloader.h"
#include <iostream>

using namespace std;
using namespace CASD;

int main(int argc, char *argv[])
{
    PyLoader::GetInstance().Init();
    PyLoader::GetInstance().Load("pytest");
    PyObject* pArgs = PyTuple_New(2);
    PyTuple_SetItem(pArgs, 0, Py_BuildValue("i",3));
    PyTuple_SetItem(pArgs, 1, Py_BuildValue("i",4));
    PyObject* pRes = PyLoader::GetInstance().CallModuleFunc("pytest", "add", pArgs);
    int retval = 0;
    int ok = 0;
    ok = PyArg_Parse(pRes, "i", &retval);
    cout << "call test.py add fucntion: 3 + 4 = " << retval << endl;
    PyObject* pIns = PyLoader::GetInstance().CreateClassInstance("pytest", "guestlist");
    PyLoader::GetInstance().CallInstanceMethod(pIns, "p");
    return 0;
}
