#include "pyloader.h"
#include <iostream>

using namespace std;
using namespace CASD;

int main(int argc, char *argv[])
{
    PyLoader::GetInstance().Init("./script");
    PyLoader::GetInstance().Load("pytest");
    //PyLoader::GetInstance().Load("leaf_grade");
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

    pArgs = PyTuple_New(1);
    const char * arg_str = "This is just a test";
    PyTuple_SetItem(pArgs, 0, Py_BuildValue("s", arg_str));
    //pRes = PyLoader::GetInstance().CallInstanceMethod(pIns, "h", pArgs);
    pRes = PyLoader::GetInstance().CallInstanceMethod(pIns, "h", Py_BuildValue("s", arg_str));
    if(pRes)
    {
        cout << "return result" << endl;
    }
    else
    {
        cout << "return none" << endl;
    }
    int group = 0;
    int rank = 0;
    ok = PyArg_ParseTuple(pRes, "ii", &group, &rank);
    cout << "call instance method h: group = " << group << ", rank = " << rank << endl;

    /*
    PyObject* pLGIns = PyLoader::GetInstance().CreateClassInstance("leaf_grade", "LeafGrade");
    if(pLGIns)
        cout << "create ins success" << endl;
    else
        cout << "create ins err" << endl;
    */
    return 0;
}
