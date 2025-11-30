#include <bits/stdc++.h>
using namespace std;

// Many-to-one Deferred Acceptance (DA) with tasks as proposers and capacities on servers
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    if (!(cin >> n >> m)) {
        // No input, terminate
        return 0;
    }

    // Server capacities (1-based indexing)
    vector<long long> C(n + 1);
    for (int s = 1; s <= n; ++s) {
        cin >> C[s];
    }

    // Job demands and preferences
    vector<long long> w(m + 1);
    vector<vector<int>> pref_server(m + 1);   // preference list of servers for each job
    vector<int> ptr(m + 1, 0);                // next position in the preference list to propose to
    vector<int> current_server(m + 1, 0);     // current server that temporarily holds the job, 0 = unmatched

    // score[s][j] = score of job j at server s
    vector<unordered_map<int, long long>> score(n + 1);

    for (int j = 1; j <= m; ++j) {
        int Lj;
        cin >> w[j] >> Lj;
        pref_server[j].resize(Lj);
        for (int k = 0; k < Lj; ++k) {
            int s;
            long long sc;
            cin >> s >> sc;
            pref_server[j][k] = s;
            score[s][j] = sc;
        }
    }

    // assigned[s] = list of jobs currently held by server s
    vector<vector<int>> assigned(n + 1);

    while (true) {
        // proposals[s] collects jobs proposing to server s in this round
        vector<vector<int>> proposals(n + 1);
        bool has_new_proposal = false;

        // Proposal phase: only unmatched jobs with remaining preferences propose
        for (int j = 1; j <= m; ++j) {
            if (current_server[j] != 0) continue;
            if (ptr[j] >= (int)pref_server[j].size()) continue;
            int s = pref_server[j][ptr[j]];
            ++ptr[j];
            proposals[s].push_back(j);
            has_new_proposal = true;
        }

        if (!has_new_proposal) {
            // No new proposals, DA terminates
            break;
        }

        // Server response phase
        for (int s = 1; s <= n; ++s) {
            if (proposals[s].empty() && assigned[s].empty()) continue;

            // Candidate set: previously held jobs + newly proposed jobs
            vector<int> cand;
            cand.reserve(assigned[s].size() + proposals[s].size());
            for (int j : assigned[s]) cand.push_back(j);
            for (int j : proposals[s]) cand.push_back(j);
            if (cand.empty()) continue;

            // Build (score, job) list and sort by score descending, tie-break by job id ascending
            vector<pair<long long, int>> cand_list;
            cand_list.reserve(cand.size());
            for (int j : cand) {
                auto it = score[s].find(j);
                if (it == score[s].end()) continue; // shouldn't happen if input is consistent
                cand_list.emplace_back(it->second, j);
            }

            sort(cand_list.begin(), cand_list.end(),
                 [](const pair<long long, int>& a,
                    const pair<long long, int>& b) {
                     if (a.first != b.first) return a.first > b.first;
                     return a.second < b.second;
                 });

            // Greedily accept jobs while respecting capacity C[s]
            long long used = 0;
            vector<int> new_assigned;
            new_assigned.reserve(cand_list.size());
            for (auto &p : cand_list) {
                int j = p.second;
                if (used + w[j] <= C[s]) {
                    used += w[j];
                    new_assigned.push_back(j);
                }
            }

            // Sort new_assigned for binary search convenience
            sort(new_assigned.begin(), new_assigned.end());

            // Jobs previously assigned to s but not in new_assigned become unmatched
            for (int j : assigned[s]) {
                if (!binary_search(new_assigned.begin(),
                                   new_assigned.end(), j)) {
                    current_server[j] = 0;
                }
            }

            // Jobs in new_assigned are now held by server s
            for (int j : new_assigned) {
                current_server[j] = s;
            }

            assigned[s].swap(new_assigned);
        }
    }

    // Output: for each job j, print the server it is matched to (or 0 if unmatched)
    for (int j = 1; j <= m; ++j) {
        cout << current_server[j] << '\n';
    }

    return 0;
}
