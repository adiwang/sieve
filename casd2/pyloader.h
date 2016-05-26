#ifndef _CASD_PYLOADER_H
#define _CASD_PYLOADER_H
#include <Python.h>
#include <map>
#include <string>

namespace CASD
{

class PyLoader
{
public:
    typedef std::map<std::string/*script name*/, PyObject* /*python module*/> NameModuleMap;
    typedef NameModuleMap::iterator NameModuleMapIter;

    ~PyLoader();

    static PyLoader& GetInstance()
    {
        static PyLoader instance;
        return instance;
    }

    bool Init(std::string script_dir);
    bool Load(std::string filename);
    PyObject* GetModule(std::string module_name);
    PyObject* GetObjectFromModule(PyObject* pModule, std::string obj_name);
    PyObject* ExecFunc(PyObject* pFunc, PyObject* pArgs);
    PyObject* CreateClassInstance(PyObject* pClass, PyObject* pArgs = NULL, PyObject* pKw = NULL);
    PyObject* GetObjectFromModule(std::string module_name, std::string obj_name);
    PyObject* CreateClassInstance(std::string module_name, std::string class_name, PyObject* pArgs = NULL, PyObject* pKw = NULL);
    PyObject* CallModuleFunc(std::string module_name, std::string func_name, PyObject* pArgs = NULL);
    PyObject* CallInstanceMethod(PyObject* pInstance, std::string method_name, PyObject* pArgs = NULL);

private:
    PyLoader();
	void LogPyError();

private:
    NameModuleMap _name2module;
};

}   // end of namespace CASD

#endif
