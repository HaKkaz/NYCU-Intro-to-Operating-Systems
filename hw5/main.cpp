#pragma GCC optimize("Ofast,fast-math")
#include <stdio.h>
#include <fstream>
#include <sys/time.h>
#include <list>

using namespace std;

const int MAX_PAGE = 0xfffff+50;

struct node {
    node *pre, *nxt;
    int pageId;
    node(int _pageId):pre(nullptr),nxt(nullptr), pageId(_pageId){}
}*where_page[MAX_PAGE];

struct pageList {
    node *head, *tail;
    pageList():head(nullptr), tail(nullptr){}
};

struct Node {
    int freq;
    Node *pre, *nxt;
    pageList PageList;
    Node(int _freq):freq(_freq), pre(nullptr),nxt(nullptr){}

    void AddPage(int pageId) { // Add a new pageId to PageList.
        if (PageList.head == nullptr) {
            PageList.head = new node(pageId);
            PageList.tail = PageList.head;
            where_page[pageId] = PageList.head;
        } else {
            PageList.tail->nxt = new node(pageId);
            PageList.tail->nxt->pre = PageList.tail;
            PageList.tail = PageList.tail->nxt;
            where_page[pageId] = PageList.tail;
        }
    }
}*where_freq[MAX_PAGE];

struct freqList {
    Node *head;
    freqList():head(nullptr){}

    void Set(int pageId) { // Add a page into freqList which freqency is 1.
        if (head == nullptr) {
            head = new Node(1);
            head->AddPage(pageId);
        } else if (head->freq == 1) { 
            // Alread have Node freqency is 1.
            head->AddPage(pageId);
        } else {
            // Have head Node, but freqency is not 1.
            head->pre = new Node(1);
            head->pre->nxt = head;
            head = head->pre;
            head->AddPage(pageId);
        }
        where_freq[pageId] = head;
    }

    void Move(int pageId) { // Add a page into freqList which freqency is `freq`.
        Node *currentNode = where_freq[pageId];
        node *cur = where_page[pageId];
        int freq = currentNode->freq;

        // Add new freq Node into FreqList and add page into pageList with frequency `freq+1`
        if (currentNode->nxt == nullptr || currentNode->nxt->freq != freq + 1) {
            Node *NextNode = currentNode->nxt;
            currentNode->nxt = new Node(freq+1);
            currentNode->nxt->pre = currentNode;
            currentNode->nxt->nxt = NextNode;
            if (NextNode) {
                NextNode->pre = currentNode->nxt;
            }
            currentNode->nxt->AddPage(pageId);
            where_freq[pageId] = currentNode->nxt;
        } else {
            currentNode->nxt->AddPage(pageId);
            where_freq[pageId] = currentNode->nxt;
        }
        // remove page from current pageList
        if (cur->pre) cur->pre->nxt = cur->nxt;
        else currentNode->PageList.head = cur->nxt;

        if (cur->nxt) cur->nxt->pre = cur->pre;
        else currentNode->PageList.tail = cur->pre;

        if (currentNode->PageList.head == nullptr) {
            if (currentNode->pre) currentNode->pre->nxt = currentNode->nxt;
            else head = currentNode->nxt;

            if (currentNode->nxt) currentNode->nxt->pre = currentNode->pre;
        }

    }
    
    void Kick() {
        while (head && head->PageList.head == nullptr) {
            head = head->nxt;
        }
        if (!head || !head->PageList.head) {
            if (!head)
            exit(0);
        }
        int removePage = head->PageList.head->pageId;
        
        // Remove page from pageList.
        Node *currentNode = where_freq[removePage];
        node *cur = where_page[removePage];
        if (cur->pre) cur->pre->nxt = cur->nxt;
        else currentNode->PageList.head = cur->nxt;

        if (cur->nxt) cur->nxt->pre = cur->pre;
        else currentNode->PageList.tail = cur->pre;

        // Remove pageList when it's empty.
        if (currentNode->PageList.head == nullptr) {
            if (currentNode->pre) currentNode->pre->nxt = currentNode->nxt;
            else head = currentNode->nxt;

            if (currentNode->nxt) currentNode->nxt->pre = currentNode->pre;
        }

        // Clear node position pointers of removed page.
        where_freq[removePage] = nullptr;
        where_page[removePage] = nullptr;
    }
};

class LFU {
public:
    int frame, hit, miss, cnt;
    freqList FreqList;
    
    LFU(int _frame):  frame(_frame), hit(0), miss(0), cnt(0){}

    void access(int page_id) {
        if (!where_freq[page_id]) { // freq is zero, miss.
            ++miss;
            if (cnt == frame) { // LFU is full, remove least frequence page
                FreqList.Kick();
            } else {
                ++cnt;
            }
            FreqList.Set(page_id);
        } else {
            ++hit;
            FreqList.Move(page_id);
        }
    }
};

class LRU {
public:
    int frame, hit, miss, cnt;
    node *head, *tail;
    LRU(int _frame):
        frame(_frame), hit(0), miss(0), cnt(0), head(nullptr), tail(nullptr){}

    void access(int page_id) {
        if (!where_page[page_id]) {
            ++miss;
            if (cnt == frame) {
                where_page[head->pageId] = nullptr;
                head = head->nxt;
                head->pre = nullptr;
            } else {
                ++cnt;
            }
            if (head == nullptr) {
                head = new node(page_id);
                tail = head;
                where_page[page_id] = head;
            } else {
                tail->nxt = new node(page_id);
                tail->nxt->pre = tail;
                tail = tail->nxt;
                where_page[page_id] = tail;
            }
        } else {
            ++hit;
            auto cur = where_page[page_id];
            if (cur->pre) cur->pre->nxt = cur->nxt;
            else head = cur->nxt;

            if (cur->nxt) cur->nxt->pre = cur->pre;
            else tail = cur->pre;

            tail->nxt = new node(page_id);
            tail->nxt->pre = tail;
            tail = tail->nxt;
            where_page[page_id] = tail;
        }
    }
};

int main(int argc, char *argv[]) {
    string input_file = argv[1];
    timeval beg, end;
    double sec;

    printf("LFU policy:\n");
    printf("Frame\tHit\t\tMiss\t\tPage fault ratio\n");
    gettimeofday(&beg, 0);
    for (int frame_sz = 64; frame_sz <= 512; frame_sz <<= 1) {
        LFU lfu(frame_sz);
        ifstream fin(input_file);
        int page_id;
        fill(where_freq, where_freq+MAX_PAGE, nullptr);
        fill(where_page, where_page+MAX_PAGE, nullptr);
        while (fin >> page_id) {
            lfu.access(page_id);
        }
        fin.close();

        int hit  = lfu.hit;
        int miss = lfu.miss;
        double fault_ratio = 1.0 * miss / (hit + miss);
        printf("%d\t%d\t\t%d\t\t%.10F\n", frame_sz, hit, miss, fault_ratio);
    }
    gettimeofday(&end, 0);
    sec = (end.tv_sec - beg.tv_sec) + (end.tv_usec - beg.tv_usec) / 1e6;
	printf("Total elapsed time %.4f sec\n\n", sec);

    printf("LRU policy:\n");
    printf("Frame\tHit\t\tMiss\t\tPage fault ratio\n");
    gettimeofday(&beg, 0);
    for (int frame_sz = 64; frame_sz <= 512; frame_sz <<= 1) {
        LRU lfu(frame_sz);
        ifstream fin(input_file);
        int page_id;
        fill(where_freq, where_freq+MAX_PAGE, nullptr);
        fill(where_page, where_page+MAX_PAGE, nullptr);
        while (fin >> page_id) {
            lfu.access(page_id);
        }
        fin.close();

        int hit  = lfu.hit;
        int miss = lfu.miss;
        double fault_ratio = 1.0 * miss / (hit + miss);
        printf("%d\t%d\t\t%d\t\t%.10F\n", frame_sz, hit, miss, fault_ratio);
    }
    gettimeofday(&end, 0);
    sec = (end.tv_sec - beg.tv_sec) + (end.tv_usec - beg.tv_usec) / 1e6;
	printf("Total elapsed time %.4f sec\n\n", sec);
    return 0;
}
