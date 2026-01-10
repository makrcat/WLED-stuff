#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <curl/curl.h>
#include "json.hpp"
#include "utils.hpp"

//

#include <thread>
#include <chrono>

using namespace std;
using json = nlohmann::json;

/*

ideas:

- reuse curl handle
- minimize json when sending
- skip terminal
- arrays (doesnt rlly matter)

*/

const string URL = "http://wled-001.local/json/";
const float bdefault = 0.3f;
const bool wifi = false;  
vector<pair<int, string>> updates;

class LED {
    private:
        float l;
        float s;
        int number;
        float hue;
        string hex;

        void updateHEX() {
            hex = numberToHEX(hue, l, s);
        }

    public:
        LED(int num, int full)
        : l(bdefault), s(0.6f), number(num) {

            hue = static_cast<float>(num) / full;
            updateHEX();
        }

        void glow() {
            l += 0.25f;
            s += 0.25f;
            updateHEX();
        }

        void unglow() {
            l = bdefault;
            s = 0.6f;
            updateHEX();
        }

        void setHue(float h) {
            hue = h;
            updateHEX();
        }

        int getNum() const { return number; }
        string getHEX() const { return hex; }
};

/*
vector<string> getHEXarray(vector<LED> lights) {
    vector<string> temp;
    temp.reserve(lights.size());
    for (const LED& light : lights) {
        temp.push_back(light.getHEX());
    }
    return temp;
}
*/


void sendArray(vector<pair<int, string>>& updates) {
    json payload;
    payload["on"] = true;
    payload["bri"] = 255;

    json seg = json::array();

    // ok. its fine. stop being ocd please stop stop stpo stpo postpostpsoek
    for (auto& u : updates) {
        seg.push_back(u.first);
        seg.push_back(u.second);
    }

    updates.clear();

    payload["seg"] = json::array();
    payload["seg"].push_back({{"i", seg}});


    CURL* curl = curl_easy_init();
    if (!curl) return;

    string data = payload.dump();
    curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        cerr << "oh no: \n" << curl_easy_strerror(res) << endl;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

void visual(vector<LED>& lights, int n) {

    /*
    std::cout << "\x1B[2J\x1B[H"; // cursor to top left corner
    std::cout << std::flush; // clear
    */

    this_thread::sleep_for(chrono::milliseconds(100));
    cout << "\r";

    for (int i = 0; i < n; i++) {
        string hex = lights[i].getHEX();
        cout << visualizer(" " + to_string(lights[i].getNum()) + " ", hex);
    }
    // cout << endl;
    cout << flush;
}

template <typename T>
void shuffleV(vector<T>& v) {
    random_device rd;
    mt19937 g(rd());
    std::shuffle(v.begin(), v.end(), g);
}


void runBubbleSort(vector<LED>& lights, int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (lights[j].getNum() > lights[j + 1].getNum()) {
                swap(lights[j], lights[j + 1]);
                visual(lights, n);
            }
        }
    }
}

void commitAndSend(int i, string newHEX) {
    updates.emplace_back(i, newHEX);

    if (!wifi) return;
    sendArray(updates);
}

void runBSlight(vector<LED>& lights, int n) {

    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            LED& a = lights[j];
            LED& b = lights[j + 1];

            a.glow();
            commitAndSend(j, a.getHEX());
            visual(lights, n);

            b.glow();
            commitAndSend(j + 1, b.getHEX());
            visual(lights, n);

            if (a.getNum() > b.getNum()) {
                swap(a, b);
                commitAndSend(j, a.getHEX());
                commitAndSend(j + 1, b.getHEX());

                visual(lights, n);
            }

            // yes I know they point to each other now, but it's ok lol
            a.unglow();
            commitAndSend(j, a.getHEX());
            b.unglow();
            commitAndSend(j + 1, b.getHEX());

            visual(lights, n);
        }
    }
}


int main() {
    int length;
    cout << "how many: ";
    cin >> length;

    vector<LED> lights;
    updates.reserve(length);
    lights.reserve(length);
    for (int i = 0; i < length; i++) {
        lights.emplace_back(i, length);
    }

    shuffleV(lights);
    runBSlight(lights, length);

    return 0;
}
