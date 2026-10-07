#include <iomanip>
#include <iostream>
#include <random>

using namespace std;

struct Event {
    int type;
    double time;
};

const int event_count = 100;
const int type_min = 1;
const int type_max = 4;
const double time_min = 0.0;
const double time_max = 100.0;

void handleEvent1(double time) {
    cout << "Произошло событие 1, время = " << time << "\n";
}

void handleEvent2(double time) {
    cout << "Произошло событие 2, время = " << time << "\n";
}

void handleEvent3(double time) {
    cout << "Произошло событие 3, время = " << time << "\n";
}

void handleEvent4(double time) {
    cout << "Произошло событие 4, время = " << time << "\n";
}

void handleEvent(int type, double time) {
    if (type == 1) {
        handleEvent1(time);
    } else if (type == 2) {
        handleEvent2(time);
    } else if (type == 3) {
        handleEvent3(time);
    } else if (type == 4) {
        handleEvent4(time);
    }
}

void generateEvents(Event events[], int count) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> typeDist(type_min, type_max);
    uniform_real_distribution<double> timeDist(time_min, time_max);

    for (int i = 0; i < count; ++i) {
        events[i].type = typeDist(gen);
        events[i].time = timeDist(gen);
    }
}

void sortByTime(Event events[], int count) {
    for (int i = 1; i < count; ++i) {
        Event key = events[i];
        int j = i - 1;
        while (j >= 0 && events[j].time > key.time) {
            events[j + 1] = events[j];
            --j;
        }
        events[j + 1] = key;
    }
}

void printEvents(const Event events[], int count) {
    for (int i = 0; i < count; ++i) {
        cout << "  тип = " << events[i].type
             << ", время = " << events[i].time << "\n";
    }
    cout << "\n";
}

int main() {
    cout << fixed << setprecision(3);

    Event events[event_count];

    generateEvents(events, event_count);
    cout << "исходное множество:\n";
    printEvents(events, event_count);

    sortByTime(events, event_count);
    cout << "отсортированное множество:\n";
    printEvents(events, event_count);

    cout << "Функции по событиям:\n";
    for (int i = 0; i < event_count; ++i) {
        handleEvent(events[i].type, events[i].time);
    }

    return 0;
}
