#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;
using State = TCPState::State;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _time_since_last_segment_received; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    if(seg.header().rst){
        setRESET();
        return;
    }
    switch (getState()) {
        case State::CLOSED:
            return;
        case State::LISTEN:
            if (seg.header().syn && seg.payload().size() == 0) {
                _receiver.segment_received(seg);
                _sender.send_empty_segment();
                TCPSegment segSend = _sender.segments_out().front();
                _sender.segments_out().pop();
                segSend.header().syn = true;
                segSend.header().ack = true;
                segSend.header().ackno = _receiver.ackno().value();
                _segments_out.push(segSend);
                return;
            }
            return;
        case State::SYN_RCVD:
            if (seg.header().ack && seg.payload().size() == 0) {
                if (_sender.ack_received(seg.header().ackno, seg.header().win)) {
                    return;
                } else {  // TODO
                    _sender.stream_in().error();
                    return;
                }
            }
            // if (seg.header().rst) {  // to LISTEN TODO
            //     return;
            // }
            return;
        case State::ESTABLISHED:
            // normal data exchange
            if (seg.header().ack) {
                // bool ack_received = _sender.ack_received(seg.header().ackno, seg.header().win);
                // bool segment_received = _receiver.segment_received(seg);
                _sender.ack_received(seg.header().ackno, seg.header().win);
                _receiver.segment_received(seg);
                _sender.fill_window();  // TODO syn?
                TCPSegment segSend;
                while (!_sender.segments_out().empty()) {
                    segSend = _sender.segments_out().front();
                    _sender.segments_out().pop();
                    if (_receiver.ackno().has_value()) {
                        segSend.header().ack = true;
                        segSend.header().ackno = _receiver.ackno().value();
                        segSend.header().win = _receiver.window_size();
                    } else {  // TODO
                    }
                    segments_out().push(segSend);
                }
                return;  // TODO continue to test FIN?
            }
            // FIN/ACK
            if (seg.header().fin) {
                _receiver.segment_received(seg);
                _sender.send_empty_segment();
                TCPSegment segSend = _sender.segments_out().front();
                _sender.segments_out().pop();
                segSend.header().ack = true;
                segSend.header().ackno = _receiver.ackno().value();
                segments_out().push(segSend);
                return;
            }
            return;
        case State::CLOSE_WAIT:
            return;
        case State::SYN_SENT:
            if (seg.header().syn && seg.header().ack && seg.payload().size() == 0) {
                if(_sender.ack_received(seg.header().ackno, seg.header().win)){
                    // bool segment_received = _receiver.segment_received(seg);
                    _receiver.segment_received(seg);
                    _sender.send_empty_segment();
                    TCPSegment segSend = _sender.segments_out().front();
                    _sender.segments_out().pop();
                    segSend.header().ack = true;
                    segSend.header().ackno = _receiver.ackno().value();
                    segments_out().push(segSend);
                    return;
                }
            }
            if (seg.header().syn && !seg.header().ack) {
                _receiver.segment_received(seg);
                _sender.send_empty_segment();
                TCPSegment segSend = _sender.segments_out().front();
                _sender.segments_out().pop();
                segSend.header().syn = true;
                segSend.header().ack = true;
                segSend.header().ackno = _receiver.ackno().value();
                _segments_out.push(segSend);
                return;
            }
            return;
        case State::FIN_WAIT_1:
            if (seg.header().fin) {
                _receiver.segment_received(seg);
                if(seg.header().ack){
                    _sender.ack_received(seg.header().ackno, seg.header().win);
                }
                _sender.send_empty_segment();
                TCPSegment segSend = _sender.segments_out().front();
                _sender.segments_out().pop();
                segSend.header().ack = true;
                segSend.header().ackno = _receiver.ackno().value();
                segments_out().push(segSend);
                return;
            }
            return;
        case State::FIN_WAIT_2:
            if (seg.header().fin && !seg.header().ack) {
                _sender.send_empty_segment();
                TCPSegment segSend = _sender.segments_out().front();
                _sender.segments_out().pop();
                segSend.header().ack = true;
                segSend.header().ackno = _receiver.ackno().value();
                segments_out().push(segSend);
                return;
            }
            return;
        case State::LAST_ACK:
            if (seg.header().ack && !seg.header().fin && seg.payload().size() == 0) {
                // bool ack_received = _sender.ack_received(seg.header().ackno, seg.header().win);
                _sender.ack_received(seg.header().ackno, seg.header().win);
            }
            _active = false;
            return;
        case State::CLOSING:
            if (seg.header().ack && !seg.header().fin && seg.payload().size() == 0) {
                // bool ack_received = _sender.ack_received(seg.header().ackno, seg.header().win);
                _sender.ack_received(seg.header().ackno, seg.header().win);
            }
            return;
        default:
            return;
    }
}

bool TCPConnection::active() const { return _active; }

size_t TCPConnection::write(const string &data) {
    DUMMY_CODE(data);
    return {};
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    if (!_active) {
        return;
    }
    _time_since_last_segment_received += ms_since_last_tick;
    switch(getState()){
        case State::TIME_WAIT:
            if(_time_since_last_segment_received >= 10 * _cfg.rt_timeout){
                _linger_after_streams_finish = false;
                _active = false;
            }
            return;
        case State::LISTEN:
        case State::SYN_SENT:
        case State::SYN_RCVD:
        case State::ESTABLISHED:
        case State::FIN_WAIT_1:
        case State::FIN_WAIT_2:
        case State::CLOSING:
        case State::LAST_ACK:
            _sender.tick(ms_since_last_tick);
            return;
        default:
            return;
    }
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    switch(getState()){
        case State::SYN_RCVD:
        case State::ESTABLISHED:
        case State::CLOSE_WAIT:
            {
                _sender.fill_window();
                TCPSegment segSend = _sender.segments_out().front();
                segSend.header().ack = true;
                segSend.header().ackno = _receiver.ackno().value();
                _sender.segments_out().pop();
                segments_out().push(segSend);
                return;
            }
        case State::LISTEN:
        case State::SYN_SENT:
            _linger_after_streams_finish = false;
            _active = false;
            return;
        default:
            break;
    }
    TCPSegment segSend;
    while (!_sender.segments_out().empty()) {
        segSend = _sender.segments_out().front();
        _sender.segments_out().pop();
        if (_receiver.ackno().has_value()) {
            segSend.header().ack = true;
            segSend.header().ackno = _receiver.ackno().value();
            segSend.header().win = _receiver.window_size();
        } else {  // TODO
        }
        segments_out().push(segSend);
    }
}

void TCPConnection::connect() {
    TCPSegment seg;
    switch (getState()) {
        case State::CLOSED:
            _active = true;
            _sender.fill_window();
            segments_out().push(_sender.segments_out().front());
            _sender.segments_out().pop();
        default:
            return;
    }
    return;
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
            TCPSegment seg;
            seg.header().rst = true;
            segments_out().push(seg);
            setRESET();
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}

void TCPConnection::setRESET() {
    _receiver.stream_out().set_error();
    _sender.stream_in().set_error();
    _linger_after_streams_finish = false;
    _active = false;
}