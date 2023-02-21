#include "scapegoattree.h"

#include <cmath>
#include <stdexcept> // for std::invalid_argument exception

using namespace details;

// Node constructor for leaf
Node::Node(int value)
    : m_depth(0)
    , m_value(value)
    , m_size(1)
    , m_left(nullptr)
    , m_right(nullptr)
    , m_parent(nullptr)
{
}

Node::~Node()
{
    delete m_left;
    delete m_right;
}

ScapegoatTree::ScapegoatTree(double alpha)
    : m_size(0)
    , m_alpha(alpha)
    , m_root(nullptr)
{
    if (0.5 > alpha || alpha >= 1) {
        throw std::invalid_argument("alpha must be [0.5, 1)");
    }
    m_log = log(1 / alpha);
}

ScapegoatTree::ScapegoatTree()
    : ScapegoatTree(0.75)
{
}

int ScapegoatTree::m_nodeSize(details::Node * ptr)
{
    if (ptr == nullptr) {
        return 0;
    }
    return ptr->m_size;
}

bool ScapegoatTree::insert(int value)
{
    Node * inserted = m_insertKey(m_root, value);
    if (inserted == nullptr) {
        return false;
    }
    ++m_size;

    if (inserted->m_depth > ceil(log(m_size) / m_log)) {
        Node * scapegoat = m_findScapegoat(inserted);
        m_rebuildSubtree(scapegoat);
    }

    return true;
}

// find scapegoat node from start to root
Node * ScapegoatTree::m_findScapegoat(Node * start) const
{
    while (m_nodeSize(start) < m_nodeSize(start->m_parent) * m_alpha) {
        start = start->m_parent;
    }
    return start->m_parent;
}

// create new node with m_value = value
Node * ScapegoatTree::m_insertKey(Node *& ptr, int value, int depth)
{
    if (ptr == nullptr) {
        ptr = new Node(value);
        ptr->m_depth = depth;
        return ptr;
    }
    Node * tmp_node = nullptr;

    if (ptr->m_value < value) {
        tmp_node = m_insertKey(ptr->m_right, value, depth + 1);
    }
    else if (ptr->m_value > value) {
        tmp_node = m_insertKey(ptr->m_left, value, depth + 1);
    }

    if (tmp_node == nullptr) {
        return nullptr;
    }
    if (tmp_node->m_parent == nullptr) {
        tmp_node->m_parent = ptr;
    }

    ++ptr->m_size;

    return tmp_node;
}

// rebuild unbalanced subtree (scapegoat)
void ScapegoatTree::m_rebuildSubtree(Node *& scapegoat)
{
    std::vector<int> values = m_values(scapegoat);
    Node * parent = scapegoat->m_parent;

    m_insertMiddle(values, (parent == nullptr ? m_root : parent->m_left == scapegoat ? parent->m_left
                                                                                     : parent->m_right),
                   parent,
                   0,
                   values.size());
    delete scapegoat;
}

void ScapegoatTree::m_insertMiddle(std::vector<int> & values, Node *& ptr, Node * parent, int beg, int end)
{
    int mid = beg + (end - beg) / 2;

    if (beg == end) {
        return;
    }

    ptr = new Node(values[mid]);

    ptr->m_size = end - beg;
    if (parent != nullptr) {
        ptr->m_depth = parent->m_depth + 1;
    }
    ptr->m_parent = parent;

    m_insertMiddle(values, ptr->m_right, ptr, mid + 1, end);
    m_insertMiddle(values, ptr->m_left, ptr, beg, mid);
}

bool ScapegoatTree::contains(int value) const
{
    return m_find(m_root, value) != nullptr;
}

// find node with key = 0 in subtree (start)
Node * ScapegoatTree::m_find(Node * start, int value)
{
    if (start == nullptr) {
        return nullptr;
    }

    if (value > start->m_value) {
        return m_find(start->m_right, value);
    }
    if (value < start->m_value) {
        return m_find(start->m_left, value);
    }
    return start;
}

// remove node with key = value
bool ScapegoatTree::remove(int value)
{
    if (m_removeKey(m_root, value)) {
        --m_size;
        return true;
    }
    return false;
}

// return free pointer of the least node
Node * ScapegoatTree::m_minNode(Node * ptr)
{
    while (ptr->m_left != nullptr) {
        ptr = ptr->m_left;
    }
    return ptr;
}

// all work with removing node with key = value in subtree (ptr)
bool ScapegoatTree::m_removeKey(Node * ptr, int value)
{
    if (ptr == nullptr) {
        return false;
    }
    bool is_removed;

    if (ptr->m_value < value) {
        is_removed = m_removeKey(ptr->m_right, value);
    }
    else if (ptr->m_value > value) {
        is_removed = m_removeKey(ptr->m_left, value);
    }
    else {
        return m_killKey(ptr);
    }

    if (is_removed) {
        --ptr->m_size;
    }
    return is_removed;
}

bool ScapegoatTree::m_killKey(Node * ptr)
{
    if (ptr->m_right != nullptr && ptr->m_left != nullptr) { // both children exist
        Node * tmp_node = m_minNode(ptr->m_right);

        m_killKeyParentInit(ptr->m_parent, ptr, ptr->m_right);
        ptr->m_left->m_parent = tmp_node;
        tmp_node->m_left = ptr->m_left;
    }
    else if (ptr->m_left != nullptr) { // and m_right == nullptr, only left exist
        m_killKeyParentInit(ptr->m_parent, ptr, ptr->m_left);
    }
    else if (ptr->m_right != nullptr) { // and m_left == nullptr, only right exist
        m_killKeyParentInit(ptr->m_parent, ptr, ptr->m_right);
    }
    else if (ptr->m_parent == nullptr) { // leaf of tree
        m_root = nullptr;
    }
    else if (ptr->m_parent->m_left == ptr) {
        ptr->m_parent->m_left = nullptr;
    }
    else {
        ptr->m_parent->m_right = nullptr;
    }

    // delete only one node
    ptr->m_left = nullptr;
    ptr->m_right = nullptr;
    delete ptr;
    return true;
}

// initialisation of new parents of child of deleted value
void ScapegoatTree::m_killKeyParentInit(Node *& parent, Node * child, Node *& child_ptr)
{
    if (parent == nullptr) {
        m_root = child_ptr;
        child_ptr->m_parent = nullptr;
        return;
    }

    if (parent->m_left == child) {
        parent->m_left = child_ptr;
    }
    else {
        parent->m_right = child_ptr;
    }
    child_ptr->m_parent = parent;
}

std::size_t ScapegoatTree::size() const
{
    return m_size;
}

bool ScapegoatTree::empty() const
{
    return m_root == nullptr;
}

std::vector<int> ScapegoatTree::values() const
{
    std::vector<int> array;
    ScapegoatTree::m_values(array, m_root);
    return array;
}

// create new vector and fill it with values of subtree (ptr)
std::vector<int> ScapegoatTree::m_values(Node * ptr)
{
    if (ptr == nullptr) {
        return {};
    }

    std::vector<int> ret_values;

    m_values(ret_values, ptr);
    return ret_values;
}

// fill vector arr with values of subtree (ptr)
void ScapegoatTree::m_values(std::vector<int> & arr, Node * ptr)
{
    if (ptr == nullptr) {
        return;
    }
    if (ptr->m_left != nullptr) {
        m_values(arr, ptr->m_left);
    }
    arr.push_back(ptr->m_value);
    if (ptr->m_right != nullptr) {
        m_values(arr, ptr->m_right);
    }
}

ScapegoatTree::~ScapegoatTree()
{
    delete m_root;
}
