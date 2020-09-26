#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _retransmission_timeout(retx_timeout)
    , _timeout(retx_timeout)
    , _stream(capacity) {}

uint64_t TCPSender::bytes_in_flight() const {
    size_t cnt = 0;
    for (const auto seg : _outgoing_segs) {
        cnt += seg._seg.length_in_sequence_space();
    }
    return cnt;
}

void TCPSender::fill_window() {
    TCPSegment seg;
    seg.header().seqno = next_seqno();
    if (_next_seqno == 0) {
        seg.header().syn = true;
        _next_seqno++;
        _outgoing_segs.push_back(make_TCPSegmentInfo(seg));
        _segments_out.push(seg);
        return;
    }

    size_t window = _received_window_size;
    if (window == 0) {
        window = 1;
    }
    size_t avaliable_window = min(
        TCPConfig::MAX_PAYLOAD_SIZE, 
        _latest_abs_ackno + _received_window_size - _next_seqno
    );

    seg.payload() = Buffer(_stream.read(avaliable_window));
    _next_seqno += seg.payload().size();
    avaliable_window -= seg.payload().size();
    if (avaliable_window > 0 && _stream.eof() && !_fin_send) {
        seg.header().fin = true;
        _fin_send = true;
        _next_seqno++;
    }
    if (seg.length_in_sequence_space() == 0) {
        return;
    }
    _outgoing_segs.push_back(make_TCPSegmentInfo(seg));
    _segments_out.push(seg);
    if (!_timeout.has_value()) {
        _timeout = make_optional<unsigned int>(_retransmission_timeout);
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
//! \returns `false` if the ackno appears invalid (acknowledges something the TCPSender hasn't sent yet)
bool TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    size_t abs_ackno = unwrap(ackno, _isn, _next_seqno);
    if (abs_ackno > _next_seqno) {
        return false;
    }
    _received_window_size = window_size;
    if (abs_ackno <= _latest_abs_ackno) {
        return true;
    }

    _outgoing_segs.remove_if([&](auto item) { return item.abs_end <= abs_ackno; });

    // 7(a)
    _retransmission_timeout = _initial_retransmission_timeout;
    // 7(b)
    if (!_outgoing_segs.empty()) {  // TODO
        _timeout = make_optional<unsigned int>(_retransmission_timeout);
    } else {
        // 5
        _timeout = {};
    }
    // 7(c)
    _outgoing_num = 0;
    _latest_abs_ackno = abs_ackno;
    return true;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if (!_timeout.has_value()) {
        return;
    }
    _timeout.value() -= ms_since_last_tick;
    if (_timeout.value() > 0) {
        return;
    }
    // 6(a)
    _segments_out.push(_outgoing_segs.front()._seg);
    // _outgoing_segs.pop_front();
    // 6(b)
    if (_received_window_size > 0) {
        _outgoing_num++;
        _retransmission_timeout *= 2;
    }
    // 6(c)
    _timeout.value() = _retransmission_timeout;
}

unsigned int TCPSender::consecutive_retransmissions() const { return _outgoing_num; }

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = next_seqno();
    _segments_out.push(seg);
}

TCPSegmentInfo TCPSender::make_TCPSegmentInfo(TCPSegment &seg) { return TCPSegmentInfo(seg, _isn, _next_seqno); }