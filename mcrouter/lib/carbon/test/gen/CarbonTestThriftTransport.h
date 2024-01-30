/*
 *  Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 *  This source code is licensed under the MIT license found in the LICENSE
 *  file in the root directory of this source tree.
 *
 */

/*
 *  THIS FILE IS AUTOGENERATED. DO NOT MODIFY IT; ALL CHANGES WILL BE LOST IN
 *  VAIN.
 *
 *  @generated
 */
#pragma once

#include <exception>
#include <memory>

#include <folly/io/async/AsyncSSLSocket.h>
#include <mcrouter/lib/network/AsyncTlsToPlaintextSocket.h>
#include <mcrouter/lib/network/RpcStatsContext.h>
#include <mcrouter/lib/network/ThriftTransport.h>
#include <mcrouter/McrouterFiberContext.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>

#include "mcrouter/lib/carbon/test/gen/gen-cpp2/CarbonTestAsyncClient.h"

namespace facebook {
namespace memcache {

template <>
class ThriftTransportMethods<carbon::test::CarbonTestRouterInfo> : public ThriftTransportUtil {
 public:
  ThriftTransportMethods() = default;
  virtual ~ThriftTransportMethods() override = default;

folly::Try<apache::thrift::RpcResponseComplete<carbon::test::TestReply>> sendSyncHelper(
    typename carbon::test::CarbonTestRouterInfo::RouteHandleAsyncClient* thriftClient,
    const carbon::test::TestRequest& request,
    apache::thrift::RpcOptions& rpcOptions,
    RpcStatsContext* rpcStatsContext = nullptr) {
  bool needServerLoad = mcrouter::fiber_local<carbon::test::CarbonTestRouterInfo>::getThriftServerLoadEnabled();
  if (FOLLY_UNLIKELY(needServerLoad)) {
    rpcOptions.setWriteHeader(kLoadHeader, kDefaultLoadCounter);
  }
  if (FOLLY_UNLIKELY(request.getCryptoAuthToken().has_value())) {
    rpcOptions.setWriteHeader(
        std::string{carbon::MessageCommon::kCryptoAuthTokenHeader}, request.getCryptoAuthToken().value());
  }
  rpcOptions.setContextPropMask(0);

#ifndef LIBMC_FBTRACE_DISABLE
  traceRequest(request, rpcOptions);
#endif
  auto reply = thriftClient->sync_complete_test(
      std::move(rpcOptions), request);
  if (rpcStatsContext && reply.hasValue()) {
      auto& stats = reply->responseContext.rpcSizeStats;
      rpcStatsContext->requestBodySize = stats.requestSerializedSizeBytes;
      rpcStatsContext->replySizeBeforeCompression = stats.responseSerializedSizeBytes;
      rpcStatsContext->replySizeAfterCompression = stats.responseWireSizeBytes;
      if (FOLLY_UNLIKELY(needServerLoad && reply->responseContext.serverLoad)) {
        rpcStatsContext->serverLoad = ServerLoad(
            static_cast<int32_t>(*reply->responseContext.serverLoad));
      }
  }
#ifndef LIBMC_FBTRACE_DISABLE
  traceResponse(request, reply);
#endif
  return reply;
}

folly::Try<apache::thrift::RpcResponseComplete<carbon::test::TestReplyStringKey>> sendSyncHelper(
    typename carbon::test::CarbonTestRouterInfo::RouteHandleAsyncClient* thriftClient,
    const carbon::test::TestRequestStringKey& request,
    apache::thrift::RpcOptions& rpcOptions,
    RpcStatsContext* rpcStatsContext = nullptr) {
  bool needServerLoad = mcrouter::fiber_local<carbon::test::CarbonTestRouterInfo>::getThriftServerLoadEnabled();
  if (FOLLY_UNLIKELY(needServerLoad)) {
    rpcOptions.setWriteHeader(kLoadHeader, kDefaultLoadCounter);
  }
  if (FOLLY_UNLIKELY(request.getCryptoAuthToken().has_value())) {
    rpcOptions.setWriteHeader(
        std::string{carbon::MessageCommon::kCryptoAuthTokenHeader}, request.getCryptoAuthToken().value());
  }
  rpcOptions.setContextPropMask(0);

#ifndef LIBMC_FBTRACE_DISABLE
  traceRequest(request, rpcOptions);
#endif
  auto reply = thriftClient->sync_complete_testStringKey(
      std::move(rpcOptions), request);
  if (rpcStatsContext && reply.hasValue()) {
      auto& stats = reply->responseContext.rpcSizeStats;
      rpcStatsContext->requestBodySize = stats.requestSerializedSizeBytes;
      rpcStatsContext->replySizeBeforeCompression = stats.responseSerializedSizeBytes;
      rpcStatsContext->replySizeAfterCompression = stats.responseWireSizeBytes;
      if (FOLLY_UNLIKELY(needServerLoad && reply->responseContext.serverLoad)) {
        rpcStatsContext->serverLoad = ServerLoad(
            static_cast<int32_t>(*reply->responseContext.serverLoad));
      }
  }
#ifndef LIBMC_FBTRACE_DISABLE
  traceResponse(request, reply);
#endif
  return reply;
}

folly::Try<apache::thrift::RpcResponseComplete<McVersionReply>> sendSyncHelper(
    typename carbon::test::CarbonTestRouterInfo::RouteHandleAsyncClient* thriftClient,
    const McVersionRequest& request,
    apache::thrift::RpcOptions& rpcOptions,
    RpcStatsContext* rpcStatsContext = nullptr) {
  bool needServerLoad = mcrouter::fiber_local<carbon::test::CarbonTestRouterInfo>::getThriftServerLoadEnabled();
  if (FOLLY_UNLIKELY(needServerLoad)) {
    rpcOptions.setWriteHeader(kLoadHeader, kDefaultLoadCounter);
  }
  if (FOLLY_UNLIKELY(request.getCryptoAuthToken().has_value())) {
    rpcOptions.setWriteHeader(
        std::string{carbon::MessageCommon::kCryptoAuthTokenHeader}, request.getCryptoAuthToken().value());
  }
  rpcOptions.setContextPropMask(0);

#ifndef LIBMC_FBTRACE_DISABLE
  traceRequest(request, rpcOptions);
#endif
  auto reply = thriftClient->sync_complete_mcVersion(
      std::move(rpcOptions), request);
  if (rpcStatsContext && reply.hasValue()) {
      auto& stats = reply->responseContext.rpcSizeStats;
      rpcStatsContext->requestBodySize = stats.requestSerializedSizeBytes;
      rpcStatsContext->replySizeBeforeCompression = stats.responseSerializedSizeBytes;
      rpcStatsContext->replySizeAfterCompression = stats.responseWireSizeBytes;
      if (FOLLY_UNLIKELY(needServerLoad && reply->responseContext.serverLoad)) {
        rpcStatsContext->serverLoad = ServerLoad(
            static_cast<int32_t>(*reply->responseContext.serverLoad));
      }
  }
#ifndef LIBMC_FBTRACE_DISABLE
  traceResponse(request, reply);
#endif
  return reply;
}

 protected:
  std::optional<apache::thrift::Client<carbon::test::thrift::CarbonTest>> thriftClient_;
};

template <>
class ThriftTransport<carbon::test::CarbonTestRouterInfo> : public ThriftTransportMethods<carbon::test::CarbonTestRouterInfo>,
                                       public ThriftTransportBase {
 public:
  ThriftTransport(folly::EventBase& eventBase, ConnectionOptions options)
      : ThriftTransportBase(eventBase, std::move(options)) {}
  ThriftTransport(folly::VirtualEventBase& eventBase, ConnectionOptions options)
      : ThriftTransportBase(eventBase.getEventBase(), std::move(options)) {}
  ~ThriftTransport() override {
    resetClient();
  }

  void setFlushList(FlushList* flushList) override final {
    flushList_ = flushList;
    if (thriftClient_) {
      auto* channel = static_cast<apache::thrift::RocketClientChannel*>(
          thriftClient_->getChannel());
      channel->setFlushList(flushList_);
    }
  }

  carbon::test::TestReply sendSync(
      const carbon::test::TestRequest& request,
      std::chrono::milliseconds timeout,
      RpcStatsContext* rpcStatsContext = nullptr) {

    return sendSyncImpl([this, &request, timeout, rpcStatsContext] {
      auto* thriftClient = getThriftClient();
      if (FOLLY_LIKELY(thriftClient != nullptr)) {
        auto rpcOptions = getRpcOptions(timeout);
        return sendSyncHelper(thriftClient, request, rpcOptions, rpcStatsContext);
      } else {
        return folly::Try<apache::thrift::RpcResponseComplete<carbon::test::TestReply>>(
            folly::make_exception_wrapper<apache::thrift::transport::TTransportException>(
              apache::thrift::transport::TTransportException::NOT_OPEN,
              "Error creating thrift client."));
      }
    });
  }

  carbon::test::TestReplyStringKey sendSync(
      const carbon::test::TestRequestStringKey& request,
      std::chrono::milliseconds timeout,
      RpcStatsContext* rpcStatsContext = nullptr) {

    return sendSyncImpl([this, &request, timeout, rpcStatsContext] {
      auto* thriftClient = getThriftClient();
      if (FOLLY_LIKELY(thriftClient != nullptr)) {
        auto rpcOptions = getRpcOptions(timeout);
        return sendSyncHelper(thriftClient, request, rpcOptions, rpcStatsContext);
      } else {
        return folly::Try<apache::thrift::RpcResponseComplete<carbon::test::TestReplyStringKey>>(
            folly::make_exception_wrapper<apache::thrift::transport::TTransportException>(
              apache::thrift::transport::TTransportException::NOT_OPEN,
              "Error creating thrift client."));
      }
    });
  }

  McVersionReply sendSync(
      const McVersionRequest& request,
      std::chrono::milliseconds timeout,
      RpcStatsContext* rpcStatsContext = nullptr) {

    return sendSyncImpl([this, &request, timeout, rpcStatsContext] {
      auto* thriftClient = getThriftClient();
      if (FOLLY_LIKELY(thriftClient != nullptr)) {
        auto rpcOptions = getRpcOptions(timeout);
        return sendSyncHelper(thriftClient, request, rpcOptions, rpcStatsContext);
      } else {
        return folly::Try<apache::thrift::RpcResponseComplete<McVersionReply>>(
            folly::make_exception_wrapper<apache::thrift::transport::TTransportException>(
              apache::thrift::transport::TTransportException::NOT_OPEN,
              "Error creating thrift client."));
      }
    });
  }

 private:
  FlushList* flushList_{nullptr};

  apache::thrift::Client<carbon::test::thrift::CarbonTest>* getThriftClient() {
    if (FOLLY_UNLIKELY(!thriftClient_)) {
      thriftClient_ = createThriftClient<apache::thrift::Client<carbon::test::thrift::CarbonTest>>();
      if (thriftClient_.has_value() && flushList_) {
        auto* channel = static_cast<apache::thrift::RocketClientChannel*>(
            thriftClient_->getChannel());
        channel->setFlushList(flushList_);
      }
    }
    if (FOLLY_LIKELY(thriftClient_.has_value())) {
      return &thriftClient_.value();
    }
    return nullptr;
  }

  void resetClient() override final {
    if (thriftClient_) {
      if (auto* channel = static_cast<apache::thrift::RocketClientChannel*>(
            thriftClient_->getChannel())) {
        if (auto* transport = channel->getTransport()) {
          const auto securityMech =
              connectionOptions_.accessPoint->getSecurityMech();
          if (securityMech == SecurityMech::TLS) {
            if (auto* socket = transport->getUnderlyingTransport<folly::AsyncSSLSocket>()) {
              socket->cancelConnect();
            }
          } else if (securityMech == SecurityMech::TLS_TO_PLAINTEXT) {
            if (auto* socket = transport->getUnderlyingTransport<AsyncTlsToPlaintextSocket>()) {
              socket->getUnderlyingTransport<AsyncTlsToPlaintextSocket>()->closeNow();
            }
          } else if (securityMech == SecurityMech::NONE) {
            if (auto* socket = transport->getUnderlyingTransport<folly::AsyncSocket>()) {
              socket->cancelConnect();
            }
          }
        }
        // Reset the callback to avoid the following cycle:
        //  ~ThriftAsyncClient() -> ~RocketClientChannel() ->
        //  channelClosed() -> ~ThriftAsyncClient()
        channel->setCloseCallback(nullptr);
      }
      thriftClient_.reset();
    }
  }
};

} // namespace memcache
} // namespace facebook