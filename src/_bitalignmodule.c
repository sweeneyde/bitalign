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
    if (block == NULL) {
        return;
    }
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

static PyObject*
to_pair(struct bitalign_result res)
{
    return Py_BuildValue("(ii)", res.shift_by, res.common_bits);
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
    free_buffer(state, buffer, (N + 1) * itemsize);
    return to_pair(res);
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

typedef void (*implfunc_multi)(void *, void *, size_t, int, void *, struct bitalign_result *);

static PyObject *
bitalign_multi_helper(PyObject *self, PyObject *const *args, Py_ssize_t nargs,
                      int itemsize, implfunc_multi func)
{
    Py_buffer a;
    PyObject *b_tuple = NULL;
    Py_buffer *pybuffers = NULL;
    void **b_bufs = NULL;
    int num_pybuffers = 0;
    PyObject *res = NULL;
    void *work_buffer = NULL;
    struct bitalign_result *result_buffer = NULL;
    PyObject *result_list = NULL;

    bitalign_module_state *state = PyModule_GetState(self);
    assert(state != NULL);
    if (nargs != 2) {
        PyErr_SetString(PyExc_TypeError,
                        "bitalign_#_xxx_multi expected 2 arguments.");
        return NULL;
    }
    if (!PyList_Check(args[1]) && !PyTuple_Check(args[1])) {
        PyErr_SetString(PyExc_TypeError,
            "bitalign_#_xxx_multi argument 2 must be a list or tuple.");
        return NULL;
    }
    if (PyObject_GetBuffer(args[0], &a, PyBUF_ND) < 0) {
        return NULL;
        // From now on, use "goto done"
    }
    if (a.len <= 0) {
        PyErr_SetString(PyExc_ValueError, "Buffer cannot be empty.");
        goto done;
    }
    if (a.len >= INT_MAX / CHAR_BIT / 2 / itemsize) {
        PyErr_SetString(PyExc_OverflowError, "Buffer is too large.");
        goto done;
    }
    b_tuple = PySequence_Tuple(args[1]);
    if (b_tuple == NULL) {
        goto done;
    }
    size_t M = PyTuple_GET_SIZE(b_tuple);
    pybuffers = PyMem_New(Py_buffer, M);
    if (pybuffers == NULL) {
        goto done;
    }
    b_bufs = PyMem_New(void *, M);
    if (b_bufs == NULL) {
        goto done;
    }
    for (; num_pybuffers < M; num_pybuffers++) {
        PyObject *b_obj = PyTuple_GET_ITEM(b_tuple, num_pybuffers);
        Py_buffer *last = &pybuffers[num_pybuffers];
        if (PyObject_GetBuffer(b_obj, last, PyBUF_ND) < 0) {
            goto done;
        }
        if (last->len != a.len) {
            PyBuffer_Release(last);
            PyErr_SetString(PyExc_ValueError,
                            "Buffers must have the same length");
            goto done;
        }
        if (last->itemsize != a.itemsize) {
            PyBuffer_Release(last);
            PyErr_SetString(PyExc_ValueError,
                            "Buffers have incorrect itemsize");
            goto done;
        }
        b_bufs[num_pybuffers] = last->buf;
    }
    int N = (int)(a.len / itemsize);
    work_buffer = allocate_buffer(state, (N + 1) * itemsize);
    if (work_buffer == NULL) {
        goto done;
    }
    result_buffer = PyMem_New(struct bitalign_result, M);
    if (result_buffer == NULL) {
        goto done;
    }
    func(a.buf, b_bufs, M, N, work_buffer, result_buffer);
    result_list = PyList_New(M);
    if (result_list == NULL) {
        goto done;
    }
    for (size_t j = 0; j < M; j++) {
        PyObject *tup = to_pair(result_buffer[j]);
        if (tup == NULL) {
            goto done;
        }
        PyList_SET_ITEM(result_list, j, tup);
    }
    res = result_list;
    Py_INCREF(res);
done:
    Py_XDECREF(result_list);
    PyMem_Free(b_bufs);
    PyMem_Free(result_buffer);
    free_buffer(state, work_buffer, (N + 1) * itemsize);
    if (pybuffers) {
        for (int k = num_pybuffers - 1; k >= 0; k--) {
            PyBuffer_Release(&pybuffers[k]);
        }
        PyMem_Free(pybuffers);
    }
    Py_XDECREF(b_tuple);
    PyBuffer_Release(&a);
    return res;
}

static PyObject *
bitalign_multi_8_lsb(PyObject *self, PyObject *const *args, Py_ssize_t nargs)
{
    return bitalign_multi_helper(self, args, nargs, sizeof(uint8_t),
                                 bitalign_multi_impl_8lsb);
}

static PyObject *
bitalign_multi_16_lsb(PyObject *self, PyObject *const *args, Py_ssize_t nargs)
{
    return bitalign_multi_helper(self, args, nargs, sizeof(uint16_t),
                                 bitalign_multi_impl_16lsb);
}

static PyObject *
bitalign_multi_32_lsb(PyObject *self, PyObject *const *args, Py_ssize_t nargs)
{
    return bitalign_multi_helper(self, args, nargs, sizeof(uint32_t),
                                 bitalign_multi_impl_32lsb);
}

static PyObject *
bitalign_multi_64_lsb(PyObject *self, PyObject *const *args, Py_ssize_t nargs)
{
    return bitalign_multi_helper(self, args, nargs, sizeof(uint64_t),
                                 bitalign_multi_impl_64lsb);
}

static PyObject *
bitalign_multi_8_msb(PyObject *self, PyObject *const *args, Py_ssize_t nargs)
{
    return bitalign_multi_helper(self, args, nargs, sizeof(uint8_t),
                                 bitalign_multi_impl_8msb);
}

static PyObject *
bitalign_multi_16_msb(PyObject *self, PyObject *const *args, Py_ssize_t nargs)
{
    return bitalign_multi_helper(self, args, nargs, sizeof(uint16_t),
                                 bitalign_multi_impl_16msb);
}

static PyObject *
bitalign_multi_32_msb(PyObject *self, PyObject *const *args, Py_ssize_t nargs)
{
    return bitalign_multi_helper(self, args, nargs, sizeof(uint32_t),
                                 bitalign_multi_impl_32msb);
}

static PyObject *
bitalign_multi_64_msb(PyObject *self, PyObject *const *args, Py_ssize_t nargs)
{
    return bitalign_multi_helper(self, args, nargs, sizeof(uint64_t),
                                 bitalign_multi_impl_64msb);
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
    {"bitalign_multi_8_lsb",  (PyCFunction)bitalign_multi_8_lsb,  METH_FASTCALL, bitalign_doc},
    {"bitalign_multi_16_lsb", (PyCFunction)bitalign_multi_16_lsb, METH_FASTCALL, bitalign_doc},
    {"bitalign_multi_32_lsb", (PyCFunction)bitalign_multi_32_lsb, METH_FASTCALL, bitalign_doc},
    {"bitalign_multi_64_lsb", (PyCFunction)bitalign_multi_64_lsb, METH_FASTCALL, bitalign_doc},
    {"bitalign_multi_8_msb",  (PyCFunction)bitalign_multi_8_msb,  METH_FASTCALL, bitalign_doc},
    {"bitalign_multi_16_msb", (PyCFunction)bitalign_multi_16_msb, METH_FASTCALL, bitalign_doc},
    {"bitalign_multi_32_msb", (PyCFunction)bitalign_multi_32_msb, METH_FASTCALL, bitalign_doc},
    {"bitalign_multi_64_msb", (PyCFunction)bitalign_multi_64_msb, METH_FASTCALL, bitalign_doc},
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
