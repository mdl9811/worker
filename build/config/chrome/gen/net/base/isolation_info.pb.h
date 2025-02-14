// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: isolation_info.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_isolation_5finfo_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_isolation_5finfo_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3021000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3021012 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/message_lite.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_isolation_5finfo_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_isolation_5finfo_2eproto {
  static const uint32_t offsets[];
};
namespace net {
namespace proto {
class IsolationInfo;
struct IsolationInfoDefaultTypeInternal;
extern IsolationInfoDefaultTypeInternal _IsolationInfo_default_instance_;
}  // namespace proto
}  // namespace net
PROTOBUF_NAMESPACE_OPEN
template<> ::net::proto::IsolationInfo* Arena::CreateMaybeMessage<::net::proto::IsolationInfo>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace net {
namespace proto {

// ===================================================================

class IsolationInfo final :
    public ::PROTOBUF_NAMESPACE_ID::MessageLite /* @@protoc_insertion_point(class_definition:net.proto.IsolationInfo) */ {
 public:
  inline IsolationInfo() : IsolationInfo(nullptr) {}
  ~IsolationInfo() override;
  explicit PROTOBUF_CONSTEXPR IsolationInfo(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  IsolationInfo(const IsolationInfo& from);
  IsolationInfo(IsolationInfo&& from) noexcept
    : IsolationInfo() {
    *this = ::std::move(from);
  }

  inline IsolationInfo& operator=(const IsolationInfo& from) {
    CopyFrom(from);
    return *this;
  }
  inline IsolationInfo& operator=(IsolationInfo&& from) noexcept {
    if (this == &from) return *this;
    if (GetOwningArena() == from.GetOwningArena()
  #ifdef PROTOBUF_FORCE_COPY_IN_MOVE
        && GetOwningArena() != nullptr
  #endif  // !PROTOBUF_FORCE_COPY_IN_MOVE
    ) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  inline const std::string& unknown_fields() const {
    return _internal_metadata_.unknown_fields<std::string>(::PROTOBUF_NAMESPACE_ID::internal::GetEmptyString);
  }
  inline std::string* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields<std::string>();
  }

  static const IsolationInfo& default_instance() {
    return *internal_default_instance();
  }
  static inline const IsolationInfo* internal_default_instance() {
    return reinterpret_cast<const IsolationInfo*>(
               &_IsolationInfo_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(IsolationInfo& a, IsolationInfo& b) {
    a.Swap(&b);
  }
  PROTOBUF_NOINLINE void Swap(IsolationInfo* other) {
    if (other == this) return;
  #ifdef PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() != nullptr &&
        GetOwningArena() == other->GetOwningArena()) {
   #else  // PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() == other->GetOwningArena()) {
  #endif  // !PROTOBUF_FORCE_COPY_IN_SWAP
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(IsolationInfo* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  IsolationInfo* New(::PROTOBUF_NAMESPACE_ID::Arena* arena = nullptr) const final {
    return CreateMaybeMessage<IsolationInfo>(arena);
  }
  void CheckTypeAndMergeFrom(const ::PROTOBUF_NAMESPACE_ID::MessageLite& from)  final;
  void CopyFrom(const IsolationInfo& from);
  void MergeFrom(const IsolationInfo& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  uint8_t* _InternalSerialize(
      uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _impl_._cached_size_.Get(); }

  private:
  void SharedCtor(::PROTOBUF_NAMESPACE_ID::Arena* arena, bool is_message_owned);
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(IsolationInfo* other);

  private:
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "net.proto.IsolationInfo";
  }
  protected:
  explicit IsolationInfo(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  public:

  std::string GetTypeName() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kTopFrameOriginFieldNumber = 2,
    kFrameOriginFieldNumber = 3,
    kSiteForCookiesFieldNumber = 4,
    kRequestTypeFieldNumber = 1,
  };
  // optional string top_frame_origin = 2;
  bool has_top_frame_origin() const;
  private:
  bool _internal_has_top_frame_origin() const;
  public:
  void clear_top_frame_origin();
  const std::string& top_frame_origin() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_top_frame_origin(ArgT0&& arg0, ArgT... args);
  std::string* mutable_top_frame_origin();
  PROTOBUF_NODISCARD std::string* release_top_frame_origin();
  void set_allocated_top_frame_origin(std::string* top_frame_origin);
  private:
  const std::string& _internal_top_frame_origin() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_top_frame_origin(const std::string& value);
  std::string* _internal_mutable_top_frame_origin();
  public:

  // optional string frame_origin = 3;
  bool has_frame_origin() const;
  private:
  bool _internal_has_frame_origin() const;
  public:
  void clear_frame_origin();
  const std::string& frame_origin() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_frame_origin(ArgT0&& arg0, ArgT... args);
  std::string* mutable_frame_origin();
  PROTOBUF_NODISCARD std::string* release_frame_origin();
  void set_allocated_frame_origin(std::string* frame_origin);
  private:
  const std::string& _internal_frame_origin() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_frame_origin(const std::string& value);
  std::string* _internal_mutable_frame_origin();
  public:

  // optional string site_for_cookies = 4;
  bool has_site_for_cookies() const;
  private:
  bool _internal_has_site_for_cookies() const;
  public:
  void clear_site_for_cookies();
  const std::string& site_for_cookies() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_site_for_cookies(ArgT0&& arg0, ArgT... args);
  std::string* mutable_site_for_cookies();
  PROTOBUF_NODISCARD std::string* release_site_for_cookies();
  void set_allocated_site_for_cookies(std::string* site_for_cookies);
  private:
  const std::string& _internal_site_for_cookies() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_site_for_cookies(const std::string& value);
  std::string* _internal_mutable_site_for_cookies();
  public:

  // optional int32 request_type = 1;
  bool has_request_type() const;
  private:
  bool _internal_has_request_type() const;
  public:
  void clear_request_type();
  int32_t request_type() const;
  void set_request_type(int32_t value);
  private:
  int32_t _internal_request_type() const;
  void _internal_set_request_type(int32_t value);
  public:

  // @@protoc_insertion_point(class_scope:net.proto.IsolationInfo)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  struct Impl_ {
    ::PROTOBUF_NAMESPACE_ID::internal::HasBits<1> _has_bits_;
    mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
    ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr top_frame_origin_;
    ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr frame_origin_;
    ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr site_for_cookies_;
    int32_t request_type_;
  };
  union { Impl_ _impl_; };
  friend struct ::TableStruct_isolation_5finfo_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// IsolationInfo

// optional int32 request_type = 1;
inline bool IsolationInfo::_internal_has_request_type() const {
  bool value = (_impl_._has_bits_[0] & 0x00000008u) != 0;
  return value;
}
inline bool IsolationInfo::has_request_type() const {
  return _internal_has_request_type();
}
inline void IsolationInfo::clear_request_type() {
  _impl_.request_type_ = 0;
  _impl_._has_bits_[0] &= ~0x00000008u;
}
inline int32_t IsolationInfo::_internal_request_type() const {
  return _impl_.request_type_;
}
inline int32_t IsolationInfo::request_type() const {
  // @@protoc_insertion_point(field_get:net.proto.IsolationInfo.request_type)
  return _internal_request_type();
}
inline void IsolationInfo::_internal_set_request_type(int32_t value) {
  _impl_._has_bits_[0] |= 0x00000008u;
  _impl_.request_type_ = value;
}
inline void IsolationInfo::set_request_type(int32_t value) {
  _internal_set_request_type(value);
  // @@protoc_insertion_point(field_set:net.proto.IsolationInfo.request_type)
}

// optional string top_frame_origin = 2;
inline bool IsolationInfo::_internal_has_top_frame_origin() const {
  bool value = (_impl_._has_bits_[0] & 0x00000001u) != 0;
  return value;
}
inline bool IsolationInfo::has_top_frame_origin() const {
  return _internal_has_top_frame_origin();
}
inline void IsolationInfo::clear_top_frame_origin() {
  _impl_.top_frame_origin_.ClearToEmpty();
  _impl_._has_bits_[0] &= ~0x00000001u;
}
inline const std::string& IsolationInfo::top_frame_origin() const {
  // @@protoc_insertion_point(field_get:net.proto.IsolationInfo.top_frame_origin)
  return _internal_top_frame_origin();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void IsolationInfo::set_top_frame_origin(ArgT0&& arg0, ArgT... args) {
 _impl_._has_bits_[0] |= 0x00000001u;
 _impl_.top_frame_origin_.Set(static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:net.proto.IsolationInfo.top_frame_origin)
}
inline std::string* IsolationInfo::mutable_top_frame_origin() {
  std::string* _s = _internal_mutable_top_frame_origin();
  // @@protoc_insertion_point(field_mutable:net.proto.IsolationInfo.top_frame_origin)
  return _s;
}
inline const std::string& IsolationInfo::_internal_top_frame_origin() const {
  return _impl_.top_frame_origin_.Get();
}
inline void IsolationInfo::_internal_set_top_frame_origin(const std::string& value) {
  _impl_._has_bits_[0] |= 0x00000001u;
  _impl_.top_frame_origin_.Set(value, GetArenaForAllocation());
}
inline std::string* IsolationInfo::_internal_mutable_top_frame_origin() {
  _impl_._has_bits_[0] |= 0x00000001u;
  return _impl_.top_frame_origin_.Mutable(GetArenaForAllocation());
}
inline std::string* IsolationInfo::release_top_frame_origin() {
  // @@protoc_insertion_point(field_release:net.proto.IsolationInfo.top_frame_origin)
  if (!_internal_has_top_frame_origin()) {
    return nullptr;
  }
  _impl_._has_bits_[0] &= ~0x00000001u;
  auto* p = _impl_.top_frame_origin_.Release();
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (_impl_.top_frame_origin_.IsDefault()) {
    _impl_.top_frame_origin_.Set("", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  return p;
}
inline void IsolationInfo::set_allocated_top_frame_origin(std::string* top_frame_origin) {
  if (top_frame_origin != nullptr) {
    _impl_._has_bits_[0] |= 0x00000001u;
  } else {
    _impl_._has_bits_[0] &= ~0x00000001u;
  }
  _impl_.top_frame_origin_.SetAllocated(top_frame_origin, GetArenaForAllocation());
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (_impl_.top_frame_origin_.IsDefault()) {
    _impl_.top_frame_origin_.Set("", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  // @@protoc_insertion_point(field_set_allocated:net.proto.IsolationInfo.top_frame_origin)
}

// optional string frame_origin = 3;
inline bool IsolationInfo::_internal_has_frame_origin() const {
  bool value = (_impl_._has_bits_[0] & 0x00000002u) != 0;
  return value;
}
inline bool IsolationInfo::has_frame_origin() const {
  return _internal_has_frame_origin();
}
inline void IsolationInfo::clear_frame_origin() {
  _impl_.frame_origin_.ClearToEmpty();
  _impl_._has_bits_[0] &= ~0x00000002u;
}
inline const std::string& IsolationInfo::frame_origin() const {
  // @@protoc_insertion_point(field_get:net.proto.IsolationInfo.frame_origin)
  return _internal_frame_origin();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void IsolationInfo::set_frame_origin(ArgT0&& arg0, ArgT... args) {
 _impl_._has_bits_[0] |= 0x00000002u;
 _impl_.frame_origin_.Set(static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:net.proto.IsolationInfo.frame_origin)
}
inline std::string* IsolationInfo::mutable_frame_origin() {
  std::string* _s = _internal_mutable_frame_origin();
  // @@protoc_insertion_point(field_mutable:net.proto.IsolationInfo.frame_origin)
  return _s;
}
inline const std::string& IsolationInfo::_internal_frame_origin() const {
  return _impl_.frame_origin_.Get();
}
inline void IsolationInfo::_internal_set_frame_origin(const std::string& value) {
  _impl_._has_bits_[0] |= 0x00000002u;
  _impl_.frame_origin_.Set(value, GetArenaForAllocation());
}
inline std::string* IsolationInfo::_internal_mutable_frame_origin() {
  _impl_._has_bits_[0] |= 0x00000002u;
  return _impl_.frame_origin_.Mutable(GetArenaForAllocation());
}
inline std::string* IsolationInfo::release_frame_origin() {
  // @@protoc_insertion_point(field_release:net.proto.IsolationInfo.frame_origin)
  if (!_internal_has_frame_origin()) {
    return nullptr;
  }
  _impl_._has_bits_[0] &= ~0x00000002u;
  auto* p = _impl_.frame_origin_.Release();
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (_impl_.frame_origin_.IsDefault()) {
    _impl_.frame_origin_.Set("", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  return p;
}
inline void IsolationInfo::set_allocated_frame_origin(std::string* frame_origin) {
  if (frame_origin != nullptr) {
    _impl_._has_bits_[0] |= 0x00000002u;
  } else {
    _impl_._has_bits_[0] &= ~0x00000002u;
  }
  _impl_.frame_origin_.SetAllocated(frame_origin, GetArenaForAllocation());
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (_impl_.frame_origin_.IsDefault()) {
    _impl_.frame_origin_.Set("", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  // @@protoc_insertion_point(field_set_allocated:net.proto.IsolationInfo.frame_origin)
}

// optional string site_for_cookies = 4;
inline bool IsolationInfo::_internal_has_site_for_cookies() const {
  bool value = (_impl_._has_bits_[0] & 0x00000004u) != 0;
  return value;
}
inline bool IsolationInfo::has_site_for_cookies() const {
  return _internal_has_site_for_cookies();
}
inline void IsolationInfo::clear_site_for_cookies() {
  _impl_.site_for_cookies_.ClearToEmpty();
  _impl_._has_bits_[0] &= ~0x00000004u;
}
inline const std::string& IsolationInfo::site_for_cookies() const {
  // @@protoc_insertion_point(field_get:net.proto.IsolationInfo.site_for_cookies)
  return _internal_site_for_cookies();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void IsolationInfo::set_site_for_cookies(ArgT0&& arg0, ArgT... args) {
 _impl_._has_bits_[0] |= 0x00000004u;
 _impl_.site_for_cookies_.Set(static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:net.proto.IsolationInfo.site_for_cookies)
}
inline std::string* IsolationInfo::mutable_site_for_cookies() {
  std::string* _s = _internal_mutable_site_for_cookies();
  // @@protoc_insertion_point(field_mutable:net.proto.IsolationInfo.site_for_cookies)
  return _s;
}
inline const std::string& IsolationInfo::_internal_site_for_cookies() const {
  return _impl_.site_for_cookies_.Get();
}
inline void IsolationInfo::_internal_set_site_for_cookies(const std::string& value) {
  _impl_._has_bits_[0] |= 0x00000004u;
  _impl_.site_for_cookies_.Set(value, GetArenaForAllocation());
}
inline std::string* IsolationInfo::_internal_mutable_site_for_cookies() {
  _impl_._has_bits_[0] |= 0x00000004u;
  return _impl_.site_for_cookies_.Mutable(GetArenaForAllocation());
}
inline std::string* IsolationInfo::release_site_for_cookies() {
  // @@protoc_insertion_point(field_release:net.proto.IsolationInfo.site_for_cookies)
  if (!_internal_has_site_for_cookies()) {
    return nullptr;
  }
  _impl_._has_bits_[0] &= ~0x00000004u;
  auto* p = _impl_.site_for_cookies_.Release();
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (_impl_.site_for_cookies_.IsDefault()) {
    _impl_.site_for_cookies_.Set("", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  return p;
}
inline void IsolationInfo::set_allocated_site_for_cookies(std::string* site_for_cookies) {
  if (site_for_cookies != nullptr) {
    _impl_._has_bits_[0] |= 0x00000004u;
  } else {
    _impl_._has_bits_[0] &= ~0x00000004u;
  }
  _impl_.site_for_cookies_.SetAllocated(site_for_cookies, GetArenaForAllocation());
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (_impl_.site_for_cookies_.IsDefault()) {
    _impl_.site_for_cookies_.Set("", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  // @@protoc_insertion_point(field_set_allocated:net.proto.IsolationInfo.site_for_cookies)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace proto
}  // namespace net

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_isolation_5finfo_2eproto
