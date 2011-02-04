// Copyright 2010 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include <ppapi/cpp/audio.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/dev/scriptable_object_deprecated.h>
#include <ppapi/cpp/var.h>

#include <cassert>
#include <cmath>
#include <limits>
#include <sstream>

namespace {
const char* const kPlaySoundId = "playSound";
const char* const kStopSoundId = "stopSound";
const char* const kFrequencyId = "frequency";
const double kPi = 3.141592653589;
const double kTwoPi = 2.0 * kPi;
// The sample count we will request.
const uint32_t kSampleFrameCount = 4096u;
// Only supporting stereo audio for now.
const uint32_t kChannels = 2u;
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
  explicit SineSynthScriptableObject(pp::Audio* audio)
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
  // Return the value of the property associated with the name |property|.
  virtual pp::Var GetProperty(const pp::Var& property,
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

  pp::Audio* const audio_;  // weak
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
    return;
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

pp::Var SineSynthScriptableObject::GetProperty(const pp::Var& property,
                                               pp::Var* exception) {
  if (!property.is_string()) {
    *exception = "Expected a property name of string type.";
    return pp::Var();
  }
  std::string property_name = property.AsString();
  if (property_name == kFrequencyId) {
    return pp::Var(frequency());
  }
  *exception = std::string("No property named ") + property_name;
  return pp::Var();
}

// The Instance class.  One of these exists for each instance of your NaCl
// module on the web page.  The browser will ask the Module object to create
// a new Instance for each occurence of the <embed> tag that has these
// attributes:
//     type="application/x-nacl"
//     nacl="sine_synth.nmf"
//
// The Instance can return a ScriptableObject representing itself.  When the
// browser encounters JavaScript that wants to access the Instance, it calls
// the GetInstanceObject() method.  All the scripting work is done though
// the returned ScriptableObject.
class SineSynthInstance : public pp::Instance {
 public:
  explicit SineSynthInstance(PP_Instance instance)
      : pp::Instance(instance), theta_(0), scriptable_object_(NULL),
        sample_frame_count_(kSampleFrameCount) {}
  virtual ~SineSynthInstance() {}

  // The pp::Var takes over ownership of the SineSynthScriptableObject.
  virtual pp::Var GetInstanceObject() {
    scriptable_object_ =
        new SineSynthScriptableObject(&audio_);
    return pp::Var(this, scriptable_object_);
  }
  virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]) {
    // Ask the device for an appropriate sample count size.
    sample_frame_count_ =
        pp::AudioConfig::RecommendSampleFrameCount(PP_AUDIOSAMPLERATE_44100,
                                                   kSampleFrameCount);
    audio_ = pp::Audio(
        this,
        pp::AudioConfig(this,
                        PP_AUDIOSAMPLERATE_44100,
                        sample_frame_count_),
        SineWaveCallback, this);
    return true;
  }

 private:
  static void SineWaveCallback(void* samples, size_t buffer_size, void* data) {
    SineSynthInstance* sine_synth_instance =
        reinterpret_cast<SineSynthInstance*>(data);
    const double frequency =
        sine_synth_instance->scriptable_object_->frequency();
    const double delta = kTwoPi * frequency / PP_AUDIOSAMPLERATE_44100;
    const int16_t max_int16 = std::numeric_limits<int16_t>::max();

    int16_t* buff = reinterpret_cast<int16_t*>(samples);

    // Make sure we can't write outside the buffer.
    assert(buffer_size >= (sizeof(*buff) * kChannels *
                           sine_synth_instance->sample_frame_count_));

    for (size_t sample_i = 0;
         sample_i < sine_synth_instance->sample_frame_count_;
         ++sample_i, sine_synth_instance->theta_ += delta) {
      // Keep theta_ from going beyond 2*Pi.
      if (sine_synth_instance->theta_ > kTwoPi) {
        sine_synth_instance->theta_ -= kTwoPi;
      }
      double sin_value(std::sin(sine_synth_instance->theta_));
      int16_t scaled_value = static_cast<int16_t>(sin_value * max_int16);
      for (size_t channel = 0; channel < kChannels; ++channel) {
        *buff++ = scaled_value;
      }
    }
  }
  // Audio resource. Allocated in Init()
  pp::Audio audio_;

  // The last parameter sent to the sin function.  Used to prevent sine wave
  // skips on buffer boundaries.
  double theta_;

  SineSynthScriptableObject* scriptable_object_;

  // The count of sample frames per channel in an audio buffer.
  uint32_t sample_frame_count_;
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
