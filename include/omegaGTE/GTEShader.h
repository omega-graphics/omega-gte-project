#include "GE.h"

#include "omegasl.h"

#ifndef OMEGAGTE_GTESHADER_H
#define OMEGAGTE_GTESHADER_H

_NAMESPACE_BEGIN_

struct GTEShader;

struct GTEShaderLibrary {
    std::map<std::string,SharedHandle<GTEShader>> shaders;
};

struct OMEGAGTE_EXPORT GEBufferWriter {
    virtual void setOutputBuffer(SharedHandle<GEBuffer> & buffer) = 0;
    virtual void structBegin() = 0;
    virtual void writeFloat(float & v) = 0;
    virtual void writeFloat2(FVec<2> & v) = 0;
    virtual void writeFloat3(FVec<3> & v) = 0;
    virtual void writeFloat4(FVec<4> & v) = 0;
    virtual void structEnd() = 0;
    virtual void finish() = 0;
    static SharedHandle<GEBufferWriter> Create();
};

struct OMEGAGTE_EXPORT GEBufferReader {
    virtual void setInputBuffer(SharedHandle<GEBuffer> & buffer) = 0;
    virtual void structBegin() = 0;
    virtual void getFloat(float & v) = 0;
    virtual void getFloat2(FVec<2> & v) = 0;
    virtual void getFloat3(FVec<3> & v) = 0;
    virtual void getFloat4(FVec<4> & v) = 0;
    virtual void structEnd() = 0;
    virtual void finish() = 0;
    static SharedHandle<GEBufferReader> Create();
};


_NAMESPACE_END_

#endif