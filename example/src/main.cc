#include <NEventEmitter.h>

#include <string.h>
#include <iostream>

class NativeTest : public NEventEmitter {
  public:
    NAN_INLINE static NAN_METHOD(Test) {
      NanScope();
      
      if (args.IsConstructCall()) {
        NativeTest *nv = new NativeTest();
        Local<Object> retval = nv->New();

        nv->On(NanNew<String>("open"), NanNew<FunctionTemplate>(onOpen)->GetFunction());
        nv->On(NanNew<String>("_dispose"), NanNew<FunctionTemplate>(onDispose)->GetFunction());

        nv->AddPrototype(NanNew<String>("newMsg"), NanNew<FunctionTemplate>(NewMsg)->GetFunction());

        NanAssignPersistent(nv->msg, NanNew<String>("Hello From Native Module!"));

        NanReturnValue(retval);
      }
      
      NanReturnValue(NanUndefined());
    }
    
  private:
    NAN_INLINE static NAN_METHOD(NewMsg) {
      NanScope();
      NanReturnValue(NanNew<String>("Message from C++"));
    }
  
    NAN_INLINE static NAN_METHOD(onOpen) {
      NanScope();
      NativeTest *nv = NativeTest::Unwrap<NativeTest>(args.This());

      if (nv) {
        nv->Emit(NanNew<String>("close"), nv->msg);
      }
      
      NanReturnValue(NanUndefined());
    }
    
    NAN_INLINE static NAN_METHOD(onDispose) {
      NanScope();
      NativeTest *nv = NativeTest::Unwrap<NativeTest>(args.This());

      if (nv) {
        ELOG("NativeTest: " << "Dispose()");
        
        NanDisposePersistent(nv->msg);
      }
      
      NanReturnValue(NanUndefined());
    }
    
    Persistent<String> msg;
};

void InitAll(Handle<Object> exports, Handle<Object> module) {
  NanScope();
  
  exports->Set(NanNew<String>("native"), NanNew<FunctionTemplate>(NativeTest::Test)->GetFunction());
  exports->Set(NanNew<String>("eventEmitter"), NanNew<FunctionTemplate>(NEventEmitter::New)->GetFunction());
}

NODE_MODULE(NEventEmitter, InitAll)