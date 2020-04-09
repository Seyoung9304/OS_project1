#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#define _CRT_SECURE_NO_WARNINGS
#pragma warning (disable:4996)
#define QSIZE 20

typedef struct timenode {
    int burst_time;
    struct timenode* next;
}TN;

typedef struct process {
    int pid;
    int AT; //프로세스 생성시 시간 (초기값. 변하지않아여)
    int arrivalT; //초기 생성 or AWAKE시 갱신. 
    int waitT; //본인 차례 왔을 때 currentT-arrivalT로 구하자
    int total_cycle;
    int RQnumber;
    TN CPUbt; //CPU burst time 타임노드 linked-list
    TN IObt; //I/O burst time 타임노드 linked-list
}Process;

typedef struct Process_element {
    int pid;
    int remain;
    struct SRTN_element* next;
}P_element;

typedef struct queue {
    int head;
    int tail;
    int arr[QSIZE]; //pid저장할 array
}Q;

void enque(Q* qname, int pid) {
    qname->tail = (qname->tail + 1) % QSIZE;
    qname->arr[qname->tail] = pid;
}

void deque(Q* qname) {
    if (is_empty(qname)) { //큐가 비어있는 경우
        printf("Empty Queue\n");
        return;
    }
    qname->head = (qname->head + 1) % QSIZE;
}

int is_empty(Q* qname) { //비어있으면 1, 그렇지 않으면 0
    if (qname->head == qname->tail)
        return 1;
    else
        return 0;
}

int main() {

    Q RR_2;
    Q RR_6;
    P_element* SRTN; //linked-list구조로 remaining CPU time이 적은 순으로 관리. head node
    Q FCFS;
    P_element* asleep; //linked-list구조로 remaining I/O time이 적은 순으로 관리. head node
    
    RR_2.head = RR_2.tail = 0;
    RR_6.head = RR_6.tail = 0;
    FCFS.head = FCFS.tail = 0;


    char fname[20]; 
    printf("input file name: \n");
    //scanf("%s", fname);
    strcpy(fname,"hello.txt");
    
    FILE* fp = fopen(fname, "r");
    if (fp == NULL) { printf("No such file/n"); return 0; }

    char temp;
    temp = fgetc(fp);
    int pcount = 0;
    int menu = 0;
    int total_process = atoi(&temp);

    Process* pcs = (Process*)malloc(sizeof(Process) * total_process);
    fgetc(fp); //#of Ps 받은 뒤 나오는 엔터 받고 버리기

    int cyclecnt = 0;
    TN* CPUcur = NULL;
    TN* IOcur = NULL; //타임노드 current 가르킬 포인터
    char buffer[10] = { NULL };
    int len = 0;
    while (pcount < total_process) { //프로세스 정보 받아서 각 구조체에 저장
        temp = fgetc(fp);
        if (temp == '\t' || temp ==' ' || temp=='\n') {
            len = 0;
        }
        else { //단어 단위로 받기
            buffer[len++] = temp;
            continue;
        }

        int value = atoi(buffer);
        switch (menu) {
        case 0: //pid 설정하고, CPUbt & IObt 노드 초기화
            pcs[pcount].pid = value;
            CPUcur = &(pcs[pcount].CPUbt);
            IOcur = &(pcs[pcount].IObt);
            pcs[pcount].waitT = 0;
            menu++;
            break;
        case 1: //init_Qnumber
            pcs[pcount].RQnumber = value;
            menu++;
            break;
        case 2: //arrival_time. 초기고정값 AT, 추후갱신할 arrivalT 초기화
            pcs[pcount].AT = value;
            pcs[pcount].arrivalT = value;
            menu++;
            break;
        case 3: //number of cycles 
            pcs[pcount].total_cycle = value;
            menu++;
            break;
        default: //sequence of burst cycles
            //menu가 짝수: CPU Burst time
            //menu가 홀수: I/O Burst time
            if (menu % 2==0) { //CPU
                TN* newnode = (TN*)malloc(sizeof(TN));
                newnode->burst_time = value;
                newnode->next = NULL;
                CPUcur->next = newnode;
                CPUcur = newnode;
                cyclecnt++;
            }
            else {
                TN* newnode = (TN*)malloc(sizeof(TN));
                newnode->burst_time = value;
                newnode->next = NULL;
                IOcur->next = newnode;
                IOcur = newnode;
                cyclecnt++;
            }
            menu++;

        }
        for (int i = 0; i < 10; i++) {
            buffer[i] = NULL;
        }
        if (temp == '\n') {
            pcount++;
            cyclecnt = 0;
            menu = 0;
            CPUcur = NULL;
            IOcur = NULL;
            len = 0;
        }
    }

    /*
    while (total_process-->0) {
        printf("%d %d %d\t", pcs[total_process].pid, pcs[total_process].AT, pcs[total_process].total_cycle);
        TN* cur = pcs[total_process].CPUbt.next;
        while (cur != NULL) {
            printf("%d ", cur->burst_time);
            cur = cur->next;
        }
        puts("");
    }
    */
















    //end
    fclose(fp);

    return 0;
}
