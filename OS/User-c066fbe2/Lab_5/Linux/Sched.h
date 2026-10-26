#pragma once
#include <algorithm>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <fstream>
#include <string>
#include <vector>

struct Task
{
    int id;
    int arrival;
    int burst;
    int priority;
    int remaining;
    int start;
    int finish;
};

struct Slice
{
    int from;
    int to;
    int id;
};

struct Result
{
    std::vector<Task> tasks;
    std::vector<Slice> gantt;
};

struct Stats
{
    double avgWait;
    double avgBurst;
    double avgTurnaround;
    double avgResponse;
    double throughput;
    double utilization;
    int makespan;
    int switches;
};

inline Task MakeTask(int id, int arrival, int burst, int priority)
{
    Task t;
    t.id = id;
    t.arrival = arrival;
    t.burst = burst;
    t.priority = priority;
    t.remaining = burst;
    t.start = -1;
    t.finish = -1;
    return t;
}

inline std::vector<Task> DefaultTasks()
{
    return {
        MakeTask(1, 0, 80, 3),
        MakeTask(2, 20, 45, 5),
        MakeTask(3, 30, 60, 2),
        MakeTask(4, 50, 30, 4),
        MakeTask(5, 70, 90, 1),
        MakeTask(6, 100, 55, 3),
        MakeTask(7, 120, 75, 2),
        MakeTask(8, 150, 40, 5),
        MakeTask(9, 180, 65, 4),
        MakeTask(10, 200, 85, 1)
    };
}

inline bool LoadTasks(const char* path, std::vector<Task>& out)
{
    std::ifstream in(path);
    if (!in)
        return false;
    out.clear();
    int id, a, b, p;
    while (in >> id >> a >> b >> p)
        out.push_back(MakeTask(id, a, b, p));
    return !out.empty();
}

inline void AddSlice(Result& r, int from, int to, int id)
{
    if (to <= from)
        return;
    if (!r.gantt.empty() && r.gantt.back().id == id && r.gantt.back().to == from)
    {
        r.gantt.back().to = to;
        return;
    }
    Slice s;
    s.from = from;
    s.to = to;
    s.id = id;
    r.gantt.push_back(s);
}

inline std::vector<int> ArrivalOrder(const std::vector<Task>& t)
{
    std::vector<int> order(t.size());
    for (size_t i = 0; i < t.size(); ++i)
        order[i] = (int)i;
    std::sort(order.begin(), order.end(), [&](int a, int b)
    {
        if (t[a].arrival != t[b].arrival)
            return t[a].arrival < t[b].arrival;
        return t[a].id < t[b].id;
    });
    return order;
}

inline int FirstArrival(const std::vector<Task>& t)
{
    int m = INT_MAX;
    for (const Task& x : t)
        m = std::min(m, x.arrival);
    return m;
}

inline Result RunSJF(std::vector<Task> t)
{
    Result r;
    int n = (int)t.size();
    int time = FirstArrival(t);
    int done = 0;
    while (done < n)
    {
        int best = -1;
        for (int i = 0; i < n; ++i)
        {
            if (t[i].finish >= 0 || t[i].arrival > time)
                continue;
            if (best < 0 || t[i].burst < t[best].burst ||
                (t[i].burst == t[best].burst && t[i].arrival < t[best].arrival) ||
                (t[i].burst == t[best].burst && t[i].arrival == t[best].arrival && t[i].id < t[best].id))
                best = i;
        }
        if (best < 0)
        {
            int next = INT_MAX;
            for (int i = 0; i < n; ++i)
                if (t[i].finish < 0)
                    next = std::min(next, t[i].arrival);
            AddSlice(r, time, next, -1);
            time = next;
            continue;
        }
        t[best].start = time;
        AddSlice(r, time, time + t[best].burst, t[best].id);
        time += t[best].burst;
        t[best].remaining = 0;
        t[best].finish = time;
        ++done;
    }
    r.tasks = t;
    return r;
}

inline Result RunRR(std::vector<Task> t, int q)
{
    Result r;
    int n = (int)t.size();
    std::vector<int> order = ArrivalOrder(t);
    size_t next = 0;
    std::deque<int> queue;
    int time = FirstArrival(t);
    int done = 0;

    auto admit = [&]()
    {
        while (next < order.size() && t[order[next]].arrival <= time)
            queue.push_back(order[next++]);
    };

    while (done < n)
    {
        admit();
        if (queue.empty())
        {
            int ta = t[order[next]].arrival;
            AddSlice(r, time, ta, -1);
            time = ta;
            continue;
        }
        int i = queue.front();
        queue.pop_front();
        if (t[i].start < 0)
            t[i].start = time;
        int run = std::min(q, t[i].remaining);
        AddSlice(r, time, time + run, t[i].id);
        time += run;
        t[i].remaining -= run;
        admit();
        if (t[i].remaining == 0)
        {
            t[i].finish = time;
            ++done;
        }
        else
            queue.push_back(i);
    }
    r.tasks = t;
    return r;
}

inline Result RunPriority(std::vector<Task> t, int q, bool bigIsHigh)
{
    Result r;
    int n = (int)t.size();
    std::vector<int> order = ArrivalOrder(t);
    size_t next = 0;
    std::vector<int> ready;
    std::vector<long long> seq(n, 0);
    long long counter = 0;
    int time = FirstArrival(t);
    int done = 0;

    auto better = [&](int a, int b)
    {
        return bigIsHigh ? t[a].priority > t[b].priority : t[a].priority < t[b].priority;
    };

    auto admit = [&]()
    {
        while (next < order.size() && t[order[next]].arrival <= time)
        {
            int j = order[next++];
            seq[j] = counter++;
            ready.push_back(j);
        }
    };

    while (done < n)
    {
        admit();
        if (ready.empty())
        {
            int ta = t[order[next]].arrival;
            AddSlice(r, time, ta, -1);
            time = ta;
            continue;
        }
        size_t pos = 0;
        for (size_t k = 1; k < ready.size(); ++k)
        {
            int a = ready[k];
            int b = ready[pos];
            if (better(a, b) || (t[a].priority == t[b].priority && seq[a] < seq[b]))
                pos = k;
        }
        int i = ready[pos];
        ready.erase(ready.begin() + pos);
        if (t[i].start < 0)
            t[i].start = time;
        int run = std::min(q, t[i].remaining);
        for (size_t k = next; k < order.size(); ++k)
        {
            int j = order[k];
            if (better(j, i))
            {
                if (time + run > t[j].arrival)
                    run = t[j].arrival - time;
                break;
            }
        }
        AddSlice(r, time, time + run, t[i].id);
        time += run;
        t[i].remaining -= run;
        admit();
        if (t[i].remaining == 0)
        {
            t[i].finish = time;
            ++done;
        }
        else
        {
            seq[i] = counter++;
            ready.push_back(i);
        }
    }
    r.tasks = t;
    return r;
}

inline Stats Compute(const Result& r)
{
    Stats s = {};
    double wait = 0, burst = 0, turn = 0, resp = 0;
    int last = 0;
    int first = INT_MAX;
    for (const Task& t : r.tasks)
    {
        int turnaround = t.finish - t.arrival;
        wait += turnaround - t.burst;
        burst += t.burst;
        turn += turnaround;
        resp += t.start - t.arrival;
        last = std::max(last, t.finish);
        first = std::min(first, t.arrival);
    }
    double n = (double)r.tasks.size();
    int busy = 0;
    int slices = 0;
    for (const Slice& sl : r.gantt)
    {
        if (sl.id < 0)
            continue;
        busy += sl.to - sl.from;
        ++slices;
    }
    s.avgWait = wait / n;
    s.avgBurst = burst / n;
    s.avgTurnaround = turn / n;
    s.avgResponse = resp / n;
    s.makespan = last - first;
    s.throughput = n / (double)s.makespan;
    s.utilization = 100.0 * busy / (double)s.makespan;
    s.switches = slices > 0 ? slices - 1 : 0;
    return s;
}

inline void PrintGantt(const Result& r)
{
    printf("Gantt chart:\n");
    int col = 0;
    for (const Slice& s : r.gantt)
    {
        if (s.id < 0)
            printf("[%d-%d idle] ", s.from, s.to);
        else
            printf("[%d-%d P%d] ", s.from, s.to, s.id);
        if (++col % 6 == 0)
            printf("\n");
    }
    if (col % 6 != 0)
        printf("\n");
}

inline void PrintTable(const Result& r)
{
    printf("%-4s %-8s %-6s %-5s %-6s %-7s %-6s %-11s %-8s\n",
           "PID", "Arrival", "Burst", "Prio", "Start", "Finish", "Wait", "Turnaround", "Response");
    std::vector<Task> sorted = r.tasks;
    std::sort(sorted.begin(), sorted.end(), [](const Task& a, const Task& b) { return a.id < b.id; });
    for (const Task& t : sorted)
    {
        int turnaround = t.finish - t.arrival;
        printf("%-4d %-8d %-6d %-5d %-6d %-7d %-6d %-11d %-8d\n",
               t.id, t.arrival, t.burst, t.priority, t.start, t.finish,
               turnaround - t.burst, turnaround, t.start - t.arrival);
    }
}

inline void PrintStats(const Stats& s)
{
    printf("Average waiting time:    %.2f\n", s.avgWait);
    printf("Average execution time:  %.2f\n", s.avgBurst);
    printf("Average turnaround time: %.2f\n", s.avgTurnaround);
    printf("Average response time:   %.2f\n", s.avgResponse);
    printf("Throughput:              %.4f tasks/unit (%.2f per 100 units)\n", s.throughput, s.throughput * 100.0);
    printf("Total time:              %d\n", s.makespan);
    printf("CPU utilization:         %.2f%%\n", s.utilization);
    printf("Context switches:        %d\n", s.switches);
}

inline void PrintAll(const Result& r)
{
    PrintGantt(r);
    printf("\n");
    PrintTable(r);
    printf("\n");
    PrintStats(Compute(r));
}

inline const std::vector<int>& QuantumList()
{
    static const std::vector<int> list = { 5, 10, 15, 20, 25, 30, 40, 50, 60, 80, 100, 200 };
    return list;
}

inline void PrintSweepHeader()
{
    printf("%-8s %-10s %-12s %-10s %-12s %-10s\n", "Quantum", "AvgWait", "AvgTurnaround", "AvgResp", "Throughput", "Switches");
}

inline void PrintSweepRow(int q, const Stats& s)
{
    printf("%-8d %-10.2f %-12.2f %-10.2f %-12.4f %-10d\n", q, s.avgWait, s.avgTurnaround, s.avgResponse, s.throughput, s.switches);
}

inline bool ParseOption(int argc, char* argv[], int& i, std::vector<Task>& tasks, bool* low)
{
    if (strcmp(argv[i], "-f") == 0 && i + 1 < argc)
    {
        if (!LoadTasks(argv[i + 1], tasks))
        {
            printf("Cannot read tasks from %s\n", argv[i + 1]);
            exit(1);
        }
        ++i;
        return true;
    }
    if (low && strcmp(argv[i], "-low") == 0)
    {
        *low = true;
        return true;
    }
    return false;
}