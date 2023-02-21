#include <iosfwd>
#include <iostream>
#include <iterator>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

class Digraph
{
public:
    void push(unsigned _id, unsigned _hypernym_id);

    const std::vector<unsigned> & get(unsigned _id) const;

    bool is_in_graph(unsigned _id) const;

    friend std::ostream & operator<<(std::ostream & stream, const Digraph & digraph);

private:
    std::unordered_map<unsigned, std::vector<unsigned>> m_edges;
};

class ShortestCommonAncestor
{
    friend class WordNet;

private:
    const Digraph * m_graph;

    ShortestCommonAncestor() = default;

    explicit ShortestCommonAncestor(const Digraph & dg)
        : m_graph(&dg)
    {
    }

    std::pair<unsigned, unsigned> bfs(const std::set<unsigned> & subset1, const std::set<unsigned> & subset2) const;

    // calculates length of shortest common ancestor path from node with id 'v' to node with id 'w'
    unsigned length(unsigned v, unsigned w);

    // returns node id of shortest common ancestor of nodes v and w
    unsigned ancestor(unsigned v, unsigned w);

    // calculates length of shortest common ancestor path from node subset 'subset_a' to node subset 'subset_b'
    unsigned length_subset(const std::set<unsigned> & subset_a, const std::set<unsigned> & subset_b) const;

    // returns node id of shortest common ancestor of node subset 'subset_a' and node subset 'subset_b'
    unsigned ancestor_subset(const std::set<unsigned> & subset_a, const std::set<unsigned> & subset_b) const;
};

class WordNet
{
public:
    WordNet(std::istream & synsets, std::istream & hypernyms);

    class Nouns
    {
        friend class WordNet;

    public:
        class iterator
        {
            friend class Nouns;

        public:
            using iterator_category = std::forward_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = std::string;
            using pointer = const value_type *;
            using reference = const value_type &;

            iterator() = default;
            ~iterator() = default;

            reference operator*() const;

            pointer operator->() const;

            iterator & operator++();

            iterator operator++(int);

            friend bool operator==(const iterator & a, const iterator & b);

            friend bool operator!=(const iterator & a, const iterator & b);

        private:
            std::unordered_map<std::string, std::vector<unsigned>>::const_iterator m_iter;

            iterator(const Nouns * _cur_nouns, bool _is_first = false)
                : m_iter(_is_first ? _cur_nouns->m_wordmap.begin() : _cur_nouns->m_wordmap.end())
            {
            }
        };

        iterator begin() const
        {
            return iterator(this, true);
        }
        iterator end() const
        {
            return iterator(this);
        }

    private:
        const std::unordered_map<std::string, std::vector<unsigned>> & m_wordmap;
        Nouns(const WordNet & wordnet)
            : m_wordmap(wordnet.m_wordmap)
        {
        }
    };

    // lists all nouns stored in WordNet
    Nouns nouns() const;

    // returns 'true' if 'word' is stored in WordNet
    bool is_noun(const std::string & word) const;

    // returns gloss of "shortest common ancestor" of noun1 and noun2
    std::string sca(const std::string & noun1, const std::string & noun2) const;

    // calculates distance between noun1 and noun2
    unsigned distance(const std::string & noun1, const std::string & noun2) const;

private:
    std::unordered_map<unsigned, std::pair<std::vector<std::string>, std::string>> m_synset_map;
    std::unordered_map<std::string, std::vector<unsigned>> m_wordmap;
    Digraph m_graph;
    ShortestCommonAncestor m_commanc;

    std::set<unsigned> get_set_id(const std::string & noun) const;
};

class Outcast
{
    const WordNet & m_wordnet;

public:
    explicit Outcast(const WordNet & _wordnet)
        : m_wordnet(_wordnet)
    {
    }

    std::string outcast(const std::set<std::string> & nouns);
};
