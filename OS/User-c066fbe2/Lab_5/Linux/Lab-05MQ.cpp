#include <algorithm>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <random>
#include <string>
#include <vector>

constexpr int QUEUES = 3;
constexpr int QUANTUM[QUEUES] = { 15, 60, 150 };
constexpr double SHARE[QUEUES] = { 0.5, 0.3, 0.2 };
constexpr int PROCESS_COUNT = 100;
constexpr int DEFAULT_MAX_ARRIVAL = 30000;

struct Proc
{
    int id;
    bool ioBound;
    int arrival;
    int burst;
    int remaining;
    int burstLeft;
    int queue;
    int lastReady;
    int waiting;
    int ioTime;
    int firstRun;
    int finish;
    int finalQueue;
    int moves;
    int dispatches;
};

struct Event
{
    int time;
    int kind;
    int id;
    int queue;
};

struct Blocked
{
    int wake;
    int id;
    int queue;
};

static std::mt19937 rng;

static int Rand(int lo, int hi)
{
    return std::uniform_int_distribution<int>(lo, hi)(rng);
}

static void PrintGroup(const char* title, const std::vector<const Proc*>& group)
{
    printf("\n=== %s ===\n", title);
    if (group.empty())
    {
        printf("No processes finished in this queue\n");
        return;
    }
    printf("%-4s %-6s %-8s %-6s %-6s %-6s %-11s %-9s %-6s\n",
           "PID", "Type", "Arrival", "Burst", "I/O", "Wait", "Turnaround", "Response", "Moves");
    double wait = 0, turn = 0;
    for (const Proc* p : group)
    {
        int turnaround = p->finish - p->arrival;
        printf("%-4d %-6s %-8d %-6d %-6d %-6d %-11d %-9d %-6d\n",
               p->id, p->ioBound ? "I/O" : "CPU", p->arrival, p->burst, p->ioTime,
               p->waiting, turnaround, p->firstRun - p->arrival, p->moves);
        wait += p->waiting;
        turn += turnaround;
    }
    printf("Processes: %d, average waiting time: %.2f, average turnaround time: %.2f\n",
           (int)group.size(), wait / group.size(), turn / group.size());
}

int main(int argc, char* argv[])
{
    unsigned seed = 42;
    bool summaryOnly = false;
    int maxArrival = DEFAULT_MAX_ARRIVAL;
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-s") == 0)
            summaryOnly = true;
        else if (strcmp(argv[i], "-sat") == 0)
            maxArrival = 0;
        else
            seed = (unsigned)strtoul(argv[i], nullptr, 10);
    }
    rng.seed(seed);

    const int n = PROCESS_COUNT;
    std::vector<Proc> procs(n);
    for (int i = 0; i < n; ++i)
    {
        Proc& p = procs[i];
        p = Proc();
        p.id = i + 1;
        p.ioBound = Rand(0, 1) == 0;
        p.arrival = Rand(0, maxArrival);
        p.burst = p.ioBound ? Rand(100, 400) : Rand(300, 1200);
        p.remaining = p.burst;
        p.burstLeft = 0;
        p.queue = 0;
        p.firstRun = -1;
        p.finish = -1;
    }

    std::vector<int> order(n);
    for (int i = 0; i < n; ++i)
        order[i] = i;
    std::sort(order.begin(), order.end(), [&](int a, int b)
    {
        if (procs[a].arrival != procs[b].arrival)
            return procs[a].arrival < procs[b].arrival;
        return procs[a].id < procs[b].id;
    });

    std::deque<int> qs[QUEUES];
    double used[QUEUES] = {};
    int runningQueue = -1;
    long long cpuByQueue[QUEUES] = {};
    long long contended[QUEUES] = {};
    long long contendedTotal = 0;
    std::vector<Blocked> blocked;
    size_t nextArr = 0;
    int time = 0;
    int done = 0;
    int lastRun = -1;
    int switches = 0;
    int dispatches = 0;
    int moves = 0;
    long long busy = 0;
    int firstArrival = procs[order[0]].arrival;

    auto enqueue = [&](int k, int id, int at)
    {
        if (qs[k].empty() && k != runningQueue)
        {
            double minVt = -1;
            for (int j = 0; j < QUEUES; ++j)
            {
                if (qs[j].empty() && j != runningQueue)
                    continue;
                double vt = used[j] / SHARE[j];
                if (minVt < 0 || vt < minVt)
                    minVt = vt;
            }
            if (minVt >= 0)
                used[k] = std::max(used[k], minVt * SHARE[k]);
        }
        procs[id].lastReady = at;
        procs[id].queue = k;
        qs[k].push_back(id);
    };

    auto admit = [&](int now)
    {
        std::vector<Event> ev;
        while (nextArr < order.size() && procs[order[nextArr]].arrival <= now)
        {
            int id = order[nextArr++];
            ev.push_back({ procs[id].arrival, 0, id, 0 });
        }
        for (size_t i = 0; i < blocked.size();)
        {
            if (blocked[i].wake <= now)
            {
                ev.push_back({ blocked[i].wake, 1, blocked[i].id, blocked[i].queue });
                blocked.erase(blocked.begin() + i);
            }
            else
                ++i;
        }
        std::sort(ev.begin(), ev.end(), [](const Event& a, const Event& b)
        {
            if (a.time != b.time)
                return a.time < b.time;
            if (a.kind != b.kind)
                return a.kind < b.kind;
            return a.id < b.id;
        });
        for (const Event& e : ev)
            enqueue(e.queue, e.id, e.time);
    };

    time = firstArrival;

    while (done < n)
    {
        admit(time);

        int k = -1;
        double best = 0;
        for (int j = 0; j < QUEUES; ++j)
        {
            if (qs[j].empty())
                continue;
            double vt = used[j] / SHARE[j];
            if (k < 0 || vt < best)
            {
                k = j;
                best = vt;
            }
        }

        if (k < 0)
        {
            int nextEvent = INT_MAX;
            if (nextArr < order.size())
                nextEvent = std::min(nextEvent, procs[order[nextArr]].arrival);
            for (const Blocked& b : blocked)
                nextEvent = std::min(nextEvent, b.wake);
            time = nextEvent;
            continue;
        }

        bool allBusy = !qs[0].empty() && !qs[1].empty() && !qs[2].empty();
        int id = qs[k].front();
        qs[k].pop_front();
        Proc& pr = procs[id];

        pr.waiting += time - pr.lastReady;
        if (pr.firstRun < 0)
            pr.firstRun = time;
        if (pr.burstLeft == 0)
        {
            int drawn = pr.ioBound ? Rand(2, 14) : Rand(80, 250);
            pr.burstLeft = std::min(drawn, pr.remaining);
        }

        int run = std::min(QUANTUM[k], pr.burstLeft);
        if (lastRun != id)
            ++switches;
        lastRun = id;
        ++dispatches;
        ++pr.dispatches;

        time += run;
        runningQueue = k;
        used[k] += run;
        cpuByQueue[k] += run;
        if (allBusy)
        {
            contended[k] += run;
            contendedTotal += run;
        }
        busy += run;
        pr.remaining -= run;
        pr.burstLeft -= run;

        admit(time);

        if (pr.remaining == 0)
        {
            pr.finish = time;
            pr.finalQueue = k;
            ++done;
        }
        else if (pr.burstLeft == 0)
        {
            int target = k == 2 ? 1 : 0;
            if (target != k)
            {
                ++pr.moves;
                ++moves;
            }
            int io = pr.ioBound ? Rand(20, 60) : Rand(5, 20);
            pr.ioTime += io;
            blocked.push_back({ time + io, id, target });
        }
        else
        {
            int target;
            if (k == 0)
                target = 1;
            else if (k == 1)
                target = 2;
            else
                target = Rand(0, 2);
            if (target != k)
            {
                ++pr.moves;
                ++moves;
            }
            enqueue(target, id, time);
        }
        runningQueue = -1;
    }

    int lastFinish = 0;
    for (const Proc& p : procs)
        lastFinish = std::max(lastFinish, p.finish);
    int total = lastFinish - firstArrival;

    printf("Multilevel Feedback Queue emulator\n");
    printf("Processes: %d, seed: %u, arrivals within 0-%d ms%s\n", n, seed, maxArrival, maxArrival == 0 ? " (saturation test)" : "");
    printf("Q0: RR, quantum %d ms, share %.0f%% | Q1: RR, quantum %d ms, share %.0f%% | Q2: FCFS, quantum %d ms, share %.0f%%\n",
           QUANTUM[0], SHARE[0] * 100, QUANTUM[1], SHARE[1] * 100, QUANTUM[2], SHARE[2] * 100);

    if (!summaryOnly)
    {
        const char* titles[QUEUES] = { "Queue Q0 (processes that finished in Q0)",
                                       "Queue Q1 (processes that finished in Q1)",
                                       "Queue Q2 (processes that finished in Q2)" };
        for (int k = 0; k < QUEUES; ++k)
        {
            std::vector<const Proc*> group;
            for (const Proc& p : procs)
                if (p.finalQueue == k)
                    group.push_back(&p);
            PrintGroup(titles[k], group);
        }
    }

    double wait = 0, turn = 0, resp = 0;
    double typeWait[2] = {}, typeTurn[2] = {}, typeResp[2] = {};
    int typeCount[2] = {};
    bool consistent = true;
    for (const Proc& p : procs)
    {
        int turnaround = p.finish - p.arrival;
        wait += p.waiting;
        turn += turnaround;
        resp += p.firstRun - p.arrival;
        int t = p.ioBound ? 0 : 1;
        typeWait[t] += p.waiting;
        typeTurn[t] += turnaround;
        typeResp[t] += p.firstRun - p.arrival;
        ++typeCount[t];
        if (turnaround != p.burst + p.ioTime + p.waiting)
            consistent = false;
    }

    printf("\n=== Overall statistics ===\n");
    printf("Average waiting time:     %.2f ms\n", wait / n);
    printf("Average turnaround time:  %.2f ms\n", turn / n);
    printf("Average response time:    %.2f ms\n", resp / n);
    printf("Total execution time:     %d ms\n", total);
    printf("Context switches:         %d (dispatches: %d)\n", switches, dispatches);
    printf("CPU utilization:          %.2f%%\n", 100.0 * busy / total);
    printf("Queue moves:              %d\n", moves);
    printf("Check turnaround = burst + I/O + waiting: %s\n", consistent ? "OK" : "FAILED");

    printf("\nCPU time by queue:\n");
    for (int k = 0; k < QUEUES; ++k)
        printf("Q%d: %lld ms (%.2f%% of busy time, target %.0f%%)\n",
               k, cpuByQueue[k], 100.0 * cpuByQueue[k] / busy, SHARE[k] * 100);

    printf("\nCPU time by queue while all three queues were non-empty (%lld ms):\n", contendedTotal);
    for (int k = 0; k < QUEUES; ++k)
        printf("Q%d: %lld ms (%.2f%%, target %.0f%%)\n", k, contended[k],
               contendedTotal > 0 ? 100.0 * contended[k] / contendedTotal : 0.0, SHARE[k] * 100);

    printf("\nBy process type:\n");
    const char* names[2] = { "I/O-bound", "CPU-bound" };
    for (int t = 0; t < 2; ++t)
        printf("%-10s count %-3d avg waiting %.2f, avg turnaround %.2f, avg response %.2f\n",
               names[t], typeCount[t], typeWait[t] / typeCount[t], typeTurn[t] / typeCount[t], typeResp[t] / typeCount[t]);

    return 0;
}
