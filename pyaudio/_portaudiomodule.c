/*
pyAudio : Python Bindings for PortAudio. 

May-2006: Supports Non-Blocking mode only

Copyright (c) 2006 Hubert Pham

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/


/* $Revision: 6 $ */

#include "Python.h"
#include "portaudio.h"
#include "_portaudiomodule.h"

#define DEFAULT_FRAMES_PER_BUFFER 1024
/* #define VERBOSE */


/************************************************************
 *
 * Table of Contents
 *
 * I. Exportable PortAudio Method Definitions
 * II. Python Object Wrappers
 *     - PaDeviceInfo
 *     - PaHostInfo
 *     - PaStream
 * III. PortAudio Method Implementations 
 *      (BLOCKING MODE ONLY!)
 *     - Initialization/Termination
 *     - HostAPI
 *     - DeviceAPI
 *     - Stream Open/Close
 *     - Stream Start/Stop/Info
 *     - Stream Read/Write
 * IV. Python Module Init
 *     - PaHostApiTypeId enum constants
 *
 ************************************************************/


/************************************************************
 *
 * I. Exportable Python Methods
 *
 ************************************************************/

static PyMethodDef paMethods[] = {
  
  /* version */
  {"get_version", pa_get_version, METH_VARARGS, "get version"},
  {"get_version_text", pa_get_version_text, METH_VARARGS, 
   "get version text"},

  /* inits */
  {"initialize", pa_initialize, METH_VARARGS, "initialize portaudio"},
  {"terminate", pa_terminate, METH_VARARGS, "terminate portaudio"},

  /* host api */
  {"get_host_api_count", pa_get_host_api_count, METH_VARARGS, 
   "get host API count"},

  {"get_default_host_api", pa_get_default_host_api, METH_VARARGS, 
   "get default host API index"},

  {"host_api_type_id_to_host_api_index", 
   pa_host_api_type_id_to_host_api_index, METH_VARARGS, 
   "get default host API index"},

  {"host_api_device_index_to_device_index", 
   pa_host_api_device_index_to_device_index,
   METH_VARARGS, 
   "get default host API index"},

  {"get_host_api_info", pa_get_host_api_info, METH_VARARGS, 
   "get host api information"},
  
  /* device api */
  {"get_device_count", pa_get_device_count, METH_VARARGS, 
   "get host API count"},

  {"get_default_input_device", pa_get_default_input_device, METH_VARARGS, 
   "get default input device index"},  

  {"get_default_output_device", pa_get_default_output_device, METH_VARARGS, 
   "get default output device index"},  

  {"get_device_info", pa_get_device_info, METH_VARARGS, 
   "get device information"},

  /* stream open/close */
  {"open", pa_open, METH_VARARGS | METH_KEYWORDS, "open port audio stream"},
  {"close", pa_close, METH_VARARGS, "close port audio stream"},
  {"get_sample_size", pa_get_sample_size, METH_VARARGS, 
   "get sample size of a format in bytes"},
  {"is_format_supported", pa_is_format_supported, 
   METH_VARARGS | METH_KEYWORDS, 
   "returns whether specified format is supported"},
   


  /* stream start/stop */
  {"start_stream", pa_start_stream, METH_VARARGS, "starts port audio stream"},
  {"stop_stream", pa_stop_stream, METH_VARARGS, "stops  port audio stream"},
  {"abort_stream", pa_abort_stream, METH_VARARGS, "aborts port audio stream"},
  {"is_stream_stopped", pa_is_stream_stopped, METH_VARARGS, 
   "returns whether stream is stopped"},
  {"is_stream_active", pa_is_stream_active, METH_VARARGS, 
   "returns whether stream is active"},
  {"get_stream_time", pa_get_stream_time, METH_VARARGS, 
   "returns stream time"},
  {"get_stream_cpu_load", pa_get_stream_cpu_load, METH_VARARGS, 
   "returns stream CPU load -- always 0 for blocking mode"},

  /* stream read/write */
  {"write_stream", pa_write_stream, METH_VARARGS, "write to stream"},  
  {"read_stream", pa_read_stream, METH_VARARGS, "read from stream"},  
  
  {"get_stream_write_available", 
   pa_get_stream_write_available, METH_VARARGS, 
   "get buffer available for writing"},  
  
  {"get_stream_read_available", 
   pa_get_stream_read_available, METH_VARARGS, 
   "get buffer available for reading"},  
  
  {NULL, NULL, 0, NULL}
};


/************************************************************
 *
 * II. Python Object Wrappers
 *
 ************************************************************/


/*************************************************************
 * PaDeviceInfo Type : Python object wrapper for PaDeviceInfo
 *************************************************************/

typedef struct {
  PyObject_HEAD
  PaDeviceInfo *devInfo;
} _pyAudio_paDeviceInfo;


/* sepcific getters into the PaDeviceInfo struct */

static PyObject *
_pyAudio_paDeviceInfo_get_structVersion(_pyAudio_paDeviceInfo *self,
					void *closure) 
{
  /* sanity check */
  if ( (!self->devInfo) ) {
    PyErr_SetString(PyExc_AttributeError, 
		    "No Device Info available");
    return NULL;
  }
      
  return PyInt_FromLong(self->devInfo->structVersion);
}

static PyObject *
_pyAudio_paDeviceInfo_get_name(_pyAudio_paDeviceInfo *self,
			      void *closure) 
{
  /* sanity check */
  if ( (!self->devInfo) || (self->devInfo->name == NULL) ) {
    PyErr_SetString(PyExc_AttributeError, 
		    "No Device Info available");
    return NULL;
  }

  return PyString_FromString(self->devInfo->name);
}

static PyObject *
_pyAudio_paDeviceInfo_get_hostApi(_pyAudio_paDeviceInfo *self,
			      void *closure) 
{
  /* sanity check */
  if ( (!self->devInfo) ) {
    PyErr_SetString(PyExc_AttributeError, 
		    "No Device Info available");
    return NULL;
  }
      
  return PyInt_FromLong(self->devInfo->hostApi);
}

static PyObject *
_pyAudio_paDeviceInfo_get_maxInputChannels(_pyAudio_paDeviceInfo *self,
					   void *closure) 
{
  /* sanity check */
  if ( (!self->devInfo) ) {
    PyErr_SetString(PyExc_AttributeError, 
		    "No Device Info available");
    return NULL;
  } 
  
  return PyInt_FromLong(self->devInfo->maxInputChannels);
}

static PyObject *
_pyAudio_paDeviceInfo_get_maxOutputChannels(_pyAudio_paDeviceInfo *self,
					    void *closure) 
{
  /* sanity check */
  if ( (!self->devInfo) ) {
    PyErr_SetString(PyExc_AttributeError, 
		    "No Device Info available");
    return NULL;
  } 
  
  return PyInt_FromLong(self->devInfo->maxOutputChannels);
}

static PyObject *
_pyAudio_paDeviceInfo_get_defaultLowInputLatency(_pyAudio_paDeviceInfo *self,
						 void *closure) 
{
  /* sanity check */
  if ( (!self->devInfo) ) {
    PyErr_SetString(PyExc_AttributeError, 
		    "No Device Info available");
    return NULL;
  } 
  
  return PyFloat_FromDouble(self->devInfo->defaultLowInputLatency);
}

static PyObject *
_pyAudio_paDeviceInfo_get_defaultLowOutputLatency(_pyAudio_paDeviceInfo *self,
						 void *closure) 
{
  /* sanity check */
  if ( (!self->devInfo) ) {
    PyErr_SetString(PyExc_AttributeError, 
		    "No Device Info available");
    return NULL;
  } 
  
  return PyFloat_FromDouble(self->devInfo->defaultLowOutputLatency);
}


static PyObject *
_pyAudio_paDeviceInfo_get_defaultHighInputLatency(_pyAudio_paDeviceInfo *self,
						 void *closure) 
{
  /* sanity check */
  if ( (!self->devInfo) ) {
    PyErr_SetString(PyExc_AttributeError, 
		    "No Device Info available");
    return NULL;
  } 
  
  return PyFloat_FromDouble(self->devInfo->defaultHighInputLatency);
}

static PyObject *
_pyAudio_paDeviceInfo_get_defaultHighOutputLatency(_pyAudio_paDeviceInfo *self,
						 void *closure) 
{
  /* sanity check */
  if ( (!self->devInfo) ) {
    PyErr_SetString(PyExc_AttributeError, 
		    "No Device Info available");
    return NULL;
  } 
  
  return PyFloat_FromDouble(self->devInfo->defaultHighOutputLatency);
}

static PyObject *
_pyAudio_paDeviceInfo_get_defaultSampleRate(_pyAudio_paDeviceInfo *self,
					    void *closure) 
{
  /* sanity check */
  if ( (!self->devInfo) ) {
    PyErr_SetString(PyExc_AttributeError, 
		    "No Device Info available");
    return NULL;
  } 
  
  return PyFloat_FromDouble(self->devInfo->defaultSampleRate);
}



static int
_pyAudio_paDeviceInfo_antiset(_pyAudio_paDeviceInfo *self,
			  PyObject *value,
			  void *closure)
{
  /* read-only: do not allow users to change values */
  PyErr_SetString(PyExc_AttributeError, 
		  "Fields read-only: cannot modify values");
  return -1;
}

static PyGetSetDef _pyAudio_paDeviceInfo_getseters[] = {
  {"name",
   (getter) _pyAudio_paDeviceInfo_get_name, 
   (setter) _pyAudio_paDeviceInfo_antiset,
   "device name",
   NULL},

  {"structVersion",
   (getter) _pyAudio_paDeviceInfo_get_structVersion, 
   (setter) _pyAudio_paDeviceInfo_antiset,
   "struct version",
   NULL},

  {"hostApi",
   (getter) _pyAudio_paDeviceInfo_get_hostApi, 
   (setter) _pyAudio_paDeviceInfo_antiset,
   "host api index",
   NULL},

  {"maxInputChannels",
   (getter) _pyAudio_paDeviceInfo_get_maxInputChannels, 
   (setter) _pyAudio_paDeviceInfo_antiset,
   "max input channels",
   NULL},

  {"maxOutputChannels",
   (getter) _pyAudio_paDeviceInfo_get_maxOutputChannels, 
   (setter) _pyAudio_paDeviceInfo_antiset,
   "max output channels",
   NULL},

  {"defaultLowInputLatency",
   (getter) _pyAudio_paDeviceInfo_get_defaultLowInputLatency, 
   (setter) _pyAudio_paDeviceInfo_antiset,
   "default low input latency",
   NULL},

  {"defaultLowOutputLatency",
   (getter) _pyAudio_paDeviceInfo_get_defaultLowOutputLatency, 
   (setter) _pyAudio_paDeviceInfo_antiset,
   "default low output latency",
   NULL},

  {"defaultHighInputLatency",
   (getter) _pyAudio_paDeviceInfo_get_defaultHighInputLatency, 
   (setter) _pyAudio_paDeviceInfo_antiset,
   "default high input latency",
   NULL},

  {"defaultHighOutputLatency",
   (getter) _pyAudio_paDeviceInfo_get_defaultHighOutputLatency, 
   (setter) _pyAudio_paDeviceInfo_antiset,
   "default high output latency",
   NULL},

  {"defaultSampleRate",
   (getter) _pyAudio_paDeviceInfo_get_defaultSampleRate, 
   (setter) _pyAudio_paDeviceInfo_antiset,
   "default sample rate",
   NULL},

  {NULL}
};

static void
_pyAudio_paDeviceInfo_dealloc(_pyAudio_paDeviceInfo* self)
{
  /* reset the pointer */
  self->devInfo = NULL;

  /* free the object */
  self->ob_type->tp_free((PyObject*) self);
}

static PyTypeObject _pyAudio_paDeviceInfoType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "_portaudio.paDeviceInfo", /*tp_name*/
    sizeof(_pyAudio_paDeviceInfo),   /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor) _pyAudio_paDeviceInfo_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "Port Audio Device Info",       /* tp_doc */
    0,  /* tp_traverse */
    0,  /* tp_clear */
    0,  /* tp_richcompare */
    0,  /* tp_weaklistoffset */
    0,  /* tp_iter */
    0,  /* tp_iternext */
    0,  /* tp_methods */
    0,  /* tp_members */
    _pyAudio_paDeviceInfo_getseters, /* tp_getset */
    0,  /* tp_base */
    0,  /* tp_dict */
    0,  /* tp_descr_get */
    0,  /* tp_descr_set */
    0,  /* tp_dictoffset */
    0,  /* tp_init */
    0,  /* tp_alloc */
    0,  /* tp_new */
};

static _pyAudio_paDeviceInfo *
_create_paDeviceInfo_object(void) 
{
  _pyAudio_paDeviceInfo *obj;

  /* don't allow subclassing? */
  obj = (_pyAudio_paDeviceInfo *) PyObject_New(_pyAudio_paDeviceInfo, 
					       &_pyAudio_paDeviceInfoType);

  /* obj = (_pyAudio_Stream*) 
     _pyAudio_StreamType.tp_alloc(&_pyAudio_StreamType, 0); */
  return obj;
}




/*************************************************************
 * PaHostApi Info Python Object
 *************************************************************/

typedef struct {
  PyObject_HEAD
  PaHostApiInfo *apiInfo;
} _pyAudio_paHostApiInfo;

/* sepcific getters into the PaDeviceInfo struct */

static PyObject *
_pyAudio_paHostApiInfo_get_structVersion(_pyAudio_paHostApiInfo *self,
					 void *closure) 
{
  /* sanity check */
  if ( (!self->apiInfo) ) {
    PyErr_SetString(PyExc_AttributeError, 
		    "No HostApi Info available");
    return NULL;
  }
      
  return PyInt_FromLong(self->apiInfo->structVersion);
}

static PyObject *
_pyAudio_paHostApiInfo_get_type(_pyAudio_paHostApiInfo *self,
				void *closure) 
{
  /* sanity check */
  if ( (!self->apiInfo) ) {
    PyErr_SetString(PyExc_AttributeError, 
		    "No HostApi Info available");
    return NULL;
  }
      
  return PyInt_FromLong((long) self->apiInfo->type);
}

static PyObject *
_pyAudio_paHostApiInfo_get_name(_pyAudio_paHostApiInfo *self,
				void *closure) 
{
  /* sanity check */
  if ( (!self->apiInfo) || (self->apiInfo->name == NULL) ) {
    PyErr_SetString(PyExc_AttributeError, 
		    "No HostApi Info available");
    return NULL;
  }

  return PyString_FromString(self->apiInfo->name);
}

static PyObject *
_pyAudio_paHostApiInfo_get_deviceCount(_pyAudio_paHostApiInfo *self,
				       void *closure) 
{
  /* sanity check */
  if ( (!self->apiInfo) ) {
    PyErr_SetString(PyExc_AttributeError, 
		    "No HostApi Info available");
    return NULL;
  }
      
  return PyInt_FromLong(self->apiInfo->deviceCount);
}

static PyObject *
_pyAudio_paHostApiInfo_get_defaultInputDevice(_pyAudio_paHostApiInfo *self,
					      void *closure) 
{
  /* sanity check */
  if ( (!self->apiInfo) ) {
    PyErr_SetString(PyExc_AttributeError, 
		    "No HostApi Info available");
    return NULL;
  }
      
  return PyInt_FromLong(self->apiInfo->defaultInputDevice);
}

static PyObject *
_pyAudio_paHostApiInfo_get_defaultOutputDevice(_pyAudio_paHostApiInfo *self,
					       void *closure) 
{
  /* sanity check */
  if ( (!self->apiInfo) ) {
    PyErr_SetString(PyExc_AttributeError, 
		    "No HostApi Info available");
    return NULL;
  }
      
  return PyInt_FromLong(self->apiInfo->defaultOutputDevice);
}


static int
_pyAudio_paHostApiInfo_antiset(_pyAudio_paDeviceInfo *self,
			       PyObject *value,
			       void *closure)
{
  /* read-only: do not allow users to change values */
  PyErr_SetString(PyExc_AttributeError, 
		  "Fields read-only: cannot modify values");
  return -1;
}

static void
_pyAudio_paHostApiInfo_dealloc(_pyAudio_paHostApiInfo* self)
{
  /* reset the pointer */
  self->apiInfo = NULL;

  /* free the object */
  self->ob_type->tp_free((PyObject*) self);
}

static PyGetSetDef _pyAudio_paHostApiInfo_getseters[] = {
  {"name",
   (getter) _pyAudio_paHostApiInfo_get_name, 
   (setter) _pyAudio_paHostApiInfo_antiset,
   "host api name",
   NULL},

  {"structVersion",
   (getter) _pyAudio_paHostApiInfo_get_structVersion, 
   (setter) _pyAudio_paHostApiInfo_antiset,
   "struct version",
   NULL},

  {"type",
   (getter) _pyAudio_paHostApiInfo_get_type,
   (setter) _pyAudio_paHostApiInfo_antiset,
   "host api type",
   NULL},

  {"deviceCount",
   (getter) _pyAudio_paHostApiInfo_get_deviceCount, 
   (setter) _pyAudio_paHostApiInfo_antiset,
   "number of devices",
   NULL},

  {"defaultInputDevice",
   (getter) _pyAudio_paHostApiInfo_get_defaultInputDevice,
   (setter) _pyAudio_paHostApiInfo_antiset,
   "default input device index",
   NULL},

  {"defaultOutputDevice",
   (getter) _pyAudio_paHostApiInfo_get_defaultOutputDevice,
   (setter) _pyAudio_paDeviceInfo_antiset,
   "default output device index",
   NULL},

  {NULL}
};

static PyTypeObject _pyAudio_paHostApiInfoType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "_portaudio.paHostApiInfo", /*tp_name*/
    sizeof(_pyAudio_paHostApiInfo),   /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor) _pyAudio_paHostApiInfo_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "Port Audio HostApi Info",       /* tp_doc */
    0,  /* tp_traverse */
    0,  /* tp_clear */
    0,  /* tp_richcompare */
    0,  /* tp_weaklistoffset */
    0,  /* tp_iter */
    0,  /* tp_iternext */
    0,  /* tp_methods */
    0,  /* tp_members */
    _pyAudio_paHostApiInfo_getseters, /* tp_getset */
    0,  /* tp_base */
    0,  /* tp_dict */
    0,  /* tp_descr_get */
    0,  /* tp_descr_set */
    0,  /* tp_dictoffset */
    0,  /* tp_init */
    0,  /* tp_alloc */
    0,  /* tp_new */
};

static _pyAudio_paHostApiInfo *
_create_paHostApiInfo_object(void) 
{
  _pyAudio_paHostApiInfo *obj;

  /* don't allow subclassing? */
  obj = (_pyAudio_paHostApiInfo *) PyObject_New(_pyAudio_paHostApiInfo, 
						&_pyAudio_paHostApiInfoType);

  /* obj = (_pyAudio_paHostApiInfo *) 
      _pyAudio_paHostApiInfoType.tp_alloc(&_pyAudio_paHostApiInfoType, 0); */
  return obj;
}


/*************************************************************
 * Stream Wrapper Python Object
 *************************************************************/

typedef struct {
  PyObject_HEAD
  PaStream *stream;
  PaStreamParameters *inputParameters;
  PaStreamParameters *outputParameters;

  /* include PaStreamInfo too! */
  PaStreamInfo *streamInfo;

  int is_open;
} _pyAudio_Stream;

static int
_is_open(_pyAudio_Stream *obj) {
  return (obj) && (obj->is_open);
}

static void
_cleanup_Stream_object(_pyAudio_Stream *streamObject)
{
  if (streamObject->stream != NULL) {
    Pa_CloseStream( streamObject->stream );
    streamObject->stream = NULL;
  }

  if (streamObject->streamInfo)
    streamObject->streamInfo = NULL;

  if (streamObject->inputParameters != NULL) {
    free( streamObject->inputParameters );
    streamObject->inputParameters = NULL;
  }

  if (streamObject->outputParameters != NULL) {
    free( streamObject->outputParameters );
    streamObject->outputParameters = NULL;
  }

  /* designate the stream as closed */
  streamObject->is_open = 0;
}

static void
_pyAudio_Stream_dealloc(_pyAudio_Stream* self)
{
  /* deallocate memory if necessary */
  _cleanup_Stream_object(self);

  /* free the object */
  self->ob_type->tp_free((PyObject*) self);
}


static PyObject *
_pyAudio_Stream_get_structVersion(_pyAudio_Stream *self,
				  void *closure) 
{
  /* sanity check */
  if (!_is_open(self)) {
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)",
				  "Stream closed",
				  paBadStreamPtr));
    return NULL;
  }

  if ( (!self->streamInfo) ) {
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)",
				  "No StreamInfo available",
				  paBadStreamPtr));
    return NULL;
  }
      
  return PyInt_FromLong(self->streamInfo->structVersion);
}

static PyObject *
_pyAudio_Stream_get_inputLatency(_pyAudio_Stream *self,
			       void *closure) 
{
  /* sanity check */
  if (!_is_open(self)) {
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)",
				  "Stream closed",
				  paBadStreamPtr));
    return NULL;
  }

  /* sanity check */
  if ( (!self->streamInfo) ) {
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)",
				  "No StreamInfo available",
				  paBadStreamPtr));
    return NULL;
  }
  
  return PyFloat_FromDouble(self->streamInfo->inputLatency);
}

static PyObject *
_pyAudio_Stream_get_outputLatency(_pyAudio_Stream *self,
			       void *closure) 
{
  /* sanity check */
  if (!_is_open(self)) {
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)",
				  "Stream closed",
				  paBadStreamPtr));
    return NULL;
  }

  /* sanity check */
  if ( (!self->streamInfo) ) {
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)",
				  "No StreamInfo available",
				  paBadStreamPtr));
    return NULL;
  }
  
  return PyFloat_FromDouble(self->streamInfo->outputLatency);
}

static PyObject *
_pyAudio_Stream_get_sampleRate(_pyAudio_Stream *self,
			       void *closure) 
{
  /* sanity check */
  if (!_is_open(self)) {
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)",
				  "Stream closed",
				  paBadStreamPtr));
    return NULL;
  }
 
  /* sanity check */
  if ( (!self->streamInfo) ) {
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)",
				  "No StreamInfo available",
				  paBadStreamPtr));
    return NULL;
  }
  
  return PyFloat_FromDouble(self->streamInfo->sampleRate);
}

static int
_pyAudio_Stream_antiset(_pyAudio_Stream *self,
			PyObject *value,
			void *closure)
{
  /* read-only: do not allow users to change values */
  PyErr_SetString(PyExc_AttributeError, 
		  "Fields read-only: cannot modify values");
  return -1;
}

static PyGetSetDef _pyAudio_Stream_getseters[] = {
  {"structVersion",
   (getter) _pyAudio_Stream_get_structVersion, 
   (setter) _pyAudio_Stream_antiset,
   "struct version",
   NULL},

  {"inputLatency",
   (getter) _pyAudio_Stream_get_inputLatency, 
   (setter) _pyAudio_Stream_antiset,
   "input latency",
   NULL},

  {"outputLatency",
   (getter) _pyAudio_Stream_get_outputLatency, 
   (setter) _pyAudio_Stream_antiset,
   "output latency",
   NULL},

  {"sampleRate",
   (getter) _pyAudio_Stream_get_sampleRate, 
   (setter) _pyAudio_Stream_antiset,
   "sample rate",
   NULL},

  {NULL}
};

static PyTypeObject _pyAudio_StreamType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "_portaudio.Stream",       /*tp_name*/
    sizeof(_pyAudio_Stream),         /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor) _pyAudio_Stream_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "Port Audio Stream",       /* tp_doc */
    0,  /* tp_traverse */
    0,  /* tp_clear */
    0,  /* tp_richcompare */
    0,  /* tp_weaklistoffset */
    0,  /* tp_iter */
    0,  /* tp_iternext */
    0,  /* tp_methods */
    0,  /* tp_members */
    _pyAudio_Stream_getseters, /* tp_getset */
    0,  /* tp_base */
    0,  /* tp_dict */
    0,  /* tp_descr_get */
    0,  /* tp_descr_set */
    0,  /* tp_dictoffset */
    0,  /* tp_init */
    0,  /* tp_alloc */
    0,  /* tp_new */
};

static _pyAudio_Stream *
_create_Stream_object(void) 
{
  _pyAudio_Stream *obj;

  /* don't allow subclassing? */
  obj = (_pyAudio_Stream *) PyObject_New(_pyAudio_Stream, 
					 &_pyAudio_StreamType);

  /* obj = (_pyAudio_Stream*) 
     _pyAudio_StreamType.tp_alloc(&_pyAudio_StreamType, 0); */
  return obj;
}


/************************************************************
 *
 * III. PortAudio Method Implementations
 *
 ************************************************************/

/*************************************************************
 * Version Info
 *************************************************************/

static PyObject *
pa_get_version(PyObject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, ""))
    return NULL;

  return PyInt_FromLong(Pa_GetVersion());
}

static PyObject *
pa_get_version_text(PyObject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, ""))
    return NULL;

  return PyString_FromString(Pa_GetVersionText());
}

/*************************************************************
 * Initialization/Termination
 *************************************************************/

static PyObject *
pa_initialize(PyObject *self, PyObject *args)
{
  int err;
  err = Pa_Initialize();
  if (err != paNoError) {
    Pa_Terminate();
#ifdef VERBOSE
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
#endif
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)", 				  
				  Pa_GetErrorText(err), err));
    return NULL;
  }

  Py_INCREF(Py_None);
  return Py_None;

}

static PyObject *
pa_terminate(PyObject *self, PyObject *args)
{
  Pa_Terminate();
  Py_INCREF(Py_None);
  return Py_None;
}

/*************************************************************
 * HostAPI
 *************************************************************/

static PyObject *
pa_get_host_api_count(PyObject *self, PyObject *args)
{
  PaHostApiIndex count;

  if (!PyArg_ParseTuple(args, ""))
    return NULL;

  count = Pa_GetHostApiCount();
  if (count < 0) {
#ifdef VERBOSE
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", count );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( count ) );
#endif
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)", 
				  Pa_GetErrorText(count), count) );

    return NULL;
  }

  return PyInt_FromLong(count);
}

static PyObject *
pa_get_default_host_api(PyObject *self, PyObject *args)
{
  PaHostApiIndex index;

  if (!PyArg_ParseTuple(args, ""))
    return NULL;

  index = Pa_GetDefaultHostApi();
  if (index < 0) {
#ifdef VERBOSE
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", index );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( index ) );
#endif
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)", 				  
				  Pa_GetErrorText(index), index));
    return NULL;
  }

  return PyInt_FromLong(index);
}

static PyObject *
pa_host_api_type_id_to_host_api_index(PyObject *self, PyObject *args)
{
  PaHostApiTypeId typeid;
  PaHostApiIndex index;

  if (!PyArg_ParseTuple(args, "i", &typeid))
    return NULL;

  index = Pa_HostApiTypeIdToHostApiIndex(typeid);

  if (index < 0) {
    /* if (index != paHostApiNotFound)
       Pa_Terminate(); */

#ifdef VERBOSE
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", index );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( index ) );
#endif

    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)", 				  
				  Pa_GetErrorText(index), index));
    return NULL;
  }

  return PyInt_FromLong(index);
}

static PyObject *
pa_host_api_device_index_to_device_index(PyObject *self, PyObject *args)
{
  PaHostApiIndex apiIndex;
  int hostApiDeviceindex;
  PaDeviceIndex devIndex;


  if (!PyArg_ParseTuple(args, "ii", &apiIndex, &hostApiDeviceindex))
    return NULL;

  devIndex = Pa_HostApiDeviceIndexToDeviceIndex(apiIndex, hostApiDeviceindex);
  if (devIndex < 0) {

    /* if (devIndex != paInvalidHostApi && devIndex != paInvalidDevice)
       Pa_Terminate(); */

#ifdef VERBOSE
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", devIndex );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( devIndex ) );
#endif

    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)", 				  
				  Pa_GetErrorText(devIndex), devIndex));

    return NULL;
  }

  return PyInt_FromLong(devIndex);
}

static PyObject *
pa_get_host_api_info(PyObject *self, PyObject *args)
{
  PaHostApiIndex index;
  PaHostApiInfo* _info;
  _pyAudio_paHostApiInfo* py_info;

  if (!PyArg_ParseTuple(args, "i", &index))
    return NULL;
  
  _info = (PaHostApiInfo *) Pa_GetHostApiInfo(index);
  
  if (!_info) {
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)", 				  
				  "Invalid host api info", 
				  paInvalidHostApi));
    return NULL;
  }
  
  py_info = _create_paHostApiInfo_object();
  py_info->apiInfo = _info;
  
  return (PyObject *) py_info;
}



/*************************************************************
 * Device API 
 *************************************************************/

static PyObject *
pa_get_device_count(PyObject *self, PyObject *args)
{
  PaDeviceIndex count;

  if (!PyArg_ParseTuple(args, ""))
    return NULL;

  count = Pa_GetDeviceCount();
  if (count < 0) {
    /*
    Pa_Terminate();
    */
#ifdef VERBOSE   
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", count );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( count ) );
#endif
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)", 				  
				  Pa_GetErrorText(count), count));
    return NULL;
  }

  return PyInt_FromLong(count);
}

static PyObject *
pa_get_default_input_device(PyObject *self, PyObject *args)
{
  PaDeviceIndex index;

  if (!PyArg_ParseTuple(args, ""))
    return NULL;

  index = Pa_GetDefaultInputDevice();
  if (index == paNoDevice) {    
    /* fprintf( stderr, "An error occured while using the portaudio stream\n" );
       fprintf( stderr, "No Default Input Device Available\n" ); */
    PyErr_SetString(PyExc_IOError, "No Default Input Device Available");
    return NULL;
  } else if (index < 0) {
    /* Pa_Terminate(); */
#ifdef VERBOSE
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", index );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( index ) );
#endif
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)", 				  
				  Pa_GetErrorText(index), index));
    return NULL;
  }

  return PyInt_FromLong(index);
}

static PyObject *
pa_get_default_output_device(PyObject *self, PyObject *args)
{
  PaDeviceIndex index;

  if (!PyArg_ParseTuple(args, ""))
    return NULL;

  index = Pa_GetDefaultOutputDevice();
  if (index == paNoDevice) {    
    /* fprintf( stderr, "An error occured while using the portaudio stream\n" ); */
    /* fprintf( stderr, "No Default Output Device Available\n" ); */
    PyErr_SetString(PyExc_IOError, "No Default Output Device Available");
    return NULL;
  } else if (index < 0) {
    /* Pa_Terminate(); */
#ifdef VERBOSE
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", index );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( index ) );
#endif
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)", 				  
				  Pa_GetErrorText(index), index));
    return NULL;
  }

  return PyInt_FromLong(index);
}

static PyObject *
pa_get_device_info(PyObject *self, PyObject *args)
{
  PaDeviceIndex index;
  PaDeviceInfo* _info;
  _pyAudio_paDeviceInfo* py_info;

  if (!PyArg_ParseTuple(args, "i", &index))
    return NULL;
  
  _info = (PaDeviceInfo* )Pa_GetDeviceInfo(index);
  
  if (!_info) {
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)", 			
				  "Invalid device info", paInvalidDevice));
				 
    return NULL;
  }
  
  py_info = _create_paDeviceInfo_object();
  py_info->devInfo = _info;
  
  return (PyObject *) py_info;
}

/*************************************************************
 * Stream Open / Close / Supported
 *************************************************************/

static PyObject *
pa_open(PyObject *self, PyObject *args, PyObject *kwargs)
{
  
  int rate, channels;
  int input, output, frames_per_buffer;
  int input_device_index = -1;
  int output_device_index = -1;
  PyObject *input_device_index_arg = NULL;
  PyObject *output_device_index_arg = NULL;
  PaSampleFormat format;
  PaError err;

  /* default to neither output nor input */
  input = 0;
  output = 0;

  frames_per_buffer = DEFAULT_FRAMES_PER_BUFFER;

  /* pass in rate, channel, width */
  static char *kwlist[] = {"rate", 
			   "channels", 
			   "format", 
			   "input", 
			   "output", 
			   "input_device_index",
			   "output_device_index",
			   "frames_per_buffer", 
			   NULL};
			   
  
  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "iii|iiOOi", kwlist, 
				   &rate, &channels, &format, 
				   &input, &output,
				   &input_device_index_arg,
				   &output_device_index_arg,
				   &frames_per_buffer))
    
    return NULL;

  
  /* check to see if device indices were specified */
  if ( (input_device_index_arg == NULL) || 
       (input_device_index_arg == Py_None)) {    

#ifdef VERBOSE
    printf("Using default input device\n");
#endif

    input_device_index = -1;

  } else { 

    if (!PyInt_Check(input_device_index_arg)) {
      PyErr_SetString(PyExc_ValueError, 
		      "input_device_index must be integer (or None)");
      return NULL;
    }

    input_device_index = PyInt_AsLong(input_device_index_arg);

#ifdef VERBOSE
    printf("Using input device index number: %d\n", input_device_index);
#endif

  }

  if ((output_device_index_arg == NULL) ||
      (output_device_index_arg == Py_None) ) {    

#ifdef VERBOSE
    printf("Using default output device\n");
#endif

    output_device_index = -1;

  } else {

    if (!PyInt_Check(output_device_index_arg)) {
      PyErr_SetString(PyExc_ValueError, 
		      "output_device_index must be integer (or None)");
      return NULL;
    }

    output_device_index = PyInt_AsLong(output_device_index_arg);

#ifdef VERBOSE
    printf("Using output device index number: %d\n", output_device_index);
#endif

  }

  /* sanity checks */
  if (input == 0 && output == 0) {
    PyErr_SetString(PyExc_ValueError, "Must specify either input or output");
    return NULL;
  }

  /* old API */
/*   if (width == 1) { */
/*     if (unsigned_8bit)  */
/*       format = paUInt8; */
/*     else */
/*       format = paInt8; */
/*   } */
/*   else if (width == 2) */
/*     format = paInt16; */
/*   else if (width == 4) */
/*     format = paFloat32; */
/*   else { */
/*     PyErr_SetString(PyExc_ValueError, "Invalid audio width"); */
/*     return NULL; */
/*   } */

  if (channels < 1) {
    PyErr_SetString(PyExc_ValueError, "Invalid audio channels");
    return NULL;
  }

  PaStreamParameters *outputParameters = NULL;
  PaStreamParameters *inputParameters = NULL;
 
  if (output) {
    outputParameters = 
      (PaStreamParameters *) malloc( sizeof(PaStreamParameters) ); 


    if (output_device_index < 0)
      /* default output device */
      outputParameters->device = Pa_GetDefaultOutputDevice(); 
    else
      outputParameters->device = output_device_index;
    
    /* final check -- ensure that there is a default device */
    if (outputParameters->device < 0) {
      free(outputParameters);
      PyErr_SetObject(PyExc_IOError, 
		      Py_BuildValue("(s,i)", 				  
				    "Invalid output device (no default output device)", 
				    paInvalidDevice));

      return NULL;
    }

    outputParameters->channelCount = channels;
    outputParameters->sampleFormat = format;
    outputParameters->suggestedLatency = 
      Pa_GetDeviceInfo( outputParameters->device )->defaultLowOutputLatency;
    outputParameters->hostApiSpecificStreamInfo = NULL;
  }

  if (input) {
    inputParameters = 
      (PaStreamParameters *) malloc( sizeof(PaStreamParameters) ); 

    if (input_device_index < 0)
      /* default output device */
      inputParameters->device = Pa_GetDefaultInputDevice(); 
    else
      inputParameters->device = input_device_index;

    /* final check -- ensure that there is a default device */
    if (inputParameters->device < 0) {
      free(inputParameters);
      PyErr_SetObject(PyExc_IOError, 
		      Py_BuildValue("(s,i)", 				  
				    "Invalid input device (no default output device)", 
				    paInvalidDevice));
      return NULL;
    }

    inputParameters->channelCount = channels;
    inputParameters->sampleFormat = format;
    inputParameters->suggestedLatency = 
      Pa_GetDeviceInfo( inputParameters->device )->defaultLowInputLatency;
    inputParameters->hostApiSpecificStreamInfo = NULL;
  }

  PaStream *stream = NULL;
  PaStreamInfo *streamInfo = NULL;

  err = Pa_OpenStream(&stream,
		      /* input/output parameters */
		      /* NULL values are ignored */
		      inputParameters,
		      outputParameters,
		      /* Samples Per Second */
		      rate, 
		      /* allocate frames in the buffer */
		      frames_per_buffer,    
		      /* we won't output out of range samples 
			 so don't bother clipping them */
		      paClipOff,      
		      /* no callback, use blocking API */
		      NULL,   
		      /* no callback, so no callback userData */
		      NULL ); 

  if( err != paNoError ) {
    /* Pa_Terminate(); */

#ifdef VERBOSE
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
#endif

    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)", 				  
				  Pa_GetErrorText(err), err));
    return NULL;
  }

  streamInfo = (PaStreamInfo *) Pa_GetStreamInfo(stream);
  if (!streamInfo) {
    /* Pa_Terminate(); */
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)",
 				  "Could not get stream information",
				  paInternalError));
    return NULL;
  }

  _pyAudio_Stream *streamObject = _create_Stream_object();
  streamObject->stream = stream;
  streamObject->inputParameters = inputParameters;
  streamObject->outputParameters = outputParameters;
  streamObject->is_open = 1;
  streamObject->streamInfo = streamInfo;

  return (PyObject *) streamObject;
}

static PyObject *
pa_close(PyObject *self, PyObject *args)
{
  
  PyObject *stream_arg;
  _pyAudio_Stream *streamObject;
  
  if (!PyArg_ParseTuple(args, "O!", &_pyAudio_StreamType, &stream_arg))
    return NULL;

  streamObject = (_pyAudio_Stream *) stream_arg;
  
  _cleanup_Stream_object(streamObject);

  Py_INCREF(Py_None);
  return Py_None;  
}

static PyObject *
pa_get_sample_size(PyObject *self, PyObject *args)
{
  PaSampleFormat format;
  int size_in_bytes;

  if (!PyArg_ParseTuple(args, "i", &format))
    return NULL;

  size_in_bytes = Pa_GetSampleSize(format);

  if (size_in_bytes < 0) {
    PyErr_SetObject(PyExc_ValueError, 
		    Py_BuildValue("(s,i)",
				  Pa_GetErrorText(size_in_bytes),
				  size_in_bytes));
    return NULL;    
  }
    
  return PyInt_FromLong(size_in_bytes);
}


static PyObject *
pa_is_format_supported(PyObject *self, PyObject *args,
		       PyObject *kwargs)
{
  /* pass in rate, channel, width */
  static char *kwlist[] = {"sample_rate",
			   "input_device",
			   "input_channels",
			   "input_format",
			   "output_device",
			   "output_channels",
			   "output_format",
			   NULL};
			   
  int input_device, input_channels;
  int output_device, output_channels;
  float sample_rate;
  PaStreamParameters inputParams;
  PaStreamParameters outputParams;
  PaSampleFormat input_format, output_format;
  PaError error;

  input_device = input_channels = 
    output_device = output_channels = -1;

  input_format = output_format = -1;

  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "f|iiiiii", kwlist,
				   &sample_rate, 
				   &input_device, 
				   &input_channels,
				   &input_format,
				   &output_device,
				   &output_channels,
				   &output_format))
    
    return NULL;

  if (!(input_device < 0)) {
    inputParams.device = input_device;
    inputParams.channelCount = input_channels;
    inputParams.sampleFormat = input_format;    
    inputParams.suggestedLatency = 0;
    inputParams.hostApiSpecificStreamInfo = NULL;
  }
  
  if (!(output_device < 0)) {
    outputParams.device = output_device;
    outputParams.channelCount = output_channels;
    outputParams.sampleFormat = output_format;
    outputParams.suggestedLatency = 0;
    outputParams.hostApiSpecificStreamInfo = NULL;
  }

  error = Pa_IsFormatSupported((input_device < 0) ? NULL : &inputParams,
			       (output_device < 0) ? NULL : &outputParams,
			       sample_rate);

  if (error == paFormatIsSupported) {
    Py_INCREF(Py_True);
    return Py_True;
  } else if (error == paInvalidSampleRate) {
    Py_INCREF(Py_False);
    return Py_False;
  } else {
    PyErr_SetObject(PyExc_ValueError, 
		    Py_BuildValue("(s,i)",
				  Pa_GetErrorText(error),
				  error));
    return NULL;
  }

}

/*************************************************************
 * Stream Start / Stop / Info
 *************************************************************/

static PyObject *
pa_start_stream(PyObject *self, PyObject *args)
{

  int err;
  PyObject *stream_arg;
  _pyAudio_Stream *streamObject;
  if (!PyArg_ParseTuple(args, "O!", &_pyAudio_StreamType, &stream_arg))
    return NULL;
  streamObject = (_pyAudio_Stream *) stream_arg;
  
  if (!_is_open(streamObject)) {
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)",
				  "Stream closed",
				  paBadStreamPtr));
    return NULL;
  }

  PaStream *stream = streamObject->stream;

  if ( ((err = Pa_StartStream( stream )) != paNoError) &&
       (err != paStreamIsNotStopped) ) {    
    _cleanup_Stream_object(streamObject);
    /* Pa_Terminate(); */
#ifdef VERBOSE
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
#endif

    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)",
				  Pa_GetErrorText(err),
				  err));
    return NULL;
  }
  
  Py_INCREF(Py_None);
  return Py_None;

}

static PyObject *
pa_stop_stream(PyObject *self, PyObject *args)
{

  int err;
  PyObject *stream_arg;
  _pyAudio_Stream *streamObject;
  if (!PyArg_ParseTuple(args, "O!", &_pyAudio_StreamType, &stream_arg))
    return NULL;
  streamObject = (_pyAudio_Stream *) stream_arg;
    
  if (!_is_open(streamObject)) {
    PyErr_SetString(PyExc_IOError, "Stream not open");
    return NULL;
  }

  PaStream *stream = streamObject->stream;

  if ( ((err = Pa_StopStream( stream )) != paNoError)  &&
       (err != paStreamIsStopped) ) {
    
    _cleanup_Stream_object(streamObject);
    /* Pa_Terminate(); */
#ifdef VERBOSE
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
#endif

    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)",
				  Pa_GetErrorText(err),
				  err));
    return NULL;
  }

  Py_INCREF(Py_None);
  return Py_None;

}

static PyObject *
pa_abort_stream(PyObject *self, PyObject *args)
{

  int err;
  PyObject *stream_arg;
  _pyAudio_Stream *streamObject;
  if (!PyArg_ParseTuple(args, "O!", &_pyAudio_StreamType, &stream_arg))
    return NULL;
  streamObject = (_pyAudio_Stream *) stream_arg;
    
  if (!_is_open(streamObject)) {
    PyErr_SetString(PyExc_IOError, "Stream not open");
    return NULL;
  }

  PaStream *stream = streamObject->stream;

  if ( ((err = Pa_AbortStream( stream )) != paNoError) &&
       (err != paStreamIsStopped) ) {
    _cleanup_Stream_object(streamObject);
    /* Pa_Terminate(); */

#ifdef VERBOSE
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
#endif

    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)",
				  Pa_GetErrorText(err),
				  err));
    return NULL;
  }


  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *
pa_is_stream_stopped(PyObject *self, PyObject *args)
{

  int err;
  PyObject *stream_arg;
  _pyAudio_Stream *streamObject;
  if (!PyArg_ParseTuple(args, "O!", &_pyAudio_StreamType, &stream_arg))
    return NULL;
  streamObject = (_pyAudio_Stream *) stream_arg;
    
  if (!_is_open(streamObject)) {
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)",
				  "Stream closed",
				  paBadStreamPtr));
    return NULL;
  }

  PaStream *stream = streamObject->stream;

  if ((err = Pa_IsStreamStopped( stream )) < 0) {
    _cleanup_Stream_object(streamObject);
    /* Pa_Terminate(); */

#ifdef VERBOSE
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
#endif

    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)",
				  Pa_GetErrorText(err),
				  err));
    return NULL;
  }

  if (err) {
    Py_INCREF(Py_True);
    return Py_True;  
  }

  Py_INCREF(Py_False);
  return Py_False;
}

static PyObject *
pa_is_stream_active(PyObject *self, PyObject *args)
{

  int err;
  PyObject *stream_arg;
  _pyAudio_Stream *streamObject;
  if (!PyArg_ParseTuple(args, "O!", &_pyAudio_StreamType, &stream_arg))
    return NULL;
  streamObject = (_pyAudio_Stream *) stream_arg;
    
  if (!_is_open(streamObject)) {
    PyErr_SetString(PyExc_IOError, "Stream not open");
    return NULL;
  }

  PaStream *stream = streamObject->stream;

  if ((err = Pa_IsStreamActive( stream )) < 0) {
    _cleanup_Stream_object(streamObject);
    /* Pa_Terminate(); */
#ifdef VERBOSE
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
#endif 

    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)",
				  Pa_GetErrorText(err),
				  err));

    return NULL;
  }

  if (err) {
    Py_INCREF(Py_True);
    return Py_True;  
  }

  Py_INCREF(Py_False);
  return Py_False;
}

static PyObject *
pa_get_stream_time(PyObject *self, PyObject *args)
{

  double time;
  PyObject *stream_arg;
  _pyAudio_Stream *streamObject;
  if (!PyArg_ParseTuple(args, "O!", &_pyAudio_StreamType, &stream_arg))
    return NULL;
  streamObject = (_pyAudio_Stream *) stream_arg;
    
  if (!_is_open(streamObject)) {
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)",
				  "Stream closed",
				  paBadStreamPtr));
    return NULL;
  }

  PaStream *stream = streamObject->stream;

  if ((time = Pa_GetStreamTime( stream )) == 0) {
    _cleanup_Stream_object(streamObject);
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)",
				  "Internal Error",
				  paInternalError));
    return NULL;
  }

  return PyFloat_FromDouble(time);

}

static PyObject *
pa_get_stream_cpu_load(PyObject *self, PyObject *args)
{

  PyObject *stream_arg;
  _pyAudio_Stream *streamObject;
  if (!PyArg_ParseTuple(args, "O!", &_pyAudio_StreamType, &stream_arg))
    return NULL;
  streamObject = (_pyAudio_Stream *) stream_arg;
    
  if (!_is_open(streamObject)) {
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)",
				  "Stream closed",
				  paBadStreamPtr));
    return NULL;
  }

  PaStream *stream = streamObject->stream;

  return PyFloat_FromDouble(Pa_GetStreamCpuLoad(stream));

}


/*************************************************************
 * Stream Read/Write
 *************************************************************/

static PyObject *
pa_write_stream(PyObject *self, PyObject *args)
{
  const char *data;
  int total_size;
  int total_frames;
  int err;
  int should_throw_exception = 0;

  PyObject *stream_arg;
  _pyAudio_Stream *streamObject;

  if (!PyArg_ParseTuple(args, "O!s#i|i", &_pyAudio_StreamType, &stream_arg, 
			&data, &total_size, &total_frames, 
			&should_throw_exception))
    return NULL;

  /* make sure total frames is larger than 0 */
  if (total_frames < 0) {
    PyErr_SetString(PyExc_ValueError, 
		    "Invalid number of frames");
    return NULL;
  }

  streamObject = (_pyAudio_Stream *) stream_arg;

  if (!_is_open(streamObject)) {
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)",
				  "Stream closed",
				  paBadStreamPtr));
    return NULL;
  }

  PaStream *stream = streamObject->stream;

  if ((err = Pa_WriteStream( stream, data, total_frames )) != paNoError) {

    if (err == paOutputUnderflowed) {
      if (should_throw_exception) 
	goto error;
    } else
      goto error;
  }

  Py_INCREF(Py_None);
  return Py_None;

 error:
  /* cleanup */
  _cleanup_Stream_object(streamObject);
  /* Pa_Terminate(); */
#ifdef VERBOSE
  fprintf( stderr, "An error occured while using the portaudio stream\n" );
  fprintf( stderr, "Error number: %d\n", err );
  fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
#endif
  PyErr_SetObject(PyExc_IOError, 
		  Py_BuildValue("(s,i)",
				Pa_GetErrorText(err),
				err));
  return NULL;
}

static PyObject *
pa_read_stream(PyObject *self, PyObject *args)
{
  int err;
  int total_frames;
  short *sampleBlock;
  int num_bytes;
  PyObject *rv;

  PyObject *stream_arg;
  _pyAudio_Stream *streamObject;

  if (!PyArg_ParseTuple(args, "O!i", &_pyAudio_StreamType, &stream_arg, 
			&total_frames))
    return NULL;

  /* make sure value is positive! */
  if (total_frames < 0) {
    PyErr_SetString(PyExc_ValueError, "Invalid number of frames");
    return NULL;
  }

  streamObject = (_pyAudio_Stream *) stream_arg;

  if (!_is_open(streamObject)) {
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)",
				  "Stream closed",
				  paBadStreamPtr));
    return NULL;
  }

  PaStream *stream = streamObject->stream;
  PaStreamParameters *inputParameters = streamObject->inputParameters;  
  num_bytes = (total_frames) * (inputParameters->channelCount) * 
    (Pa_GetSampleSize(inputParameters->sampleFormat));

#ifdef VERBOSE
  fprintf(stderr, "Allocating %d bytes\n", num_bytes); 
#endif

  rv = PyString_FromStringAndSize(NULL, num_bytes);
  sampleBlock = (short *) PyString_AsString(rv);
  
  if (sampleBlock == NULL) {
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)",
				  "Out of memory",
				  paInsufficientMemory));
    return NULL;
  }

  err = Pa_ReadStream(stream, sampleBlock, total_frames);

  if (err) {

    /* ignore input overflow and output underflow */

    if( err & paInputOverflowed ) {
#ifdef VERBOSE     
      fprintf( stderr, "Input Overflow.\n" );
#endif
    }
    else if( err & paOutputUnderflowed ) {
#ifdef VERBOSE
      fprintf( stderr, "Output Underflow.\n" );
#endif
    }
    else {
      /* clean up */
      _cleanup_Stream_object(streamObject);
      /* Pa_Terminate(); */
    }
           
    /* free the string buffer */
    Py_XDECREF(rv);
      
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)", 				  
				  Pa_GetErrorText(err), err));
      
    return NULL;
  }

  return rv;
}

static PyObject *
pa_get_stream_write_available(PyObject *self, PyObject *args) 
{
  signed long frames;
  PyObject *stream_arg;
  _pyAudio_Stream *streamObject;
  if (!PyArg_ParseTuple(args, "O!", &_pyAudio_StreamType, &stream_arg))
    return NULL;
  streamObject = (_pyAudio_Stream *) stream_arg;

  if (!_is_open(streamObject)) {
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)",
				  "Stream closed",
				  paBadStreamPtr));
    return NULL;
  }

  PaStream *stream = streamObject->stream;

  frames = Pa_GetStreamWriteAvailable(stream);
  
  return PyInt_FromLong(frames);  
}

static PyObject *
pa_get_stream_read_available(PyObject *self, PyObject *args) 
{
  signed long frames;
  PyObject *stream_arg;
  _pyAudio_Stream *streamObject;
  if (!PyArg_ParseTuple(args, "O!", &_pyAudio_StreamType, &stream_arg))
    return NULL;
  streamObject = (_pyAudio_Stream *) stream_arg;

  if (!_is_open(streamObject)) {
    PyErr_SetObject(PyExc_IOError, 
		    Py_BuildValue("(s,i)",
				  "Stream closed",
				  paBadStreamPtr));
    return NULL;
  }

  PaStream *stream = streamObject->stream;

  frames = Pa_GetStreamReadAvailable(stream);
  return PyInt_FromLong(frames);  
}


/************************************************************
 *
 * IV. Python Module Init
 *
 ************************************************************/

PyMODINIT_FUNC
init_portaudio(void)
{
  PyObject* m;

  _pyAudio_StreamType.tp_new = PyType_GenericNew;
  if (PyType_Ready(&_pyAudio_StreamType) < 0)
    return;
  
  _pyAudio_paDeviceInfoType.tp_new = PyType_GenericNew;
  if (PyType_Ready(&_pyAudio_paDeviceInfoType) < 0)
    return;

  _pyAudio_paHostApiInfoType.tp_new = PyType_GenericNew;
  if (PyType_Ready(&_pyAudio_paHostApiInfoType) < 0)
    return;

  m = Py_InitModule("_portaudio", paMethods);

  Py_INCREF(&_pyAudio_StreamType);
  Py_INCREF(&_pyAudio_paDeviceInfoType);
  Py_INCREF(&_pyAudio_paHostApiInfoType);

  /* Add PortAudio constants */

  /* host apis */
  PyModule_AddIntConstant(m, "paInDevelopment", paInDevelopment);
  PyModule_AddIntConstant(m, "paDirectSound", paDirectSound);
  PyModule_AddIntConstant(m, "paMME", paMME);
  PyModule_AddIntConstant(m, "paASIO", paASIO);
  PyModule_AddIntConstant(m, "paSoundManager", paSoundManager);
  PyModule_AddIntConstant(m, "paCoreAudio", paCoreAudio);
  PyModule_AddIntConstant(m, "paOSS", paOSS);
  PyModule_AddIntConstant(m, "paALSA", paALSA);
  PyModule_AddIntConstant(m, "paAL", paAL);
  PyModule_AddIntConstant(m, "paBeOS", paBeOS);
  PyModule_AddIntConstant(m, "paWDMKS", paWDMKS);
  PyModule_AddIntConstant(m, "paJACK", paJACK);
  PyModule_AddIntConstant(m, "paWASAPI", paWASAPI);
  PyModule_AddIntConstant(m, "paNoDevice", paNoDevice);

  /* formats */
  PyModule_AddIntConstant(m, "paFloat32", paFloat32);
  PyModule_AddIntConstant(m, "paInt32", paInt32);
  PyModule_AddIntConstant(m, "paInt24", paInt24);
  PyModule_AddIntConstant(m, "paInt16", paInt16);
  PyModule_AddIntConstant(m, "paInt8", paInt8);
  PyModule_AddIntConstant(m, "paUInt8", paUInt8);
  PyModule_AddIntConstant(m, "paCustomFormat", paCustomFormat);
  
  /* error codes */
  PyModule_AddIntConstant(m, "paNoError", paNoError);
  PyModule_AddIntConstant(m, "paNotInitialized", paNotInitialized);
  PyModule_AddIntConstant(m, "paUnanticipatedHostError", 
			  paUnanticipatedHostError);
  PyModule_AddIntConstant(m, "paInvalidChannelCount", 
			  paInvalidChannelCount);
  PyModule_AddIntConstant(m, "paInvalidSampleRate", 
			  paInvalidSampleRate);
  PyModule_AddIntConstant(m, "paInvalidDevice", paInvalidDevice);
  PyModule_AddIntConstant(m, "paInvalidFlag", paInvalidFlag);
  PyModule_AddIntConstant(m, "paSampleFormatNotSupported", 
			  paSampleFormatNotSupported);
  PyModule_AddIntConstant(m, "paBadIODeviceCombination", 
			  paBadIODeviceCombination);
  PyModule_AddIntConstant(m, "paInsufficientMemory", 
			  paInsufficientMemory);
  PyModule_AddIntConstant(m, "paBufferTooBig", paBufferTooBig);
  PyModule_AddIntConstant(m, "paBufferTooSmall", paBufferTooSmall);
  PyModule_AddIntConstant(m, "paNullCallback", paNullCallback);
  PyModule_AddIntConstant(m, "paBadStreamPtr", paBadStreamPtr);
  PyModule_AddIntConstant(m, "paTimedOut", paTimedOut);
  PyModule_AddIntConstant(m, "paInternalError", paInternalError);
  PyModule_AddIntConstant(m, "paDeviceUnavailable", paDeviceUnavailable);
  PyModule_AddIntConstant(m, "paIncompatibleHostApiSpecificStreamInfo", 
			  paIncompatibleHostApiSpecificStreamInfo);
  PyModule_AddIntConstant(m, "paStreamIsStopped", paStreamIsStopped);
  PyModule_AddIntConstant(m, "paStreamIsNotStopped", paStreamIsNotStopped);
  PyModule_AddIntConstant(m, "paInputOverflowed", paInputOverflowed);
  PyModule_AddIntConstant(m, "paOutputUnderflowed", paOutputUnderflowed);
  PyModule_AddIntConstant(m, "paHostApiNotFound", paHostApiNotFound);
  PyModule_AddIntConstant(m, "paInvalidHostApi", paInvalidHostApi);
  PyModule_AddIntConstant(m, "paCanNotReadFromACallbackStream", 
			  paCanNotReadFromACallbackStream);
  PyModule_AddIntConstant(m, "paCanNotWriteToACallbackStream", 
			  paCanNotWriteToACallbackStream);
  PyModule_AddIntConstant(m, "paCanNotReadFromAnOutputOnlyStream", 
			  paCanNotReadFromAnOutputOnlyStream);
  PyModule_AddIntConstant(m, "paCanNotWriteToAnInputOnlyStream",  
			  paCanNotWriteToAnInputOnlyStream);
  PyModule_AddIntConstant(m, "paIncompatibleStreamHostApi", 
			  paIncompatibleStreamHostApi);

}
