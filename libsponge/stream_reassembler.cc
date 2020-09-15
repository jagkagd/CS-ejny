#include "stream_reassembler.hh"
#include <memory>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity), _capacity(capacity), _unused{ByteStreamSegment{0, 0, make_unique<ByteStream>(_output)}} {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    size_t beginIndex = index;
    size_t endIndex = index + data.length();
    pair<bool, size_t> beginIn = searchIndexInStreams(beginIndex);
    pair<bool, size_t> endIn = searchIndexInStreams(endIndex);
    if((!beginIn.first) && (!endIn.first) && (beginIn.second == endIn.second)) {
        auto iter = _unused.cbegin();
        advance(iter, beginIn.second + 1);
        _unused.emplace(iter, beginIn.second, endIn.second, std::move(data));
        return;
    } else if((beginIn.first && endIn.first) && (beginIn.second == endIn.second)) {
        return;
    } else {
        streamMerge(data, beginIndex, endIndex, beginIn.second, endIn.second);
        return;
    }
}

pair<bool, size_t> StreamReassembler::searchIndexInStreams(size_t index) const {
    bool inRange = false;
    size_t cnt = 0;
    auto item = _unused.cbegin();
    for(; item != _unused.cend(); ++item){
        if(item->end > index){
            break;
        }
    }
    if(item->begin >= )
}

size_t StreamReassembler::unassembled_bytes() const {
    size_t sum = 0;
    std::for_each(_unused.begin()++, _unused.end(), [&](ByteStreamSegment stream){
        sum += stream.end - stream.begin;
    });
    return sum;
}

bool StreamReassembler::empty() const { return {}; }
