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
	    //LOG_ERROR("PyLoader::Load|load %s failed", filename.c_str());
        LOG_TRACE("load %s failed", filename.c_str());
        return false;
    }
    _name2module.insert(std::make_pair(filename, pModule));
    LOG_TRACE("load %s success", filename.c_str());
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
    return ExecFunc(pFunc, pArgs);
}

PyObject* PyLoader::CallInstanceMethod(PyObject* pInstance, std::string method_name, PyObject* pArgs)
{
    if(!pInstance || method_name.empty()) return NULL;
    PyObject* pName = PyString_FromString(method_name.c_str());
    PyObject* pRes = PyObject_CallMethodObjArgs(pInstance, pName, pArgs, NULL);
    Py_DECREF(pName);
    return pRes;
}

}   // end of namespace CASD
