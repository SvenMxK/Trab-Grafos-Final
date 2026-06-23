#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stack>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <algorithm>
#include <stdexcept>
#include <iomanip>
 
using namespace std;
 
pair<vector<string>, vector<vector<int>>> ler_grafo_texto(const string& caminho) {
    ifstream arq(caminho);
    if (!arq.is_open())
        throw runtime_error("Nao foi possivel abrir o arquivo: " + caminho);
 
    unordered_map<string, int> nome_para_id;
    vector<string> id_para_nome;
    vector<vector<int>> adj;
 
    auto obter_id = [&](const string& nome) -> int {
        auto it = nome_para_id.find(nome);
        if (it != nome_para_id.end()) return it->second;
        int id = (int)id_para_nome.size();
        nome_para_id[nome] = id;
        id_para_nome.push_back(nome);
        adj.push_back({});
        return id;
    };
 
    string linha;
    while (getline(arq, linha)) {
        if (linha.empty() || linha[0] == '#') continue;
        istringstream iss(linha);
        vector<string> tokens;
        string token;
        while (iss >> token) tokens.push_back(token);
        if (tokens.empty()) continue;
        int u = obter_id(tokens[0]);
        for (int i = 1; i < (int)tokens.size(); i++) {
            int v = obter_id(tokens[i]);
            if (find(adj[u].begin(), adj[u].end(), v) == adj[u].end())
                adj[u].push_back(v);
        }
    }
    return {id_para_nome, adj};
}
 
void dfs_transitiva_rec(int u, const vector<vector<int>>& adj, vector<bool>& visitado, vector<int>& resultado) {
    visitado[u] = true;
    for (int v : adj[u]) {
        if (!visitado[v]) {
            dfs_transitiva_rec(v, adj, visitado, resultado);
        }
    }
    resultado.push_back(u); 
}
 
vector<int> dependencias_transitivas(const vector<vector<int>>& adj, int inicio) {
    vector<bool> visitado(adj.size(), false);
    vector<int> resultado;
    dfs_transitiva_rec(inicio, adj, visitado, resultado);
    resultado.erase(remove(resultado.begin(), resultado.end(), inicio), resultado.end());
    return resultado;
}
 
 
vector<int> st;          // pilha
vector<int> roots;       // vetor de raizes dos SCcs
int timer;
vector<int> t_in, t_low;
vector<vector<int>> sccs;
 
void tarjan_dfs(int v, const vector<vector<int>>& adj) {
    t_low[v] = t_in[v] = timer++;
    st.push_back(v);
 
    for (int u : adj[v]) {
        if (t_in[u] == -1) {
            tarjan_dfs(u, adj);
            t_low[v] = min(t_low[v], t_low[u]);
        } else if (roots[u] == -1) {
            t_low[v] = min(t_low[v], t_in[u]);
        }
    }
 
    if (t_low[v] == t_in[v]) {
        vector<int> scc;
        while (true) {
            int u = st.back();
            st.pop_back();
            roots[u] = v;
            scc.push_back(u);
            if (u == v) break;
        }
        sccs.push_back(scc);
    }
}
 
vector<vector<int>> tarjan_sccs(const vector<vector<int>>& adj) {
    int n = adj.size();
    st.clear();
    roots.assign(n, -1);
    timer = 0;
    t_in.assign(n, -1);
    t_low.assign(n, -1);
    sccs.clear();
 
    for (int v = 0; v < n; v++) {
        if (t_in[v] == -1) {
            tarjan_dfs(v, adj);
        }
    }
    return sccs;
}
 
vector<vector<int>> condensar(const vector<vector<int>>& adj, const vector<vector<int>>& sccs, vector<int>& rotulo) {
    int n = adj.size();
    rotulo.assign(n, -1);
    for (int i = 0; i < (int)sccs.size(); i++) {
        for (int v : sccs[i]) {
            rotulo[v] = i;
        }
    }
 
    int k = sccs.size();
    vector<vector<int>> adj_cond(k);
    set<pair<int,int>> arestas_vistas;
    for (int u = 0; u < n; u++) {
        for (int v : adj[u]) {
            int cu = rotulo[u];
            int cv = rotulo[v];
            if (cu != cv && arestas_vistas.insert({cu, cv}).second) {
                adj_cond[cu].push_back(cv);
            }
        }
    }
    return adj_cond;
}
 
vector<int> ordenacao_topologica(const vector<vector<int>>& adj_cond) {
    int k = adj_cond.size();
    vector<int> grau(k, 0);
    for (int u = 0; u < k; u++) {
        for (int v : adj_cond[u]) grau[v]++;
    }
 
    queue<int> q;
    for (int i = 0; i < k; i++) {
        if (grau[i] == 0) q.push(i);
    }
 
    vector<int> ordem;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        ordem.push_back(u);
        for (int v : adj_cond[u]) {
            if (--grau[v] == 0) q.push(v);
        }
    }
 
    if ((int)ordem.size() != k) {
        throw runtime_error("Ciclo no grafo condensado (não deveria ocorrer)");
    }
    return ordem;
}
 
 
static void separador(char c = '-', int n = 60) {
    cout << string(n, c) << '\n';
}
 
void imprimir_grafo(const vector<string>& nomes, const vector<vector<int>>& adj) {
    cout << string(60, '=') << '\n';
    cout << "GRAFO DE DEPENDENCIAS\n";
    separador('=');
    cout << "Vertices : " << nomes.size() << '\n';
    int arestas = 0;
    for (int i = 0; i < (int)adj.size(); i++)
        arestas += (int)adj[i].size();
    cout << "Arestas  : " << arestas << '\n';
    separador();
    cout << "Lista de adjacencia:\n";
    for (int u = 0; u < (int)nomes.size(); u++) {
        cout << "  " << setw(20) << left << nomes[u] << " -> ";
        if (adj[u].empty()) {
            cout << "(sem dependencias)";
        } else {
            for (int i = 0; i < (int)adj[u].size(); i++) {
                if (i > 0) cout << ", ";
                cout << nomes[adj[u][i]];
            }
        }
        cout << '\n';
    }
}
 
void imprimir_transitivas(const vector<string>& nomes, const vector<vector<int>>& adj, const string& pacote) {
    auto it = find(nomes.begin(), nomes.end(), pacote);
    if (it == nomes.end()) {
        cout << "[AVISO] Pacote '" << pacote << "' nao encontrado.\n";
        return;
    }
    int inicio = it - nomes.begin();
    vector<int> deps = dependencias_transitivas(adj, inicio);
    separador('=');
    cout << "DEPENDENCIAS TRANSITIVAS DE '" << pacote << "'\n";
    separador('=');
    if (deps.empty()) {
        cout << "  Nenhuma dependencia transitiva encontrada.\n";
        return;
    }
    vector<string> nomes_deps;
    for (int id : deps) nomes_deps.push_back(nomes[id]);
    sort(nomes_deps.begin(), nomes_deps.end());
    cout << "  Total: " << nomes_deps.size() << " pacotes\n";
    for (auto& n : nomes_deps) cout << "  - " << n << '\n';
}
 
void imprimir_sccs(const vector<string>& nomes, const vector<vector<int>>& sccs) {
    separador('=');
    cout << "COMPONENTES FORTEMENTE CONEXOS (TARJAN)\n";
    separador('=');
    int n_ciclicos = 0;
    for (auto& scc : sccs) if (scc.size() > 1) n_ciclicos++;
    cout << "Total de SCCs     : " << sccs.size() << '\n';
    cout << "SCCs com ciclos   : " << n_ciclicos << '\n';
    cout << "SCCs triviais     : " << sccs.size() - n_ciclicos << '\n';
    separador();
 
    for (int i = 0; i < (int)sccs.size(); i++) {
        const auto& scc = sccs[i];
        bool tem_ciclo = scc.size() > 1;
        cout << "  SCC #" << i << " [" << (tem_ciclo ? "CICLO" : "trivial") << "]"
             << " (" << scc.size() << " pacote(s)): ";
        for (int j = 0; j < (int)scc.size(); j++) {
            if (j > 0) cout << ", ";
            cout << nomes[scc[j]];
        }
        cout << '\n';
        if (tem_ciclo) {
            cout << "    *** Dependencia circular detectada! "
                 << "Esses pacotes nao podem ser instalados\n"
                 << "    *** em nenhuma ordem linear valida individualmente.\n";
        }
    }
}
 
void imprimir_condensacao_e_topologica(const vector<string>& nomes,
                                       const vector<vector<int>>& sccs,
                                       const vector<vector<int>>& adj_cond,
                                       const vector<int>& ordem) {
    separador('=');
    cout << "GRAFO DE CONDENSACAO\n";
    separador('=');
    cout << "Vertices (super-nos) : " << adj_cond.size() << '\n';
    int arestas = 0;
    for (int i = 0; i < (int)adj_cond.size(); i++)
        arestas += (int)adj_cond[i].size();
    cout << "Arestas              : " << arestas << '\n';
    separador();
 
    auto nome_scc = [&](int i) -> string {
        const auto& scc = sccs[i];
        if (scc.size() == 1) return nomes[scc[0]];
        string s = "{";
        for (int j = 0; j < (int)scc.size(); j++) {
            if (j > 0) s += ",";
            s += nomes[scc[j]];
        }
        s += "}";
        return s;
    };
 
    for (int u = 0; u < (int)adj_cond.size(); u++) {
        cout << "  " << setw(25) << left << nome_scc(u) << " -> ";
        if (adj_cond[u].empty()) {
            cout << "(sem saidas)";
        } else {
            for (int j = 0; j < (int)adj_cond[u].size(); j++) {
                if (j > 0) cout << ", ";
                cout << nome_scc(adj_cond[u][j]);
            }
        }
        cout << '\n';
    }
 
    separador('=');
    cout << "ORDENACAO TOPOLOGICA (KAHN)\n";
    separador('=');
    cout << "Sequencia valida de instalacao:\n\n";
    for (int pos = 0; pos < (int)ordem.size(); pos++) {
        int scc_id = ordem[pos];
        const auto& scc = sccs[scc_id];
        cout << "  Passo " << setw(3) << pos + 1 << ": "
             << nome_scc(scc_id);
        if (scc.size() > 1)
            cout << "  <-- CICLO: requer intervencao manual";
        cout << '\n';
    }
}

// Inspiracao de CG
static void uso(const char* prog) {
    cerr << "Uso:\n"
         << "  " << prog << " ARQUIVO.txt [--pacote NOME] [--dot ARQUIVO.dot]\n"
         << '\n'
         << "Opcoes:\n"
         << "  ARQUIVO.txt      Arquivo de dependencias no formato texto\n"
         << "                     (uma linha por pacote: PACOTE DEP1 DEP2 ...)\n"
         << "  --pacote NOME    Exibe dependencias transitivas do pacote NOME\n"
         << '\n';
}
 
int main(int argc, char* argv[]) {
    if (argc < 2) { uso(argv[0]); return 1; }
 
    string arquivo_entrada, pacote_consulta, arquivo_dot;
 
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "--pacote" && i + 1 < argc) {
            pacote_consulta = argv[++i];
        } else if (arg == "--dot" && i + 1 < argc) {
            arquivo_dot = argv[++i];
        } else if (arquivo_entrada.empty()) {
            arquivo_entrada = arg;
        } else {
            cerr << "[ERRO] Argumento desconhecido: " << arg << '\n';
            uso(argv[0]);
            return 1;
        }
    }
 
    if (arquivo_entrada.empty()) {
        cerr << "[ERRO] Nenhum arquivo de entrada especificado.\n";
        uso(argv[0]);
        return 1;
    }
 
    vector<string> nomes;
    vector<vector<int>> adj;
    try {
        tie(nomes, adj) = ler_grafo_texto(arquivo_entrada);
    } catch (const exception& ex) {
        cerr << "[ERRO] " << ex.what() << '\n';
        return 1;
    }
 
    separador('*');
    cout << "  ANALISADOR DE GRAFOS DE DEPENDENCIA (estilo simples)\n";
    separador('*');
    cout << "Arquivo : " << arquivo_entrada << '\n';
    cout << '\n';
 
    imprimir_grafo(nomes, adj);
    cout << '\n';
 
    if (!pacote_consulta.empty()) {
        imprimir_transitivas(nomes, adj, pacote_consulta);
        cout << '\n';
    }
 
    vector<vector<int>> sccs = tarjan_sccs(adj);
    imprimir_sccs(nomes, sccs);
    cout << '\n';
 
    vector<int> rotulo;
    vector<vector<int>> cond = condensar(adj, sccs, rotulo);
    vector<int> ordem = ordenacao_topologica(cond);
    imprimir_condensacao_e_topologica(nomes, sccs, cond, ordem);
    
    separador('*');
    cout << "  Analise concluida.\n";
    separador('*');
 
    return 0;
}