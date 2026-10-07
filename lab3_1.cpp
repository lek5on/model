#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

#if defined(_WIN32)
    #define OPEN_FILE_COMMAND "start \"\" waiting.xlsx"
#elif defined(__APPLE__)
    #define OPEN_FILE_COMMAND "open waiting.xlsx"
#else
    #define OPEN_FILE_COMMAND "xdg-open waiting.xlsx"
#endif

using namespace std;

const int maxTasks = 10000;

const double arrivalMin = 1.0;
const double arrivalMax = 10.0;

const double serviceMin = 1.0;
const double serviceMax = 8.0;

struct Task {
    int id;
    double arrivalTime;
};

int main() {
    random_device randomDevice;
    mt19937 randomGenerator(randomDevice());

    uniform_real_distribution<double> arrivalGenerator(
        arrivalMin, arrivalMax
    );

    uniform_real_distribution<double> serviceGenerator(
        serviceMin, serviceMax
    );

    vector<Task> queue;
    vector<double> waitTimes;       

    int waiting[10] = {};
    int waitingMoreThan10 = 0;

    int currentTask = 1;
    int completedTasks = 0;
    int queuedTasks = 0;
    size_t maxQueueSize = 0;        

    double totalWaitingTime = 0.0;

    double currentTime = 0.0;
    double incomingTime = arrivalGenerator(randomGenerator);
    double outgoingTime = 1e100;

    bool busy = false;

    while (currentTask <= maxTasks || busy || !queue.empty()) {

        if (currentTask <= maxTasks && incomingTime <= outgoingTime) {
            currentTime = incomingTime;

            Task task;
            task.id = currentTask;
            task.arrivalTime = currentTime;

            cout << fixed << setprecision(2);

            cout << "Пришла задача " << task.id
                 << " в момент " << currentTime << endl;

            incomingTime += arrivalGenerator(randomGenerator);
            currentTask++;

            if (!busy) {
                busy = true;

                double serviceTime =
                    serviceGenerator(randomGenerator);

                outgoingTime = currentTime + serviceTime;

                cout << "Задача " << task.id
                     << " сразу поступила в работу" << endl;

                waiting[0]++;
                waitTimes.push_back(0.0);
            }
            else {
                queue.push_back(task);
                queuedTasks++;

                if (queue.size() > maxQueueSize) {
                    maxQueueSize = queue.size();
                }

                cout << "Задача " << task.id
                     << " добавлена в очередь" << endl;
            }
        }
        else {
            currentTime = outgoingTime;

            completedTasks++;

            cout << fixed << setprecision(2);

            cout << "Обслуживание завершено в момент "
                 << currentTime << endl;

            busy = false;

            if (!queue.empty()) {

                uniform_int_distribution<int> queueGenerator(
                    0,
                    static_cast<int>(queue.size()) - 1
                );

                int index = queueGenerator(randomGenerator);

                Task task = queue[index];

                queue.erase(queue.begin() + index);

                double waitingTime =
                    currentTime - task.arrivalTime;

                totalWaitingTime += waitingTime;
                waitTimes.push_back(waitingTime);

                if (waitingTime < 1.0) {
                    waiting[0]++;
                }
                else if (waitingTime < 2.0) {
                    waiting[1]++;
                }
                else if (waitingTime < 3.0) {
                    waiting[2]++;
                }
                else if (waitingTime < 4.0) {
                    waiting[3]++;
                }
                else if (waitingTime < 5.0) {
                    waiting[4]++;
                }
                else if (waitingTime < 6.0) {
                    waiting[5]++;
                }
                else if (waitingTime < 7.0) {
                    waiting[6]++;
                }
                else if (waitingTime < 8.0) {
                    waiting[7]++;
                }
                else if (waitingTime < 9.0) {
                    waiting[8]++;
                }
                else if (waitingTime < 10.0) {
                    waiting[9]++;
                }
                else {
                    waitingMoreThan10++;
                }

                cout << "Задача " << task.id
                     << " взята из очереди" << endl;

                cout << "Время ожидания: "
                     << waitingTime << " сек." << endl;

                busy = true;

                double serviceTime =
                    serviceGenerator(randomGenerator);

                outgoingTime =
                    currentTime + serviceTime;
            }
            else {
                outgoingTime = 1e100;
            }
        }
    }

    cout << "\nРАСПРЕДЕЛЕНИЕ ВРЕМЕНИ ОЖИДАНИЯ\n";

    for (int i = 0; i < 10; i++) {
        cout << i << "-" << i + 1
             << " секунд: "
             << waiting[i]
             << " задач" << endl;
    }

    cout << "10+ секунд: "
         << waitingMoreThan10
         << " задач" << endl;

    cout << "\nСТАТИСТИКА\n";

    cout << "Всего задач: "
         << maxTasks << endl;

    cout << "Задач попало в очередь: "
         << queuedTasks << endl;

    cout << "Завершено задач: "
         << completedTasks << endl;

    if (queuedTasks > 0) {
        cout << fixed << setprecision(2);

        cout << "Среднее время ожидания (среди задач из очереди): "
             << totalWaitingTime / queuedTasks
             << " секунд" << endl;
    }

    // параметры системы
    double meanTau = (arrivalMin + arrivalMax) / 2.0;   // среднее время между приходами
    double meanSigma = (serviceMin + serviceMax) / 2.0; // среднее время обслуживания
    double rho = meanSigma / meanTau;                   // загрузка сервера

    // мат. ожидание, дисперсия и СКО времени ожидания по всем задачам
    double waitSum = 0.0, maxWait = 0.0;
    int zeroWaitCount = 0;
    for (double w : waitTimes) {
        waitSum += w;
        if (w > maxWait) maxWait = w;
        if (w == 0.0) zeroWaitCount++;
    }
    double waitMean = waitSum / waitTimes.size();

    double waitSq = 0.0;
    for (double w : waitTimes) {
        waitSq += (w - waitMean) * (w - waitMean);
    }
    double waitDisp = waitSq / waitTimes.size();

    cout << fixed << setprecision(3);

    cout << "\nПараметры\n";
    cout << "загрузка rho:           " << rho << endl;
    cout << "lambda (интенс. прихода): " << 1.0 / meanTau << endl;
    cout << "mu (интенс. обслуж.):     " << 1.0 / meanSigma << endl;

    cout << "\nСтатистика по времени ожидания (все задачи)\n";
    cout << "мат. ожидание:          " << waitMean << endl;
    cout << "дисперсия:              " << waitDisp << endl;
    cout << "ср.кв. отклонение:      " << sqrt(waitDisp) << endl;
    cout << "максимальное ожидание:  " << maxWait << endl;
    cout << "доля без ожидания:      "
         << (double)zeroWaitCount / waitTimes.size() << endl;
    cout << "макс. размер очереди:   " << maxQueueSize << endl;

    ofstream dataFile("waiting_data.txt");

    for (int i = 0; i < 10; i++) {
        dataFile << waiting[i] << " ";
    }

    dataFile << waitingMoreThan10;

    dataFile.close();

    ofstream pythonFile("create_excel.py");

    pythonFile << "from openpyxl import Workbook\n";
    pythonFile << "from openpyxl.chart import LineChart, Reference\n";
    pythonFile << "\n";

    pythonFile << "with open('waiting_data.txt', 'r') as file:\n";
    pythonFile << "    values = list(map(int, file.read().split()))\n";
    pythonFile << "\n";

    pythonFile << "intervals = [\n";
    pythonFile << "    '0-1', '1-2', '2-3', '3-4', '4-5',\n";
    pythonFile << "    '5-6', '6-7', '7-8', '8-9', '9-10', '10+'\n";
    pythonFile << "]\n";
    pythonFile << "\n";

    pythonFile << "total = sum(values)\n";
    pythonFile << "\n";

    pythonFile << "workbook = Workbook()\n";
    pythonFile << "sheet = workbook.active\n";
    pythonFile << "sheet.title = 'Функция распределения'\n";
    pythonFile << "\n";

    pythonFile << "sheet['A1'] = 'Время ожидания'\n";
    pythonFile << "sheet['B1'] = 'F(x)'\n";
    pythonFile << "\n";

    pythonFile << "cumulative = 0\n";
    pythonFile << "\n";

    pythonFile << "for i in range(11):\n";
    pythonFile << "    cumulative += values[i]\n";
    pythonFile << "    sheet.cell(row=i + 2, column=1, value=intervals[i])\n";
    pythonFile << "    sheet.cell(row=i + 2, column=2, value=cumulative / total)\n";
    pythonFile << "\n";

    pythonFile << "chart = LineChart()\n";
    pythonFile << "chart.title = 'Функция распределения времени ожидания'\n";
    pythonFile << "chart.x_axis.title = 'Время ожидания'\n";
    pythonFile << "chart.y_axis.title = 'F(x)'\n";
    pythonFile << "\n";

    pythonFile << "data = Reference(sheet, min_col=2, min_row=1, max_row=12)\n";
    pythonFile << "categories = Reference(sheet, min_col=1, min_row=2, max_row=12)\n";
    pythonFile << "\n";

    pythonFile << "chart.add_data(data, titles_from_data=True)\n";
    pythonFile << "chart.set_categories(categories)\n";
    pythonFile << "\n";

    pythonFile << "chart.width = 20\n";
    pythonFile << "chart.height = 10\n";
    pythonFile << "\n";

    pythonFile << "sheet.add_chart(chart, 'D2')\n";
    pythonFile << "\n";

    pythonFile << "workbook.save('waiting.xlsx')\n";

    pythonFile.close();

    system("python3 create_excel.py");

    cout << "\nExcel-файл waiting.xlsx создан" << endl;

    system(OPEN_FILE_COMMAND);

    return 0;
}