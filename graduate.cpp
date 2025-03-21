#include <bits/stdc++.h>
#define ll long long
#define lll __int128
#define ld long double
#define rep(i,l,r)for(int i=(l);i<(r);i++)
using namespace std;
typedef pair<int, int> pii;
typedef pair<ll, ll> pll;

vector<vector<double>> readCSV(string filename, int r, int c) {
    ifstream file(filename);
    vector<vector<double>> data;
    data = vector<vector<double>>(r, vector<double>(c, 0));
    string line;
    
    // 첫줄은 헤더이므로 무시
    getline(file, line);
    
    rep(i, 0, r) rep(j, 0, c){
        getline(file, line);
        stringstream ss(line);
        string cell;
        while(getline(ss, cell, ',')){
            try{
                data[i][j] = stod(cell);
            } catch(const std::invalid_argument& e){
                data[i][j] = 1e8;
            }
        }
    }
    
    file.close();
    return data;
}

int SS, DD; // Super Source, Super Sink
int Depot_cnt, Site_cnt, Disposal_cnt; // Depot, Site, Disposal
int Truck_Unit = 2000; // 트럭 한대당 처리량
const int mxN = 1000;
struct MCC{
    struct Edge {
        int to, rev;
        int cap;
        int cost;
        int flow;
        int lower; // 하한
    };

    int N;
    vector<Edge> G[mxN];
    int h[mxN]; // 잠재값 (potential)
    int dist[mxN];
    int prevv[mxN], preve[mxN];
    int demand[mxN]; // 각 노드의 수요량
    int excess[mxN]; // 간선의 하한으로 인한 초과 흐름

    void init(int n) {
        N = n;
        for(int i = 0; i < N; ++i) {
            G[i].clear();
            demand[i] = 0;
            excess[i] = 0;
        }
    }

    // 간선 추가 함수 (하한과 상한 포함)
    void addEdge(int from, int to, int lower, int upper, int cost) {
        // 하한을 처리하기 위해 노드의 수요를 조정
        demand[from] -= lower;
        demand[to] += lower;
        // 간선의 용량을 (상한 - 하한)으로 설정
        G[from].push_back((Edge){to, (int)G[to].size(), upper - lower, cost, 0, lower});
        G[to].push_back((Edge){from, (int)G[from].size()-1, 0, -cost, 0, 0});
    }

    void addDemand(int v, int d) {
        demand[v] += d;
    }

    // 최소 비용 순환을 찾는 함수
    int minCostCirculation() {
        int res = 0;
        // 전체 네트워크의 수요 균형 확인 및 슈퍼소스와 슈퍼싱크 추가
        int totalDemand = 0;
        int S = N;
        int T = N + 1;
        N += 2;
        for(int i = 0; i < N - 2; ++i) {
            if(demand[i] > 0) {
                // 슈퍼소스에서 해당 노드로 수요만큼의 간선 추가
                addEdge(S, i, 0, demand[i], 0);
                totalDemand += demand[i];
            } else if(demand[i] < 0) {
                // 해당 노드에서 슈퍼싱크로 (-수요)만큼의 간선 추가
                addEdge(i, T, 0, -demand[i], 0);
            }
        }

        fill(h, h + N, 0);

        while(true) {
            priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
            fill(dist, dist + N, 1e9);
            dist[S] = 0;
            pq.push({0, S});
            while(!pq.empty()) {
                int c = pq.top().first;
                int v = pq.top().second;
                pq.pop();
                // cout << "pq: " << v << ' ' << c << '\n';
                if(dist[v] < c) continue;
                for(int i = 0; i < G[v].size(); ++i) {
                    Edge &e = G[v][i];
                    if(e.cap > 0 && dist[e.to] > dist[v] + e.cost + h[v] - h[e.to]) {
                        dist[e.to] = dist[v] + e.cost + h[v] - h[e.to];
                        prevv[e.to] = v;
                        preve[e.to] = i;
                        pq.push({dist[e.to], e.to});
                    }
                }
            }
            if(dist[T] == 1e9) {
                // 더 이상 증가 경로가 없음
                break;
            }
            for(int v = 0; v < N; ++v) h[v] += dist[v];

            // S에서 T로의 최소 비용 증가 경로의 용량 계산
            int d = 1e9;
            for(int v = T; v != S; v = prevv[v]) {
                d = min(d, G[prevv[v]][preve[v]].cap);
            }
            res += d * h[T];
            for(int v = T; v != S; v = prevv[v]) {
                Edge &e = G[prevv[v]][preve[v]];
                e.cap -= d;
                G[v][e.rev].cap += d;

                // 유량 업데이트
                e.flow += d;    
                G[v][e.rev].flow -= d;
            }
        }

        // 전체 수요를 만족하지 못한 경우
        for(int i = 0; i < G[S].size(); ++i) {
            if(G[S][i].cap > 0) {
                return -1; // 순환을 찾을 수 없음
            }
        }

        return res;
    }

    void printFlows() {
        ofstream file("output.csv");
        file << "from,to,flow,cost\n";

        int RET = 0;
        for(int u = 0; u < N; ++u) {
            for(auto &e : G[u]) {
                if(e.flow + e.lower > 0) {
                    int from = u;
                    int to = e.to;
                    int flow = e.flow + e.lower; // 하한 유량을 더해 실제 유량 계산
                    int cost = e.cost;
                    if(from%2 == 0 || to%2 == 1) continue;
                    cout << "Node " << from/2 << " -> Node " << to/2 << " : flow = " << flow << ", cost = " << cost << endl;

                    file << from/2 << ',' << to/2 << ',' << flow << ',' << cost << '\n';
                    RET += ((flow-1)/Truck_Unit+1)*cost;
                }
            }
        }

        file.close();
        cout << "Output Done" << endl;
        cout << "Total Cost: " << RET << '\n';
    }
}MCC;

void solve(){
    Depot_cnt = 4;
    Site_cnt = 150;
    Disposal_cnt = 1;

    SS = 0; DD = 1;
    /*
    Super Source : 0
    Super Sink : 1
    Depot : 2 ~ Depot_cnt+1
    Site : Depot_cnt+2 ~ Depot_cnt+Site_cnt+1
    Disposal : Depot_cnt+Site_cnt+2 ~ Depot_cnt+Site_cnt+Disposal_cnt+1

    하지만 정점분할에 의해 각 값*2, *2+1로 나누어서 사용
    */

    MCC.init((Depot_cnt+Site_cnt+Disposal_cnt+2)*2);    

    // 외부 간선 추가

    // SS -> Depot
    rep(i, 0, Depot_cnt){
        int depot_idx = i+2;
        MCC.addEdge(SS*2+1, depot_idx*2, 0, 1e9, 0);
    }

    // Depot -> Site
    vector<vector<double>> data;
    data = readCSV("data/ST.csv", Depot_cnt, Site_cnt);
    rep(i, 0, Depot_cnt) rep(j, 0, Site_cnt){
        int depot_idx = i+2, site_idx = Depot_cnt+j+2;
        int cost; cost = data[i][j];
        MCC.addEdge(depot_idx*2+1, site_idx*2, 0, 1e9, cost);
    }
    cout << "Depot -> Site Done" << endl;

    // Site -> Site
    data = readCSV("data/TT.csv", Site_cnt, Site_cnt);
    rep(i, 0, Site_cnt) rep(j, 0, Site_cnt){
        int site_idx = Depot_cnt+i+2, site_idx2 = Depot_cnt+j+2;
        int cost; cost = data[i][j];
        if(i <= j) continue;
        if(cost > 1e4) continue;
        MCC.addEdge(site_idx*2+1, site_idx2*2, 0, 1e9, cost);
    }
    cout << "Site -> Site Done" << endl;

    // Site -> Disposal
    data = readCSV("data/TD.csv", Site_cnt, Disposal_cnt);
    rep(i, 0, Site_cnt) rep(j, 0, Disposal_cnt){
        int site_idx = Depot_cnt+i+2, disposal_idx = Depot_cnt+Site_cnt+j+2;
        int cost; cost = data[i][j];
        MCC.addEdge(site_idx*2+1, disposal_idx*2, 0, 1e9, cost);
    }
    cout << "Site -> Disposal Done" << endl;

    // Disposal -> DD
    rep(i, 0, Disposal_cnt){
        int disposal_idx = Depot_cnt+Site_cnt+i+2;
        MCC.addEdge(disposal_idx*2+1, DD*2, 0, 1e9, 0);
    }

    // DD -> SS
    MCC.addEdge(DD*2+1, SS*2, 0, 1e9, 0);

    // 내부 간선 추가
    int trash_tot = 0;

    // Depot
    data = readCSV("data/Source.csv", Depot_cnt, 1);
    rep(i, 0, Depot_cnt){
        int depot_idx = i+2;
        int cap; cap = data[i][0];
        MCC.addEdge(depot_idx*2, depot_idx*2+1, 0, cap*Truck_Unit, 0);
    }
    cout << "Depot Done" << endl;

    // Site
    data = readCSV("data/Trash.csv", Site_cnt, 1);
    rep(i, 0, Site_cnt){
        int site_idx = Depot_cnt+i+2;
        int trash; trash = data[i][0];
        trash_tot += trash;
        MCC.addEdge(site_idx*2, site_idx*2+1, trash, 1e9, 0);

        // Depot -> Site 계산시
        // MCC.addDemand(site_idx*2, -trash);

        // Site -> Disposal 계산시
        // MCC.addDemand(site_idx*2+1, trash);

        // Depot -> Disposal로 계산시
        // 필요없음
    }
    cout << "Site Done" << endl;

    // Disposal
    data = readCSV("data/Sink.csv", Disposal_cnt, 1);
    rep(i, 0, Disposal_cnt){
        int disposal_idx = Depot_cnt+Site_cnt+i+2;
        int L = 0, R;
        R = data[i][0];
        MCC.addEdge(disposal_idx*2, disposal_idx*2+1, 0, R, 0);
    }
    cout << "Disposal Done" << endl;

    // SS, DD
    MCC.addEdge(SS*2, SS*2+1, 0, 1e9, 0);
    MCC.addEdge(DD*2, DD*2+1, 0, 1e9, 0);

    // Depot -> Site 계산시
    // MCC.addDemand(SS*2, trash_tot);

    // Site -> Disposal 계산시
    // MCC.addDemand(DD*2+1, -trash_tot);
    
    // Depot -> Disposal로 계산시
    MCC.addDemand(SS*2, trash_tot);
    MCC.addDemand(DD*2+1, -trash_tot);

    cout << "trash_tot: " << trash_tot << endl;
    cout << "Input Done" << endl;
    rep(i, 0, MCC.N) cout << MCC.demand[i] << ' ';
    cout << endl;

    // 최소 비용 순환 찾기
    cout << "Calc start" << endl;
    cout << "Total Cost: " << MCC.minCostCirculation() << '\n';
    cout << "Calc end" << endl;
    MCC.printFlows();
    cout << "Print Done" << endl;
}

int main(){
    int tc = 1;
    // cin >> tc;
    rep(TC, 1, tc+1){
        solve();
    }
    return 0;
}