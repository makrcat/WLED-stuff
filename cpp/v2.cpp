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
const bool wifi = true;
const bool console = true;
const int delay = 100; // milliseconds
const string URL = "http://wled-001.local/json";

CURL* curl = nullptr;
struct curl_slist* headers = nullptr;
/////

void initHTTP() {
    curl = curl_easy_init();
    if (!curl) {
        cerr << "Failed to init CURL" << endl;
        return;
    }

    headers = curl_slist_append(nullptr, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);  // keep the connection alive
    curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
}


//"http://127.0.0.1:21324/json"; 

void sendArray(vector<pair<int, string>>& updates) {
    // this only sends the updates!! which means it doesn't rewrite every LED. idk how much of a difference this will make to be honest but yeah
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

    string data = payload.dump();
    curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        cerr << "Curl error: " << curl_easy_strerror(res) << endl;
    }
}


void clearLights() {
    vector<pair<int, string>> clear;
    for (int i = 0; i < 300; i++) {
        clear.emplace_back(i, "000000");
    }
    sendArray(clear);
}

class LED {
private:
    float l;
    float s;
    int number;
    float hue;
    string hex;
    int full;

    void updateHEX() {
        hex = numberToHEX(hue, l, s);
    }

public:
    LED(int num, int full) : l(bdefault), s(0.6f), number(num) {
        hue = static_cast<float>(num) / full;
        this->full = full;
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

    void setHue(float h) {  // idk why
        hue = h; 
        updateHEX(); 
    }

    void setNum(int num) {
        number = num;
        hue = static_cast<float>(num) / full;
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
    int N;

    void commitUpdate(int i) {
        // add to update
        updates.emplace_back(i, lights[i].getHEX());
        // try to send? if wifi: or else, updates just kinda stack up
        if (wifi) sendArray(updates);
    }


public:
    LightArray(int length) {
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

    void move(int pos1, int pos2) {
        LED temp = lights[pos1];

        if (pos1 < pos2) {
            // shift elements left
            for (int i = pos1; i < pos2; i++) {
                lights[i] = lights[i + 1];
                commitUpdate(i);
            }
            lights[pos2] = temp;
            commitUpdate(pos2);
        } else { 
            // pos1 > pos2
            // shift elements right
            for (int i = pos1; i > pos2; i--) {
                lights[i] = lights[i - 1];
                commitUpdate(i);
            }
            lights[pos2] = temp;
            commitUpdate(pos2);
        }

    }

    void insert(LED l, int pos) { 
        lights.insert(lights.begin() + pos, l);
        for (int i = pos; i < lights.size(); i++) {
            commitUpdate(i);
        }
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
    //std::this_thread::sleep_for(std::chrono::milliseconds(delay));
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
void runInsertion(LightArray& arr) {
    int n = arr.size();

    for (int i = 1; i < n; i++) {
        // arr.glow(i);
        rewriteLine(arr.getVisual());
        int j = i;

        while (j > 0 && arr[j - 1].getNum() > arr[j].getNum()) {
            // arr.glow(j - 1);
            rewriteLine(arr.getVisual());

            arr.move(j, j - 1);
            rewriteLine(arr.getVisual());

            // arr.unglow(j);
            // arr.unglow(j - 1);
            rewriteLine(arr.getVisual());

            j--;
        }

        // arr.unglow(j);
        rewriteLine(arr.getVisual());
    }
}

// 

void merge(LightArray& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    vector<int> L(n1);
    vector<int> R(n2);
    for (int i = 0; i < n1; i++) L[i] = arr[left + i].getNum();
    for (int i = 0; i < n2; i++) R[i] = arr[mid + 1 + i].getNum();

    int i = 0, j = 0, k = left;

    while (i < n1 && j < n2) {
        // arr.glow(k);
        rewriteLine(arr.getVisual());

        if (L[i] <= R[j]) {
            arr[k].setNum(L[i]);
            i++;
        } else {
            arr[k].setNum(R[j]);
            j++;
        }

        rewriteLine(arr.getVisual());
        // arr.unglow(k);
        k++;
    }

    while (i < n1) {
        // arr.glow(k);
        rewriteLine(arr.getVisual());
        arr[k].setNum(L[i]);
        rewriteLine(arr.getVisual());
        // arr.unglow(k);
        i++; 
        k++;
    }

    while (j < n2) {
        // arr.glow(k);
        rewriteLine(arr.getVisual());
        arr[k].setNum(R[j]);
        rewriteLine(arr.getVisual());
        // arr.unglow(k);
        j++; 
        k++;
    }
}

void mergeSort(LightArray& arr, int left, int right) {
    if (left >= right) return;

    int mid = left + (right - left) / 2;

    mergeSort(arr, left, mid);
    mergeSort(arr, mid + 1, right);
    merge(arr, left, mid, right);
}

void runMerge(LightArray& arr) {
    mergeSort(arr, 0, arr.size() - 1);
}



int main() {
    // "http://wled-001.local/json/";
    initHTTP(); 
    int length;
    cout << "how many: ";
    cin >> length;

    LightArray lights(length);

    lights.shuffle();
    clearLights();
    // runMerge(lights);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return 0;
}
