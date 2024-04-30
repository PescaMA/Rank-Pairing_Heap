#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <limits>

template <typename T>
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
        void print(std::ostream& out){
            if(lChild != NULL)
                lChild->print(out);
            out << val << ' ';
            if(rChild != NULL)
                rChild->print(out);
        }
    };
private:
    Node *firstNode;
    int unsigned n;
public:
    int unsigned size(){return n;}
    RankPairingHeap(){
        nullify();
    }
    void nullify(){
        n = 0;
        firstNode = NULL;
    }
    bool empty(){return firstNode == NULL || n == 0;}
    Node* make_heap(T val){
        if( firstNode != NULL)
            return NULL;
        firstNode = new Node(val);
        n = 1;
        return firstNode;
    }
    RankPairingHeap move(RankPairingHeap& other){
        firstNode = other.firstNode;
        n = other.size();
        other.nullify();
        return *this;
    }
    RankPairingHeap meld(RankPairingHeap& other){
        if(empty()){
            move(other);
            return *this;
        }
        if(other.empty())
            return *this;

        /// catenation of 2 circular simply linked list
        Node * h1Next = firstNode->next;
        firstNode->next = other.firstNode->next;
        other.firstNode->next = h1Next;

        if(firstNode->val > other.firstNode->val)
            firstNode = other.firstNode;

        n += other.size();
        other.nullify();
        return *this;
    }
    Node* push(T val){
        RankPairingHeap heap2;
        Node* result = heap2.make_heap(val);
        meld(heap2);
        return result;
    }
    void push(Node *node){
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

        /// link half trees with buckets for same rank.
        int maxRank = log2(n) + 3;
        Node** Buckets = new Node*[maxRank];

        for(int i = 0; i < maxRank; i++)
            Buckets[i] = NULL;

        std::vector<Node*> result; /// will contain list of half trees after one-pass joins.

        Node* nextNode = firstNode->next;
        Buckets[nextNode->rank] = nextNode;

        nextNode = nextNode->next;
        while(nextNode != firstNode){
            Node* currentNode = nextNode;
            nextNode = nextNode->next;
            /// combine half trees of same rank.
            if(Buckets[currentNode->rank] != NULL){
                Node * sameRankNode = Buckets[currentNode->rank];
                Buckets[currentNode->rank] = NULL;

                if(sameRankNode->val < currentNode->val)
                    std::swap(sameRankNode,currentNode);
                sameRankNode->rChild = currentNode->lChild;
                if(currentNode->lChild != NULL)
                    currentNode->lChild->parent = sameRankNode;
                sameRankNode->parent = currentNode;
                sameRankNode->next = NULL;
                currentNode->lChild = sameRankNode;

                currentNode->rank++;

                result.push_back(currentNode);
            }
            else{
                Buckets[currentNode->rank] = currentNode;
            }
        }


        for(int i = 0; i < maxRank; i++)
            if(Buckets[i] != NULL)
                result.push_back(Buckets[i]);

        delete[] Buckets;
        delete firstNode;

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

        if(parent == NULL){
            if(node->val < firstNode->val)
                firstNode = node;
            return;
        }

        push(node);
        if(parent->lChild == node)
            parent->lChild = node->rChild;
        else if(parent->rChild == node)
            parent->rChild = node->rChild;
        else throw std::runtime_error("parent wrongly formed!");

        if(node->rChild != NULL)
            node->rChild->parent = parent;
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
            node->print(out);
            out << "\n";
            node = node->next;
        }
        while(node != firstNode);
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
    void tiny_test(){
        RankPairingHeap<int> test;
        test.push(3);
        test.afis();
    }
}
int main()
{
    Infoarena::runMergeHeap();
    ///Infoarena::runHeapuri();
}
