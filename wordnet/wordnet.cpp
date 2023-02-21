#include "wordnet.h"

// Wordnet section
WordNet::WordNet(std::istream & synsets, std::istream & hypernyms)
{
    Digraph dgrth;
    std::string line;
    while (std::getline(synsets, line)) {
        std::string token;
        std::vector<std::string> synonyms;
        std::stringstream line_stream(line);
        // id
        std::getline(line_stream, token, ',');
        if (token.empty()) {
            continue;
        }
        const unsigned id = std::stoul(token);
        // synonyms
        std::getline(line_stream, token, ',');
        std::stringstream synonyms_stream(token);
        std::string token1;
        while (std::getline(synonyms_stream, token1, ' ')) {
            synonyms.push_back(token1);
            if (m_wordmap.find(token1) == m_wordmap.end()) {
                m_wordmap.insert({token1, {id}});
            }
            else {
                m_wordmap.at(token1).push_back(id);
            }
        }
        // gloss
        std::getline(line_stream, token, ',');

        std::pair<std::vector<std::string>, std::string> tmp_pair;
        tmp_pair.first = synonyms;
        tmp_pair.second = token;
        m_synset_map.emplace(id, std::move(tmp_pair));
    }
    while (std::getline(hypernyms, line)) {
        std::string token;
        unsigned id;
        std::vector<unsigned> hypernym_list;
        std::stringstream line_stream(line);
        // id
        std::getline(line_stream, token, ',');
        if (token.empty()) {
            continue;
        }
        id = std::stoul(token);
        // hypernyms
        while (std::getline(line_stream, token, ',')) {
            unsigned hypernym = std::stoul(token);
            dgrth.push(id, hypernym);
            hypernym_list.push_back(hypernym);
        }
    }
    m_graph = std::move(dgrth);
    m_commanc = ShortestCommonAncestor(m_graph); // NOLINT
}

WordNet::Nouns WordNet::nouns() const
{
    return Nouns(*this);
}

bool WordNet::is_noun(const std::string & word) const
{
    return m_wordmap.find(word) != m_wordmap.end();
}

std::string WordNet::sca(const std::string & noun1, const std::string & noun2) const
{
    return m_synset_map.at(m_commanc.ancestor_subset(get_set_id(noun1), get_set_id(noun2))).second;
}

unsigned WordNet::distance(const std::string & noun1, const std::string & noun2) const
{
    return m_commanc.length_subset(get_set_id(noun1), get_set_id(noun2));
}

std::set<unsigned> WordNet::get_set_id(const std::string & noun) const
{
    std::vector<unsigned> vec = m_wordmap.at(noun);
    return {vec.begin(), vec.end()};
}

// Iterator in wn section
const WordNet::Nouns::iterator::value_type & WordNet::Nouns::iterator::operator*() const { return m_iter->first; }

WordNet::Nouns::iterator::pointer WordNet::Nouns::iterator::operator->() const { return &m_iter->first; }

bool operator==(const WordNet::Nouns::iterator & a, const WordNet::Nouns::iterator & b)
{
    return a.m_iter == b.m_iter;
}

bool operator!=(const WordNet::Nouns::iterator & a, const WordNet::Nouns::iterator & b)
{
    return !(a == b);
}

WordNet::Nouns::iterator WordNet::Nouns::iterator::operator++(int)
{
    auto tmp_to_ret = *this;
    operator++();
    return tmp_to_ret;
}

WordNet::Nouns::iterator & WordNet::Nouns::iterator::operator++()
{
    ++m_iter;
    return *this;
}

// Digraph section
void Digraph::push(unsigned int _id, unsigned int _hypernym_id)
{
    const auto [it, inserted] = m_edges.emplace(_id, 0);
    it->second.push_back(_hypernym_id);
}

const std::vector<unsigned> & Digraph::get(unsigned int _id) const
{
    return m_edges.at(_id);
}

bool Digraph::is_in_graph(unsigned int _id) const
{
    return m_edges.find(_id) != m_edges.end();
}

std::ostream & operator<<(std::ostream & stream, const Digraph & digraph)
{
    for (const auto & [id, edges] : digraph.m_edges) {
        stream << id << " -->";
        for (const auto v_id : edges) {
            stream << " " << v_id;
        }
        stream << '\n';
    }
    return stream;
}

// ShortestCommonAncestor section
std::pair<unsigned, unsigned> ShortestCommonAncestor::bfs(const std::set<unsigned int> & subset1, const std::set<unsigned int> & subset2) const
{
    std::unordered_map<unsigned, std::pair<bool, unsigned>> info;
    std::vector<unsigned> ids;
    for (const auto id : subset1) {
        info[id] = {true, 0};
        ids.push_back(id);
    }
    for (const auto id : subset2) {
        if (info.find(id) != info.end()) {
            return {id, 0};
        }
        info[id] = {false, 0};
        ids.push_back(id);
    }
    std::pair<unsigned, unsigned> min_ancestor(static_cast<unsigned>(-1), static_cast<unsigned>(-1));
    for (std::size_t i = 0; i < ids.size(); ++i) {
        unsigned id = ids[i];
        if (!m_graph->is_in_graph(id)) {
            continue;
        }
        bool is_first_noun = info.at(id).first;
        for (const auto id_h : m_graph->get(id)) {
            auto [iter, inserted] = info.try_emplace(id_h);
            if (inserted) {
                ids.push_back(id_h);
                iter->second = {is_first_noun, info.at(id).second + 1};
                continue;
            }
            if (iter->second.first != is_first_noun) {
                unsigned sum = iter->second.second + info.at(id).second + 1; //
                if (sum < min_ancestor.second) {
                    min_ancestor = {id_h, sum};
                }
            }
        }
    }

    return min_ancestor;
}
unsigned ShortestCommonAncestor::ancestor_subset(const std::set<unsigned int> & subset_a, const std::set<unsigned int> & subset_b) const
{
    return bfs(subset_a, subset_b).first;
}

unsigned ShortestCommonAncestor::length_subset(const std::set<unsigned int> & subset_a, const std::set<unsigned int> & subset_b) const
{
    return bfs(subset_a, subset_b).second;
}

unsigned ShortestCommonAncestor::ancestor(unsigned int v, unsigned int w)
{
    return bfs({v}, {w}).first;
}

unsigned ShortestCommonAncestor::length(unsigned int v, unsigned int w)
{
    return bfs({v}, {w}).second;
}

// Outcast section
std::string Outcast::outcast(const std::set<std::string> & nouns)
{
    std::vector<unsigned> sums(nouns.size(), 0);
    std::vector<std::vector<unsigned>> results(nouns.size());
    unsigned cnt1 = 0;
    for (auto iter1 = nouns.begin(); iter1 != nouns.end(); ++iter1, ++cnt1) {
        unsigned cnt2 = 0;
        for (auto iter2 = nouns.begin(); iter2 != iter1; ++iter2, ++cnt2) {
            auto distance = m_wordnet.distance(*iter1, *iter2);
            sums[cnt1] += distance;
            sums[cnt2] += distance;
        }
    }
    bool is_repeated = true;
    auto max_iter = nouns.begin();
    unsigned max_sum = 0;
    unsigned ind = 0;
    for (auto iter = nouns.begin(); iter != nouns.end(); ++iter, ++ind) {
        if (max_sum < sums[ind]) {
            max_sum = sums[ind];
            max_iter = iter;
            is_repeated = false;
        }
        else if (max_sum == sums[ind]) {
            is_repeated = true;
        }
    }
    return is_repeated ? "" : *max_iter;
}