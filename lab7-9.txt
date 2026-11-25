//лабораторная 1

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

// лабораторная 2

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

#if defined(_WIN32)
    #define PYTHON_COMMAND "python create_excel_lab8.py"
    #define OPEN_FILE_COMMAND "start \"\" network.xlsx"
#elif defined(__APPLE__)
    #define PYTHON_COMMAND "python3 create_excel_lab8.py"
    #define OPEN_FILE_COMMAND "open network.xlsx"
#else
    #define PYTHON_COMMAND "python3 create_excel_lab8.py"
    #define OPEN_FILE_COMMAND "xdg-open network.xlsx"
#endif

using namespace std;

const double areaSize = 200.0;        // помещение 200 x 200

const double roomMin = 75.0;          // комната 50 x 50 в центре: от 75 до 125 по x и по y
const double roomMax = 125.0;

const int minNodes = 2;               // размеры сети от 2 до 20 узлов
const int maxNodes = 20;

const int radiusCount = 5;
const double radii[radiusCount] = { 50, 70, 90, 110, 130 };   // радиусы передачи

const int experiments = 10000;        

struct Point {
    double x;
    double y;
};

// с какой стороны от прямой AB лежит точка C (знак векторного произведения)
double cross(Point a, Point b, Point c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

// пересекаются ли отрезки AB и CD
bool segmentsIntersect(Point a, Point b, Point c, Point d) {
    double d1 = cross(c, d, a);
    double d2 = cross(c, d, b);
    double d3 = cross(a, b, c);
    double d4 = cross(a, b, d);

    // концы каждого отрезка лежат по разные стороны от другого отрезка
    return d1 * d2 < 0 && d3 * d4 < 0;
}

// проходит ли сигнал от A к B сквозь стену комнаты
bool crossesWall(Point a, Point b) {
    Point p1 = { roomMin, roomMin };   // углы комнаты
    Point p2 = { roomMax, roomMin };
    Point p3 = { roomMax, roomMax };
    Point p4 = { roomMin, roomMax };

    if (segmentsIntersect(a, b, p1, p2)) return true;   // нижняя стена
    if (segmentsIntersect(a, b, p2, p3)) return true;   // правая
    if (segmentsIntersect(a, b, p3, p4)) return true;   // верхняя
    if (segmentsIntersect(a, b, p4, p1)) return true;   // левая
    return false;
}

// связаны ли два узла напрямую: достаёт сигнал и не мешает стена
bool linked(Point a, Point b, double r) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;

    if (dx * dx + dy * dy > r * r) {
        return false;
    }
    return !crossesWall(a, b);
}

// связна ли сеть: обходом в ширину из узла 0 должны дойти до всех узлов
bool isConnected(const vector<Point>& nodes, double r) {
    int n = nodes.size();
    vector<bool> visited(n, false);
    vector<int> queue;

    visited[0] = true;
    queue.push_back(0);

    for (int head = 0; head < (int)queue.size(); head++) {
        int current = queue[head];

        for (int i = 0; i < n; i++) {
            if (!visited[i] && linked(nodes[current], nodes[i], r)) {
                visited[i] = true;
                queue.push_back(i);
            }
        }
    }

    return (int)queue.size() == n;       // дошли до всех — сеть связна
}

int main() {
    random_device randomDevice;
    mt19937 randomGenerator(randomDevice());

    uniform_real_distribution<double> coordinateGenerator(0.0, areaSize);

    // probability[n][j] — вероятность связности сети из n узлов при радиусе radii[j]
    vector<vector<double>> probability(maxNodes + 1, vector<double>(radiusCount, 0.0));

    for (int n = minNodes; n <= maxNodes; n++) {
        for (int j = 0; j < radiusCount; j++) {
            int connectedCount = 0;

            for (int e = 0; e < experiments; e++) {
                // случайно расставляем n узлов по всему помещению (в том числе внутри комнаты)
                vector<Point> nodes(n);
                for (int i = 0; i < n; i++) {
                    nodes[i].x = coordinateGenerator(randomGenerator);
                    nodes[i].y = coordinateGenerator(randomGenerator);
                }

                if (isConnected(nodes, radii[j])) {
                    connectedCount++;
                }
            }

            probability[n][j] = (double)connectedCount / experiments;
        }
    }

    // ---------- вывод ----------
    cout << fixed << setprecision(4);

    cout << "Помещение " << areaSize << " x " << areaSize
         << ", комната [" << roomMin << "; " << roomMax << "] x [" << roomMin << "; " << roomMax << "]\n";
    cout << "экспериментов на каждую точку: " << experiments << "\n\n";

    cout << "Вероятность связности сети\n";
    cout << "   n";
    for (int j = 0; j < radiusCount; j++) {
        cout << "     r = " << setw(3) << (int)radii[j];
    }
    cout << endl;

    ofstream dataFile("network_data.txt");

    for (int n = minNodes; n <= maxNodes; n++) {
        cout << setw(4) << n;
        dataFile << n;

        for (int j = 0; j < radiusCount; j++) {
            cout << "     " << setw(8) << probability[n][j];
            dataFile << " " << probability[n][j];
        }

        cout << endl;
        dataFile << "\n";
    }

    dataFile.close();

    // ---------- Excel: график P(связности) от n для каждого радиуса ----------
    ofstream pythonFile("create_excel_lab8.py");

    pythonFile << "from openpyxl import Workbook\n";
    pythonFile << "from openpyxl.chart import LineChart, Reference\n";
    pythonFile << "\n";
    pythonFile << "workbook = Workbook()\n";
    pythonFile << "sheet = workbook.active\n";

    pythonFile << "sheet.append(['n'";
    for (int j = 0; j < radiusCount; j++) {
        pythonFile << ", 'r = " << (int)radii[j] << "'";
    }
    pythonFile << "])\n";

    pythonFile << "\n";
    pythonFile << "with open('network_data.txt') as file:\n";
    pythonFile << "    for line in file:\n";
    pythonFile << "        values = line.split()\n";
    pythonFile << "        sheet.append([int(values[0])] + [float(v) for v in values[1:]])\n";
    pythonFile << "\n";
    pythonFile << "chart = LineChart()\n";
    pythonFile << "chart.title = 'Вероятность связности сети'\n";
    pythonFile << "chart.x_axis.title = 'Число узлов n'\n";
    pythonFile << "chart.y_axis.title = 'P(связности)'\n";
    pythonFile << "chart.add_data(Reference(sheet, min_col=2, max_col=" << radiusCount + 1
               << ", min_row=1, max_row=sheet.max_row), titles_from_data=True)\n";
    pythonFile << "chart.set_categories(Reference(sheet, min_col=1, min_row=2, max_row=sheet.max_row))\n";
    pythonFile << "chart.width = 22\n";
    pythonFile << "chart.height = 12\n";
    pythonFile << "sheet.add_chart(chart, 'H2')\n";
    pythonFile << "\n";
    pythonFile << "workbook.save('network.xlsx')\n";

    pythonFile.close();

    system(PYTHON_COMMAND);

    cout << "\nExcel-файл network.xlsx создан" << endl;

    system(OPEN_FILE_COMMAND);

    return 0;
}

//лабораторная 3

#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <fstream>
#include <queue>
#include <string>
#include <algorithm>
#include <cstdlib>

using namespace std;

struct Point {
    double x;
    double y;
};

string to_excel(double val) {
    string s = to_string(val);
    replace(s.begin(), s.end(), '.', ',');
    return s;
}

vector<Point> make_points(int n, double a, double b, mt19937& gen) {
    uniform_real_distribution<double> dx(-a, a);
    uniform_real_distribution<double> dy(-b, b);

    vector<Point> pts;

    while ((int)pts.size() < n) {
        double x = dx(gen);
        double y = dy(gen);

        if ((x * x) / (a * a) +
            (y * y) / (b * b) <= 1.0) {
            pts.push_back({x, y});
        }
    }

    return pts;
}

bool is_connected(int n, const vector<Point>& pts, double r) {
    vector<vector<int>> adj(n);

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            double dx = pts[i].x - pts[j].x;
            double dy = pts[i].y - pts[j].y;

            if (dx * dx + dy * dy <= r * r) {
                adj[i].push_back(j);
                adj[j].push_back(i);
            }
        }
    }

    vector<bool> vis(n, false);
    queue<int> q;

    q.push(0);
    vis[0] = true;

    int count = 1;

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        for (int v : adj[u]) {
            if (!vis[v]) {
                vis[v] = true;
                count++;
                q.push(v);
            }
        }
    }

    return count == n;
}

void create_excel() {
    ofstream py("create_excel.py");

    py << R"PY(
from openpyxl import Workbook
from openpyxl.chart import LineChart, Reference

wb = Workbook()

ws1 = wb.active
ws1.title = "График 1"

with open("results_graph1.csv", "r", encoding="utf-8") as f:
    lines = f.readlines()

for row, line in enumerate(lines, 1):
    values = line.strip().split(";")
    for col, value in enumerate(values, 1):
        if value != "":
            try:
                value = float(value.replace(",", "."))
            except:
                pass
        ws1.cell(row=row, column=col, value=value)

chart1 = LineChart()
chart1.title = "Вероятность связности при различном количестве вершин"
chart1.y_axis.title = "Вероятность связности"
chart1.x_axis.title = "Расстояние между фокусами"

data1 = Reference(ws1, min_col=2, max_col=6, min_row=1, max_row=ws1.max_row)
cats1 = Reference(ws1, min_col=1, min_row=2, max_row=ws1.max_row)

chart1.add_data(data1, titles_from_data=True)
chart1.set_categories(cats1)
chart1.height = 12
chart1.width = 22

ws1.add_chart(chart1, "H2")

ws2 = wb.create_sheet("График 2")

with open("results_graph2.csv", "r", encoding="utf-8") as f:
    lines = f.readlines()

for row, line in enumerate(lines, 1):
    values = line.strip().split(";")
    for col, value in enumerate(values, 1):
        if value != "":
            try:
                value = float(value.replace(",", "."))
            except:
                pass
        ws2.cell(row=row, column=col, value=value)

chart2 = LineChart()
chart2.title = "Вероятность связности при различном радиусе связи"
chart2.y_axis.title = "Вероятность связности"
chart2.x_axis.title = "Расстояние между фокусами"

data2 = Reference(ws2, min_col=2, max_col=6, min_row=1, max_row=ws2.max_row)
cats2 = Reference(ws2, min_col=1, min_row=2, max_row=ws2.max_row)

chart2.add_data(data2, titles_from_data=True)
chart2.set_categories(cats2)
chart2.height = 12
chart2.width = 22

ws2.add_chart(chart2, "H2")

wb.save("network.xlsx")
print("network.xlsx создан")
)PY";

    py.close();
    system("python3 create_excel.py");
}

int main() {
    const int exp_count = 10000;
    double b = 50.0;

    vector<double> focus_dists = {0, 50, 100, 150, 200, 250, 300};

    random_device rd;
    mt19937 gen(rd());

    ofstream out1("results_graph1.csv");

    vector<int> nodes_list = {4, 8, 12, 18, 24};
    double fixed_r = 45.0;

    out1 << "2c (fixed R=45);"
         << "N=4;"
         << "N=8;"
         << "N=12;"
         << "N=18;"
         << "N=24\n";

    for (double f : focus_dists) {
        double c = f / 2.0;
        double a = sqrt(b * b + c * c);

        out1 << to_excel(f);

        for (int n : nodes_list) {
            int success = 0;

            for (int e = 0; e < exp_count; e++) {
                vector<Point> pts = make_points(n, a, b, gen);

                if (is_connected(n, pts, fixed_r)) {
                    success++;
                }
            }

            double probability = (double)success / exp_count;
            out1 << ";" << to_excel(probability);
        }

        out1 << "\n";
    }

    out1.close();

    ofstream out2("results_graph2.csv");

    vector<double> radii_list = {30, 40, 50, 60, 70};
    int fixed_n = 18;

    out2 << "2c (fixed N=18);"
         << "R=30;"
         << "R=40;"
         << "R=50;"
         << "R=60;"
         << "R=70\n";

    for (double f : focus_dists) {
        double c = f / 2.0;
        double a = sqrt(b * b + c * c);

        out2 << to_excel(f);

        for (double r : radii_list) {
            int success = 0;

            for (int e = 0; e < exp_count; e++) {
                vector<Point> pts = make_points(fixed_n, a, b, gen);

                if (is_connected(fixed_n, pts, r)) {
                    success++;
                }
            }

            double probability = (double)success / exp_count;
            out2 << ";" << to_excel(probability);
        }

        out2 << "\n";
    }

    out2.close();

    cout << "Создание Excel..." << endl;

    create_excel();

    cout << "ГОТОВО" << endl;
    cout << "Файл: network.xlsx" << endl;

    return 0;
}
