#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

bool TCPReceiver::segment_received(const TCPSegment &seg) {
    size_t length = seg.length_in_sequence_space();
    size_t seq_begin_offset = 0;
    if(seg.header().syn){
        --length;
        ++seq_begin_offset;
        if(!_isn.has_value()){
            _isn = make_optional<WrappingInt32>(seg.header().seqno.raw_value()+seq_begin_offset);
        }else{
            return false;
        }
    }
    if(!_isn.has_value()){
        return false;
    }
    if(seg.header().fin && _fin_set){
        return false;
    }
    _fin_set |= seg.header().fin;
    uint64_t abs_seqno = unwrap(WrappingInt32(seg.header().seqno.raw_value()+seq_begin_offset), _isn.value(), _reassembler.getLastSeq());
    if(seg.header().fin){
        --length;
    }
    uint64_t window_begin = _reassembler.getLastSeq();
    uint64_t window_end = window_begin + window_size();
    if(window_end == window_begin){
        window_end++;
    }
    _reassembler.push_substring(seg.payload().copy(), abs_seqno, seg.header().fin);
    uint64_t seg_begin = abs_seqno;
    uint64_t seg_end = seg_begin + length;
    if(length == 0){
        seg_end++;
    }
    bool seg_left = seg_end <= window_begin ;
    bool seg_right = seg_begin >= window_end;
    return !(seg_left || seg_right);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if(_isn.has_value()){
        return wrap(_reassembler.getLastSeq() + static_cast<uint64_t>(_reassembler.stream_out().input_ended()), _isn.value());
    }else{
        return nullopt;
    }
}

size_t TCPReceiver::window_size() const { return _reassembler.stream_out().remaining_capacity(); }
