

namespace eger {

    template<class Term, class Document, class Position>
    class indexer {
        public:
            typedef std::vector<Position> position_list;
            struct term_description {
                Term term;
                position_list positions;
            };
            typedef std::vector<term_description> term_list;
            struct term_data {
                Document document;
                position_list positions;
            };
            typedef std::vector<Term> phrase;
            struct document_data {
                Document document;
                position_list positions;
            };
            typedef std::vector<document_data> document_list;
            struct index_element {
                Document document;
                position_list positions;
            };
            typedef std::deque<index_element> index;
            typedef std::unordered_map<Term, index> lexicon;

        public:

            // to index call index for all the terms in the document
            // terms can be precise, or fuzzy (common part of a word)
            // one can split term_list into parts and index every part separately
            bool index(term_list, Document);

            // return documents containing given phrase and positions
            document_list search(phrase);
    };

};
