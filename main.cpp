#include <iostream>
#include <vector>

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
    };
    Node *firstNode,*lastNode;
public:
    RankPairingHeap(){
        nullify();
    }
    void nullify(){
        firstNode = lastNode = NULL;
    }
    bool empty(){return firstNode == NULL;}
    bool make_heap(T val){
        if( firstNode != NULL)
            return false;
        firstNode = new Node(val);
        lastNode = firstNode;
        return true;
    }
    RankPairingHeap move(RankPairingHeap& other){
        firstNode = other.firstNode;
        lastNode = other.lastNode;
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
        return result.move(h1);
    }
    void push(T val){
        RankPairingHeap heap2;
        heap2.make_heap(val);
        *this = meld(*this,heap2);
    }
    /*void push(T val){
        if(make_heap(val))
            return;

        Node *node = new Node(val);
        if(val < firstNode->val){
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
    }*/
    void push(std::vector<T> v){
        for(int unsigned i = 0; i < v.size(); i ++)
            push(v[i]);
    }
    T find_Min(){
        if(firstNode == NULL)
            throw std::runtime_error("Heap is empty!");
        return firstNode->val;
    }


    void afis(std::ostream& out){
        if(empty())return;

        Node* node = firstNode;
        do{
            out << node->val << ' ';
            node = node->next;
        }
        while(node != firstNode);
    }
};

int main()
{
    RankPairingHeap<int> myHeap;

    static const int arr[] = {16,2,77,29};
    std::vector<int> vec (arr, arr + sizeof(arr) / sizeof(arr[0]) );

    myHeap.push(vec);
    myHeap.afis(std::cout);
}
