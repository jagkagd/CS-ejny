#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _time_since_last_segment_received; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    switch (getState()) {
        case TCPState::State::CLOSED:
            return;
        case TCPState::State::LISTEN:
            if (seg.header().syn && seg.payload().size() == 0) {
                _receiver.segment_received(seg);
                TCPSegment segSend;
                segSend.header().syn = true;
                segSend.header().seqno = _sender.next_seqno();
                segSend.header().ack = true;
                segSend.header().ackno = _receiver.ackno().value();
                _segments_out.push(segSend);
                return;
            } else {
                return;  // TODO
            }
        case TCPState::State::SYN_RCVD:
            if (seg.header().ack && seg.payload().size() == 0) {
                if (_sender.ack_received(seg.header().ackno, seg.header().win)) {
                    return;
                } else {  // TODO
                    _sender.stream_in().error();
                    return;
                }
            } else {  // TODO
                _sender.stream_in().error();
                return;
            }
        case TCPState::State::ESTABLISHED:
            // normal data exchange
            bool ack_received;
            if (seg.header().ack) {
                ack_received = _sender.ack_received(seg.header().ackno, seg.header().win);
            }
            bool segment_received = _receiver.segment_received(seg);
            _sender.fill_window();  // TODO syn?
            while (!_sender.segments_out().empty()) {
                TCPSegment segSend = _sender.segments_out().front();
                _sender.segments_out().pop();
                if(_receiver.ackno().has_value()){
                    segSend.header().ack = true;
                    segSend.header().ackno = _receiver.ackno().value();
                    segSend.header().win = _receiver.window_size();
                }else{ // TODO
                }
                segments_out().push(segSend);
            }
            return;
            // FIN/ACK

        case TCPState::State::CLOSE_WAIT
    }
}

bool TCPConnection::active() const { return {}; }

size_t TCPConnection::write(const string &data) {
    DUMMY_CODE(data);
    return {};
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) { DUMMY_CODE(ms_since_last_tick); }

void TCPConnection::end_input_stream() {}

void TCPConnection::connect() {}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
