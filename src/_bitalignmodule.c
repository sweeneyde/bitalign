#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "./_bitalign.h"

typedef struct {
    void *freeblock;
    size_t size;
} bitalign_module_state;


static void *
allocate_buffer(bitalign_module_state *state, size_t size)
{
    void *block = state->freeblock;
    if (block != NULL && state->size == size) {
        state->freeblock = NULL;
        return block;
    }
    return PyMem_Malloc(size);
}

static void
free_buffer(bitalign_module_state *state, void *block, size_t size)
{
    assert(block != NULL);
    void *oldblock = state->freeblock;
    state->freeblock = block;
    state->size = size;
    if (oldblock) {
        PyMem_Free(oldblock);
    }
}

static int
bitalign_clear(PyObject *module)
{
    bitalign_module_state *state = PyModule_GetState(module);
    assert(state != NULL);
    void *block = state->freeblock;
    if (block) {
        state->freeblock = NULL;
        PyMem_Free(block);
    }
    return 0;
}

typedef struct bitalign_result (*implfunc)(void *, void *, int, void *);

static PyObject *
bitalign_helper(PyObject *self, PyObject *const *args, Py_ssize_t nargs,
                int itemsize, implfunc func)
{
    bitalign_module_state *state = PyModule_GetState(self);
    assert(state != NULL);
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
    void *buffer = allocate_buffer(state, (N + 1) * itemsize);
    if (buffer == NULL) {
        PyBuffer_Release(&a);
        PyBuffer_Release(&b);
        return PyErr_NoMemory();
    }
    struct bitalign_result res = func(a.buf, b.buf, N, buffer);
    PyBuffer_Release(&a);
    PyBuffer_Release(&b);
    free_buffer(state, buffer, N + 1);
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

static void
bitalign_free(void *module)
{
    (void)bitalign_clear((PyObject *)module);
}

PyDoc_STRVAR(bitalign_doc,"\
bitalign_#_???(arr1, arr2) --> (shift_by, num_common_bits);\n\
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
If more than one shift is optimal, the negative-most shift is used.\
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

PyDoc_STRVAR(module_doc, "some kind of module");

static struct PyModuleDef _bitalignmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "bitalign",
    .m_doc = module_doc,
    .m_size = sizeof(bitalign_module_state),
    .m_methods = bitalign_methods,
    .m_clear = bitalign_clear,
    .m_free = bitalign_free,
};

PyMODINIT_FUNC
PyInit__bitalign(void)
{
    return PyModuleDef_Init(&_bitalignmodule);
}
