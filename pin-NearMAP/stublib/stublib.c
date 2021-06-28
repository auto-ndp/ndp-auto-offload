#ifndef NO_PYTHON
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#endif

void __attribute__((noinline)) pinnearmap_phase(const char *name) { asm(""); }

#ifndef NO_PYTHON
static PyObject *pypinnearmap_phase(PyObject *self, PyObject *args) {
  const char *name;
  if (!PyArg_ParseTuple(args, "s", &name))
    return NULL;
  pinnearmap_phase(name);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyMethodDef PythonMethods[] = {
    {"pinnearmap_phase", pypinnearmap_phase, METH_VARARGS,
     "Message about a program phase change to the instrumentation tool"},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef pinnearmapmodule = {
    PyModuleDef_HEAD_INIT, "pinnearmap", /* name of module */
    "PIN NearMAP",                       /* module documentation, may be NULL */
    -1, /* size of per-interpreter state of the module,
           or -1 if the module keeps state in global variables. */
    PythonMethods};

PyMODINIT_FUNC PyInit_pinnearmap(void) {
  return PyModule_Create(&pinnearmapmodule);
}
#endif
