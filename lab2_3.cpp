#include <algorithm>
#include <iomanip>
#include <iostream>
#include <random>

using namespace std;

const int in = 1;
const int out = 2;

const int maxTasks = 100;

const double arrivalFrom = 1.0;
const double arrivalTo = 10.0;

const double serviceFrom = 1.0;
const double serviceTo = 5.0;

const char* getEventName(int event) {
    if (event == in) {
        return "IN";
    }
    return "OUT";
}

int main() {

    cout << fixed << setprecision(2);

    random_device randomSource;
    mt19937 randomGenerator(randomSource());

    uniform_real_distribution<double> arrivalGenerator(
        arrivalFrom, arrivalTo
    );

    uniform_real_distribution<double> serviceGenerator(
        serviceFrom, serviceTo
    );

    bool busy = false;
    int rejectedTasks = 0;
    int currentTask = 1;
    double currentTime = 0.0;

    double incomingTime = arrivalGenerator(randomGenerator);
    double outgoingTime = incomingTime + serviceGenerator(randomGenerator);

    while (currentTask <= maxTasks) {

        int eventType;

        if (incomingTime <= outgoingTime) {
            eventType = in;
            currentTime = incomingTime;
        }
        else {
            eventType = out;
            currentTime = outgoingTime;
        }

        if (eventType == in) {

            incomingTime += arrivalGenerator(randomGenerator);

            if (!busy) {
                busy = true;

                cout << "время = " << currentTime
                     << ", " << getEventName(in)
                     << ", id = " << currentTask << "\n";
            }
            else {
                rejectedTasks++;

                cout << "время = " << currentTime
                     << ", отказано"
                     << ", id = " << currentTask << "\n";
            }

            currentTask++;
        }
        else {

            busy = false;

            cout << "время = " << currentTime
                 << ", " << getEventName(out) << "\n";

            outgoingTime = incomingTime + serviceGenerator(randomGenerator);
        }
    }

    cout << "\n событий IN отказано: " << rejectedTasks << "\n";

    return 0;
}