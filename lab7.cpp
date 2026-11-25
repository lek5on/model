#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

using namespace std;

const int queueLength = 6;          // ищем вероятность того, что в очереди ровно 6 заданий
const int totalTasks = 1000000;     // сколько заданий моделируем

int main() {
    int n;                          // число серверов
    double lambda;                  // интенсивность прихода заданий
    double mu;                      // интенсивность обслуживания одним сервером

    cout << "Введите n, lambda и mu: ";
    cin >> n >> lambda >> mu;

    if (n < 1 || lambda <= 0 || mu <= 0) {
        cout << "Нужно n >= 1, lambda > 0, mu > 0" << endl;
        return 1;
    }

    double rho = lambda / mu;       // приведённая интенсивность

    // все n серверов вместе обслуживают n * mu заданий в единицу времени
    if (rho >= n) {
        cout << "Система неустойчива: нужно lambda < n * mu" << endl;
        return 1;
    }

    int maxState = n + queueLength + 4;     // до какого состояния выводим таблицу

    // теория
    // p[k] = rho^k / k! * p0,                    если k <= n (очереди нет)
    // p[k] = rho^n / n! * (rho / n)^(k - n) * p0, если k > n  (в очереди k - n заданий)
    vector<double> pTheory(maxState + 1);
    pTheory[0] = 1.0;
    for (int k = 1; k <= maxState; k++) {
        if (k <= n) {
            pTheory[k] = pTheory[k - 1] * rho / k;
        }
        else {
            pTheory[k] = pTheory[k - 1] * rho / n;
        }
    }

    // сумма всех вероятностей = 1, отсюда p0
    // хвост k >= n — геометрическая прогрессия со знаменателем rho / n
    double sum = 0.0;
    for (int k = 0; k < n; k++) {
        sum += pTheory[k];
    }
    sum += pTheory[n] / (1 - rho / n);

    double p0 = 1.0 / sum;
    for (int k = 0; k <= maxState; k++) {
        pTheory[k] *= p0;
    }

    // моделирование
    random_device randomDevice;
    mt19937 randomGenerator(randomDevice());

    // пуассоновский поток: интервалы экспоненциальные
    exponential_distribution<double> arrivalGenerator(lambda);
    exponential_distribution<double> serviceGenerator(mu);

    vector<double> outgoingTime(n, 1e100);      // конец обслуживания на каждом сервере 
    vector<double> timeInState(maxState + 2, 0.0);  // сколько времени система была в каждом состоянии

    int inSystem = 0;               // сколько заданий в системе (на серверах + в очереди)
    int arrived = 0;

    double currentTime = 0.0;
    double incomingTime = arrivalGenerator(randomGenerator);

    while (arrived < totalTasks) {

        // сервер, который освободится раньше всех
        int server = 0;
        for (int i = 1; i < n; i++) {
            if (outgoingTime[i] < outgoingTime[server]) {
                server = i;
            }
        }

        double nextTime = outgoingTime[server];
        if (incomingTime < nextTime) {
            nextTime = incomingTime;
        }

        // до следующего события система находилась в состоянии inSystem
        int state = inSystem;
        if (state > maxState) {
            state = maxState + 1;               // все большие состояния — в последнюю ячейку
        }
        timeInState[state] += nextTime - currentTime;

        currentTime = nextTime;

        if (incomingTime <= outgoingTime[server]) {
            // пришло задание
            arrived++;
            inSystem++;

            if (inSystem <= n) {
                // есть свободный сервер — сразу на обслуживание
                for (int i = 0; i < n; i++) {
                    if (outgoingTime[i] == 1e100) {
                        outgoingTime[i] = currentTime + serviceGenerator(randomGenerator);
                        break;
                    }
                }
            }
            // иначе все серверы заняты — задание просто встаёт в очередь

            incomingTime = currentTime + arrivalGenerator(randomGenerator);
        }
        else {
            // сервер закончил обслуживание
            inSystem--;

            if (inSystem >= n) {
                // в очереди кто-то есть — этот же сервер берёт следующее задание
                outgoingTime[server] = currentTime + serviceGenerator(randomGenerator);
            }
            else {
                outgoingTime[server] = 1e100;   // очередь пуста — сервер простаивает
            }
        }
    }

    // вывод
    cout << fixed << setprecision(4);

    cout << "\nn = " << n << ", lambda = " << lambda << ", mu = " << mu << endl;
    cout << "rho = lambda / mu = " << rho << ", загрузка сервера rho / n = " << rho / n << endl;
    cout << "p0 = " << p0 << endl;

    cout << "\nсостояние  в очереди   P теория   P модель\n";
    for (int k = 0; k <= maxState; k++) {
        int queue = k - n;
        if (queue < 0) {
            queue = 0;
        }

        cout << "  s" << left << setw(6) << k << right
             << setw(6) << queue << "     "
             << pTheory[k] << "     "
             << timeInState[k] / currentTime;

        if (k == n + queueLength) {
            cout << "  очередь = " << queueLength;
        }
        cout << endl;
    }

    cout << "\nP(очередь = " << queueLength << ")\n";
    cout << "теория:  " << pTheory[n + queueLength] << endl;
    cout << "модель:  " << timeInState[n + queueLength] / currentTime << endl;

    return 0;
}
