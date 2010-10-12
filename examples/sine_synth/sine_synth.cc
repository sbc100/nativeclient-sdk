// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include <ppapi/cpp/dev/audio_dev.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/dev/scriptable_object_deprecated.h>
#include <ppapi/cpp/var.h>

#include <cmath>
#include <limits>
#include <sstream>

namespace {
const char* const kPlaySoundId = "playSound";
const char* const kStopSoundId = "stopSound";
const char* const kFrequencyId = "frequency";
const double kPi = 3.141592653589;
const uint32_t kSampleCount = 4096;
}  // namespace

namespace sine_synth {
// This class exposes the scripting interface for this NaCl module.  The
// HasMethod method is called by the browser when executing a method call on
// the |sineSynth| object (see, e.g. the moduleDidLoad() function in
// sine_synth.html).  The name of the JavaScript function (e.g. "playSound") is
// passed in the |method| paramter as a string pp::Var.  If HasMethod()
// returns |true|, then the browser will call the Call() method to actually
// invoke the method.
class SineSynthScriptableObject : public pp::deprecated::ScriptableObject {
 public:
  // Single parameter ctor that recives a weak reference to an audio device.
  explicit SineSynthScriptableObject(pp::Audio_Dev* audio)
      : pp::deprecated::ScriptableObject(), audio_(audio), frequency_(440) {}
  // Return |true| if |method| is one of the exposed method names.
  virtual bool HasMethod(const pp::Var& method, pp::Var* exception);
  // Return |true| if |property| is one of the exposed properties.
  virtual bool HasProperty(const pp::Var& property, pp::Var* exception);

  // Invoke the function associated with |method|.  The argument list passed in
  // via JavaScript is marshaled into a vector of pp::Vars.  None of the
  // functions in this example take arguments, so this vector is always empty.
  virtual pp::Var Call(const pp::Var& method,
                       const std::vector<pp::Var>& args,
                       pp::Var* exception);
  // Assign the value of the property associated with |property|.  |value| is
  // the new value of the property.
  virtual void SetProperty(const pp::Var& property,
                           const pp::Var& value,
                           pp::Var* exception);

  double frequency() const { return frequency_; }
  void set_frequency(double frequency) {
    frequency_ = frequency;
  }

 private:
  bool PlaySound() {
    audio_->StartPlayback();
    return true;
  }
  bool StopSound() {
    audio_->StopPlayback();
    return true;
  }

  pp::Audio_Dev* const audio_;  // weak
  double frequency_;
};

pp::Var SineSynthScriptableObject::Call(const pp::Var& method,
                                        const std::vector<pp::Var>& args,
                                        pp::Var* exception) {
  if (!method.is_string()) {
    return pp::Var();
  }
  const std::string method_name = method.AsString();
  if (method_name == kPlaySoundId) {
    return pp::Var(PlaySound());
  } else if (method_name == kStopSoundId) {
    return pp::Var(StopSound());
  } else {
    *exception = std::string("No method named ") + method_name;
  }
  return pp::Var();
}

bool SineSynthScriptableObject::HasMethod(const pp::Var& method,
                                          pp::Var* exception) {
  if (!method.is_string()) {
    return false;
  }
  const std::string method_name = method.AsString();
  const bool has_method = method_name == kPlaySoundId ||
      method_name == kStopSoundId;
  return has_method;
}

bool SineSynthScriptableObject::HasProperty(const pp::Var& property,
                                            pp::Var* exception) {
  if (!property.is_string()) {
    return false;
  }
  const std::string property_name = property.AsString();
  const bool has_property = (property_name == kFrequencyId);
  return has_property;
}

void SineSynthScriptableObject::SetProperty(const pp::Var& property,
                                            const pp::Var& value,
                                            pp::Var* exception) {
  if (!property.is_string()) {
    *exception = "Expected a property name of string type.";
  }
  std::string property_name = property.AsString();
  if (property_name == kFrequencyId) {
    // The value could come to us as an int32_t or a double, so we use pp::Var's
    // is_number function which returns true when it's either kind of number.
    if (value.is_number()) {
      // And we get the value as an int32_t, and pp::Var does the conversion for
      // us, if one is necessary.
      set_frequency(value.AsDouble());
      return;
    } else if (value.is_string()) {
      // We got the value as a string.  We'll try to convert it to a number.
      std::istringstream stream(value.AsString());
      double double_value;
      if (stream >> double_value) {
        set_frequency(double_value);
        return;
      } else {
        std::string error_msg("Expected a number value for ");
        error_msg += kFrequencyId;
        error_msg += ".  Instead, got a non-numeric string: ";
        error_msg += value.AsString();
        *exception = error_msg;
        return;
      }
      *exception = std::string("Expected a number value for ") + kFrequencyId;
      return;
    }
  }
  *exception = std::string("No property named ") + property_name;
}

// The Instance class.  One of these exists for each instance of your NaCl
// module on the web page.  The browser will ask the Module object to create
// a new Instance for each occurence of the <embed> tag that has these
// attributes:
//     type="application/x-ppapi-nacl-srpc"
//     nexes="ARM: sine_synth_arm.nexe
//            ..."
// The Instance can return a ScriptableObject representing itself.  When the
// browser encounters JavaScript that wants to access the Instance, it calls
// the GetInstanceObject() method.  All the scripting work is done though
// the returned ScriptableObject.
class SineSynthInstance : public pp::Instance {
 public:
  explicit SineSynthInstance(PP_Instance instance)
      : pp::Instance(instance), audio_time_(0), scriptable_object_(NULL) {}
  virtual ~SineSynthInstance() {}

  // The pp::Var takes over ownership of the SineSynthScriptableObject.
  virtual pp::Var GetInstanceObject() {
    scriptable_object_ =
        new SineSynthScriptableObject(&audio_);
    return pp::Var(scriptable_object_);
  }
  virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]) {
    uint32_t obtained_sample_count = 0;
    audio_ = pp::Audio_Dev(
        *this,
        pp::AudioConfig_Dev(PP_AUDIOSAMPLERATE_44100,
                            kSampleCount,
                            &obtained_sample_count),
        SineWaveCallback, this);
    return true;
  }

 private:
  static void SineWaveCallback(void* samples, size_t buffer_size, void* data) {
    SineSynthInstance* sine_synth_instance =
        reinterpret_cast<SineSynthInstance*>(data);
    size_t& t = sine_synth_instance->audio_time_;
    const double frequency =
        sine_synth_instance->scriptable_object_->frequency();
    const double theta = 2 * kPi * frequency / PP_AUDIOSAMPLERATE_44100;
    uint16_t* buff = reinterpret_cast<uint16_t*>(samples);
    const size_t channel_count = 2;  // stereo
    size_t j = 0;
    for (size_t k = 0; k < kSampleCount; ++k) {
      for (size_t m = 0; m < channel_count && j < buffer_size; ++m, ++j) {
        *buff++ = static_cast<uint16_t>(std::sin(theta * t++) *
                                        std::numeric_limits<uint16_t>::max());
      }
    }
  }
  // Audio resource. Allocated in Init()
  pp::Audio_Dev audio_;
  // Audio buffer time. Used to prevent sine wave skips on buffer
  // boundaries.
  size_t audio_time_;
  SineSynthScriptableObject* scriptable_object_;
};

// The Module class.  The browser calls the CreateInstance() method to create
// an instance of you NaCl module on the web page.  The browser creates a new
// instance for each <embed> tag with type="application/x-ppapi-nacl-srpc".
class SineSynthModule : public pp::Module {
 public:
  SineSynthModule() : pp::Module() {}
  ~SineSynthModule() {}

  // Create and return a HelloWorldInstance object.
  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new SineSynthInstance(instance);
  }
};

}  // namespace sine_synth

// Factory function called by the browser when the module is first loaded.
// The browser keeps a singleton of this module.  It calls the
// CreateInstance() method on the object you return to make instances.  There
// is one instance per <embed> tag on the page.  This is the main binding
// point for your NaCl module with the browser.
namespace pp {
Module* CreateModule() {
  return new sine_synth::SineSynthModule();
}
}  // namespace pp
