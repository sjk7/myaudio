#include "../include/myaudio.hpp"
#include <chrono>
#include <iostream>
#include <set>
#include <thread>
using namespace std::chrono_literals;

using namespace std;

void create_specific_audio(const audio::HostApi &api)
{
    audio::myaudio audio(api);
    auto &e = audio.enumerator();
    cout << "******************************************************************"
            "*********\n";
    std::cerr << "Enumerating devices ONLY in hostApi: " << api.displayName
              << std::endl;
    cout << "Using api: " << api.displayName << " There are "
         << e.systemDevices().size() << " devices" << endl;
    for (const auto &d : e.systemDevices())
    {
        assert(d.Host_Api() == audio.currentApi());
    }
    cout << "******************************************************************"
            "*********\n";
    cout << "******************************************************************"
            "*********\n";
}

int main()
{
    RtAudio rtaudio;
    cout << "RtAudio detail: " << RtAudio::getVersion() << endl << endl;

    audio::myaudio audio;
    auto &the_enumerator = audio.enumerator();
    cout << "There are a total of: " << the_enumerator.apis().size()
         << " host apis compiled for this system" << endl
         << endl;

    for (const auto &api : the_enumerator.apis())
    {
        cout << "Have api, with name: " << api.name
             << ", and display name: " << api.displayName << endl;
        cout << "This api has " << api.systemDevices().size()
             << " active devices:" << endl;

        for (const auto &d : api.systemDevices())
        {
            cout << "***************************" << endl;
            std::string s{audio::deviceToString(d)};
            const auto &dev = d.get();
            const auto dbak =
                the_enumerator.findDevice(api.displayName, d.get().info.name);
            assert(dbak && dbak->info.name == dev.info.name);
            assert(dev.Host_Api() == &api);
            assert(dev.Host_Api()->displayName == api.displayName);
            assert(dev.deviceInfo().name == dbak->deviceInfo().name);
            assert(dev.DeviceId() >= 0);
            cout << "This device's global device id is: " << dev.DeviceId()
                 << endl;
            cout << s << endl;
        }
        cout << endl << "---------------------------------" << endl;
        const auto found_api =
            the_enumerator.apiFromDisplayName(api.displayName);
        assert(found_api && found_api->displayName == api.displayName);
        const auto found_api2 = the_enumerator.apiFromName(api.name);
        assert(found_api2 && found_api2->name == api.name);
    }

    cout << "Api and device enumeration 'complet'" << endl;

    cout << "Total Devices on system "
         << audio.enumerator().systemDevices().size() << endl;
    for (const auto &d : audio.enumerator().systemDevices())
    {
        assert(the_enumerator.findHostApi(d.rtapi) != nullptr);
    }

    cout << "Duplex devices on system: "
         << audio.enumerator().duplexDevices().size() << endl;
    for (const auto &d : audio.enumerator().duplexDevices())
    {
        cout << audio::deviceToString(d) << endl;
    }
    cout << "---------------------------------------" << endl << endl;

    cout << "Input devices on system: "
         << audio.enumerator().inputDevices().size() << endl;
    for (const auto &d : audio.enumerator().inputDevices())
    {
        cout << audio::deviceToString(d) << endl;
    }
    cout << "---------------------------------------" << endl << endl;

    cout << "Output devices on system: "
         << audio.enumerator().outputDevices().size() << endl;
    for (const auto &d : audio.enumerator().outputDevices())
    {
        cout << audio::deviceToString(d) << endl;
    }
    cout << "---------------------------------------" << endl << endl;

    for (const auto &api : the_enumerator.apis())
        create_specific_audio(api);
    cout.flush();

    cout << flush;
    int i = 0;
    while (i++ < 1000)
    {
        std::this_thread::sleep_for(1000000ns);
        std::cerr << "\r";
    }
    return 0;
}
