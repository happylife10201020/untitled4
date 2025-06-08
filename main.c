#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SWAP(a,b) int temp = a; a = b; b = temp;
#define WEIGHT_RATIO_1 0.5
#define WEIGHT_RATIO_2 0.5

typedef struct Edge {
  int vertex_1, vertex_2;
  int weight_1, weight_2;
}Edge;

FILE* openFile(char * filename, const char *mode);
int getEdgeCount(FILE* input);
void BellmanFord(Edge* edgeList, int EdgeCount, int VertexCount, int startPoint);

int main(void) {
    int VertexCount, EdgeCount, startPoint, endingPoint;
    int ret; //fscanf 반환 개수
    FILE * input = openFile("input.txt", "r"), *output = openFile("output.txt", "w");

    EdgeCount = getEdgeCount(input);
    Edge* edgeList = (Edge*)malloc(sizeof(Edge) * EdgeCount);

    char line[100];
    if (fgets(line, sizeof(line), input) != NULL) {
        sscanf(line, "%d,%d,%d", &VertexCount, &startPoint, &endingPoint);
        //printf("%d %d %d\n", VertexCount, startPoint, endingPoint);
    } else {
        printf("Can`t find lines.\n");
    }

    //
    for (int i = 0; i < EdgeCount; i++) {
        if (fgets(line, sizeof(line), input) != NULL) {
            sscanf(line, "(%d,%d,%d,%d)", &edgeList[i].vertex_1, &edgeList[i].vertex_2, &edgeList[i].weight_1, &edgeList[i].weight_2);
            if (edgeList[i].vertex_1 > edgeList[i].vertex_2) {
                SWAP(edgeList[i].vertex_1, edgeList[i].vertex_2);
            }
            printf("%d %d %d %d\n", edgeList[i].vertex_1, edgeList[i].vertex_2, edgeList[i].weight_1, edgeList[i].weight_2);
        }
    }

    return 0;
}

FILE* openFile(char * filename, const char *mode) {
    FILE* file = fopen(filename, mode);
    if (file == NULL) {
        fprintf(stderr, "Can`t open file. FILENAME : %s", filename);
        exit(1);
    }
    return file;
}

int getEdgeCount(FILE* input) {
    int EdgeCount = 0;
    char ch;
    while ((ch = fgetc(input)) != EOF) {
        if (ch == '\n') EdgeCount++;
    }
    fseek(input, -1, SEEK_END);
    if (fgetc(input) == '\n') EdgeCount--;
    rewind(input);
    return EdgeCount;
}

void BellmanFord(Edge* edgeList, int EdgeCount, int VertexCount, int startPoint) {
    int dp[100];
    for (int i = 0; i < VertexCount; i++) {
        dp[i] = INT_MAX;
    }
    dp[startPoint] = 0;

    for (int i = 0; i < VertexCount - 1; i++) {
        for (int j = 0; j < EdgeCount; j++) {
            int u = edgeList[j].vertex_1, v = edgeList[j].vertex_2;
            int weight1 = edgeList[j].weight_1, weight2 = edgeList[j].weight_2;
            int weightSum = WEIGHT_RATIO_1 * weight1 + WEIGHT_RATIO_2 * weight2; // 각 값에 가중치
            if (dp[u] != INT_MAX && dp[u] + weightSum < dp[v]) {
                dp[v] = dp[u] + weightSum;
            }
        }
    }
}