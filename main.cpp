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
        if(other.empty())
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
    void delete_min(){
        /// handles edge cases.
        if(edgeCase_delete_min())
            return;
        /// adds right spine of half trees to the list of roots.
        addRightSpine(firstNode->lChild);

        Buckets onePass(firstNode,size());
        std::vector<Node*> result = onePass.run();
        /// link half trees with buckets for same rank.

        firstNode = NULL;
        n--;

        push(result);
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
namespace Tests{
    int inline randInt(int a, int b) {
        return rand() % (b-a+1) + a;
    }
    void tiny_test(){
        RankPairingHeap<int> test;
        test.push(3);
        test.afis();
    }
    void runTest(){/*
        std::ifstream fin("test.in");
        std::ofstream fout("test.out");

        int q;
        while(fin>>q){

            if(q == 1){
                int val;
                fin >> val;

            }

        }*/
    }
    void generateTest(){
        srand(time(NULL));
        std::ofstream fout("test.in");

        std::vector<int> ids;
        std::map<int,int> idCount;
        std::set<RankPairingHeap<int>::Node*> nodes;
        int const testSize = 1e2;

        int inserted = 0;

        for(int i = 0; i < testSize; i++){
            int rnd = randInt(-2,5);
            if(ids.empty())
                rnd = 1;
            int id ;
            if(rnd == 1)
                id = randInt(1,std::numeric_limits<int>::max());
            else
                id = ids[randInt(0,ids.size()-1)];
            if(rnd <= 0)
                rnd = 1;


            if(rnd == 1){
                if(idCount.find(id) == idCount.end()){
                    ids.push_back(id);
                    idCount[id] = 0;
                }
                int val = randInt(1,std::numeric_limits<int>::max());
                fout << '\n' << rnd << ' ' << val;
                idCount[id]++;
                inserted++;
                continue;
            }
            if(idCount[id] == 0){ /// operation on empty heap
                i--;
                continue;
            }
            fout << '\n' << rnd << ' ';

            if(rnd == 2){ /// getMin
                fout << id;
            }
            if(rnd == 3){ /// extractMin
                fout << id;
                idCount[id]--;
                inserted--;
            }
            if(rnd == 4){ /// meld (combine heaps)
                int id2 = ids[randInt(0,ids.size()-1)];
                fout << id << ' ' << id2;
            }
            if(rnd == 5){ /// decrease nth inserted node (excluding deleted)
                fout << randInt(1,inserted) << ' ';
                fout << randInt(1,inserted);
            }
        }
    }
}
int main()
{

    Tests::generateTest();

    Infoarena::runMergeHeap();
    Infoarena::runHeapuri();
}
