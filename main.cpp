#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <limits>
#include <map>
#include <set>
#include <chrono>
#include <random>
#include <algorithm>

template <typename T = int>
class RankPairingHeap{
public:
    struct Node{
        T val;
        int rank;
        Node* next;
        Node* lChild,*rChild,*parent;
        Node(){rank = 0;
            next = this;
            lChild = rChild = parent = NULL;}
        Node(T val){
            this->val = val;
            rank = 0;
            next = this;
            lChild = rChild = parent = NULL;
        }
        void getChildren(std::vector<Node*>& result){
            /// Root, left, right (preorder) traversal
            result.push_back(this);
            if(lChild != NULL)
                lChild->getChildren(result);
            if(rChild != NULL)
                rChild->getChildren(result);
        }
        void linkLists(Node* other){
            Node* nextNode = this->next;
            this->next = other->next;
            other->next = nextNode;
        }
        static Node* combineHT(Node* x, Node* y){
        /// combine 2 half-trees. returns parent.

            if(x == NULL)
                return y;
            if(y == NULL)
                return x;
            if(y->val < x->val)
                return combineHT(y,x);

            y->rChild = x->lChild;
            if(y->rChild != NULL)
                y->rChild->parent = y;

            x->lChild = y;
            y->parent = x;
            y->next = NULL;

           x->rank++;

            return x;
        }
        bool operator<(Node &other){
            return this < other;
        }
    };
private:
    Node *firstNode;
    int unsigned n;

    void push(Node *node){
        ///links half tree (keeps children)
        node->parent = NULL;
        if(empty()){
            firstNode  = node;
            firstNode->next = firstNode;
            return;
        }
        node->next = firstNode->next;
        firstNode ->next = node;
        if(node->val < firstNode->val)
            firstNode = node;
    }
    void push(std::vector<Node*> v){
        for(int unsigned i = 0; i < v.size(); i ++)
            push(v[i]);
    }
    class Buckets{
        Node** buckets;
        Node* firstNode;
        int maxRank;
    public:
        Buckets(Node* const firstNodePtr, int n){
            this->firstNode = firstNodePtr;
            maxRank = log2(n) + 3; /// +3 just in case
            buckets = new Node*[maxRank];

            for(int i = 0; i < maxRank; i++)
                buckets[i] = NULL;
        }
        std::vector<Node*> run(){
            std::vector<Node*> result;

            if(firstNode->next == firstNode)
                return result;

            Node* currentNode = firstNode->next;
            buckets[currentNode->rank] = currentNode;
            currentNode = currentNode->next;

            while(currentNode != firstNode){
                Node* nextNode = currentNode->next; /// save it since can become NULL

                /// combine half trees of same rank.
                if(buckets[currentNode->rank] != NULL){
                    int rank = currentNode->rank;
                    Node* parent = Node::combineHT(currentNode,buckets[rank]);

                    buckets[rank] = NULL;

                    result.push_back(parent);/// since onePass we don't combine it again.
                }
                else
                    buckets[currentNode->rank] = currentNode;

                currentNode = nextNode;
            }


            for(int i = 0; i < maxRank; i++)
                if(buckets[i] != NULL)
                    result.push_back(buckets[i]);

            return result;
        }
        ~Buckets(){
            delete[] buckets;
            delete firstNode;
        }
    };

public:
    int unsigned size(){return n;}
    RankPairingHeap(){
        nullify();
    }
    void nullify(){
        n = 0;
        firstNode = NULL;
    }
    bool inline empty(){return firstNode == NULL || n == 0;}
    Node* make_heap(T val){
        if( firstNode != NULL)
            return NULL;
        firstNode = new Node(val);
        n = 1;
        return firstNode;
    }
    void movePtr(RankPairingHeap& other){
        firstNode = other.firstNode;
        n = other.size();
        other.nullify();
    }
    void meld(RankPairingHeap& other){
        if(empty()){
            movePtr(other);
            return;
        }
        if(other.empty() || &other == this)
            return;

        firstNode->linkLists(other.firstNode);

        if(firstNode->val > other.firstNode->val)
            firstNode = other.firstNode;

        n += other.size();
        other.nullify();
    }
    Node* push(T val){
        RankPairingHeap heap2;
        Node* result = heap2.make_heap(val);
        meld(heap2);
        return result;
    }
    void push(std::vector<T> v){
        for(int unsigned i = 0; i < v.size(); i ++)
            push(v[i]);
    }

    T find_Min(){
        if(empty())
            throw std::runtime_error("Heap is empty!");
        return firstNode->val;
    }
    bool edgeCase_delete_min(){
        ///edge cases
        if(empty())
            throw std::runtime_error("Heap is empty!");
        if(size() == 1){
            delete firstNode;
            nullify();
            return true;
        }
        return false;
    }
    Node* delete_min(){
        Node* oldMin = firstNode;
        /// handles edge cases.
        if(edgeCase_delete_min())
            return oldMin;
        /// adds right spine of half trees to the list of roots.
        addRightSpine(firstNode->lChild);

        Buckets onePass(firstNode,size());
        std::vector<Node*> result = onePass.run();
        /// link half trees with buckets for same rank.

        firstNode = NULL;
        n--;

        push(result);

        return oldMin;
    }
    void addRightSpine(Node* &node){
        if(node == NULL) return;
        node->parent = NULL;
        Node* rChild = node->rChild;
        node->rChild = NULL;

        node->next = firstNode->next;
        firstNode->next = node;

        node = rChild;
        addRightSpine(node);
    }

    void decreaseKey(Node* node, T newVal){

        node->val = newVal;
        Node* parent = node->parent;

        if(parent == NULL){ /// case of already root
            if(node->val < firstNode->val)
                firstNode = node;
            return;
        }

        push(node); /// add to root list (with children!)

        if(parent->lChild == node)
            parent->lChild = node->rChild;
        else if(parent->rChild == node)
            parent->rChild = node->rChild;
        else
            throw std::runtime_error("parent wrongly formed!");

        if(node->rChild != NULL)
            node->rChild->parent = parent; /// link rightChild so node becomes half tree
        node->rChild = NULL;

        node->parent = NULL;

        recalculateRank(parent);
    }
    int getRank(Node* node){
        if(node == NULL)
            return -1;
        return node->rank;
    }
    void recalculateRank(Node* &node){
        /// type 1 rank calculation. Complexity is "magic"

        if(node->parent == NULL){
            node->rank = getRank(node->lChild) + 1;
            return;
        }
        int newRank = -1;
        int lRank,rRank;

        lRank = getRank(node->lChild);
        rRank = getRank(node->rChild);

        if(lRank == rRank)
            newRank = lRank + 1;
        else
            newRank = std::max(lRank,rRank);

        if(newRank >= node->rank)
            return;

        node->rank = newRank;
        node = node->parent;
        recalculateRank(node);
    }
    void deleteNode(Node* node){
        decreaseKey(node,std::numeric_limits<T>::min());
        delete_min();
    }

    void afis(std::ostream& out = std::cout){
        out << "Rank-pairing heap looks like this:";
        if(empty()){
            out << "Empty heap!\n";
            return;
        }

        Node* node = firstNode;
        do{
            out << "BINARY TREE rooted at " << node->val << ": ";
            std::vector<Node*> result;
            node->getChildren(result);

            for(unsigned i = 0; i< result.size(); i++)
                out << result[i]->val << ' ';
            out << "\n";
            node = node->next;
        }
        while(node != firstNode);
    }
    ~RankPairingHeap(){
        Node* node = firstNode;

        do{
            if(node == NULL) return;
            std::vector<Node*> result;
            node->getChildren(result);

            node = node->next;
            for(unsigned i = 0; i< result.size(); i++)
                delete result[i];
        }
        while(node != firstNode);
        nullify();
    }

};





namespace Infoarena{
    int randInt(int a, int b) {
        static unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        static std::mt19937 gen(seed);
        std::uniform_int_distribution<int> dist(a, b);
        return dist(gen);
    }
    long long getTimeMCS(){
        return std::chrono::system_clock::now().time_since_epoch() /
        std::chrono::microseconds(1);
    }
    void runMergeHeap(){
        std::ifstream fin("mergeheap.in");
        std::ofstream fout("mergeheap.out");
        int n,q;
        fin >> n >> q;
        RankPairingHeap<int> heaps[n + 1];
        while(q--){
            int type;
            fin >> type;
            if(type == 1){
                int val, nr;
                fin >> nr >> val;
                heaps[nr].push(-val);
            }
            else if(type == 2){
                int nr;
                fin >> nr;
                fout << -heaps[nr].find_Min() << '\n';

                heaps[nr].delete_min();
            }
            else if(type == 3){
                int nr1, nr2;
                fin >> nr1 >> nr2;
                heaps[nr1].meld(heaps[nr2]);
            }

        }
        /// gets 100 points. Surprisingly didn't have implementation problems.
    }
    void generateMerge(){

        std::ofstream fout("mergeheap.in");

        /*** Edit these for different test size ***/
        std::vector<int> chances = {20,50,10,20};
        int n = 1e4;
        int q = 1e7;

        for(unsigned i = 1; i < chances.size(); ++i)
            chances[i]+=chances[i-1];
        int lastChance = chances[chances.size() - 1];

        fout << n << ' ' <<  q;

        int mxId = 0;
        std::map<int,int> elCount;

        for(int i=0; i< q;i ++){
            int op = randInt(0,lastChance);
            if(mxId == 0)
                op = 0; /// first operation has to be insertion.
            if(mxId >= n-2){
                mxId = n-2;
                op = randInt(chances[0] + 1,lastChance);
            }
            if(op <= chances[0]){ /// insert in new heap

                mxId++;
                int val = randInt(0,std::numeric_limits<int>::max());
                fout << "\n1 "<< mxId << ' ' << val;
                elCount[mxId] = 0;
                continue;
            }
            if(op <= chances[1]){ /// insert element in old heap
                int val = randInt(0,std::numeric_limits<int>::max());
                int id = randInt(1,mxId);
                fout << "\n1 "<< id << ' ' << val;
                elCount[id]++;
                continue;
            }
            if(op <= chances[2]){ /// extract_min on this heap
                int id = randInt(1,mxId);
                if(elCount[id]  == 0){
                    i--;
                    continue; /// retry
                }
                fout << "\n2 "<< id;
                elCount[id]--;
                continue;
            }
            if(op <= chances[3]){ /// meld heap2 into heap1
                int id1 = randInt(1,mxId);
                int id2 = randInt(1,mxId);
                fout << "\n3 "<< id1 << ' ' << id2;
                if(id1 == id2)
                    continue;
                elCount[id1] += elCount[id2];
                elCount[id2] = 0;
                continue;
            }
        }
    }
    void runHeapuri(){
        int n;
        std::ifstream fin("heapuri.in");
        std::ofstream fout("heapuri.out");
        fin >> n;
        RankPairingHeap<int> rpHeap;
        typedef RankPairingHeap<int>::Node* rpNode;
        std::vector< rpNode> ptrs;
        while(n--){
            int type;
            fin >> type;
            if(type == 1){
                int x;
                fin >> x;
                rpNode newPtr = rpHeap.push(x);
                ptrs.push_back(newPtr);
            }
            else if(type == 2){
                ///rpHeap.afis(std::cout);
                int nr;
                fin >> nr;
                rpHeap.deleteNode(ptrs[nr - 1]);
            }
            else if(type == 3){
                fout << rpHeap.find_Min() << '\n';
            }
            /// 100p after a 2 hours of debugging
        }
    }

}


int main()
{
    Infoarena::generateMerge();
    long long st = Infoarena::getTimeMCS();
    Infoarena::runMergeHeap();
    long long result = Infoarena::getTimeMCS() - st;/// ~20 sec for 1e7 queries
    std::cout << result << ' ' << result/1000/1000 << "seconds";

    Infoarena::runHeapuri();
}
