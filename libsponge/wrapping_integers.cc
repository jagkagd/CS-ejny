#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    return WrappingInt32{uint32_t(n+isn.raw_value())};
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
// int33: [-2^32, ..., -2^31-1, -2^31, ..., -1, 0, ..., 2^31-1,  2^31, ..., 2^32-1]
// int32: [    0, ...   2^31-1, -2^31, ..., -1, 0, ..., 2^31-1, -2^31, ...,     -1]
// diff \in [-2^32+1, 2^32-1]
// (cp-2^31, cp+2^31]
// n >= wrap_cp && diff0 \in [0, 2^31) | [0, ... 2^31-1) return cp + diff
// n >= wrap_cp && diff0 = 2^31 | -2^31 return cp + 2^31
// n >= wrap_cp && diff0 \in (2^31, 2^32-1) | (-2^31, -1) && cp >= abs(diff) return cp - (2^32-diff0) = cp + diff
// n >= wrap_cp && diff0 \in (2^31, 2^32-1) | (-2^31, -1) && cp < abs(diff) return cp + diff0 = cp + diff + 2^32
// n <  wrap_cp && diff0 \in (-2^31, 0) | (-2^31, 0) && cp >= abs(diff) return cp + diff
// n <  wrap_cp && diff0 \in (-2^31, 0) | (-2^31, 0) && cp < abs(diff) return cp + 2^32 + diff
// n <  wrap_cp && diff0 \in (-2^32+1, -2^31) | (1, ..., 2^31-1) return cp + (diff0+2^32) = cp + diff
// n <  wrap_cp && diff0 = -2^31 | -2^31 return cp + 2^31
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    WrappingInt32 wrap_cp = wrap(checkpoint, isn);
    int32_t diff = n - wrap_cp;
    cout << "n: " << n << " wrap_cp: " << wrap_cp << " diff: " << diff << " cp: " << checkpoint << endl;
    if(diff == static_cast<int32_t>(1ul << 31)){
        return checkpoint + (1ul<<31);
    }
    if(diff < 0 && checkpoint < static_cast<uint64_t>(-diff)){
        return checkpoint + (1ul<<32) + diff;
    }
    return checkpoint + diff;
}
