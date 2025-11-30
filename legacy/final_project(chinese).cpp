#include <bits/stdc++.h>
using namespace std;

// 多对一 DA：任务提案方，带容量
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    if (!(cin >> n >> m)) {
        return 0; // 无输入直接结束
    }

    // 服务器容量（1-based）
    vector<long long> C(n + 1);
    for (int s = 1; s <= n; ++s) {
        cin >> C[s];
    }

    // 任务需求 & 偏好
    vector<long long> w(m + 1);
    vector<vector<int>> pref_server(m + 1);         // 每个任务的服务器偏好序列
    vector<int> ptr(m + 1, 0);                      // 每个任务下一次要提案的位置
    vector<int> current_server(m + 1, 0);           // 任务当前暂时匹配的服务器，0 表示未匹配

    // 为了快速取 score_{s,j}，按 server 存一张表
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

    // 每个服务器暂时接收的任务集合
    vector<vector<int>> assigned(n + 1);

    while (true) {
        // proposals[s] 收集本轮向服务器 s 提案的任务
        vector<vector<int>> proposals(n + 1);
        bool has_new_proposal = false;

        // 提案阶段：只有“当前未匹配且还有偏好”的任务会提案
        for (int j = 1; j <= m; ++j) {
            if (current_server[j] != 0) continue;
            if (ptr[j] >= (int)pref_server[j].size()) continue;
            int s = pref_server[j][ptr[j]];
            ++ptr[j];
            proposals[s].push_back(j);
            has_new_proposal = true;
        }

        if (!has_new_proposal) {
            break; // 没有新的提案，算法终止
        }

        // 服务器响应阶段
        for (int s = 1; s <= n; ++s) {
            if (proposals[s].empty() && assigned[s].empty()) continue;

            // 候选集合：上一轮暂时接收的任务 + 本轮新提案
            vector<int> cand;
            cand.reserve(assigned[s].size() + proposals[s].size());
            for (int j : assigned[s]) cand.push_back(j);
            for (int j : proposals[s]) cand.push_back(j);
            if (cand.empty()) continue;

            // 计算分数并排序
            vector<pair<long long, int>> cand_list;
            cand_list.reserve(cand.size());
            for (int j : cand) {
                auto it = score[s].find(j);
                if (it == score[s].end()) continue; // 理论上不会发生
                cand_list.emplace_back(it->second, j);
            }

            sort(cand_list.begin(), cand_list.end(),
                 [](const pair<long long, int>& a,
                    const pair<long long, int>& b) {
                     if (a.first != b.first) return a.first > b.first;
                     return a.second < b.second;
                 });

            // 贪心装入容量，形成新的 assigned 集合
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

            // 方便后面二分查找
            sort(new_assigned.begin(), new_assigned.end());

            // 之前在 s 上的任务，如果不在 new_assigned 中，则变为“未匹配”
            for (int j : assigned[s]) {
                if (!binary_search(new_assigned.begin(),
                                   new_assigned.end(), j)) {
                    current_server[j] = 0;
                }
            }

            // 新集合里的任务统一标记为匹配到 s
            for (int j : new_assigned) {
                current_server[j] = s;
            }

            assigned[s].swap(new_assigned);
        }
    }

    // 输出：第 j 行是任务 j 最终匹配到的服务器编号（或 0）
    for (int j = 1; j <= m; ++j) {
        cout << current_server[j] << '\n';
    }

    return 0;
}
