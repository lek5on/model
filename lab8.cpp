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
