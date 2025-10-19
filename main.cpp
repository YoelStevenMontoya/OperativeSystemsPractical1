/*
Implementación MLQ (RR(1), RR(3), SJF)
Autor: Yoel Steven Montoya Hernandez 2416571
*/

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstring>

#define MAXP 200
#define QUEUE_SIZE 200

using namespace std;

/* Clase que representa un proceso */
class Process {
public:
    string label;
    int BT;
    int AT;
    int Q;
    int Pr;
    int rem;
    int start;
    int CT;

    /* Inicializa atributos del proceso */
    Process() {
        label = "";
        BT = AT = Q = Pr = rem = 0;
        start = -1;
        CT = -1;
    }
};

/* Clase que implementa una cola simple circular */
class SimpleQueue {
public:
    int arr[QUEUE_SIZE];
    int front;
    int rear;
    int count;

    /* Inicializa la cola vacía */
    SimpleQueue() {
        front = 0; rear = 0; count = 0;
        for (int i=0;i<QUEUE_SIZE;i++) arr[i] = -1;
    }

    /* Verifica si está vacía */
    bool empty() { return count == 0; }

    /* Verifica si está llena */
    bool full() { return count == QUEUE_SIZE; }

    /* Limpia la cola */
    void clear() { front = rear = count = 0; }

    /* Encola un elemento */
    void enqueue(int x) {
        if (full()) return;
        arr[rear] = x;
        rear = (rear + 1) % QUEUE_SIZE;
        count++;
    }

    /* Desencola elemento en posición dada */
    int dequeueAt(int pos) {
        if (pos < 0 || pos >= count) return -1;
        int idx = (front + pos) % QUEUE_SIZE;
        int val = arr[idx];
        for (int i = pos; i < count - 1; ++i) {
            int from = (front + i + 1) % QUEUE_SIZE;
            int to = (front + i) % QUEUE_SIZE;
            arr[to] = arr[from];
        }
        rear = (rear - 1 + QUEUE_SIZE) % QUEUE_SIZE;
        arr[rear] = -1;
        count--;
        return val;
    }

    /* Retorna posición del proceso con mayor prioridad */
    int findHighestPriority(Process procs[]) {
        if (count == 0) return -1;
        int bestPos = 0;
        int bestPr = procs[arr[front]].Pr;
        for (int i = 1; i < count; ++i) {
            int idx = (front + i) % QUEUE_SIZE;
            int pid = arr[idx];
            if (procs[pid].Pr > bestPr) {
                bestPr = procs[pid].Pr;
                bestPos = i;
            }
        }
        return bestPos;
    }

    /* Retorna posición del proceso más corto */
    int findShortest(Process procs[]) {
        if (count == 0) return -1;
        int bestPos = 0;
        int bestRem = procs[arr[front]].rem;
        for (int i = 1; i < count; ++i) {
            int idx = (front + i) % QUEUE_SIZE;
            int pid = arr[idx];
            if (procs[pid].rem < bestRem) {
                bestRem = procs[pid].rem;
                bestPos = i;
            }
        }
        return bestPos;
    }

    /* Desencola el de mayor prioridad */
    int dequeueByPriority(Process procs[]) {
        int pos = findHighestPriority(procs);
        if (pos == -1) return -1;
        return dequeueAt(pos);
    }

    /* Desencola el más corto */
    int dequeueShortest(Process procs[]) {
        int pos = findShortest(procs);
        if (pos == -1) return -1;
        return dequeueAt(pos);
    }
};

/* Función principal: lee archivo, simula y genera salida */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Uso: " << argv[0] << " <archivo_entrada> [archivo_salida]" << endl;
        return 1;
    }

    string inname = argv[1];
    string outname = "mlq_output.txt";
    if (argc >= 3) outname = argv[2];

    ifstream fin(inname.c_str());
    if (!fin) {
        cout << "No se pudo abrir el archivo de entrada: " << inname << endl;
        return 1;
    }

    Process procs[MAXP];
    int n = 0;
    string line;

    /* Lee procesos del archivo */
    while (getline(fin, line)) {
        if (line.size() == 0) continue;
        int p = 0;
        while (p < (int)line.size() && isspace((unsigned char)line[p])) p++;
        if (p == (int)line.size()) continue;
        if (line[p] == '#') continue;

        string token;
        string toks[5];
        int tcount = 0;
        stringstream ss(line);
        while (getline(ss, token, ';') && tcount < 5) {
            int a = 0;
            while (a < (int)token.size() && isspace((unsigned char)token[a])) a++;
            int b = (int)token.size() - 1;
            while (b >= 0 && isspace((unsigned char)token[b])) { token.erase(b,1); b--; }
            string tk = token.substr(a);
            toks[tcount++] = tk;
        }
        if (tcount < 5) continue;

        procs[n].label = toks[0];
        procs[n].BT = atoi(toks[1].c_str());
        procs[n].AT = atoi(toks[2].c_str());
        procs[n].Q  = atoi(toks[3].c_str());
        procs[n].Pr = atoi(toks[4].c_str());
        procs[n].rem = procs[n].BT;
        procs[n].start = -1;
        procs[n].CT = -1;
        n++;
        if (n >= MAXP) break;
    }
    fin.close();

    if (n == 0) {
        cout << "No se encontró ningún proceso en el archivo de entrada." << endl;
        return 1;
    }

    SimpleQueue q1, q2, q3;
    int time = 0, completed = 0;
    bool arrived[MAXP];
    for (int i=0;i<n;i++) arrived[i] = false;

    /* Añade procesos que llegan en el tiempo actual */
    auto addArrivals = [&](int t) {
        for (int i=0;i<n;i++) {
            if (!arrived[i] && procs[i].AT <= t) {
                if (procs[i].Q == 1) q1.enqueue(i);
                else if (procs[i].Q == 2) q2.enqueue(i);
                else q3.enqueue(i);
                arrived[i] = true;
            }
        }
    };

    addArrivals(0);

    /* Bucle principal de simulación */
    while (completed < n) {
        if (!q1.empty() || !q2.empty() || !q3.empty()) {
            if (!q1.empty()) {
                int pid = q1.dequeueByPriority(procs);
                if (pid < 0) { time++; addArrivals(time); continue; }
                if (procs[pid].start == -1) procs[pid].start = time;
                int q = 1;
                int exec = (procs[pid].rem < q) ? procs[pid].rem : q;
                for (int i=0;i<exec;i++) { time++; addArrivals(time); }
                procs[pid].rem -= exec;
                if (procs[pid].rem == 0) { procs[pid].CT = time; completed++; }
                else q1.enqueue(pid);
            } else if (!q2.empty()) {
                int pid = q2.dequeueByPriority(procs);
                if (pid < 0) { time++; addArrivals(time); continue; }
                if (procs[pid].start == -1) procs[pid].start = time;
                int q = 3;
                int exec = (procs[pid].rem < q) ? procs[pid].rem : q;
                for (int i=0;i<exec;i++) { time++; addArrivals(time); }
                procs[pid].rem -= exec;
                if (procs[pid].rem == 0) { procs[pid].CT = time; completed++; }
                else q2.enqueue(pid);
            } else {
                int pid = q3.dequeueShortest(procs);
                if (pid < 0) { time++; addArrivals(time); continue; }
                if (procs[pid].start == -1) procs[pid].start = time;
                int exec = procs[pid].rem;
                for (int i=0;i<exec;i++) { time++; addArrivals(time); }
                procs[pid].rem = 0;
                procs[pid].CT = time;
                completed++;
            }
        } else {
            int nextAT = -1;
            for (int i=0;i<n;i++) {
                if (!arrived[i]) {
                    if (nextAT == -1 || procs[i].AT < nextAT) nextAT = procs[i].AT;
                }
            }
            if (nextAT == -1) break;
            time = (time < nextAT) ? nextAT : time + 1;
            addArrivals(time);
        }
    }

    /* Calcula métricas y genera salida */
    ofstream fout(outname.c_str());
    if (!fout) {
        cout << "No se pudo crear el archivo de salida: " << outname << endl;
        return 1;
    }

    fout << "# archivo: " << inname << endl;
    fout << "# etiqueta; BT; AT; Q; Pr; WT; CT; RT; TAT" << endl;

    double sumWT = 0.0, sumCT = 0.0, sumRT = 0.0, sumTAT = 0.0;
    for (int i=0;i<n;i++) {
        int TAT = procs[i].CT - procs[i].AT;
        int WT  = TAT - procs[i].BT;
        int RT  = procs[i].start - procs[i].AT;
        int CT  = procs[i].CT;
        sumWT += WT;
        sumCT += CT;
        sumRT += RT;
        sumTAT += TAT;
        fout << procs[i].label << ";" << procs[i].BT << ";" << procs[i].AT << ";" << procs[i].Q << ";" << procs[i].Pr
             << ";" << WT << ";" << CT << ";" << RT << ";" << TAT << endl;
    }

    double avgWT = sumWT / n;
    double avgCT = sumCT / n;
    double avgRT = sumRT / n;
    double avgTAT = sumTAT / n;

    fout << fixed << setprecision(2);
    fout << endl;
    fout << "WT=" << avgWT << "; CT=" << avgCT << "; RT=" << avgRT << "; TAT=" << avgTAT << ";" << endl;

    fout.close();
    cout << "Simulación completada. Salida escrita en: " << outname << endl;
    return 0;
}
