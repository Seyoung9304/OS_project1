#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#define _CRT_SECURE_NO_WARNINGS
#pragma warning (disable:4996)
#define QSIZE 20
#define TIME 1000

typedef struct timenode {
    int burst_time;
    struct timenode* next;
}TN;

typedef struct process {
    int pid;
    int AT; //프로세스 생성시 시간 (초기값. 변하지않아여)
    int arrivalT; //초기 생성 or AWAKE시 갱신. 
    int waitT; //본인 차례 왔을 때 currentT-arrivalT로 구하자. 누적하는 방식
    int total_cycle;
    int RQnumber; //초기 생성 or 상황에 따라 dequeue되기 전 갱신. 
    int TT; //Turnaround Time - 프로세스 종료시에 curTIME-AT로 구함
    TN CPUbt; //CPU burst time 타임노드 linked-list
    TN IObt; //I/O burst time 타임노드 linked-list
}Process;

typedef struct Process_element {
    int pid;
    int remain;
    struct Process_element* next;
}P_element;

typedef struct queue {
    int head;
    int tail;
    int arr[QSIZE]; //pid저장할 array
}Q;

Q* RR_2;
Q* RR_6;
P_element* SRTN; //linked-list구조로 remaining CPU time이 적은 순으로 관리. head node
Q* FCFS;
//P_element* asleep; //linked-list구조로 remaining I/O time이 적은 순으로 관리. head node
int gantt_record[TIME] = { 0, };

int is_terminated(Process* process) { //프로세스의 CPUburst가 남아있는지 확인 후 종료되었다고 판단할 시 TT값을 저장해줌
    if (process->CPUbt.next == NULL) {
        return 1;
    }
    else
        return 0;
}

int is_all_terminated(Process* process, int total_process) { //모든 프로세스가 종료되었는지 확인
    for (int i = 0; i < total_process; i++) {
        if (is_terminated(&process[i]) == 0) //아직 안끝난게 하나라도 있으면 바로 0 리턴 (not terminated)
            return 0;
    }
    return 1;
}

int is_empty(Q* qname) { //비어있으면 1, 그렇지 않으면 0
    if (qname->head == qname->tail)
        return 1;
    else
        return 0;
}
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


void insert_SRTN(int pid, int remainT) {
    P_element* newnode = (P_element*)malloc(sizeof(P_element));
    newnode->next = NULL;
    newnode->pid = pid;
    newnode->remain = remainT;

    P_element* cur = SRTN;
    while (cur->next != NULL && cur->next->remain < remainT) {
        cur = cur->next;
    }
    newnode->next = cur->next;
    cur->next = newnode;

    return;
}

Process* find_process_by_id(Process* plist, int plist_size, int pid) {
    for (int i = 0; i < plist_size; i++) {
        if (plist[i].pid == pid) {
            return &plist[i];
        }
    }
    printf("Cannot find process. Error occured\n");
}

int main() {
    /*
    Initialize Ready Queues
    */   
    RR_2 = (Q*)malloc(sizeof(Q));
    RR_6 = (Q*)malloc(sizeof(Q));
    FCFS = (Q*)malloc(sizeof(Q));
    RR_2->head = RR_2->tail = -1;
    RR_6->head = RR_6->tail = -1;
    FCFS->head = FCFS->tail = -1;
    SRTN = (P_element*)malloc(sizeof(P_element));
    //asleep = (P_element*)malloc(sizeof(P_element));
    SRTN->remain = 0;
    SRTN->next = NULL;

    /*
    Open file
    */
    char fname[20]; 
    printf("input file name: \n");
    scanf("%s", fname);
    FILE* fp = fopen(fname, "r");
    if (fp == NULL) { printf("No such file/n"); return 0; }

    /*
    Get contents from file
    */
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

    /*
    Start scheduling. 
    Count time from 0
    */
    int curTIME = 0;
    int RUNNING = 0; //1인경우 프로세스 실행중 
    int TQ = 0; //time quantum counter
    P_element* now_running_srtn=NULL;
    Process* now_running = NULL;
    do {
        // 1. Arrive한 프로세스를 각 해당 큐로 Enqueue
        for (int i = 0; i < total_process; i++) {
            if (pcs[i].arrivalT == curTIME) {
                switch (pcs[i].RQnumber){
                case 0: 
                    enque(RR_2, pcs[i].pid);
                    break;
                case 1:
                    enque(RR_6, pcs[i].pid);
                    break;
                case 2:
                    insert_SRTN(pcs[i].pid, pcs[i].CPUbt.next->burst_time);
                    break;
                case 3:
                    enque(FCFS, pcs[i].pid);
                    break;
                default:
                    printf("ERR\n");
                    break;
                }
            }
        }


        
        //2. Q0부터 Q3까지 훑고 내려가면서 실행
        int pid_temp = 0;
        if (RUNNING == 0) {
            TQ = 0;
            if (!is_empty(RR_2)) {
                pid_temp = RR_2->arr[RR_2->head + 1];
                now_running = find_process_by_id(pcs, total_process, pid_temp);
                RUNNING = 1;
            }
            else if (!is_empty(RR_6)) {
                pid_temp = RR_6->arr[RR_6->head + 1];
                now_running = find_process_by_id(pcs, total_process, pid_temp);
                RUNNING = 2;
            }
            else if (SRTN->next != NULL) {
                now_running_srtn = SRTN->next;
                RUNNING = 3;
            }
            else if (!is_empty(FCFS)) {
                pid_temp = FCFS->arr[FCFS->head + 1];
                now_running = find_process_by_id(pcs, total_process, pid_temp);
                RUNNING = 4;
            }
            else
                RUNNING = 0; //큐 비어있음
        }
        if (RUNNING == 1) {
            if (TQ < 2) { //NOTE: 원래는 TQ<2였당.......
                //CPUtime-->0인지 검사
                //0아니면 TQ++
                //0이면 Deque&awake시간 설정하기(arrivalT)&RUNNING=0
                if (TQ == 0)
                    now_running->waitT += (curTIME - now_running->arrivalT); //해결함-/*이게아니고....pcs전체중에 pid_temp랑 pcs[].pid랑 같은 애를 찾아와야해*/
                if (--now_running->CPUbt.next->burst_time > 0) {
                    TQ++;
                }
                else {
                    now_running->CPUbt.next = now_running->CPUbt.next->next;
                    if (!is_terminated(now_running)) { //프로세스 terminate되지 않은 경우
                        now_running->arrivalT = curTIME + now_running->IObt.next->burst_time + 1;
                        now_running->IObt.next = now_running->IObt.next->next;
                    }
                    else //프로세스 종료 - Turnaround Time 저장.
                        now_running->TT = curTIME - now_running->AT + 1;
                    deque(RR_2);
                    RUNNING = 0;
                }
                gantt_record[curTIME] = now_running->pid;
            }
            if(TQ==2) { //preempt
                //DeQue, 우선순위 1단계 낮아짐&RUNNING=0
                deque(RR_2);
                now_running->RQnumber++;
                now_running->arrivalT = curTIME+1;
                RUNNING = 0;
            }
        }
        else if (RUNNING == 2) {
            if (TQ < 6) {
                //CPUtime-->0인지 검사
                //0아니면 TQ++
                //0이면 Deque&우선순위 1단계 높아짐&awake시간 설정하기(arrivalT)&RUNNING=0
                if (TQ == 0)
                    now_running->waitT += (curTIME - now_running->arrivalT);
                if (--now_running->CPUbt.next->burst_time > 0) {
                    TQ++;
                }
                else {
                    now_running->RQnumber--;
                    now_running->CPUbt.next = now_running->CPUbt.next->next;
                    if (!is_terminated(now_running)) {//프로세스 terminate되지 않은 경우
                        now_running->arrivalT = curTIME + now_running->IObt.next->burst_time + 1;
                        now_running->IObt.next = now_running->IObt.next->next;
                    }
                    else
                        now_running->TT = curTIME - now_running->AT + 1;
                    deque(RR_6);
                    RUNNING = 0;
                }
                gantt_record[curTIME] = now_running->pid;
            }
            if (TQ == 6) { //preempt
                //DeQue, 우선순위 1단계 낮아짐&RUNNING=0
                deque(RR_6);
                now_running->RQnumber++;
                now_running->arrivalT = curTIME+1;
                RUNNING = 0;
            }
        }
        else if (RUNNING == 3) {
            int tpid = now_running_srtn->pid;
            now_running = find_process_by_id(pcs, total_process, tpid);
            gantt_record[curTIME] = tpid;
            if (SRTN->next != NULL && SRTN->next->pid != tpid && now_running_srtn->remain - 1 > SRTN->next->remain ) { //preemption 필요. SRTN큐 실행중에는 Q0, Q1에 새 프로세스 생겨도 SRTN 우선..?
                SRTN->next->next = SRTN->next->next->next;
                now_running->arrivalT = curTIME + 1;
                now_running->RQnumber++;
                RUNNING = 0;
            }
            if (TQ == 0)
                now_running->waitT += (curTIME - now_running->arrivalT);
            if (--now_running->CPUbt.next->burst_time == 0) {
                RUNNING = 0;
                now_running->RQnumber--;
                now_running->CPUbt.next = now_running->CPUbt.next->next;
                if (!is_terminated(now_running)) { //프로세스 terminate되지 않은 경우
                    now_running->arrivalT = curTIME + now_running->IObt.next->burst_time + 1;
                    now_running->IObt.next = now_running->IObt.next->next;
                }
                else
                    now_running->TT = curTIME - now_running->AT + 1;
                SRTN->next = now_running_srtn->next;
            }
            else 
                TQ++;

            //맨앞에 있는 프로세스 CPUtime-->0인지 검사
            //0아니면 다음 프로세스와 remain time 비교, 다음 프로세스가 rt 작으면 delete&우선순위 낮아짐&arrivalT설정하기(1초뒤로) 아니면 TQ++
            //0이면 Delete&우선순위 1단계 높아짐&awake시간 설정하기(arrivalT)&RUNNING=0
        }
        else if (RUNNING == 4) {
            //CPUtime-->0인지 검사, 0이면 RUNNING=0&우선순위 한단계 높아짐&deque&arrivalT설정
            if (TQ == 0)
                now_running->waitT += (curTIME - now_running->arrivalT);
            if (--now_running->CPUbt.next->burst_time == 0) {
                RUNNING = 0;
                now_running->RQnumber--;
                now_running->CPUbt.next = now_running->CPUbt.next->next;
                if (!is_terminated(now_running)) {//프로세스 terminate되지 않은 경우
                    now_running->arrivalT = curTIME + now_running->IObt.next->burst_time + 1;
                    now_running->IObt.next = now_running->IObt.next->next;
                }
                else
                    now_running->TT = curTIME - now_running->AT + 1;
                deque(FCFS);
            }
            else
                TQ++;
            gantt_record[curTIME] = now_running->pid;
        }
    } while (++curTIME && is_all_terminated(pcs, total_process) != 1);


    /*
    for (int i = 0; i < QSIZE; i++) {
        printf("%d ", RR_2->arr[i]);
    }
    puts("");
    for (int i = 0; i < QSIZE; i++) {
        printf("%d ", RR_6->arr[i]);
    }
    puts("");
    P_element* cur = SRTN;
    while (cur != NULL) {
        printf("%d ", cur->pid);
        cur = cur->next;
    }
    puts("");
    for (int i = 0; i < QSIZE; i++) {
        printf("%d ", FCFS->arr[i]);
    }
    puts("");
    */
    

    printf("\nScheduling Complete\n---------------<Gantt Chart>---------------\n\n");
    for (int i = 0; i < curTIME; i++) {
        if (i == 0 && gantt_record[i] != 0)
            printf("|P%d", gantt_record[i]);
        else if (gantt_record[i - 1] != gantt_record[i]) {
            if (gantt_record[i] == 0)
                printf(" | ");
            else
                printf("|P%d", gantt_record[i]);
        }
        else
            printf("---");
    }
    puts("|");
    for (int i = 0; i < curTIME; i++) {
        if (i == 0 && gantt_record[i] != 0)
            printf("%-3d", i);
        else if (gantt_record[i - 1] != gantt_record[i]) {
                printf("%-3d", i);
        }
        else
            printf("   ");
    }
    printf("%-3d", curTIME);
    puts("\n\n");


    printf("===============================\n");
    printf("|   pid   |   TT    |   WT    |\n");
    for (int i = 0; i < total_process; i++) {
        printf("===============================\n");
        printf("     %-9d%-10d%-9d\n", pcs[i].pid, pcs[i].TT, pcs[i].waitT);
    }
    printf("===============================\n");
    int sum_TT = 0, sum_WT = 0;
    for (int i = 0; i < total_process; i++) {
        sum_TT += pcs[i].TT;
        sum_WT += pcs[i].waitT;
    }
    printf("Avg TT: %d\n", sum_TT/total_process);
    printf("Avg WT: %d\n", sum_WT/total_process);
   

    //end
    fclose(fp);

    return 0;
}
