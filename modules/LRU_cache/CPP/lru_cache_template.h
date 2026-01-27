#include <algorithm>
#include <iostream>
#include <mutex>
#include <optional>
using namespace std;

template <typename T>
class LRUCache
{
public:
    /*
    * @brief: class to hold doubly linked list node
    *         holds key, value, pointer to previous, pointer to next
    * @params: integer key 'k', Template type value 'val'
    * @returns: None
    */
    class Node
    {
    public:
        Node* prev;
        Node* next;
        int key;
        T value;
        Node( int k = 0, T val = T() );
    };

    /*
    * @brief: Constructor for LRUCache 
    * @params: integer for capacity of cache_map
    * @returns: None
    */
    LRUCache( int capacity );

    ~LRUCache();

    /*
    * @brief: Put key, value in cache_map
    * @params: integer for key 'key', template type value 'value'
    * @returns: None
    */
    void put( int key, T value );
    
    /*
    * @brief: Get value from cache_map for key
    * @params: integer for key 'key'
    * @returns: template type value 'value'
    */
    std::optional<T> get( int key );

    /*
    * @brief: Prints current cache_map
    * @params: None
    * @returns: None
    */
    void print();

private:
    /*
    * @brief: Insert node holding key, value from cache_map into linked list
    * @params: Pointer to Node to be entered
    * @returns: None
    */
    void insertNode( Node* node );
    
    /*
    * @brief: Delete node from linked list
    * @params: Pointer to Node to be deleted
    * @returns: None
    */
    void deleteNode( Node* node );

private:
    Node* head;
    Node* tail;
    int cap;
    unordered_map< int, Node* > cache_map;
    mutex cache_mutex;
};

template <typename T>
LRUCache<T>::Node::Node( int k, T val):
    key( k ),
    value( val )
{

}

template <typename T>
LRUCache<T>::LRUCache( int capacity ):
    cap( capacity )
{
    head = new Node();
    tail = new Node();
    head->next = tail;
    tail->prev = head;
}

template <typename T>
LRUCache<T>::~LRUCache()
{
    Node* curr = head->next;
    while( curr!=tail )
    {
        Node* next = curr->next;
        delete curr;
        curr = next;
    }
    delete head;
    delete tail;
}

template <typename T>
void LRUCache<T>::insertNode( Node* node )
{
    Node* old_mru = head->next;
    node->next = old_mru;
    old_mru->prev = node;
    node->prev = head;
    head->next = node;
}

template <typename T>
void LRUCache<T>::deleteNode( Node* node )
{
    Node* prevv = node->prev;
    Node* nextt = node->next;

    prevv->next = nextt;
    nextt->prev = prevv;
}

template <typename T>
std::optional<T> LRUCache<T>::get( int key )
{
    lock_guard<mutex> lock(cache_mutex);
    if(cache_map.find( key ) != cache_map.end())
    {
        // If key exists in map, get value, and move node from wherever it is to MRU
        T ret = cache_map[key]->value;
        deleteNode( cache_map[key] );
        insertNode( cache_map[key] );
        return ret;
    }
    return std::nullopt;
}

template <typename T>
void LRUCache<T>::put( int key, T value )
{
    lock_guard<mutex> lock( cache_mutex );
    if(cache_map.find( key ) != cache_map.end())
    {
        // If key exists in map, remove node from linked list
        Node* node = cache_map[key];
        deleteNode( node );
    }
    else
    {
        // Else create new entry in map for key
        Node* node = new Node();
        node->key = key;
        cache_map[key] = node;
    }
    if(cache_map.size() > cap)
    {
        // If we are at capacity, delete LRU
        Node* lru = tail->prev;
        cache_map.erase( lru->key );
        deleteNode( lru );
        delete( lru );
    }
    // Attach new value, and put node at MRU
    cache_map[key]->value = value;
    insertNode( cache_map[key] );
}

template <typename T>
void LRUCache<T>::print()
{
    LRUCache::Node* curr = head->next;
    cout << "cache { ";
    while(curr->key != 0)
    {
        cout << "key: " << curr->key << ", val: " << curr->value << endl;
        curr = curr->next;
    }
    cout << "}" << endl;
}
