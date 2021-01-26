#pragma once
#include "../rtAudio/RtAudio.h"

#include <vector>
struct SystemDevice {
  const RtAudio::DeviceInfo info;
  const RtAudio::Api api;
};

namespace audio {
struct HostApi {
  const RtAudio::Api api;
  const std::string name;
  const std::string displayName;
  const std::vector<SystemDevice> systemDevices;
};

struct DeviceInstance {
private:
  const SystemDevice &m_systemDevice;

public:
  DeviceInstance(const SystemDevice &sd) : m_systemDevice(sd) {}
  const SystemDevice &systemDevice() const noexcept { return m_systemDevice; }
};

using SysDevList = std::vector<SystemDevice>;
using ApiDevList = std::vector<DeviceInstance>;
using ApiList = std::vector<HostApi>;

struct DeviceEnumerator {
public:
private:
  SysDevList m_sysDevs;
  ApiDevList m_apiDevs;
  ApiList m_apis;

  void clear() {
    m_sysDevs.clear();
    m_apis.clear();
    m_apiDevs.clear();
  }

  void enum_apis() {
    m_apis.clear();
    using rtapiList = std::vector<RtAudio::Api>;
    rtapiList myapiList;
    RtAudio::getCompiledApi(myapiList);
    std::string sname;
    m_sysDevs.clear();

    try {
      for (const auto &a : myapiList) {
        sname = RtAudio::getApiDisplayName(a);
        RtAudio rt(a);

        SysDevList sysDevs;
        for (auto i = 0u; i < rt.getDeviceCount(); ++i) {
          sysDevs.emplace_back(SystemDevice{rt.getDeviceInfo(i), a});
        }

        m_apis.emplace_back(HostApi{a, RtAudio::getApiName(a), sname, sysDevs});
      }

    } catch (std::runtime_error &e) {
        std::cerr << "Failed to instantiate an instance of the backend using hostApi: " << sname
                  << ", with error: " << e.what() << std::endl;
    }
  }
  void enum_all() {
    clear();
    enum_apis();
  }

public:
  DeviceEnumerator() { enum_all(); }
  const ApiList &apis() const noexcept { return m_apis; }
  const SysDevList systemDevices() const noexcept { return m_sysDevs; }
};

class myaudio : public RtAudio {
  DeviceEnumerator m_enum;

public:
  DeviceEnumerator &enumerator() { return m_enum; }
};

namespace tests {}
} // namespace audio
