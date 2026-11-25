#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define MAX_NODES 100000
#define MAX_EDGES 1500000
#define MAX_LAYERS 64
#define LAYER_NAME_LENGTH 32

typedef struct
{
    int node_count;
    int edge_count;
    uint64_t total_paths;

    char layer_names[MAX_LAYERS][LAYER_NAME_LENGTH];

    int node_layer[MAX_NODES];
    int node_value[MAX_NODES];
    int head_edge_index[MAX_NODES];
    uint64_t node_path_count[MAX_NODES];

    int edge_to_node_id[MAX_EDGES];
    int edge_next_index[MAX_EDGES];
} PathDAG;

// グラフ構造体の初期化
void init_graph(PathDAG* graph)
{
    graph->node_count = 0;
    graph->edge_count = 0;
    graph->total_paths = 0;

    for (int layer_index = 0; layer_index < MAX_LAYERS; layer_index++)
    {
        snprintf(graph->layer_names[layer_index], LAYER_NAME_LENGTH, "Layer%d", layer_index);
    }

    for (int node_index = 0; node_index < MAX_NODES; node_index++)
    {
        graph->head_edge_index[node_index] = -1;
        graph->node_path_count[node_index] = 0;
        graph->node_value[node_index] = 0;
    }
}

// レイヤーに名前を設定
void set_layer_name(PathDAG* graph, int target_layer, const char* layer_name)
{
    if (target_layer >= 0 && target_layer < MAX_LAYERS)
    {
        snprintf(graph->layer_names[target_layer], LAYER_NAME_LENGTH, "%s", layer_name);
    }
}

// 単一ノードの追加（戻り値：割り当てられたノードID、失敗時は -1）
int add_node(PathDAG* graph, int target_layer, int value)
{
    if (graph->node_count >= MAX_NODES || target_layer < 0 || target_layer >= MAX_LAYERS)
    {
        return -1;
    }

    int new_node_id = graph->node_count;
    graph->node_layer[new_node_id] = target_layer;
    graph->node_value[new_node_id] = value;
    graph->node_count++;

    return new_node_id;
}

// 連続する整数値を持つノードを一括追加
void add_nodes_sequence(PathDAG* graph, int target_layer, int start_value, int end_value)
{
    for (int current_value = start_value; current_value <= end_value; current_value++)
    {
        add_node(graph, target_layer, current_value);
    }
}

// ノードID指定によるエッジ（接続）の追加
bool add_edge(PathDAG* graph, int from_node_id, int to_node_id)
{
    if (from_node_id < 0 || from_node_id >= graph->node_count)
    {
        return false;
    }
    if (to_node_id < 0 || to_node_id >= graph->node_count)
    {
        return false;
    }
    if (from_node_id >= to_node_id || graph->edge_count >= MAX_EDGES)
    {
        return false;
    }

    int new_edge_index = graph->edge_count;
    graph->edge_to_node_id[new_edge_index] = to_node_id;
    graph->edge_next_index[new_edge_index] = graph->head_edge_index[from_node_id];
    graph->head_edge_index[from_node_id] = new_edge_index;
    graph->edge_count++;

    return true;
}

// レイヤー番号とノードの整数値の範囲を指定して一括接続
int add_edge_by_value_range(
    PathDAG* graph,
    int from_layer,
    int from_min_value,
    int from_max_value,
    int to_layer,
    int to_min_value,
    int to_max_value)
{
    int added_edge_count = 0;

    for (int from_node_id = 0; from_node_id < graph->node_count; from_node_id++)
    {
        if (graph->node_layer[from_node_id] != from_layer)
        {
            continue;
        }
        if (graph->node_value[from_node_id] < from_min_value || graph->node_value[from_node_id] > from_max_value)
        {
            continue;
        }

        for (int to_node_id = 0; to_node_id < graph->node_count; to_node_id++)
        {
            if (graph->node_layer[to_node_id] != to_layer)
            {
                continue;
            }
            if (graph->node_value[to_node_id] < to_min_value || graph->node_value[to_node_id] > to_max_value)
            {
                continue;
            }

            if (add_edge(graph, from_node_id, to_node_id))
            {
                added_edge_count++;
            }
        }
    }

    return added_edge_count;
}

// 指定したレイヤーを起点とした総経路数を計算
uint64_t count_paths_from_layer(const PathDAG* graph, int start_layer)
{
    uint64_t layer_total_paths = 0;

    for (int node_id = 0; node_id < graph->node_count; node_id++)
    {
        if (graph->node_layer[node_id] == start_layer)
        {
            layer_total_paths += graph->node_path_count[node_id];
        }
    }

    return layer_total_paths;
}

// 全ノードの経路数およびLayer0起点の総経路数を事前計算
void build_path_counts(PathDAG* graph)
{
    for (int node_id = graph->node_count - 1; node_id >= 0; node_id--)
    {
        if (graph->head_edge_index[node_id] == -1)
        {
            graph->node_path_count[node_id] = 1;
        }
        else
        {
            uint64_t accumulated_paths = 0;
            int edge_index = graph->head_edge_index[node_id];

            while (edge_index != -1)
            {
                int destination_node_id = graph->edge_to_node_id[edge_index];
                accumulated_paths += graph->node_path_count[destination_node_id];
                edge_index = graph->edge_next_index[edge_index];
            }

            graph->node_path_count[node_id] = accumulated_paths;
        }
    }

    graph->total_paths = count_paths_from_layer(graph, 0);
}

// 経路番号から具体的なノードIDの並び（ルート）を復元
int get_path_by_id(const PathDAG* graph, uint64_t target_path_id, int* output_route)
{
    if (target_path_id >= graph->total_paths)
    {
        return -1;
    }

    uint64_t remaining_offset = target_path_id;
    int current_node_id = -1;

    for (int node_id = 0; node_id < graph->node_count; node_id++)
    {
        if (graph->node_layer[node_id] == 0)
        {
            uint64_t branch_paths = graph->node_path_count[node_id];
            if (remaining_offset < branch_paths)
            {
                current_node_id = node_id;
                break;
            }
            remaining_offset -= branch_paths;
        }
    }

    if (current_node_id == -1)
    {
        return -1;
    }

    int route_step_count = 0;
    output_route[route_step_count] = current_node_id;
    route_step_count++;

    while (graph->head_edge_index[current_node_id] != -1)
    {
        int edge_index = graph->head_edge_index[current_node_id];

        while (edge_index != -1)
        {
            int next_node_id = graph->edge_to_node_id[edge_index];
            uint64_t branch_paths = graph->node_path_count[next_node_id];

            if (remaining_offset < branch_paths)
            {
                current_node_id = next_node_id;
                output_route[route_step_count] = current_node_id;
                route_step_count++;
                break;
            }

            remaining_offset -= branch_paths;
            edge_index = graph->edge_next_index[edge_index];
        }
    }

    return route_step_count;
}

// デバッグ用：グラフ全体の接続構造と総経路数を表示
void debug_print_graph(const PathDAG* graph, bool is_enabled)
{
    if (!is_enabled)
    {
        return;
    }

    printf("\n========================================\n");
    printf(" ノード数: %d / エッジ数: %d\n", graph->node_count, graph->edge_count);
    printf(" 総経路数 (Layer0起点): %llu 通り\n", (unsigned long long)graph->total_paths);
    printf("========================================\n");

    int previous_layer = -1;

    for (int node_id = 0; node_id < graph->node_count; node_id++)
    {
        int current_layer = graph->node_layer[node_id];
        if (current_layer != previous_layer)
        {
            previous_layer = current_layer;
            printf("\n[%s (Layer %d)]\n", graph->layer_names[current_layer], current_layer);
        }

        printf("  ID:%02d (値:%d / 経路数:%llu) ",
               node_id,
               graph->node_value[node_id],
               (unsigned long long)graph->node_path_count[node_id]);

        int edge_index = graph->head_edge_index[node_id];
        if (edge_index == -1)
        {
            printf("---> [END]\n");
        }
        else
        {
            printf("--->");
            while (edge_index != -1)
            {
                int destination_node_id = graph->edge_to_node_id[edge_index];
                int destination_layer = graph->node_layer[destination_node_id];

                printf(" %s=%d",
                       graph->layer_names[destination_layer],
                       graph->node_value[destination_node_id]);

                edge_index = graph->edge_next_index[edge_index];
                if (edge_index != -1)
                {
                    printf(",");
                }
            }
            printf("\n");
        }
    }

    printf("========================================\n\n");
}

// デバッグ用：経路番号から復元したルート一覧を表示（max_display_count が 0 なら全件表示）
void debug_print_routes(const PathDAG* graph, bool is_enabled, uint64_t max_display_count)
{
    if (!is_enabled)
    {
        return;
    }

    uint64_t display_limit = graph->total_paths;
    if (max_display_count > 0 && max_display_count < display_limit)
    {
        display_limit = max_display_count;
    }

    int restored_route[MAX_LAYERS];

    printf("--- 経路一覧 (全 %llu 通り中 %llu 件表示) ---\n",
           (unsigned long long)graph->total_paths,
           (unsigned long long)display_limit);

    for (uint64_t path_id = 0; path_id < display_limit; path_id++)
    {
        int route_length = get_path_by_id(graph, path_id, restored_route);
        printf("経路[%llu]: ", (unsigned long long)path_id);

        for (int step_index = 0; step_index < route_length; step_index++)
        {
            int node_id = restored_route[step_index];
            int layer_id = graph->node_layer[node_id];
            const char* separator = (step_index == route_length - 1) ? "" : " -> ";

            printf("%s=%d%s",
                   graph->layer_names[layer_id],
                   graph->node_value[node_id],
                   separator);
        }
        printf("\n");
    }

    if (display_limit < graph->total_paths)
    {
        uint64_t omitted_count = graph->total_paths - display_limit;
        printf("... (以下 %llu 件省略)\n", (unsigned long long)omitted_count);
    }

    printf("------------------------------------------\n\n");
}


// 指定したレイヤーに属するノードの「値」をランダムにシャッフルする
// ※必ず add_edge で接続を行う前に呼び出すこと
void shuffle_layer_nodes(PathDAG* graph, int target_layer)
{
    int target_node_ids[MAX_NODES];
    int target_count = 0;

    // 対象レイヤーに属するノードIDを収集
    for (int node_id = 0; node_id < graph->node_count; node_id++)
    {
        if (graph->node_layer[node_id] == target_layer)
        {
            target_node_ids[target_count] = node_id;
            target_count++;
        }
    }

    if (target_count <= 1)
    {
        return;
    }

    // Fisher-Yates アルゴリズムでノードの整数値をシャッフル
    for (int current_index = target_count - 1; current_index > 0; current_index--)
    {
        int random_index = rand() % (current_index + 1);

        int first_node_id = target_node_ids[current_index];
        int second_node_id = target_node_ids[random_index];

        int temp_value = graph->node_value[first_node_id];
        graph->node_value[first_node_id] = graph->node_value[second_node_id];
        graph->node_value[second_node_id] = temp_value;
    }
}

int main(void)
{
    const bool IS_DEBUG_MODE = true;
    PathDAG *graph = (PathDAG *)malloc(sizeof(PathDAG));
    if (graph == NULL)
    {
        return 1;
    }
    init_graph(graph);
    set_layer_name(graph, 0, "Mode");
    set_layer_name(graph, 1, "ParamA");
    set_layer_name(graph, 2, "ParamB");

    add_node(graph, 0, 1);
    add_node(graph, 0, 2);

    add_nodes_sequence(graph, 1, 10, 20);
    add_nodes_sequence(graph, 2, 100, 101);
    // shuffle_layer_nodes(graph, 1);

    add_edge_by_value_range(graph, 0, 1, 2,     1, 10, 20);
    // add_edge_by_value_range(graph, 0, 2, 2,     1, 12, 15);
    add_edge_by_value_range(graph, 1, 10, 20,   2, 100, 101);
    // add_edge_by_value_range(graph, 0, 1, 1,     2, 100, 100);

    build_path_counts(graph);

    debug_print_graph(graph, IS_DEBUG_MODE);
    debug_print_routes(graph, IS_DEBUG_MODE, 0);

    uint64_t layer1_paths = count_paths_from_layer(graph, 1);
    printf("ParamA(Layer 1) 起点の総経路数: %llu 通り\n", (unsigned long long)layer1_paths);

    free(graph);
    return 0;
}