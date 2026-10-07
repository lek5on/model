#include <deque>
#include <iomanip>
#include <iostream>
#include <random>

using namespace std;

const double sigma1 = 11.0;       // время обработки на 1 фазе
const double sigma2 = 17.0;       // время обработки на 2 фазе

const int maxK = 3;               // k от 1 до 3 — сколько циклов на 1 фазе
const int maxM = 2;               // m от 1 до 2 — сколько циклов на 2 фазе

const int totalTasks = 10000;

struct Task {
    double arrivalTime;           // момент прихода в систему
    int k;                        // сколько циклов осталось на 1 фазе
    int m;                        // сколько циклов осталось на 2 фазе
};

int main() {
    double a, b;                  // интервалы между приходами заданий ~ U(a, b)
    cout << "Введите a и b: ";
    cin >> a >> b;

    if (a < 0 || b <= a) {
        cout << "Нужно 0 <= a < b" << endl;
        return 1;
    }

    // теория
    double meanK = (1 + maxK) / 2.0;     // M[k] = 2
    double meanM = (1 + maxM) / 2.0;     // M[m] = 1.5
    double work1 = meanK * sigma1;       // средняя работа сервера 1 на одно задание
    double work2 = meanM * sigma2;       // средняя работа сервера 2 на одно задание
    double meanTau = (a + b) / 2.0;

    // Mtau должно быть больше работы самого загруженного сервера
    double minTau = work1;
    if (work2 > minTau) {
        minTau = work2;
    }

    cout << "Условие устойчивости: a + b > " << 2 * minTau << endl;
    cout << "  сервер 1: M[k] * sigma1 = " << meanK << " * " << sigma1 << " = " << work1 << endl;
    cout << "  сервер 2: M[m] * sigma2 = " << meanM << " * " << sigma2 << " = " << work2 << endl;
    cout << "  берём большее: (a + b) / 2 > " << minTau << ", значит a + b > " << 2 * minTau << endl;

    cout << fixed << setprecision(3);

    cout << "\na = " << a << ", b = " << b << ", Mtau = " << meanTau << endl;
    cout << "rho1 = " << work1 / meanTau << ", rho2 = " << work2 / meanTau << endl;

    if (meanTau > minTau) {
        cout << "система устойчива" << endl;
    }
    else {
        cout << "система неустойчива — очередь будет расти бесконечно" << endl;
    }

    // моделирование
    random_device randomDevice;
    mt19937 randomGenerator(randomDevice());

    uniform_real_distribution<double> arrivalGenerator(a, b);
    uniform_int_distribution<int> kGenerator(1, maxK);
    uniform_int_distribution<int> mGenerator(1, maxM);

    deque<Task> queue1;
    deque<Task> queue2;

    Task task1;                   // задание на сервере 1
    Task task2;                   // задание на сервере 2
    bool busy1 = false;
    bool busy2 = false;

    double incomingTime = arrivalGenerator(randomGenerator);
    double outgoingTime1 = 1e100; // 1e100 — сервер свободен
    double outgoingTime2 = 1e100;

    double currentTime = 0.0;
    double busyTime1 = 0.0;
    double busyTime2 = 0.0;
    double sumTime = 0.0;

    int arrived = 0;
    int finished = 0;

    while (finished < totalTasks) {

        if (incomingTime <= outgoingTime1 && incomingTime <= outgoingTime2) {
            // пришло новое задание
            currentTime = incomingTime;
            arrived++;

            Task task;
            task.arrivalTime = currentTime;
            task.k = kGenerator(randomGenerator);
            task.m = mGenerator(randomGenerator);
            queue1.push_back(task);

            if (arrived < totalTasks) {
                incomingTime = currentTime + arrivalGenerator(randomGenerator);
            }
            else {
                incomingTime = 1e100;
            }
        }
        else if (outgoingTime1 <= outgoingTime2) {
            // сервер 1 закончил цикл
            currentTime = outgoingTime1;
            busy1 = false;
            outgoingTime1 = 1e100;

            task1.k--;
            if (task1.k > 0) {
                queue1.push_back(task1);   // ещё раз на 1 фазу
            }
            else {
                queue2.push_back(task1);   // переходит на 2 фазу
            }
        }
        else {
            // сервер 2 закончил цикл
            currentTime = outgoingTime2;
            busy2 = false;
            outgoingTime2 = 1e100;

            task2.m--;
            if (task2.m > 0) {
                queue2.push_back(task2);   // ещё раз на 2 фазу
            }
            else {
                sumTime += currentTime - task2.arrivalTime;   // уходит из системы
                finished++;
            }
        }

        // свободный сервер берёт первое задание из своей очереди
        if (!busy1 && !queue1.empty()) {
            task1 = queue1.front();
            queue1.pop_front();
            busy1 = true;
            outgoingTime1 = currentTime + sigma1;
            busyTime1 += sigma1;
        }

        if (!busy2 && !queue2.empty()) {
            task2 = queue2.front();
            queue2.pop_front();
            busy2 = true;
            outgoingTime2 = currentTime + sigma2;
            busyTime2 += sigma2;
        }
    }

    cout << "\nМоделирование\n";
    cout << "загрузка сервера 1:        " << busyTime1 / currentTime << endl;
    cout << "загрузка сервера 2:        " << busyTime2 / currentTime << endl;
    cout << "среднее время пребывания:  " << sumTime / totalTasks << endl;
    cout << "только обработка:          " << work1 + work2 << endl;

    return 0;
}
