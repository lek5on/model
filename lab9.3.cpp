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