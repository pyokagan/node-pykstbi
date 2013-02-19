#include <node.h>
#include <node_buffer.h>
#include <iostream>
#include <cstring>

#define STBI_HEADER_FILE_ONLY
#include "stb_image.c"

using namespace v8;
using namespace node;

#define JS_METHOD(name) Handle<Value> name(const Arguments &args)
#define JS_GET(name) Handle<Value> name(Local<String> property, const AccessorInfo &info)
#define JS_SET(name)  void name(Local<String> property, Local<Value> value, const AccessorInfo &info)

inline int sizeOfArrayElementForType(v8::ExternalArrayType type) {
    switch (type) {
        case v8::kExternalByteArray:
        case v8::kExternalUnsignedByteArray:
            return 1;
        case v8::kExternalShortArray:
        case v8::kExternalUnsignedShortArray:
            return 2;
        case v8::kExternalIntArray:
        case v8::kExternalUnsignedIntArray:
        case v8::kExternalFloatArray:
            return 4;
        default:
            return 0;
    }
}

template<typename Type>
Type* getArrayData(Local<Value> arg, int* len = NULL, int *element_size = NULL) {
    Type *data=NULL;
    if(len) *len=0;
    if (element_size) *element_size = 0;

    if(!arg->IsNull()) {
        if(arg->IsArray()) {
            Local<Array> arr = Array::Cast(*arg);
            if(len) *len=arr->Length();
            if (element_size) *element_size = sizeOfArrayElementForType(arr->GetIndexedPropertiesExternalArrayDataType());
            data = reinterpret_cast<Type*>(arr->GetIndexedPropertiesExternalArrayData());
        }
        else if(arg->IsObject()) {
            if(len) *len= arg->ToObject()->GetIndexedPropertiesExternalArrayDataLength();
            if(element_size) *element_size = sizeOfArrayElementForType(arg->ToObject()->GetIndexedPropertiesExternalArrayDataType());
            data = reinterpret_cast<Type*>(arg->ToObject()->GetIndexedPropertiesExternalArrayData());
        }
        else
            ThrowException(String::New("Bad array argument"));
    }
    return data;
}


namespace stbi {

    JS_METHOD(loadFromMemory) {
        // object loadFromMemory(buffer, req_comp = 0)
        // stbi_uc *stbi_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp);

        HandleScope scope;
        int len;
        stbi_uc *buffer = getArrayData<stbi_uc>(args[0], &len);
        int req_comp = args[1]->IsInt32() ? args[1]->Int32Value() : 0;

        int outwidth, outheight, outcomp;
        stbi_uc *outbuffer = stbi_load_from_memory(buffer, len, &outwidth, &outheight, &outcomp, req_comp);
        if (outbuffer == NULL) {
            // Parsing failed for some reason. Return Null.
            return scope.Close(Null());
        }

        int bufferlen = outheight * outwidth * (req_comp == 0 ? outcomp : req_comp) * sizeof(stbi_uc);
        node::Buffer *outBuffer = node::Buffer::New(bufferlen);
        std::memcpy(node::Buffer::Data(outBuffer), outbuffer, bufferlen);


        Local<Object> globalObj = Context::GetCurrent()->Global();
        v8::Local<v8::Function> bufferConstructor = v8::Local<v8::Function>::Cast(globalObj->Get(v8::String::New("Buffer")));
        const unsigned argc = 3;
        Handle<Value> argv[argc] = {outBuffer->handle_, v8::Integer::New(Buffer::Length(outBuffer)), v8::Integer::New(0)};
        v8::Local<v8::Object> outoutBuffer = bufferConstructor->NewInstance(argc, argv);

        // Create output object
        Local<Object> outObj = Object::New();
        outObj->Set(String::NewSymbol("data"), outoutBuffer);
        outObj->Set(String::NewSymbol("width"), Integer::New(outwidth));
        outObj->Set(String::NewSymbol("height"), Integer::New(outheight));
        outObj->Set(String::NewSymbol("comp"), Integer::New(outcomp));

        stbi_image_free(outbuffer);
        return scope.Close(outObj);
    }

}

#define M(name) NODE_SET_METHOD(target, #name, stbi::name)

void InitModule(Handle<Object> target) {
    M(loadFromMemory);
}

NODE_MODULE(pykstbi, InitModule);
