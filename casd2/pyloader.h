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

	typedef std::function<void(std::string)> stdout_write_type;
	struct Stdout
	{
		PyObject_HEAD
		stdout_write_type write;
	};

    ~PyLoader();

    static PyLoader& GetInstance()
    {
        static PyLoader instance;
        return instance;
    }


public:
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
	PyObject* Stdout_write(PyObject* self, PyObject* args);
	PyObject* Stdout_flush(PyObject* self, PyObject* args);
	PyMODINIT_FUNC PyInit_pyloader();
	void set_stdout(stdout_write_type write);
	void reset_stdout();

private:
    NameModuleMap _name2module;
	PyTypeObject _StdoutType;				// 为了捕获python的stdout, 自定义了一个类型
	PyObject* _stdout;						// 当前python的stdout
	PyObject* _stdout_saved;				// 保留着之前的系统的stdout

};

}   // end of namespace CASD

#endif
