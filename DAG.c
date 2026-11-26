#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

// 初期容量の定義（上限に達すると自動で2倍に拡張される）
#define INITIAL_LAYER_CAPACITY 16
#define INITIAL_NODE_CAPACITY  1024
#define INITIAL_EDGE_CAPACITY  4096
#define LAYER_NAME_LENGTH      32

typedef struct
{
    // レイヤー管理（動的拡張）
    int layer_capacity;
    char (*layer_names)[LAYER_NAME_LENGTH];

    // ノード管理（動的拡張）
    int node_count;
    int node_capacity;
    int* node_layer;
    int* node_value;
    int* head_edge_index;
    uint64_t* node_path_count;

    // エッジ管理（動的拡張）
    int edge_count;
    int edge_capacity;
    int* edge_to_node_id;
    int* edge_next_index;

    // Layer0起点の総経路数
    uint64_t total_paths;
} PathDAG;

// グラフ構造体の初期化（初期容量分のメモリを確保）
bool init_graph(PathDAG* graph)
{
    graph->layer_capacity = INITIAL_LAYER_CAPACITY;
    graph->node_count = 0;
    graph->node_capacity = INITIAL_NODE_CAPACITY;
    graph->edge_count = 0;
    graph->edge_capacity = INITIAL_EDGE_CAPACITY;
    graph->total_paths = 0;

    graph->layer_names = (char (*)[LAYER_NAME_LENGTH])malloc(sizeof(char[LAYER_NAME_LENGTH]) * (size_t)graph->layer_capacity);

    graph->node_layer = (int*)malloc(sizeof(int) * (size_t)graph->node_capacity);
    graph->node_value = (int*)malloc(sizeof(int) * (size_t)graph->node_capacity);
    graph->head_edge_index = (int*)malloc(sizeof(int) * (size_t)graph->node_capacity);
    graph->node_path_count = (uint64_t*)malloc(sizeof(uint64_t) * (size_t)graph->node_capacity);

    graph->edge_to_node_id = (int*)malloc(sizeof(int) * (size_t)graph->edge_capacity);
    graph->edge_next_index = (int*)malloc(sizeof(int) * (size_t)graph->edge_capacity);

    if (graph->layer_names == NULL ||
        graph->node_layer == NULL || graph->node_value == NULL ||
        graph->head_edge_index == NULL || graph->node_path_count == NULL ||
        graph->edge_to_node_id == NULL || graph->edge_next_index == NULL)
    {
        return false;
    }

    for (int layer_index = 0; layer_index < graph->layer_capacity; layer_index++)
    {
        snprintf(graph->layer_names[layer_index], LAYER_NAME_LENGTH, "Layer%d", layer_index);
    }

    for (int node_index = 0; node_index < graph->node_capacity; node_index++)
    {
        graph->head_edge_index[node_index] = -1;
        graph->node_path_count[node_index] = 0;
        graph->node_value[node_index] = 0;
    }

    return true;
}

// グラフが保持する全動的メモリの解放
void free_graph(PathDAG* graph)
{
    if (graph == NULL)
    {
        return;
    }

    free(graph->layer_names);
    free(graph->node_layer);
    free(graph->node_value);
    free(graph->head_edge_index);
    free(graph->node_path_count);
    free(graph->edge_to_node_id);
    free(graph->edge_next_index);

    graph->layer_names = NULL;
    graph->node_layer = NULL;
    graph->node_value = NULL;
    graph->head_edge_index = NULL;
    graph->node_path_count = NULL;
    graph->edge_to_node_id = NULL;
    graph->edge_next_index = NULL;
}

// 【内部関数】レイヤー配列の容量拡張（2倍ずつ拡張）
bool ensure_layer_capacity(PathDAG* graph, int required_layer_index)
{
    if (required_layer_index < graph->layer_capacity)
    {
        return true;
    }

    int old_capacity = graph->layer_capacity;
    int new_capacity = old_capacity;
    while (new_capacity <= required_layer_index)
    {
        new_capacity *= 2;
    }

    char (*new_layer_names)[LAYER_NAME_LENGTH] = (char (*)[LAYER_NAME_LENGTH])realloc(
        graph->layer_names,
        sizeof(char[LAYER_NAME_LENGTH]) * (size_t)new_capacity
    );

    if (new_layer_names == NULL)
    {
        return false;
    }

    graph->layer_names = new_layer_names;
    graph->layer_capacity = new_capacity;

    for (int layer_index = old_capacity; layer_index < new_capacity; layer_index++)
    {
        snprintf(graph->layer_names[layer_index], LAYER_NAME_LENGTH, "Layer%d", layer_index);
    }

    return true;
}

// 【内部関数】ノード配列の容量拡張（2倍に拡張）
bool expand_node_capacity(PathDAG* graph)
{
    int old_capacity = graph->node_capacity;
    int new_capacity = old_capacity * 2;

    int* new_node_layer = (int*)realloc(graph->node_layer, sizeof(int) * (size_t)new_capacity);
    int* new_node_value = (int*)realloc(graph->node_value, sizeof(int) * (size_t)new_capacity);
    int* new_head_edge_index = (int*)realloc(graph->head_edge_index, sizeof(int) * (size_t)new_capacity);
    uint64_t* new_node_path_count = (uint64_t*)realloc(graph->node_path_count, sizeof(uint64_t) * (size_t)new_capacity);

    if (new_node_layer == NULL || new_node_value == NULL ||
        new_head_edge_index == NULL || new_node_path_count == NULL)
    {
        return false;
    }

    graph->node_layer = new_node_layer;
    graph->node_value = new_node_value;
    graph->head_edge_index = new_head_edge_index;
    graph->node_path_count = new_node_path_count;
    graph->node_capacity = new_capacity;

    for (int node_index = old_capacity; node_index < new_capacity; node_index++)
    {
        graph->head_edge_index[node_index] = -1;
        graph->node_path_count[node_index] = 0;
        graph->node_value[node_index] = 0;
    }

    return true;
}

// 【内部関数】エッジ配列の容量拡張（2倍に拡張）
bool expand_edge_capacity(PathDAG* graph)
{
    int new_capacity = graph->edge_capacity * 2;

    int* new_edge_to_node_id = (int*)realloc(graph->edge_to_node_id, sizeof(int) * (size_t)new_capacity);
    int* new_edge_next_index = (int*)realloc(graph->edge_next_index, sizeof(int) * (size_t)new_capacity);

    if (new_edge_to_node_id == NULL || new_edge_next_index == NULL)
    {
        return false;
    }

    graph->edge_to_node_id = new_edge_to_node_id;
    graph->edge_next_index = new_edge_next_index;
    graph->edge_capacity = new_capacity;

    return true;
}
// レイヤーに名前を設定（必要なら自動でレイヤー配列を拡張）
bool set_layer_name(PathDAG* graph, int target_layer, const char* layer_name)
{
    if (target_layer < 0)
    {
        return false;
    }
    if (!ensure_layer_capacity(graph, target_layer))
    {
        return false;
    }

    snprintf(graph->layer_names[target_layer], LAYER_NAME_LENGTH, "%s", layer_name);
    return true;
}

// 単一ノードの追加（上限に達したら自動で2倍に拡張）
int add_node(PathDAG* graph, int target_layer, int value)
{
    if (target_layer < 0)
    {
        return -1;
    }
    if (!ensure_layer_capacity(graph, target_layer))
    {
        return -1;
    }
    if (graph->node_count >= graph->node_capacity)
    {
        if (!expand_node_capacity(graph))
        {
            return -1;
        }
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

// 指定したレイヤーに属するノードの「値」をランダムにシャッフルする
// ※必ず add_edge で接続を行う前に呼び出すこと
void shuffle_layer_nodes(PathDAG* graph, int target_layer)
{
    if (graph->node_count <= 1)
    {
        return;
    }

    int* target_node_ids = (int*)malloc(sizeof(int) * (size_t)graph->node_count);
    if (target_node_ids == NULL)
    {
        return;
    }

    int target_count = 0;
    for (int node_id = 0; node_id < graph->node_count; node_id++)
    {
        if (graph->node_layer[node_id] == target_layer)
        {
            target_node_ids[target_count] = node_id;
            target_count++;
        }
    }

    for (int current_index = target_count - 1; current_index > 0; current_index--)
    {
        int random_index = rand() % (current_index + 1);

        int first_node_id = target_node_ids[current_index];
        int second_node_id = target_node_ids[random_index];

        int temp_value = graph->node_value[first_node_id];
        graph->node_value[first_node_id] = graph->node_value[second_node_id];
        graph->node_value[second_node_id] = temp_value;
    }

    free(target_node_ids);
}

// 指定したレイヤーのノードを間引く（例: keep_interval = 2 なら 1/2 に削減）
// ※必ず add_edge で接続を行う前に呼び出すこと
int thin_out_layer_nodes(PathDAG* graph, int target_layer, int keep_interval)
{
    if (keep_interval <= 1)
    {
        return 0;
    }

    int write_node_id = 0;
    int layer_occurrence_count = 0;
    int removed_node_count = 0;

    for (int read_node_id = 0; read_node_id < graph->node_count; read_node_id++)
    {
        bool should_keep = true;

        if (graph->node_layer[read_node_id] == target_layer)
        {
            if (layer_occurrence_count % keep_interval != 0)
            {
                should_keep = false;
                removed_node_count++;
            }
            layer_occurrence_count++;
        }

        if (should_keep)
        {
            if (write_node_id != read_node_id)
            {
                graph->node_layer[write_node_id] = graph->node_layer[read_node_id];
                graph->node_value[write_node_id] = graph->node_value[read_node_id];
                graph->head_edge_index[write_node_id] = graph->head_edge_index[read_node_id];
                graph->node_path_count[write_node_id] = graph->node_path_count[read_node_id];
            }
            write_node_id++;
        }
    }

    graph->node_count = write_node_id;
    return removed_node_count;
}

// ノードID指定によるエッジの追加（上限に達したら自動で2倍に拡張）
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
    if (from_node_id >= to_node_id)
    {
        return false;
    }

    if (graph->edge_count >= graph->edge_capacity)
    {
        if (!expand_edge_capacity(graph))
        {
            return false;
        }
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
// ※ output_route には少なくとも graph->node_count（または最大レイヤー数）分の要素数が必要
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
    printf(" ノード数: %d (容量:%d) / エッジ数: %d (容量:%d)\n",
           graph->node_count, graph->node_capacity,
           graph->edge_count, graph->edge_capacity);
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

// ソート用の一時構造体（ステップ配列も動的確保に対応）
typedef struct
{
    uint64_t original_path_id;
    int step_count;
    int* node_values;
    int* node_layers;
} RouteRecord;

// デバッグ用：経路一覧を綺麗な昇順にソートして表示（max_display_count が 0 なら全件表示）
void debug_print_routes_sorted(const PathDAG* graph, bool is_enabled, uint64_t max_display_count)
{
    if (!is_enabled || graph->total_paths == 0)
    {
        return;
    }

    const uint64_t MAX_SORTABLE_ROUTES = 100000;
    uint64_t display_limit = graph->total_paths;

    if (max_display_count > 0 && max_display_count < display_limit)
    {
        display_limit = max_display_count;
    }
    if (display_limit > MAX_SORTABLE_ROUTES)
    {
        display_limit = MAX_SORTABLE_ROUTES;
    }

    RouteRecord* route_list = (RouteRecord*)malloc(sizeof(RouteRecord) * (size_t)display_limit);
    int* temp_node_ids = (int*)malloc(sizeof(int) * (size_t)graph->node_count);

    if (route_list == NULL || temp_node_ids == NULL)
    {
        free(route_list);
        free(temp_node_ids);
        return;
    }

    // 1. 経路を復元して一時配列に格納
    for (uint64_t path_id = 0; path_id < display_limit; path_id++)
    {
        int step_count = get_path_by_id(graph, path_id, temp_node_ids);
        route_list[path_id].original_path_id = path_id;
        route_list[path_id].step_count = step_count;
        route_list[path_id].node_values = (int*)malloc(sizeof(int) * (size_t)step_count);
        route_list[path_id].node_layers = (int*)malloc(sizeof(int) * (size_t)step_count);

        for (int step_index = 0; step_index < step_count; step_index++)
        {
            int node_id = temp_node_ids[step_index];
            route_list[path_id].node_values[step_index] = graph->node_value[node_id];
            route_list[path_id].node_layers[step_index] = graph->node_layer[node_id];
        }
    }

    // 2. シェルソートでノードの整数値の辞書順（昇順）に並び替え
    int total_records = (int)display_limit;
    for (int gap = total_records / 2; gap > 0; gap /= 2)
    {
        for (int current_index = gap; current_index < total_records; current_index++)
        {
            RouteRecord target_record = route_list[current_index];
            int insert_index = current_index;

            while (insert_index >= gap)
            {
                RouteRecord compare_record = route_list[insert_index - gap];
                bool is_compare_greater = false;

                int min_steps = compare_record.step_count;
                if (target_record.step_count < min_steps)
                {
                    min_steps = target_record.step_count;
                }

                bool is_decided = false;
                for (int step_index = 0; step_index < min_steps; step_index++)
                {
                    if (compare_record.node_values[step_index] > target_record.node_values[step_index])
                    {
                        is_compare_greater = true;
                        is_decided = true;
                        break;
                    }
                    if (compare_record.node_values[step_index] < target_record.node_values[step_index])
                    {
                        is_compare_greater = false;
                        is_decided = true;
                        break;
                    }
                }

                if (!is_decided && compare_record.step_count > target_record.step_count)
                {
                    is_compare_greater = true;
                }

                if (!is_compare_greater)
                {
                    break;
                }

                route_list[insert_index] = route_list[insert_index - gap];
                insert_index -= gap;
            }

            route_list[insert_index] = target_record;
        }
    }

    // 3. 綺麗に整列された状態で画面出力
    printf("--- 経路一覧【昇順ソート済】 (全 %llu 通り中 %llu 件表示) ---\n",
           (unsigned long long)graph->total_paths,
           (unsigned long long)display_limit);

    for (uint64_t record_index = 0; record_index < display_limit; record_index++)
    {
        RouteRecord current_record = route_list[record_index];
        printf("経路[ID:%4llu]: ", (unsigned long long)current_record.original_path_id);

        for (int step_index = 0; step_index < current_record.step_count; step_index++)
        {
            int layer_id = current_record.node_layers[step_index];
            int node_val = current_record.node_values[step_index];
            const char* separator = (step_index == current_record.step_count - 1) ? "" : " -> ";

            printf("%s=%d%s", graph->layer_names[layer_id], node_val, separator);
        }
        printf("\n");
    }

    if (display_limit < graph->total_paths)
    {
        uint64_t omitted_count = graph->total_paths - display_limit;
        printf("... (以下 %llu 件省略)\n", (unsigned long long)omitted_count);
    }

    printf("------------------------------------------------------------\n\n");

    // 一時確保したメモリの解放
    for (uint64_t record_index = 0; record_index < display_limit; record_index++)
    {
        free(route_list[record_index].node_values);
        free(route_list[record_index].node_layers);
    }
    free(route_list);
    free(temp_node_ids);
}

int main(void)
{
    const bool IS_DEBUG_MODE = true;
    srand((unsigned int)time(NULL));

    PathDAG graph;
    if (!init_graph(&graph))
    {
        printf("グラフの初期化に失敗しました。\n");
        return 1;
    }

    set_layer_name(&graph, 0, "Mode");
    set_layer_name(&graph, 1, "ParamA");
    set_layer_name(&graph, 2, "ParamB");

    add_node(&graph, 0, 1);
    add_node(&graph, 0, 2);
    add_nodes_sequence(&graph, 1, 10, 15);
    add_nodes_sequence(&graph, 2, 100, 101);

    // 時短用間引きとシャッフル
    // thin_out_layer_nodes(&graph, 1, 2);
    shuffle_layer_nodes(&graph, 1);

    // 範囲指定接続
    add_edge_by_value_range(&graph, 0, 1, 1,   1, 10, 15);
    add_edge_by_value_range(&graph, 0, 2, 2,   1, 10, 15);
    add_edge_by_value_range(&graph, 1, 10, 15, 2, 100, 101);

    // 経路数の計算
    build_path_counts(&graph);

    // デバッグ表示（容量の自動拡張状況も確認できる）
    debug_print_graph(&graph, IS_DEBUG_MODE);
    debug_print_routes_sorted(&graph, IS_DEBUG_MODE, 0);

    // 終了時のメモリ解放
    free_graph(&graph);
    return 0;
}
