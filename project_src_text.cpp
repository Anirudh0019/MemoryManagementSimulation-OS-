#include<iostream>
#include<vector>
#include<queue>
#include<string>
#include<random>
#include<algorithm>
#include<iomanip>
#include<numeric>
#include<map>

using namespace std;

struct MemoryAccess {
    string address;
    int access_time;
    string found_in;
};

struct Process {
    int id;
    int arrival_time;
    vector<string> addresses_to_access;
    vector<MemoryAccess> memory_accesses;
    int total_execution_time;
    int start_time;
    int end_time;
};

class MemorySimulator {
private:
    vector<string> cache;
    vector<string> page;
    vector<string> disk;
    vector<Process> processes;
    int current_time;
    map<string, int> access_counts;
    int cache_size, page_size, disk_size;

    string access_memory(const string& address) {
        access_counts[address]++;
        
        auto cache_it = find(cache.begin(), cache.end(), address);
        if (cache_it != cache.end()) {
            return "cache";
        }

        auto page_it = find(page.begin(), page.end(), address);
        if (page_it != page.end()) {
            move_to_cache(address);
            return "page";
        }

        auto disk_it = find(disk.begin(), disk.end(), address);
        if (disk_it != disk.end()) {
            move_to_page(address);
            move_to_cache(address);
            return "disk";
        }

        // If not found,add to disk, then move to page and cache
        if (disk.size() >= disk_size) {
            disk.erase(disk.begin());
        }
        disk.push_back(address);
        move_to_page(address);
        move_to_cache(address);
        return "not found";
    }

    void move_to_cache(const string& address) {
        auto page_it = find(page.begin(), page.end(), address);
        if (page_it != page.end()) {
            page.erase(page_it);
        }
        
        if (cache.size() >= cache_size) {
            auto lru_it = min_element(cache.begin(), cache.end(),
                [this](const string& a, const string& b) {
                    return access_counts[a] < access_counts[b];
                });
            page.push_back(*lru_it);
            cache.erase(lru_it);
        }
        cache.push_back(address);
    }

    void move_to_page(const string& address) {
        auto disk_it = find(disk.begin(), disk.end(), address);
        if (disk_it != disk.end()) {
            disk.erase(disk_it);
        }
        
        if (page.size() >= page_size) {
            auto lru_it = min_element(page.begin(), page.end(),
                [this](const string& a, const string& b) {
                    return access_counts[a] < access_counts[b];
                });
            disk.push_back(*lru_it);
            page.erase(lru_it);
        }
        page.push_back(address);
    }

public:
    MemorySimulator(int c_size, int p_size, int d_size) 
        : cache_size(c_size), page_size(p_size), disk_size(d_size) {
        for (int i = 0; i < disk_size; i++) {
            string address = "A" + to_string(i);
            disk.push_back(address);
            if (i < p_size) {
                move_to_page(address);
                if (i < c_size) {
                    move_to_cache(address);
                }
            }
        }
        current_time = 0;
    }

    void add_process(int arrival_time, const vector<string>& addresses) {
        Process p;
        p.id = processes.size() + 1;
        p.arrival_time = arrival_time;
        p.addresses_to_access = addresses;
        p.total_execution_time = 0;
        processes.push_back(p);
    }

    void run_simulation() {
        sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
            return a.arrival_time < b.arrival_time;
        });

        for (auto& p : processes) {
            if (current_time < p.arrival_time) current_time = p.arrival_time;
            p.start_time = current_time;
            for (const auto& addr : p.addresses_to_access) {
                string found_in = access_memory(addr);
                int access_time = (found_in == "cache") ? 1 : (found_in == "page") ? 10 : 100;
                p.memory_accesses.push_back({addr, access_time, found_in});
                p.total_execution_time += access_time;
                current_time += access_time;
            }
            p.end_time = current_time;
        }
    }

    void print_stats() {
        cout << "\nSimulation Statistics:" << endl;
        cout << "Total time: " << current_time << endl;
        
        map<string, int> hit_counts;
        for (const auto& p : processes) {
            for (const auto& access : p.memory_accesses) {
                hit_counts[access.found_in]++;
            }
        }
        
        int total_hits = hit_counts["cache"] + hit_counts["page"] + hit_counts["disk"] + hit_counts["not found"];
        
        cout << "Hit counts:" << endl;
        cout << "  Cache: " << hit_counts["cache"] << endl;
        cout << "  Page: " << hit_counts["page"] << endl;
        cout << "  Disk: " << hit_counts["disk"] << endl;
        cout << "  Not found: " << hit_counts["not found"] << endl;
        
        cout << "Hit ratios:" << endl;
        cout << "  Cache: " << fixed << setprecision(2) << (float)hit_counts["cache"] / total_hits << endl;
        cout << "  Page: " << fixed << setprecision(2) << (float)hit_counts["page"] / total_hits << endl;
        cout << "  Disk: " << fixed << setprecision(2) << (float)hit_counts["disk"] / total_hits << endl;
        cout << "  Not found: " << fixed << setprecision(2) << (float)hit_counts["not found"] / total_hits << endl;
        
        cout << "\nAverage access times:" << endl;
        for (const auto& p : processes) {
            int total_time = accumulate(p.memory_accesses.begin(), p.memory_accesses.end(), 0,
                [](int sum, const MemoryAccess& access) { return sum + access.access_time; });
            cout << "  Process " << p.id << ": " << fixed << setprecision(2) 
                 << (float)total_time / p.memory_accesses.size() << endl;
        }
    }

    void print_gantt_chart() {
        cout << "\nGantt Chart:" << endl;
        int chart_width = 50;
        int total_time = processes.back().end_time;
        
        for (const auto& p : processes) {
            cout << "P" << p.id << " |";
            int start_pos = (p.start_time * chart_width) / total_time;
            int end_pos = (p.end_time * chart_width) / total_time;
            
            for (int i = 0; i < chart_width; i++) {
                if (i >= start_pos && i < end_pos) cout << "=";
                else cout << " ";
            }
            cout << "| " << p.start_time << " - " << p.end_time << endl;
        }
    }

    void print_detailed_access() {
        for (const auto& p : processes) {
            cout << "\nProcess " << p.id << " memory accesses:" << endl;
            for (const auto& access : p.memory_accesses) {
                cout << "Address: " << access.address 
                     << ", Access time: " << access.access_time 
                     << ", Found in: " << access.found_in << endl;
            }
        }
    }
};

int main() {
    MemorySimulator sim(20, 40, 80); // create 20 in cache, 40 in page, and 80 in disk

    
    vector<string> preload;
    for (int i = 0; i < 50; i++) {
        preload.push_back("A" + to_string(i));
    }
    sim.add_process(0, preload); // pre loading every addresss into singular process to fill caches,page

    
    sim.add_process(1, {"A0", "A1", "A2", "A0", "A3", "A1", "A4", "A2", "A5", "A3"});
   
    sim.add_process(2, {"A6", "A7", "A0", "A8", "A1", "A9", "A2", "A10", "A3", "A11"});
    sim.add_process(4, {"A12", "A13", "A4", "A14", "A5", "A15", "A6", "A16", "A7", "A17"});
    sim.add_process(6, {"A18", "A19", "A0", "A20", "A1", "A21", "A12", "A22", "A13", "A23"});
    sim.add_process(8, {"A24", "A25", "A26", "A24", "A27", "A25", "A28", "A26", "A29", "A27"});//Mostly new addresses with some repeats

    sim.run_simulation();
    sim.print_stats();
    sim.print_gantt_chart();
    sim.print_detailed_access();

    return 0;
}
