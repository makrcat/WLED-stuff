#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <curl/curl.h>
#include "json.hpp"
#include "utils.hpp"
#include <thread>
#include <chrono>

using namespace std;
using json = nlohmann::json;

const float bdefault = 0.3f;
const bool wifi = false;
const bool console = true;

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
    LED(int num, int full) : l(bdefault), s(0.6f), number(num) {
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

    int getNum() const { 
        return number; 
    }

    string getHEX() const { 
        return hex; 
    }
};
class LightArray {
private:
    vector<LED> lights;
    vector<pair<int, string>> updates;
    string URL;
    int N;

    void commitUpdate(int i) {
        // add to update
        updates.emplace_back(i, lights[i].getHEX());
        // try to send? if wifi: or else, updates just kinda stack up
        if (wifi) sendArray(updates);
    }

    void sendArray(vector<pair<int, string>>& updates) {
        json payload;
        payload["on"] = true;
        payload["bri"] = 255;

        json seg = json::array();
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
            cerr << "Curl error: " << curl_easy_strerror(res) << endl;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }



public:
    LightArray(int length, const string URL) {
        this->URL = URL;
        this->N = length;
        
        lights.reserve(length);
        updates.reserve(length); // under typical wifi connection it would not have to allocate more

        for (int i = 0; i < length; i++) {
            lights.emplace_back(i, length);
        }
    }

    int size() const { 
        return N;
    }

    LED& operator[](int i) {
        return lights[i]; 
    }

    void setHue(int i, float h) {
        lights[i].setHue(h);
        commitUpdate(i);
    }

    void glow(int i) {
        lights[i].glow();
        commitUpdate(i);
    }

    void unglow(int i) {
        lights[i].unglow();
        commitUpdate(i);
    }

    void swap(int i, int j) {
        std::swap(lights[i], lights[j]);
        commitUpdate(i);
        commitUpdate(j);
    }

    void shuffle() {
        random_device rd;
        mt19937 g(rd());
        std::shuffle(lights.begin(), lights.end(), g);

        // commit all positions after shuffle
        for (int i = 0; i < N; i++) {
            commitUpdate(i);
        }
    }

    LED& get(int i) { return lights[i]; }

    // utility

    string getVisual() const {
        string V;
        for (int i = 0; i < N; i++) {
            const LED& led = lights[i];
            V += visualizer(" " + to_string(led.getNum()) + " ", led.getHEX());
        }
        return V;
    }
};

void rewriteLine(string text) {
    cout << "\r";
    cout << text;
    cout << flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}


void runBSlight(LightArray& arr) {
    int n = arr.size();
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            arr.glow(j);
            rewriteLine(arr.getVisual());
            arr.glow(j + 1);
            rewriteLine(arr.getVisual());

            if (arr[j].getNum() > arr[j + 1].getNum()) {
                arr.swap(j, j + 1);
                rewriteLine(arr.getVisual());
            }

            arr.unglow(j);
            arr.unglow(j + 1);
              rewriteLine(arr.getVisual());

        }
    }
}

// 

int main() {
    const string URL = "http://wled-001.local/json/";
    int length;
    cout << "how many: ";
    cin >> length;

    LightArray lights(length, URL);

    lights.shuffle();
    runBSlight(lights);

    return 0;
}
