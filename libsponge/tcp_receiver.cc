#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

bool TCPReceiver::segment_received(const TCPSegment &seg) {
    size_t length = seg.length_in_sequence_space();
    uint64_t abs_seqno;
    if(seg.header().syn && !_isn.has_value()){
        _isn = make_optional<WrappingInt32>(seg.header().seqno.raw_value());
        length--;
    if(!_isn.has_value()){
        return false;
    }
    abs_seqno = unwrap(WrappingInt32(seg.header().seqno.raw_value()), _isn, _reassembler.getLastSeq());
    if(seg.header().fin){
        length--;
    }
    uint64_t window_begin = _reassembler.getLastSeq() + 1;
    _reassembler.push_substring(seg.payload().copy(), abs_seqno, seg.header().fin);
    uint64_t window_end = window_begin + window_size();
    uint64_t seg_begin = abs_seqno;
    uint64_t seg_end = seg_begin + length;
    bool seg_begin_in = seg_begin >= window_begin && seg_begin < window_end;
    bool seg_end_in = seg_end >= window_begin && seg_end < window_end;
    return seg_begin_in || seg_end_in;
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if(_isn.has_value()){
        return wrap(_reassembler.getLastSeq()+1, _isn);
    }else{
        return nullopt;
    }
}

size_t TCPReceiver::window_size() const { return _reassembler.stream_out().remaining_capacity(); }
