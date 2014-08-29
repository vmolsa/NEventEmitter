#ifndef NEventEmitter_H
#define NEventEmitter_H

// Native Abstractions for Node.js API @ https://github.com/rvagg/nan

#include <node_modules/nan/nan.h>

#ifndef ELOG
  #include <stdarg.h>
  #include <iostream>
  
  #ifndef WITHOUT_ELOG 
    #define ELOG(...) std::cout << __VA_ARGS__ << std::endl;
  #else
    #define ELOG(...)
  #endif
#endif

using namespace v8;

class NEventEmitter {
  public:
    virtual ~NEventEmitter() {
      NEventEmitter::Dispose();
    }
    
    NAN_INLINE Local<Object> New() {
      NanScope();
      
      Wrap(NEventEmitter::NewObject());
      NanReturnValue(_obj);
    }
    
    NAN_INLINE virtual void Dispose() {
      NanScope();

      if (!_obj.IsEmpty() && !_dispose) {
          _dispose = true;
          Emit("_dispose");
          
          NanSetInternalFieldPointer(_obj, 0, NULL);
          NanDisposePersistent(_obj);
          NanDisposePersistent(_events);
      }
    }
    
    NAN_INLINE void Wrap(Handle<Object> obj) {
      NanScope();
      
      if (_obj.IsEmpty()) {
        _dispose = false;
        NanAssignPersistent(_obj, obj);
        NanAssignPersistent(_events, NanNew<Object>());
        NanSetInternalFieldPointer(_obj, 0, this);
      }
    }
    
    template <class S> NAN_INLINE static S* Unwrap(Handle<Object> obj) {
      void *retval = NanGetInternalFieldPointer(obj, 0);     
      return (retval) ? static_cast<S*>(retval) : NULL;
    }
    
    NAN_INLINE void AddPrototype(Handle<String> fn, Handle<Function> listener) {
      NanScope();
      
      Local<Value> proto_val = _obj->GetPrototype(); 
      if (proto_val->IsObject()) {
        Local<Object> proto = proto_val->ToObject();
        proto->Set(fn, listener);
      }
    }
    
    NAN_INLINE void On(Handle<String> event, Handle<Function> listener) {
      NanScope();
      
      if (_events->Get(event)->IsArray()) {
        Local<Array> listeners = Local<Array>::Cast(_events->Get(event));
        uint32_t index;

        for (index = 0; index < listeners->Length(); index++) {
          Handle<Value> listener2 = listeners->Get(index);

          if (listener->Equals(listener2)) {
            return;
          }
        }

        listeners->Set(listeners->Length(), listener);
      } else {
        Local<Array> listeners = NanNew<Array>();
        listeners->Set(0, listener);
        _events->Set(event, listeners);
      }
    }
    
    NAN_INLINE void Off(Handle<String> event, Handle<Function> listener) {
      NanScope();

      if (_events->Get(event)->IsArray()) {
        Local<Array> listeners = Local<Array>::Cast(_events->Get(event));
        uint32_t index;

        for (index = 0; index < listeners->Length(); index++) {
          Handle<Value> listener = listeners->Get(index);

          if (listener->Equals(listener)) {
            listeners->Delete(index);
            break;
          }
        }
      }
    }
    
    NAN_INLINE void Emit(const char *event) {
      NanScope();
      Emit(NanNew<String>(event), 0, NULL);
    }
    
    NAN_INLINE void Emit(Handle<String> event) {
      Emit(event, 0, NULL);
    }
    
    NAN_INLINE void Emit(Handle<String> event, Handle<Value> arg1) {
      Handle<Value> argv[1] = { arg1 };
      Emit(event, 1, argv);
    }
    
    NAN_INLINE void Emit(Handle<String> event, Handle<Value> arg1, Handle<Value> arg2) {
      Handle<Value> argv[2] = { arg1 };
      Emit(event, 2, argv);   
    }
    
    NAN_INLINE void Emit(Handle<String> event, Handle<Value> arg1, Handle<Value> arg2, Handle<Value> arg3) {
      Handle<Value> argv[3] = { arg1, arg2, arg3 };
      Emit(event, 3, argv);  
    }
    
    NAN_INLINE void Emit(Handle<String> event, int argc, Handle<Value> *argv) {
      NanScope();
      
      if (_events->Get(event)->IsArray()) {
        Local<Array> listeners = Local<Array>::Cast(_events->Get(event));

        uint32_t index;
        for (index = 0; index < listeners->Length(); index++) {
          if (listeners->Get(index)->IsFunction()) {
            Local<Function> callback = Local<Function>::Cast(listeners->Get(index));

            NanMakeCallback(_obj, callback, argc, argv);
          }
        }
      }
    }
    
    NAN_INLINE static NAN_METHOD(New) {
      NanScope();
      
      if (args.IsConstructCall()) {
		NEventEmitter *ee = new NEventEmitter();
        Local<Object> events = ee->New();
        
        NanReturnValue(events);
      } else {
          ELOG("use: new events.EventEmitter();");
          NanReturnValue(NanUndefined());
      }
      
      NanReturnValue(NanUndefined());
    }
    
  private:
    NAN_INLINE static Local<Object> NewObject() {
      NanScope();
      Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>();
      Local<ObjectTemplate> instance = tpl->InstanceTemplate();
      Local<ObjectTemplate> proto = tpl->PrototypeTemplate();
      
      tpl->SetClassName(NanNew<String>("eventEmitter"));
      
      instance->SetInternalFieldCount(1);
      
      proto->Set(NanNew<String>("on"), 
                 NanNew<FunctionTemplate>(On)->GetFunction());
      proto->Set(NanNew<String>("off"), 
                 NanNew<FunctionTemplate>(Off)->GetFunction());
      proto->Set(NanNew<String>("emit"), 
                 NanNew<FunctionTemplate>(Emit)->GetFunction());
      proto->Set(NanNew<String>("removeAllListeners"), 
                 NanNew<FunctionTemplate>(RemoveAllListeners)->GetFunction());
      proto->Set(NanNew<String>("listeners"), 
                 NanNew<FunctionTemplate>(Listeners)->GetFunction());
      proto->Set(NanNew<String>("listenerCount"), 
                 NanNew<FunctionTemplate>(ListenerCount)->GetFunction());
      proto->Set(NanNew<String>("addListener"), 
                 NanNew<FunctionTemplate>(On)->GetFunction());
      proto->Set(NanNew<String>("removeListener"), 
                 NanNew<FunctionTemplate>(Off)->GetFunction());
      proto->Set(NanNew<String>("dispose"), 
                 NanNew<FunctionTemplate>(Dispose)->GetFunction());

      NanReturnValue(instance->NewInstance());
    }  
  
    NAN_INLINE static NAN_METHOD(On) {
      NanScope();
      NEventEmitter *ee = NEventEmitter::Unwrap<NEventEmitter>(args.This());
      
      if (ee && args.Length() == 2 && args[0]->IsString() && args[1]->IsFunction()) {
        ee->On(args[0]->ToString(), Handle<Function>::Cast(args[1]));
      }
      
      NanReturnValue(NanUndefined());
    }

    NAN_INLINE static NAN_METHOD(Off) {
      NanScope();
      NEventEmitter *ee = NEventEmitter::Unwrap<NEventEmitter>(args.This());
      
      if (ee && args.Length() == 2 && args[0]->IsString() && args[1]->IsFunction()) {
        ee->Off(args[0]->ToString(), Handle<Function>::Cast(args[1]));
      }
      
      NanReturnValue(NanUndefined());
    }
    
    NAN_INLINE static NAN_METHOD(Emit) {
      NanScope();
      NEventEmitter *ee = NEventEmitter::Unwrap<NEventEmitter>(args.This());
      
      if (ee && args.Length() >= 1 && args[0]->IsString()) {
        uint32_t index, argc = args.Length() - 1;
        Persistent<Value> *argv = NULL;

        if (argc) {
          argv = new Persistent<Value>[argc];

          for (index = 0; index < argc; index++) {
            NanAssignPersistent(argv[index], args[index + 1]);
          }
        }

        ee->Emit(args[0]->ToString(), argc, argv);

        if (argc) {
          for (index = 0; index < argc; index++) {
            NanDisposePersistent(argv[index]);
          }

          delete [] argv;
        }
      }
      
      NanReturnValue(NanUndefined());    
    }
    
    NAN_INLINE static NAN_METHOD(RemoveAllListeners) {
      NanScope();
      NEventEmitter *ee = NEventEmitter::Unwrap<NEventEmitter>(args.This());
      
      if (ee && args.Length() >= 1 && args[0]->IsString() && ee->_events->Get(args[0])->IsArray()) {
        ee->_events->Delete(args[0]->ToString());
      }
      
      NanReturnValue(NanUndefined());    
    }
    
    NAN_INLINE static NAN_METHOD(Listeners) {
      NanScope();
      NEventEmitter *ee = NEventEmitter::Unwrap<NEventEmitter>(args.This());
      
      if (ee && args.Length() >= 1 && args[0]->IsString() && ee->_events->Get(args[0])->IsArray()) {
      	NanReturnValue(ee->_events->Get(args[0]));
      }
      
      NanReturnValue(NanUndefined());    
    }
    
    NAN_INLINE static NAN_METHOD(ListenerCount) {
      NanScope();
      NEventEmitter *ee = NEventEmitter::Unwrap<NEventEmitter>(args.This());
      
      if (ee && args.Length() >= 1 && args[0]->IsString() && ee->_events->Get(args[0])->IsArray()) {
        Local<Array> listeners = Local<Array>::Cast(ee->_events->Get(args[0]));
        NanReturnValue(NanNew<Number>(listeners->Length()));
      }
      
      NanReturnValue(NanNew<Number>(0));
    }
    
    NAN_INLINE static NAN_METHOD(Dispose) {
      NanScope();
      NEventEmitter *ee = NEventEmitter::Unwrap<NEventEmitter>(args.This());
      
      if (ee) { delete ee; } 
      
      NanReturnValue(NanUndefined());
    }
    
    Persistent<Object> _obj;
    Persistent<Object> _events;
    bool _dispose;
};

#endif
