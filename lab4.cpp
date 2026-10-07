#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

#if defined(_WIN32)
    #define PYTHON_COMMAND "python create_excel_lab4.py"
    #define OPEN_FILE_COMMAND "start \"\" flows.xlsx"
#elif defined(__APPLE__)
    #define PYTHON_COMMAND "python3 create_excel_lab4.py"
    #define OPEN_FILE_COMMAND "open flows.xlsx"
#else
    #define PYTHON_COMMAND "python3 create_excel_lab4.py"
    #define OPEN_FILE_COMMAND "xdg-open flows.xlsx"
#endif

using namespace std;

const int N = 5;        // число потоков
const double a = 0.0;       // интервалы между событиями
const double b = 8.0;

const int totalEvents = 100000;     // сколько событий 
const int binCount = 20;    // число столбцов для гистограммы 

int main() {
    random_device randomDevice;
    mt19937 randomGenerator(randomDevice());

    uniform_real_distribution<double> intervalGenerator(a, b);

    
    vector<double> nextEvent(N);
    for (int i = 0; i < N; i++) {
        nextEvent[i] = intervalGenerator(randomGenerator); // момент следующего события в каждом потоке
    }

    double binWidth = b / binCount;   //ширина интервала
    vector<int> hist(binCount, 0);  //для гистрограммы

    double sum = 0.0;       //сумма интервалов суммарного потока
    double sumSq = 0.0;     //функция распределения суммарного потока
    double lastEventTime = 0.0;     //время последнего события

    for (int event = 0; event < totalEvents; event++) {
        // ищем поток, у которого событие наступит раньше всех
        int flow = 0;
        for (int i = 1; i < N; i++) {
            if (nextEvent[i] < nextEvent[flow]) {
                flow = i;
            }
        }

        double currentTime = nextEvent[flow];
        double interval = currentTime - lastEventTime;

        sum += interval;
        sumSq += interval * interval;

        int k = (int)(interval / binWidth);     // номер интервала
        if (k >= binCount) k = binCount - 1;        // если интервал больше b, то помещаем в последний столбец
        hist[k]++;

        lastEventTime = currentTime;    // запоминаем момент последнего события
        nextEvent[flow] = currentTime + intervalGenerator(randomGenerator);     // генерируем момент следующего события в этом потоке
    }

    // теория: интенсивность суммы потоков = сумма интенсивностей
    double lambdaOne = 2.0 / (a + b);       // интенсивность одного потока 
    double lambdaSum = N * lambdaOne;       // интенсивность суммарного потока

    // на практике
    double mean = sum / totalEvents;    // средний интервал суммарного потока
    double disp = sumSq / totalEvents - mean * mean;    //дисперсия
    double sigma = sqrt(disp);      // среднеквадратичное отклонение

    cout << fixed << setprecision(4);

    cout << "N = " << N << ", a = " << a << ", b = " << b << endl;

    cout << "\nДолжно быть в теории\n";
    cout << "интенсивность одного потока:  " << lambdaOne << endl;
    cout << "интенсивность суммарного:     " << lambdaSum << endl;
    cout << "средний интервал суммарного:  " << 1.0 / lambdaSum << endl;

    cout << "\nНа практике\n";
    cout << "интенсивность суммарного:     " << totalEvents / lastEventTime << endl;
    cout << "средний интервал:             " << mean << endl;
    cout << "дисперсия:                    " << disp << endl;
    cout << "ср.кв. отклонение:            " << sigma << endl;
    cout << "коэф. вариации sigma/M:       " << sigma / mean
         << "  (у пуассоновского = 1)" << endl;

    cout << "\nПлотность распределения\n";
    cout << "   интервал        f(x) модел.   % интерв.   f(x) пуассон   % интерв.\n";

    ofstream dataFile("flows_data.txt");

    for (int k = 0; k < binCount; k++) {
        double from = k * binWidth;
        double to = (k + 1) * binWidth;
        double x = (k + 0.5) * binWidth;

        double density = hist[k] / (totalEvents * binWidth);
        double percent = 100.0 * hist[k] / totalEvents;

        double poissonDensity = lambdaSum * exp(-lambdaSum * x);    // плотность пуассоновского потока
        // вероятность попасть в [from; to) у пуассоновского потока: F(to) - F(from)
        double poissonPercent = 100.0 * (exp(-lambdaSum * from) - exp(-lambdaSum * to));

        cout << setprecision(1)
             << "[" << setw(4) << from << "; " << setw(4) << to << ")   "
             << setprecision(4) << setw(10) << density << "   "
             << setprecision(2) << setw(7) << percent << " %   "
             << setprecision(4) << setw(10) << poissonDensity << "   "
             << setprecision(2) << setw(7) << poissonPercent << " %" << endl;

        dataFile << x << " " << density << " " << poissonDensity << "\n";
    }

    dataFile.close();

    ofstream pythonFile("create_excel_lab4.py");

    pythonFile << "from openpyxl import Workbook\n";
    pythonFile << "from openpyxl.chart import LineChart, Reference\n";
    pythonFile << "\n";
    pythonFile << "workbook = Workbook()\n";
    pythonFile << "sheet = workbook.active\n";
    pythonFile << "sheet.append(['x', 'f(x) моделирование', 'f(x) пуассоновский'])\n";
    pythonFile << "\n";
    pythonFile << "with open('flows_data.txt') as file:\n";
    pythonFile << "    for line in file:\n";
    pythonFile << "        sheet.append([round(float(v), 4) for v in line.split()])\n";
    pythonFile << "\n";
    pythonFile << "chart = LineChart()\n";
    pythonFile << "chart.title = 'Плотность распределения интервалов суммарного потока'\n";
    pythonFile << "chart.x_axis.title = 'x'\n";
    pythonFile << "chart.y_axis.title = 'f(x)'\n";
    pythonFile << "chart.add_data(Reference(sheet, min_col=2, max_col=3, min_row=1, max_row=sheet.max_row), titles_from_data=True)\n";
    pythonFile << "chart.set_categories(Reference(sheet, min_col=1, min_row=2, max_row=sheet.max_row))\n";
    pythonFile << "chart.width = 20\n";
    pythonFile << "chart.height = 10\n";
    pythonFile << "sheet.add_chart(chart, 'E2')\n";
    pythonFile << "\n";
    pythonFile << "workbook.save('flows.xlsx')\n";

    pythonFile.close();

    system(PYTHON_COMMAND);

    cout << "\nExcel-файл flows.xlsx создан" << endl;

    system(OPEN_FILE_COMMAND);

    return 0;
}
