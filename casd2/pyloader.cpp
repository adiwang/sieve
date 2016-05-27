#include "pyloader.h"
#include "log.h"

namespace CASD
{

PyLoader::PyLoader() : _stdout(NULL), _stdout_saved(NULL)
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
	PyImport_AppendInittab("pyloader", PyInit_pyloader);
    Py_Initialize();
    if(!Py_IsInitialized())
        return false;
	PyImport_ImportModule("pyloader");
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
	    //LOG_ERROR("PyLoader::Load|load %s failed", filename.c_str());
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
	// catch python stdout
	std::string buffer;
	stdout_write_type write = [&buffer] (std::string s) { buffer += s; };
    set_stdout(write);
    PyObject* pRes = ExecFunc(pFunc, pArgs);
	reset_stdout();
	if(!buffer.empty())
	{
		LOG_TRACE("python output: %s", buffer.c_str());
	}
	LogPyError();
	return pRes;

}

PyObject* PyLoader::CallInstanceMethod(PyObject* pInstance, std::string method_name, PyObject* pArgs)
{
    if(!pInstance || method_name.empty()) return NULL;
    PyObject* pName = PyString_FromString(method_name.c_str());
	// catch python stdout
	std::string buffer;
	stdout_write_type write = [&buffer] (std::string s) { buffer += s; };
    set_stdout(write);
    PyObject* pRes = PyObject_CallMethodObjArgs(pInstance, pName, pArgs, NULL);
	Py_DECREF(pName);
	reset_stdout();
	if(!buffer.empty())
	{
		LOG_TRACE("python output: %s", buffer.c_str());
	}
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

PyObject* PyLoader::Stdout_write(PyObject* self, PyObject* args)
{
    std::size_t written(0);
    Stdout* selfimpl = reinterpret_cast<Stdout*>(self);
    if (selfimpl->write)
    {
        char* data;
        if (!PyArg_ParseTuple(args, "s", &data))
            return 0;

        std::string str(data);
        selfimpl->write(str);
        written = str.size();
    }
    return PyLong_FromSize_t(written);
}

PyObject* PyLoader::Stdout_flush(PyObject* self, PyObject* args)
{
    // no-op
    return Py_BuildValue("");
}

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC PyObject*
#endif
PyMODINIT_FUNC PyLoader::PyInit_pyloader() 
{
    _stdout = 0;
    _stdout_saved = 0;

	PyMethodDef Stdout_methods[] =
	{
		{"write", Stdout_write, METH_VARARGS, "sys.stdout.write"},
		{"flush", Stdout_flush, METH_VARARGS, "sys.stdout.write"},
		{0, 0, 0, 0} // sentinel
	};
	
	_StdoutType = 
	{
		PyObject_HEAD_INIT(NULL)
		0,									/*ob_size*/
		"pyloader.StdoutType",				/*tp_name*/
		sizeof(Stdout),						/*tp_basicsize*/
		0,									/*tp_itemsize*/
		0,									/*tp_dealloc*/
		0,									/*tp_print*/
		0,									/*tp_getattr*/
		0,									/*tp_setattr*/
		0,									/*tp_compare*/
		0,									/*tp_repr*/
		0,									/*tp_as_number*/
		0,									/*tp_as_sequence*/
		0,									/*tp_as_mapping*/
		0,									/*tp_hash */
		0,									/*tp_call*/
		0,									/*tp_str*/
		0,									/*tp_getattro*/
		0,									/*tp_setattro*/
		0,									/*tp_as_buffer*/
		Py_TPFLAGS_DEFAULT,					/*tp_flags*/
		"pyloader.Stdout objects",			/* tp_doc */
		0,									/* tp_traverse */
		0,									/* tp_clear */
		0,									/* tp_richcompare */
		0,									/* tp_weaklistoffset */
		0,									/* tp_iter */
		0,									/* tp_iternext */
		Stdout_methods,						/* tp_methods */
		0,									/* tp_members */
		0,									/* tp_getset */
		0,									/* tp_base */
		0,									/* tp_dict */
		0,									/* tp_descr_get */
		0,									/* tp_descr_set */
		0,									/* tp_dictoffset */
		0,									/* tp_init */
		0,									/* tp_alloc */
		0,									/* tp_new */
	};

	_StdoutType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&_StdoutType) < 0)
        return 0;

	PyModuleDef pyloader_module =
	{
		PyModuleDef_HEAD_INIT,
		"pyloader", 0, -1, 0,
	};

    PyObject* m = PyModule_Create(&pyloader_module);
    if (m)
    {
        Py_INCREF(&_StdoutType);
        PyModule_AddObject(m, "Stdout", reinterpret_cast<PyObject*>(&_StdoutType));
    }
    return m;
}

void PyLoader::set_stdout(stdout_write_type write)
{
    if (!_stdout)
    {
        _stdout_saved = PySys_GetObject("stdout"); // borrowed
        _stdout = StdoutType.tp_new(&_StdoutType, 0, 0);
    }

    Stdout* impl = reinterpret_cast<Stdout*>(_stdout);
    impl->write = write;
    PySys_SetObject("stdout", _stdout);    
}

void PyLoader::reset_stdout()
{
    if (_stdout_saved)
        PySys_SetObject("stdout", _stdout_saved);

    Py_XDECREF(_stdout);
    _stdout = 0;
}

}   // end of namespace CASD
