#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "./_bitalign.h"

static PyObject *
bitalign_helper(PyObject *self, PyObject *const *args, Py_ssize_t nargs,
                int itemsize, implfunc func)
{
    if (nargs != 2) {
        PyErr_SetString(PyExc_TypeError,
                        "bitalign_#_xxx expected 2 arguments.");
        return NULL;
    }
    Py_buffer a, b;
    if (PyObject_GetBuffer(args[0], &a, PyBUF_ND) < 0) {
        return NULL;
    }
    if (PyObject_GetBuffer(args[1], &b, PyBUF_ND) < 0) {
        PyBuffer_Release(&a);
        return NULL;
    }
    const char *msg = NULL;
    if (a.len != b.len) {
        msg = "Buffers must have the same length.";
    }
    else if (a.itemsize != itemsize ||
             b.itemsize != itemsize) {
        msg = "Buffers have incorrect itemsize.";
    }
    else if (a.len >= INT_MAX / CHAR_BIT / 2 / itemsize) {
        msg = "Buffers are too large.";
    }
    else if (a.len <= 0) {
        msg = "Buffers cannot be empty.";
    }
    if (msg) {
        PyBuffer_Release(&a);
        PyBuffer_Release(&b);
        PyErr_SetString(PyExc_ValueError, msg);
        return NULL;
    }
    int N = (int)(a.len / itemsize);
    void *buffer = PyMem_Malloc((N + 1) * itemsize);
    if (buffer == NULL) {
        PyBuffer_Release(&a);
        PyBuffer_Release(&b);
        return PyErr_NoMemory();
    }
    struct bitalign_result res = func(a.buf, b.buf, N, buffer);
    PyBuffer_Release(&a);
    PyBuffer_Release(&b);
    PyMem_Free(buffer);
    return Py_BuildValue("(ii)", res.shift_by, res.common_bits);
}

static PyObject *
bitalign_8_lsb(PyObject *self, PyObject *const *args, Py_ssize_t nargs)
{
    return bitalign_helper(self, args, nargs,
                           sizeof(uint8_t), bitalign_impl_8lsb);
}

static PyObject *
bitalign_16_lsb(PyObject *self, PyObject *const *args, Py_ssize_t nargs)
{
    return bitalign_helper(self, args, nargs,
                           sizeof(uint16_t), bitalign_impl_16lsb);
}

static PyObject *
bitalign_32_lsb(PyObject *self, PyObject *const *args, Py_ssize_t nargs)
{
    return bitalign_helper(self, args, nargs,
                           sizeof(uint32_t), bitalign_impl_32lsb);
}

static PyObject *
bitalign_64_lsb(PyObject *self, PyObject *const *args, Py_ssize_t nargs)
{
    return bitalign_helper(self, args, nargs,
                           sizeof(uint64_t), bitalign_impl_64lsb);
}

static PyObject *
bitalign_8_msb(PyObject *self, PyObject *const *args, Py_ssize_t nargs)
{
    return bitalign_helper(self, args, nargs,
                           sizeof(uint8_t), bitalign_impl_8msb);
}

static PyObject *
bitalign_16_msb(PyObject *self, PyObject *const *args, Py_ssize_t nargs)
{
    return bitalign_helper(self, args, nargs,
                           sizeof(uint16_t), bitalign_impl_16msb);
}

static PyObject *
bitalign_32_msb(PyObject *self, PyObject *const *args, Py_ssize_t nargs)
{
    return bitalign_helper(self, args, nargs,
                           sizeof(uint32_t), bitalign_impl_32msb);
}

static PyObject *
bitalign_64_msb(PyObject *self, PyObject *const *args, Py_ssize_t nargs)
{
    return bitalign_helper(self, args, nargs,
                           sizeof(uint64_t), bitalign_impl_64msb);
}

PyDoc_STRVAR(bitalign_doc,"\
bitalign_#_?sb(arr1, arr2) --> (shift_by, num_common_bits);\n\
\n\
Return a tuple (x, y) such that when arr1 is shifted by x bits,\n\
the number of bits in common between arr1 and arr2 is y.\n\
\n\
Positive shifts indicate that arr1 needs to be shifted toward the back:\n\
\n\
    arr1 =                    -->  0001001011111111  -->\n\
    arr2 =                    1111100010010111\n\
    gives (shift_by=5, num_common_bits=11)\n\
\n\
Negative shifts indicate that arr1 needs to be shifted toward the front:\n\
\n\
    arr1 =          <--  1111100010010111  <--\n\
    arr2 =                    0001001011111111\n\
    gives (shift_by=-5, num_common_bits=11)\n\
\n\
The number (8, 16, 32, or 64) in the function name the number of bits\n\
that must be in each array entry.  'lsb'/'msb' indicates whether the\n\
0th bit of each logical bit-array is to be stored in the least or most\n\
significant bit of arr[0].\n\
\n\
If more than one shift is optimal, the negative-most shift is used.\n\
If there are no bits in common (i.e., all zeros with all ones),\n\
then (-num_bits, 0) is returned.\
");

static PyMethodDef bitalign_methods[] = {
    {"bitalign_8_lsb",  (PyCFunction)bitalign_8_lsb,  METH_FASTCALL, bitalign_doc},
    {"bitalign_16_lsb", (PyCFunction)bitalign_16_lsb, METH_FASTCALL, bitalign_doc},
    {"bitalign_32_lsb", (PyCFunction)bitalign_32_lsb, METH_FASTCALL, bitalign_doc},
    {"bitalign_64_lsb", (PyCFunction)bitalign_64_lsb, METH_FASTCALL, bitalign_doc},
    {"bitalign_8_msb",  (PyCFunction)bitalign_8_msb,  METH_FASTCALL, bitalign_doc},
    {"bitalign_16_msb", (PyCFunction)bitalign_16_msb, METH_FASTCALL, bitalign_doc},
    {"bitalign_32_msb", (PyCFunction)bitalign_32_msb, METH_FASTCALL, bitalign_doc},
    {"bitalign_64_msb", (PyCFunction)bitalign_64_msb, METH_FASTCALL, bitalign_doc},
    {NULL, NULL}
};

static struct PyModuleDef _bitalignmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "_bitalign",
    .m_methods = bitalign_methods,
};

PyMODINIT_FUNC
PyInit__bitalign(void)
{
    return PyModuleDef_Init(&_bitalignmodule);
}
