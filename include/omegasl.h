

#ifndef omegasl_h
#define  omegasl_h

typedef const char * CString;

struct omegasl_lib_entry {
    CString name;
    const char *data;
    CString *shaders;
    unsigned shadersN;
};
     
#endif