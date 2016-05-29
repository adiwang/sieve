#include "pyloader.h"
#include "log.h"

namespace CASD
{

PyLoader::PyLoader()
{
}

PyLoader::~PyLoader()
{
    for(NameModuleMapIter it = _name2module.begin();
        it != _name2module.end();
        ++it)
    {
        if(it->second)
        {
            Py_DECREF(it->second);
            it->second = NULL;
        }
    }
    _name2module.clear();
    Py_Finalize();
}

bool PyLoader::Init(std::string script_dir)
{
    Py_Initialize();
    if(!Py_IsInitialized())
        return false;
    static PyMethodDef RedirectionMethods[] = 
    {
        {"stdoutredirect", redirection_stdoutredirect, METH_VARARGS,
        "stdout redirection helper"},
        {NULL, NULL, 0, NULL}

    };
    Py_InitModule("redirection", RedirectionMethods);
    PyRun_SimpleString("\
import redirection\n\
import sys\n\
class StdoutCatcher:\n\
    def write(self, stuff):\n\
        redirection.stdoutredirect(stuff)\n\
sys.stdout = StdoutCatcher()");

    // 添加当前路径和script路径
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./')");
    PyRun_SimpleString("sys.path.append('./script/')");
    if(!script_dir.empty())
    {
        char append_str[256] = {0};
        snprintf(append_str, 256, "sys.path.append('%s')", script_dir.c_str());
        PyRun_SimpleString(append_str);
    }
	LogPyError();
    return true;
}

bool PyLoader::Load(std::string filename)
{
    if(filename.empty()) return false;
    NameModuleMapIter it = _name2module.find(filename);
    if(it != _name2module.end() && it->second)
    {
        // 已加载
        LOG_TRACE("%s already loadded", filename.c_str());
        return true;
    }
    
    PyObject* pName = PyString_FromString(filename.c_str());
    PyObject* pModule = PyImport_Import(pName);
    Py_DECREF(pName);
    if(!pModule)
    {
        // 未找到该文件      
        LOG_TRACE("load %s failed", filename.c_str());
        return false;
    }
    _name2module.insert(std::make_pair(filename, pModule));
    LOG_TRACE("load %s success", filename.c_str());
	LogPyError();
    return true;
}

PyObject* PyLoader::GetModule(std::string module_name)
{
    if(module_name.empty()) return NULL;
    NameModuleMapIter it = _name2module.find(module_name);
    if(it != _name2module.end())
        return it->second;
    return NULL;
}

PyObject* PyLoader::GetObjectFromModule(PyObject* pModule, std::string obj_name)
{
    if(!pModule || obj_name.empty()) return NULL;
    /*
    PyObject* pObj = PyObject_GetAttrString(pModule, obj_name.c_str());
    return pObj;
    */
    PyObject* pDict = PyModule_GetDict(pModule);
    if(!pDict) return NULL;
    PyObject* pObj = PyDict_GetItemString(pDict, obj_name.c_str());
    return pObj;
}

PyObject* PyLoader::ExecFunc(PyObject* pFunc, PyObject* pArgs)
{
    if(!pFunc || !PyCallable_Check(pFunc)) return NULL;
    return PyObject_CallObject(pFunc, pArgs);
}

PyObject* PyLoader::CreateClassInstance(PyObject* pClass, PyObject* pArgs, PyObject* pKw)
{
    if(!pClass) return NULL;
    return PyInstance_New(pClass, pArgs, pKw);
}

PyObject* PyLoader::GetObjectFromModule(std::string module_name, std::string obj_name)
{
    if(module_name.empty() || obj_name.empty()) return NULL;
    PyObject* pModule = GetModule(module_name);
    if(!pModule) return NULL;
    return GetObjectFromModule(pModule, obj_name);
}

PyObject* PyLoader::CreateClassInstance(std::string module_name, std::string class_name, PyObject* pArgs, PyObject* pKw)
{
    if(module_name.empty() || class_name.empty()) return NULL;
    PyObject* pModule = GetModule(module_name);
    if(!pModule) return NULL;
    PyObject* pClass = GetObjectFromModule(pModule, class_name);
    if(!pClass) return NULL;
    return CreateClassInstance(pClass, pArgs, pKw);
}

PyObject* PyLoader::CallModuleFunc(std::string module_name, std::string func_name, PyObject* pArgs)
{
    if(module_name.empty() || func_name.empty()) return NULL;
    PyObject* pModule = GetModule(module_name);
    if(!pModule) return NULL;
    PyObject* pFunc = GetObjectFromModule(pModule, func_name);
    PyObject* pRes = ExecFunc(pFunc, pArgs);
	LogPyError();
	return pRes;

}

PyObject* PyLoader::CallInstanceMethod(PyObject* pInstance, std::string method_name, PyObject* pArgs)
{
    if(!pInstance || method_name.empty()) return NULL;
    PyObject* pName = PyString_FromString(method_name.c_str());
    PyObject* pRes = PyObject_CallMethodObjArgs(pInstance, pName, pArgs, NULL);
	LogPyError();
    return pRes;
}

void PyLoader::LogPyError()
{
	if(PyErr_Occurred() == NULL) return;
	// Python equivilant:
    // import traceback, sys
    // return "".join(traceback.format_exception(sys.exc_type,
    //    sys.exc_value, sys.exc_traceback))

    PyObject *type, *value, *traceback;
    PyObject *tracebackModule;
    char *chrRetval = NULL;

    PyErr_Fetch(&type, &value, &traceback);

    tracebackModule = PyImport_ImportModule("traceback");
    if (tracebackModule != NULL)
    {
        PyObject *tbList, *emptyString, *strRetval;

        char format_exception_name[] = "format_exception";
        char format_str[] = "OOO";

        tbList = PyObject_CallMethod(
            tracebackModule,
            format_exception_name,
            format_str,
            type,
            value == NULL ? Py_None : value,
            traceback == NULL ? Py_None : traceback);

        emptyString = PyString_FromString("");

        char join_method_name[] = "join";
        char join_format_str[] = "O";
        strRetval = PyObject_CallMethod(emptyString, join_method_name, join_format_str, tbList);

        chrRetval = strdup(PyString_AsString(strRetval));

        Py_DECREF(tbList);
        Py_DECREF(emptyString);
        Py_DECREF(strRetval);
        Py_DECREF(tracebackModule);
    }
    else
    {
        chrRetval = strdup("Unable to import traceback module.");
    }

    Py_DECREF(type);
    Py_XDECREF(value);
    Py_XDECREF(traceback);

	if(chrRetval != NULL)
	{
		LOG_TRACE("%s", chrRetval);
		free(chrRetval);
		chrRetval = NULL;
	}
}


PyObject* redirection_stdoutredirect(PyObject *self, PyObject *args)
{
    const char *string = NULL;
    if(!PyArg_ParseTuple(args, "s", &string))
        return NULL;
    if(string && strlen(string))
    {
        LOG_TRACE("python output|%s", string);
    }
    Py_INCREF(Py_None);
    return Py_None;
}

}   // end of namespace CASD
