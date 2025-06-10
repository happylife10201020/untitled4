/*
 * 자료구조 과제
 * 담당교수: 이의종
 * 2022041069 이인수
 */


#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define WEIGHT_SCALE 100 // 가중치 스케일링 상수
#define DEBUG_MODE 0 //디버그 출력

// 간선 구조체
typedef struct Edge {
    int vertex_1, vertex_2;
    int weight_1, weight_2;
} Edge;

// 파일 열기 함수
FILE* openFile(char * filename, const char *mode) {
    FILE* file = fopen(filename, mode);
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file. FILENAME: %s\n", filename);
        exit(1);
    }
    return file;
}

// BellmanFord 알고리즘
// parent 배열을 인자로 받아서 직전 정점을 저장
void BellmanFord(Edge* edgeList, int EdgeCount, int VertexCount_Adjusted, int startPoint, int* dp, int* parent, double ratio1, double ratio2) {
    // dp 배열 초기화
    for (int i = 0; i < VertexCount_Adjusted; i++) {
        dp[i] = INT_MAX; // int형 무한대
        parent[i] = -1;  // parent 배열 초기화: -1은 선행 정점이 없음을 의미
    }
    dp[startPoint] = 0;
    parent[startPoint] = startPoint; // 시작점의 부모 설정

    // (정점 개수 - 1)번 반복하여 최단 거리 갱신
    for (int i = 0; i < VertexCount_Adjusted - 1; i++) {
        for (int j = 0; j < EdgeCount; j++) {
            int u = edgeList[j].vertex_1;
            int v = edgeList[j].vertex_2;

            // 정수형 가중치 합 계산 (100배 스케일링)
            int current_weight_sum = (int)(ratio1 * edgeList[j].weight_1 * WEIGHT_SCALE + ratio2 * edgeList[j].weight_2 * WEIGHT_SCALE);

            if (dp[u] != INT_MAX) { // 도달 가능한 경우에만 릴렉스 시도
                if (dp[u] + current_weight_sum < dp[v]) {
                    dp[v] = dp[u] + current_weight_sum;
                    parent[v] = u; // v의 선행 정점을 u로 설정
                    if (DEBUG_MODE) {
                        fprintf(stderr, "DEBUG: parent[%d]: %d, current_weight_sum: %d\n",v , parent[v], current_weight_sum/WEIGHT_SCALE/10);
                    }
                }
            }
        }
    }
}

// 단일 최단 경로를 출력 함수
void printSingleShortestPath(int endingPoint, int startPoint, int* parent, FILE *output) {
    int path[200]; // 임시 경로 저장 배열 (VertexCount_Adjusted보다 충분히 크게)
    int path_idx = 0;
    int current = endingPoint;

    // 경로 역추적
    while (current != startPoint && current != -1) {
        path[path_idx++] = current;
        current = parent[current];
    }

    if (current == -1 && endingPoint != startPoint) { // 경로 없음
        fprintf(output, "No path exists.\n");
        return;
    }

    path[path_idx++] = startPoint; // 시작점 추가

    // 경로 역순으로 출력
    for (int i = path_idx - 1; i >= 0; i--) {
        fprintf(output, "%d", path[i]);
        if (i > 0) {
            fprintf(output, " -> ");
        }
    }
    fprintf(output, "\n");
}

// 간선 개수 가져오기 함수
int getEdgeCount(FILE* input) {
    int EdgeCount = 0;
    char line[200];

    // 첫 번째 줄은 읽고 버림
    if (fgets(line, sizeof(line), input) == NULL) {
        rewind(input);
        return 0;
    }
    // 나머지 줄의 개수를 count.
    while (fgets(line, sizeof(line), input) != NULL) {
        EdgeCount++;
    }
    rewind(input); // 파일 포인터를 다시 처음으로
    return EdgeCount;
}


int main() {
    int VertexCount, EdgeCount, startPoint, endingPoint;
    FILE * input = openFile("input.txt", "r"), *output = openFile("output.txt", "w");

    // 간선 개수 얻기
    EdgeCount = getEdgeCount(input);

    char line_buffer[200]; // 한 줄을 읽기 위한 버퍼

    // 첫 줄 읽기
    if (fgets(line_buffer, sizeof(line_buffer), input) != NULL) {
        sscanf(line_buffer, "%d,%d,%d", &VertexCount, &startPoint, &endingPoint);
        if (DEBUG_MODE) {
            fprintf(stderr, "DEBUG : \nVertexCount: %d startPoint: %d endinPoint: %d\n", VertexCount, startPoint, endingPoint);
        }
    } else {
        fprintf(stderr, "Error: Cannot find first line in input file.\n");
        exit(1);
    }

    // 인덱싱을 위해 배열 크기 조정
    int VertexCount_Adjusted = VertexCount + 1;

    // 동적 할당: EdgeList, dp, parent
    Edge* edgeList = (Edge*)malloc(sizeof(Edge) * EdgeCount);
    if (edgeList == NULL) { fprintf(stderr, "Memory allocation failed for edgeList.\n"); exit(1); }

    int* dp = (int*)malloc(sizeof(int) * VertexCount_Adjusted);
    if (dp == NULL) { fprintf(stderr, "Memory allocation failed for dp array.\n"); exit(1); }


    // parent 배열 할당
    int* parent = (int*)malloc(sizeof(int) * VertexCount_Adjusted);
    if (parent == NULL) { fprintf(stderr, "Memory allocation failed for parent array.\n"); exit(1); }

    // 간선 정보 읽기
    for (int i = 0; i < EdgeCount; i++) {
        if (fgets(line_buffer, sizeof(line_buffer), input) != NULL) {
            sscanf(line_buffer, "(%d,%d,%d,%d)",
                   &edgeList[i].vertex_1, &edgeList[i].vertex_2,
                   &edgeList[i].weight_1, &edgeList[i].weight_2);
        } else {
            fprintf(stderr, "Error: Reading edge %d from input file failed.\n", i);
            exit(1);
        }
    }

    // 각 가중치 값에 대해 BellmanFord 실행 및 결과 출력
    for (int ratio1 = 0; ratio1 <= 10; ratio1++) { // 가중치 10배 << double오류 방지
        // 소수점 정확도 문제 해결을 위해 반올림
        int ratio2 = 10 - ratio1;

        // 벨만포드 알고리즘 실행
        BellmanFord(edgeList, EdgeCount, VertexCount_Adjusted, startPoint, dp, parent, ratio1, ratio2);

        if (DEBUG_MODE) {
            fprintf(stderr, "DEBUG: scale alpha : %.1f, beta: %.1f \n", (double)ratio1/10, (double)ratio2/10);
            for (int i = 1; i < VertexCount_Adjusted; i++) {
                fprintf(stderr, "dp[%d]: %d, ", i, dp[i] / WEIGHT_SCALE / 10);
            }
            fprintf(stderr, "\n");
        }

        // 결과 출력
        if (dp[endingPoint] == INT_MAX) { // 목적지에 도달할 수 없는 경우
            fprintf(output, "(%.1f, %.1f, NoPath)\n", (double)ratio1/10, (double)ratio2/10);
            fprintf(output, "No path exists.\n"); // Path 출력 대신 메시지
        } else { // 도달가능할 때 경로 출력
            fprintf(output, "(%.1f, %.1f, %d)\n", (double)ratio1/10, (double)ratio2/10, (int)(double)dp[endingPoint] / WEIGHT_SCALE / 10);
            fprintf(output, "Path: ");
            printSingleShortestPath(endingPoint, startPoint, parent, output); // 단일 경로 출력 함수 호출
        }
    }

    // 메모리 해제
    free(edgeList);
    free(dp);
    free(parent);

    fclose(input);
    fclose(output);

    return 0;
}