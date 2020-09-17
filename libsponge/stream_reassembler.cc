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
    : _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    if(eof){
        _eof_index = index + data.length();
    }
    _eof |= eof;
    size_t beginIndex = index;
    size_t endIndex = index + data.length();
    pair<bool, size_t> beginIn = searchIndexInStreams(beginIndex);
    pair<bool, size_t> endIn = searchIndexInStreams(endIndex);
    if ((!beginIn.first) && (!endIn.first) && (beginIn.second == endIn.second)) {
        auto iter = _unused.cbegin();
        advance(iter, beginIn.second);
        _unused.emplace(iter, beginIndex, endIndex, std::move(data));
    } else if ((beginIn.first && endIn.first) && (beginIn.second == endIn.second)) {
    } else {
        streamMerge(data, beginIndex, endIndex, beginIn, endIn);
    }
    updateOutput();
    if(_eof && (_eof_index == _current_end)){
        _output.end_input();
    }
    return;
}

//! \details This function returns the position where index should be located, 
//! true if index in range [N1.begin, N1.end]
//! return 1 if in range (N0.end, N1.end]
pair<bool, size_t> StreamReassembler::searchIndexInStreams(size_t index) const {
    size_t cnt = 0;
    auto item = _unused.cbegin();
    for (; item != _unused.cend(); ++item) {
        if (index <= item->end) {
            break;
        }
        cnt++;
    }
    if (item->begin > index) {
        return make_pair(false, cnt);
    } else {
        return make_pair(true, cnt);
    }
}

void StreamReassembler::streamMerge(const string &data, size_t begin, size_t end, pair<bool, size_t> beginAt, pair<bool, size_t> endAt) {
    auto beginSegment = _unused.begin();
    advance(beginSegment, beginAt.second);
    auto endSegment = _unused.begin();
    advance(endSegment, endAt.first?endAt.second:(endAt.second-1));
    ByteStreamSegment newStreamSegment{begin, end, ""};
    int offset = begin - _current_end;
    if(beginAt.first){
        if(beginAt.second == 0){
            newStreamSegment.byteStream.append(beginSegment->byteStream, 0, (offset>0)?offset:0);
        }else{
            newStreamSegment.byteStream.append(beginSegment->byteStream, 0, begin-beginSegment->begin);
        }
        newStreamSegment.begin = beginSegment->begin;
    }
    if(beginAt.second == 0){
        newStreamSegment.byteStream.append(data, (-offset>0)?(-offset):0, data.length()-(-offset));
    }else{
        newStreamSegment.byteStream.append(data);
    }
    if(endAt.first){
        newStreamSegment.byteStream.append(endSegment->byteStream, end-endSegment->begin, endSegment->end-end);
        newStreamSegment.end = endSegment->end;
    }
    auto newBeginSegment = _unused.insert(beginSegment, newStreamSegment);
    _unused.erase(++newBeginSegment, ++endSegment);
    return;
}

void StreamReassembler::updateOutput() {
    auto item = _unused.begin();
    size_t end = item->end;
    if(end > _current_end) {
        size_t written = min<size_t>(_output.remaining_capacity(), item->byteStream.length());
        _output.write(item->byteStream);
        _current_end += written;
        item->byteStream.erase(0, written);
    }
    return;
}

size_t StreamReassembler::unassembled_bytes() const {
    size_t sum = 0;
    std::for_each(
        ++_unused.begin(), _unused.end(), [&](ByteStreamSegment stream) { sum += stream.end - stream.begin; });
    return sum;
}

bool StreamReassembler::empty() const {
    return _unused.size() == 1;
}
