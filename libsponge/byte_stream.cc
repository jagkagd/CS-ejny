#include "byte_stream.hh"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <stdexcept>

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity)
    : _stream(), _capacity(capacity), _end_input(false), _bytes_written(0), _bytes_read(0) {}

size_t ByteStream::write(const string &data) {
    size_t size = min(remaining_capacity(), data.length());
    for (size_t i = 0; i < size; i++) {
        _stream.push_back(data[i]);
    }
    _bytes_written += size;
    return size;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    stringstream out;
    size_t size = min(len, _stream.size());
    for (size_t i = 0; i < size; i++) {
        out << _stream.at(i);
    }
    return out.str();
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t size = min(len, _stream.size());
    for (size_t i = 0; i < size; i++) {
        _stream.pop_front();
    }
    _bytes_read += size;
    return;
}

void ByteStream::end_input() { _end_input = true; }

bool ByteStream::input_ended() const { return _end_input; }

size_t ByteStream::buffer_size() const { return _stream.size(); }

bool ByteStream::buffer_empty() const { return _stream.empty(); }

bool ByteStream::eof() const { return input_ended() && buffer_empty(); }

size_t ByteStream::bytes_written() const { return _bytes_written; }

size_t ByteStream::bytes_read() const { return _bytes_read; }

size_t ByteStream::remaining_capacity() const { return _capacity - _stream.size(); }
