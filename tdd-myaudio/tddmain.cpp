#include "../include/myaudio.hpp"
#include <iostream>

using namespace std;

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
        cout << "This api has " << api.systemDevices.size()
             << " active devices:\n";

        for (const auto &d : api.systemDevices)
        {
            cout << "***************************\n";
            std::string s{audio::deviceToString(d)};
            cout << s;
        }
        cout << endl << "---------------------------------" << endl;
    }

    cout << "Api and device enumeration 'complet'" << endl;
    cout.flush();
    return 0;
}
