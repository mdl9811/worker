// Copyright (c) 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef QUICHE_QUIC_CORE_FRAMES_QUIC_RST_STREAM_FRAME_H_
#define QUICHE_QUIC_CORE_FRAMES_QUIC_RST_STREAM_FRAME_H_

#include <ostream>

#include "quiche/quic/core/quic_constants.h"
#include "quiche/quic/core/quic_error_codes.h"
#include "quiche/quic/core/quic_types.h"

namespace quic {

struct QUICHE_EXPORT QuicRstStreamFrame {
  QuicRstStreamFrame() = default;
  QuicRstStreamFrame(QuicControlFrameId control_frame_id,
                     QuicStreamId stream_id, QuicRstStreamErrorCode error_code,
                     QuicStreamOffset bytes_written);
  QuicRstStreamFrame(QuicControlFrameId control_frame_id,
                     QuicStreamId stream_id, QuicResetStreamError error,
                     QuicStreamOffset bytes_written);

  friend QUICHE_EXPORT std::ostream& operator<<(std::ostream& os,
                                                const QuicRstStreamFrame& r);

  bool operator==(const QuicRstStreamFrame& rhs) const;
  bool operator!=(const QuicRstStreamFrame& rhs) const;

  // A unique identifier of this control frame. 0 when this frame is received,
  // and non-zero when sent.
  QuicControlFrameId control_frame_id = kInvalidControlFrameId;

  QuicStreamId stream_id = 0;

  // When using Google QUIC: the RST_STREAM error code on the wire.
  // When using IETF QUIC: for an outgoing RESET_STREAM frame, the error code
  // generated by the application that determines |ietf_error_code| to be sent
  // on the wire; for an incoming RESET_STREAM frame, the error code inferred
  // from the |ietf_error_code| received on the wire.
  QuicRstStreamErrorCode error_code = QUIC_STREAM_NO_ERROR;

  // Application error code of RESET_STREAM frame.  Used for IETF QUIC only.
  uint64_t ietf_error_code = 0;

  // Used to update flow control windows. On termination of a stream, both
  // endpoints must inform the peer of the number of bytes they have sent on
  // that stream. This can be done through normal termination (data packet with
  // FIN) or through a RST.
  QuicStreamOffset byte_offset = 0;

  // Returns a tuple of both |error_code| and |ietf_error_code|.
  QuicResetStreamError error() const {
    return QuicResetStreamError(error_code, ietf_error_code);
  }
};

}  // namespace quic

#endif  // QUICHE_QUIC_CORE_FRAMES_QUIC_RST_STREAM_FRAME_H_
