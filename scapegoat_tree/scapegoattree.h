#pragma once

#include <vector>
namespace details {
struct Node
{
    int m_depth;
    int m_value;
    std::size_t m_size;

    Node * m_left;
    Node * m_right;
    Node * m_parent;

    Node() = default;
    Node(int value);

    ~Node();
};

} // namespace details

class ScapegoatTree
{
private:
    std::size_t m_size;
    double m_alpha;
    double m_log;

    details::Node * m_root;

    static int m_nodeSize(details::Node * ptr);

    details::Node * m_findScapegoat(details::Node * start) const;
    details::Node * m_insertKey(details::Node *& ptr, int value, int depth = 0);
    void m_rebuildSubtree(details::Node *& scapegoat);
    void m_insertMiddle(std::vector<int> & values, details::Node *& ptr, details::Node * parent, int beg, int end);

    static details::Node * m_find(details::Node * start, int value);

    details::Node * m_minNode(details::Node * ptr);
    bool m_removeKey(details::Node * ptr, int value);
    bool m_killKey(details::Node * ptr);
    void m_killKeyParentInit(details::Node *& parent, details::Node * child, details::Node *& child_ptr);

    static void m_values(std::vector<int> & arr, details::Node * ptr);
    static std::vector<int> m_values(details::Node * ptr);

public:
    ScapegoatTree();
    ScapegoatTree(double alpha);

    bool contains(int value) const;
    bool insert(int value);
    bool remove(int value);

    std::size_t size() const;
    bool empty() const;

    std::vector<int> values() const;

    ~ScapegoatTree();
};
