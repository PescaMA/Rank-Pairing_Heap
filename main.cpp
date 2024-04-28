#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>

template <typename T>
class RankPairingHeap{
    struct Node{
        T val;
        int rank;
        Node* next;
        Node* lChild,*rChild;
        Node(){rank = 0;
            next = this;
            lChild = rChild = NULL;}
        Node(T val){
            this->val = val;
            rank = 0;
            next = this;
            lChild = rChild = NULL;
        }
        void print(std::ostream& out){
            if(lChild != NULL)
                lChild->print(out);
            out << val << ' ';
            if(rChild != NULL)
                rChild->print(out);
        }
    };
    Node *firstNode,*lastNode;
    int unsigned n;
public:
    int unsigned size(){return n;}
    RankPairingHeap(){
        nullify();
    }
    void nullify(){
        n = 0;
        firstNode = lastNode = NULL;
    }
    bool empty(){return firstNode == NULL || n == 0;}
    bool make_heap(T val){
        if( firstNode != NULL)
            return false;
        firstNode = new Node(val);
        lastNode = firstNode;
        n = 1;
        return true;
    }
    RankPairingHeap move(RankPairingHeap& other){
        firstNode = other.firstNode;
        lastNode = other.lastNode;
        n = other.size();
        other.nullify();
        return *this;
    }
    static RankPairingHeap meld(RankPairingHeap& h1, RankPairingHeap& h2){
        RankPairingHeap result;
        if(h1.empty()) return result.move(h2);
        if(h2.empty()) return result.move(h1);
        if(h1.firstNode->val > h2.firstNode->val)
            return meld(h2,h1);
        if(h1.firstNode == h1.lastNode)
            h1.lastNode = h2.lastNode;
        h2.lastNode->next = h1.firstNode->next;
        h1.firstNode->next = h2.firstNode;
        h1.n += h2.size();
        h2.nullify();
        return result.move(h1);
    }
    void push(T val){
        RankPairingHeap heap2;
        heap2.make_heap(val);
        *this = meld(*this,heap2);
    }
    void push(Node *node){
        if(empty()){
            firstNode = lastNode = node;
            firstNode->next = firstNode;
            return;
        }

        if(node->val < firstNode->val){
            node->next = firstNode;
            lastNode->next = node;
            firstNode = node;
        }
        else{
            node->next = firstNode->next;
            if(firstNode->next == firstNode)
                lastNode = node;
            firstNode->next = node;
        }
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
    void delete_min(){
        if(empty())
            throw std::runtime_error("Heap is empty!");
        if(size() == 1){
            delete firstNode;
            nullify();
            return;
        }


        addRightSpine(firstNode->lChild);

        lastNode->next = firstNode->next;
        delete firstNode;
        n--;

        firstNode = lastNode->next;

        int maxRank = log2(n) + 1;
        Node** Buckets = new Node*[maxRank];
        for(int i = 0; i < maxRank; i++)
            Buckets[i] = NULL;

        std::vector<Node*> result;

        Buckets[firstNode->rank] = firstNode;

        Node* nextNode = firstNode->next;
        while(nextNode != firstNode){
            Node* currentNode = nextNode;
            if(Buckets[currentNode->rank] != NULL){
                Node * sameRankNode = Buckets[currentNode->rank];
                Buckets[currentNode->rank] = NULL;

                if(sameRankNode->val < currentNode->val)
                    std::swap(sameRankNode,currentNode);
                sameRankNode->rChild = currentNode->lChild;
                currentNode->lChild = sameRankNode;

                currentNode->rank++;

                result.push_back(currentNode);
            }
            else{
                Buckets[currentNode->rank] = currentNode;
            }
            nextNode = nextNode->next;
        }


        for(int i = 0; i < maxRank; i++)
            if(Buckets[i] != NULL)
                result.push_back(Buckets[i]);

        delete[] Buckets;

        firstNode = lastNode = NULL;
        push(result);
    }
    void addRightSpine(Node* &node){
        if(node == NULL) return;
        Node* rChild = node->rChild;
        node->rChild = NULL;

        node->next = firstNode->next;
        if(firstNode->next == firstNode)
            lastNode = node;
        firstNode->next = node;

        node = rChild;
        addRightSpine(node);
    }


    void afis(std::ostream& out){
        if(empty()){
            out << "Empty heap!\n";
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
void runInfoarenaMergeHeap(){
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
            heaps[nr1] = RankPairingHeap<int>::meld(heaps[nr1],heaps[nr2]);
        }
    }
    /// gets 100 points. nice
}
int main()
{
    RankPairingHeap<int> myHeap;

    static const int arr[] = {1, 10, 2, 3, 4};
    std::vector<int> vec (arr, arr + sizeof(arr) / sizeof(arr[0]) );


    myHeap.push(vec);
    myHeap.delete_min();
    myHeap.afis(std::cout);
}
