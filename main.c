#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h> // INT_MAX 사용
#include <math.h>   // round() 함수를 위해 추가

// #define SWAP(a,b) { int temp = a; a = b; b = temp; } // 단방향 그래프에서는 필요 없음
#define WEIGHT_SCALE 100 // 가중치 스케일링 상수

// 간선 구조체
typedef struct Edge {
    int vertex_1, vertex_2;
    int weight_1, weight_2;
} Edge;

// 선행 정점 리스트 노드 구조체
typedef struct ParentNode {
    int vertex;
    struct ParentNode *next;
} ParentNode;

// 전역 변수: 선행 정점 리스트 헤드를 저장하는 배열
ParentNode** predecessors_global;

// 파일 열기 함수
FILE* openFile(char * filename, const char *mode) {
    FILE* file = fopen(filename, mode);
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file. FILENAME: %s\n", filename);
        exit(1);
    }
    return file;
}

// predecessors 리스트의 모든 노드 해제 함수 (전체 배열 초기화)
void freePredecessors(int VertexCount_Adjusted) {
    for (int i = 0; i < VertexCount_Adjusted; i++) {
        ParentNode* current = predecessors_global[i];
        while (current != NULL) {
            ParentNode* temp = current;
            current = current->next;
            free(temp);
        }
        predecessors_global[i] = NULL; // 해제 후 NULL로 초기화
    }
}

// 특정 정점의 predecessors 리스트만 비우는 함수 (BellmanFord 내부에서 사용)
void clearPredecessorsForVertex(int vertex_idx) {
    ParentNode* current = predecessors_global[vertex_idx];
    while (current != NULL) {
        ParentNode* temp = current;
        current = current->next;
        free(temp);
    }
    predecessors_global[vertex_idx] = NULL;
}


// 선행 정점 리스트에 노드 추가 함수 (중복 추가 방지)
void addPredecessor(int targetVertex, int preVertex) {
    ParentNode* current = predecessors_global[targetVertex];
    while (current != NULL) {
        if (current->vertex == preVertex) {
            return; // 이미 존재하면 추가하지 않음
        }
        current = current->next;
    }
    ParentNode* newNode = (ParentNode*)malloc(sizeof(ParentNode));
    if (newNode == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for ParentNode.\n");
        exit(1);
    }
    newNode->vertex = preVertex;
    newNode->next = predecessors_global[targetVertex];
    predecessors_global[targetVertex] = newNode;
}

// BellmanFord 알고리즘 구현
void BellmanFord(Edge* edgeList, int EdgeCount, int VertexCount_Adjusted, int startPoint, int* dp, double ratio1, double ratio2) {
    // dp 배열 초기화
    for (int i = 0; i < VertexCount_Adjusted; i++) {
        dp[i] = INT_MAX; // int형 무한대 사용
    }
    dp[startPoint] = 0;

    // (정점 개수 - 1)번 반복하여 최단 거리 갱신
    for (int i = 0; i < VertexCount_Adjusted - 1; i++) {
        for (int j = 0; j < EdgeCount; j++) {
            int u = edgeList[j].vertex_1;
            int v = edgeList[j].vertex_2;
            // 정수형 가중치 합 계산 (100배 스케일링)
            int current_weight_sum = (int)(ratio1 * edgeList[j].weight_1 * WEIGHT_SCALE + ratio2 * edgeList[j].weight_2 * WEIGHT_SCALE);

            // u -> v 방향 릴렉스만 수행 (단방향)
            if (dp[u] != INT_MAX) { // 도달 가능한 경우에만 릴렉스 시도
                if (dp[u] + current_weight_sum < dp[v]) {
                    dp[v] = dp[u] + current_weight_sum;
                    clearPredecessorsForVertex(v); // 해당 정점의 선행 리스트만 비움
                    addPredecessor(v, u);
                } else if (dp[u] + current_weight_sum == dp[v]) { // 정수형 비교
                    addPredecessor(v, u);
                }
            }
            // v -> u 방향 릴렉스 제거 (단방향 그래프이므로 이 부분은 필요 없음)
            // if (dp[v] != INT_MAX) {
            //     if (dp[v] + current_weight_sum < dp[u]) {
            //          dp[u] = dp[v] + current_weight_sum;
            //          clearPredecessorsForVertex(u);
            //          addPredecessor(u, v);
            //     } else if (dp[v] + current_weight_sum == dp[u]) {
            //          addPredecessor(u, v);
            //     }
            // }
        }
    }
}

// 모든 최단 경로 재귀적으로 출력 (목적지 -> 시작점 역추적)
void printAllShortestPathsRecursive(int currentVertex, int startPoint, int path[], int path_idx, FILE *output) {
    path[path_idx] = currentVertex;

    if (currentVertex == startPoint) {
        for (int i = path_idx; i >= 0; i--) {
            fprintf(output, "%d", path[i]);
            if (i > 0) {
                fprintf(output, " -> ");
            }
        }
        fprintf(output, "\n");
    } else {
        ParentNode* current = predecessors_global[currentVertex];
        while (current != NULL) {
            printAllShortestPathsRecursive(current->vertex, startPoint, path, path_idx + 1, output);
            current = current->next;
        }
    }
}

// 간선 개수 가져오기 함수 (첫 줄 제외)
int getEdgeCount(FILE* input) {
    int EdgeCount = 0;
    char line[200];

    // 첫 번째 줄은 읽고 버립니다. (VertexCount, startPoint, endingPoint 정보)
    if (fgets(line, sizeof(line), input) == NULL) {
        rewind(input);
        return 0;
    }
    // 나머지 줄의 개수를 셉니다.
    while (fgets(line, sizeof(line), input) != NULL) {
        EdgeCount++;
    }
    rewind(input); // 파일 포인터를 다시 처음으로
    return EdgeCount;
}


int main(void) {
    int VertexCount, EdgeCount, startPoint, endingPoint;
    FILE * input = openFile("input.txt", "r"), *output = openFile("output.txt", "w");

    // 간선 개수 먼저 얻기 (fscanf 앞에 호출)
    EdgeCount = getEdgeCount(input);

    char line_buffer[200]; // 한 줄을 읽기 위한 버퍼

    // 첫 줄 읽기: VertexCount, startPoint, endingPoint
    if (fgets(line_buffer, sizeof(line_buffer), input) != NULL) {
        sscanf(line_buffer, "%d,%d,%d", &VertexCount, &startPoint, &endingPoint);
    } else {
        fprintf(stderr, "Error: Cannot find first line in input file.\n");
        exit(1);
    }

    // 1-인덱싱을 위해 배열 크기 조정 (VertexCount + 1)
    int VertexCount_Adjusted = VertexCount + 1;

    // 동적 할당: EdgeList, dp, predecessors_global
    Edge* edgeList = (Edge*)malloc(sizeof(Edge) * EdgeCount);
    if (edgeList == NULL) { fprintf(stderr, "Memory allocation failed for edgeList.\n"); exit(1); }

    int* dp = (int*)malloc(sizeof(int) * VertexCount_Adjusted);
    if (dp == NULL) { fprintf(stderr, "Memory allocation failed for dp array.\n"); exit(1); }

    predecessors_global = (ParentNode**)malloc(sizeof(ParentNode*) * VertexCount_Adjusted);
    if (predecessors_global == NULL) { fprintf(stderr, "Memory allocation failed for predecessors_global.\n"); exit(1); }

    // predecessors_global 초기화 (모든 포인터를 NULL로)
    for (int i = 0; i < VertexCount_Adjusted; i++) {
        predecessors_global[i] = NULL;
    }

    // 간선 정보 읽기
    for (int i = 0; i < EdgeCount; i++) {
        if (fgets(line_buffer, sizeof(line_buffer), input) != NULL) {
            sscanf(line_buffer, "(%d,%d,%d,%d)",
                   &edgeList[i].vertex_1, &edgeList[i].vertex_2,
                   &edgeList[i].weight_1, &edgeList[i].weight_2);
            // 단방향 그래프에서는 정점 번호 정렬 (SWAP) 로직 제거
            // if (edgeList[i].vertex_1 > edgeList[i].vertex_2) {
            //     SWAP(edgeList[i].vertex_1, edgeList[i].vertex_2);
            // }
        } else {
            fprintf(stderr, "Error: Reading edge %d from input file failed.\n", i);
            exit(1);
        }
    }

    int* path = (int*)malloc(sizeof(int) * VertexCount_Adjusted);
    if (path == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for path array.\n");
        exit(1);
    }
    // 각 alpha 값에 대해 BellmanFord 실행 및 결과 출력
    for (double alpha = 0.0; alpha <= 1.0; alpha += 0.1) {
        double ratio1 = alpha;
        double ratio2 = 1.0 - alpha;

        // 소수점 정확도 문제 해결을 위해 반올림 (출력 포맷을 위해)
        ratio1 = round(ratio1 * 10.0) / 10.0;
        ratio2 = round(ratio2 * 10.0) / 10.0;

        // BellmanFord 호출 전 predecessors_global 배열 전체 초기화 (매 알파마다 새로운 계산)
        freePredecessors(VertexCount_Adjusted);

        // 벨만-포드 알고리즘 실행
        BellmanFord(edgeList, EdgeCount, VertexCount_Adjusted, startPoint, dp, ratio1, ratio2);

        // 결과 출력
        if (dp[endingPoint] == INT_MAX) { // 목적지에 도달할 수 없는 경우
            fprintf(output, "(%.1f, %.1f, NoPath)\n", ratio1, ratio2);
            fprintf(output, "No path exists.\n"); // Path 출력 대신 메시지
        } else {
            fprintf(output, "(%.1f, %.1f, %.2f)\n", ratio1, ratio2, (double)dp[endingPoint] / WEIGHT_SCALE);
            fprintf(output, "Path: ");
            printAllShortestPathsRecursive(endingPoint, startPoint, path, 0, output);
        }
    }

    // 모든 동적 할당 메모리 해제
    free(edgeList);
    free(dp);
    freePredecessors(VertexCount_Adjusted); // 최종적으로 predecessors_global의 모든 노드 해제
    free(predecessors_global); // predecessors_global 배열 자체 해제

    fclose(input);
    fclose(output);

    return 0;
}