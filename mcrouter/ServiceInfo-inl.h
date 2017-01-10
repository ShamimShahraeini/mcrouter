/*
 *  Copyright (c) 2016, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include <folly/Format.h>
#include <folly/json.h>
#include <folly/Memory.h>
#include <folly/Range.h>

#include "mcrouter/CarbonRouterInstance.h"
#include "mcrouter/McrouterFiberContext.h"
#include "mcrouter/ProxyBase.h"
#include "mcrouter/ProxyConfigBuilder.h"
#include "mcrouter/ProxyRequestContext.h"
#include "mcrouter/config-impl.h"
#include "mcrouter/config.h"
#include "mcrouter/lib/McRequestList.h"
#include "mcrouter/lib/RouteHandleTraverser.h"
#include "mcrouter/lib/fbi/cpp/globals.h"
#include "mcrouter/lib/fbi/cpp/util.h"
#include "mcrouter/lib/network/CarbonMessageList.h"
#include "mcrouter/lib/network/gen/MemcacheRouterInfo.h"
#include "mcrouter/lib/network/TypedMsg.h"
#include "mcrouter/options.h"
#include "mcrouter/routes/ProxyRoute.h"
#include "mcrouter/standalone_options.h"

namespace facebook {
namespace memcache {
namespace mcrouter {
namespace detail {

template <class RouterInfo>
class RouteHandlesCommandDispatcher {
 public:
  bool dispatch(
      size_t typeId,
      folly::StringPiece key,
      const ProxyRoute<RouterInfo>& proxyRoute,
      std::string& outStr) {
    return dispatcher_.dispatch(typeId, *this, key, proxyRoute, outStr);
  }

  template <class Request>
  static void processMsg(
      RouteHandlesCommandDispatcher<RouterInfo>& me,
      folly::StringPiece key,
      const ProxyRoute<RouterInfo>& proxyRoute,
      std::string& outStr) {
    outStr = me.processMsgInternal<Request>(key, proxyRoute);
  }

 private:
  CallDispatcher<
      // Request List
      typename RouterInfo::RoutableRequests,
      // Dispatcher class
      detail::RouteHandlesCommandDispatcher<RouterInfo>,
      // List of types of args to Dispatcher::processMsg()
      folly::StringPiece,
      const ProxyRoute<RouterInfo>&,
      std::string&>
      dispatcher_;

  template <class Request>
  std::string processMsgInternal(
      folly::StringPiece key,
      const ProxyRoute<RouterInfo>& proxyRoute) {
    std::string tree;
    int level = 0;
    RouteHandleTraverser<typename RouterInfo::RouteHandleIf> t(
        [&tree, &level](const typename RouterInfo::RouteHandleIf& rh) {
          tree.append(std::string(level, ' ') + rh.routeName() + '\n');
          ++level;
        },
        [&level]() { --level; });
    proxyRoute.traverse(Request(key), t);
    return tree;
  }
};

template <class RouterInfo>
class RouteCommandDispatcher {
 public:
  bool dispatch(
      size_t typeId,
      const std::shared_ptr<
          ProxyRequestContextTyped<RouterInfo, ServiceInfoRequest>>& ctx,
      folly::StringPiece keyStr,
      ProxyBase& proxy,
      const ProxyRoute<RouterInfo>& proxyRoute) {
    return dispatcher_.dispatch(typeId, *this, ctx, keyStr, proxy, proxyRoute);
  }

  template <class Request>
  static void processMsg(
      RouteCommandDispatcher<RouterInfo>& me,
      const std::shared_ptr<
          ProxyRequestContextTyped<RouterInfo, ServiceInfoRequest>>& ctx,
      folly::StringPiece keyStr,
      ProxyBase& proxy,
      const ProxyRoute<RouterInfo>& proxyRoute) {
  proxy.fiberManager().addTaskFinally(
      [keyStr, &proxy, &proxyRoute]() {
        auto destinations = folly::make_unique<std::vector<std::string>>();
        folly::fibers::Baton baton;
        auto rctx =
            ProxyRequestContextWithInfo<RouterInfo>::createRecordingNotify(
                proxy,
                baton,
                [&destinations](
                    folly::StringPiece, size_t, const AccessPoint& dest) {
                  destinations->push_back(dest.toHostPortString());
                });
        Request recordingReq(keyStr);
        fiber_local<RouterInfo>::runWithLocals(
            [ ctx = std::move(rctx), &recordingReq, &proxyRoute ]() mutable {
              fiber_local<RouterInfo>::setSharedCtx(std::move(ctx));
              /* ignore the reply */
              proxyRoute.route(recordingReq);
            });
        baton.wait();
        return destinations;
      },
      [ctx](folly::Try<std::unique_ptr<std::vector<std::string>>>&& data) {
        std::string str;
        const auto& destinations = *data;
        for (const auto& d : *destinations) {
          if (!str.empty()) {
            str.push_back('\r');
            str.push_back('\n');
          }
          str.append(d);
        }
        ReplyT<ServiceInfoRequest> reply(mc_res_found);
        reply.value() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, str);
        ctx->sendReply(std::move(reply));
      });
  }

 private:
  CallDispatcher<
      // Request List
      typename RouterInfo::RoutableRequests,
      // Dispatcher class
      detail::RouteCommandDispatcher<RouterInfo>,
      // List of types of args to Dispatcher::processMsg()
      const std::shared_ptr<
          ProxyRequestContextTyped<RouterInfo, ServiceInfoRequest>>&,
      folly::StringPiece,
      ProxyBase&,
      const ProxyRoute<RouterInfo>&>
      dispatcher_;
};

} // detail

template <class RouterInfo>
struct ServiceInfo<RouterInfo>::ServiceInfoImpl {
  ProxyBase* proxy_;
  ProxyRoute<RouterInfo>& proxyRoute_;
  std::unordered_map<
    std::string,
    std::function<std::string(const std::vector<folly::StringPiece>& args)>>
  commands_;

  detail::RouteHandlesCommandDispatcher<RouterInfo>
      routeHandlesCommandDispatcher_;
  mutable detail::RouteCommandDispatcher<RouterInfo> routeCommandDispatcher_;

  ServiceInfoImpl(ProxyBase* proxy, const ProxyConfig<RouterInfo>& config);

  void handleRequest(
      folly::StringPiece req,
      const std::shared_ptr<
          ProxyRequestContextTyped<RouterInfo, ServiceInfoRequest>>& ctx) const;

  void handleRouteCommand(
      const std::shared_ptr<
          ProxyRequestContextTyped<RouterInfo, ServiceInfoRequest>>& ctx,
      const std::vector<folly::StringPiece>& args) const;
};

/* Must be here since unique_ptr destructor needs to know complete
   ServiceInfoImpl type */
template <class RouterInfo>
ServiceInfo<RouterInfo>::~ServiceInfo() {
}

template <class RouterInfo>
ServiceInfo<RouterInfo>::ServiceInfo(
    ProxyBase* proxy,
    const ProxyConfig<RouterInfo>& config)
    : impl_(folly::make_unique<ServiceInfoImpl>(proxy, config)) {}

template <class RouterInfo>
ServiceInfo<RouterInfo>::ServiceInfoImpl::ServiceInfoImpl(
    ProxyBase* proxy,
    const ProxyConfig<RouterInfo>& config)
    : proxy_(proxy), proxyRoute_(config.proxyRoute()) {
  commands_.emplace("version",
    [] (const std::vector<folly::StringPiece>& args) {
      return MCROUTER_PACKAGE_STRING;
    }
  );

  commands_.emplace("config_age",
    [proxy] (const std::vector<folly::StringPiece>& args) {
      /* capturing this and accessing proxy_ crashes gcc-4.7 */
      return std::to_string(proxy->stats().getConfigAge(time(nullptr)));
    }
  );

  commands_.emplace("config_file",
    [this] (const std::vector<folly::StringPiece>& args) {
      folly::StringPiece configStr = proxy_->router().opts().config;
      if (configStr.startsWith(ConfigApi::kFilePrefix)) {
        configStr.removePrefix(ConfigApi::kFilePrefix);
        return configStr.str();
      }

      if (proxy_->router().opts().config_file.empty()) {
        throw std::runtime_error("no config file found!");
      }

      return proxy_->router().opts().config_file;
    }
  );

  commands_.emplace("options",
    [this] (const std::vector<folly::StringPiece>& args) {
      if (args.size() > 1) {
        throw std::runtime_error("options: 0 or 1 args expected");
      }

      auto optDict = proxy_->router().getStartupOpts();

      if (args.size() == 1) {
        auto it = optDict.find(args[0].str());
        if (it == optDict.end()) {
          throw std::runtime_error("options: option " + args[0].str() +
                                   " not found");
        }
        return it->second;
      }

      // Print all options in order listed in the file
      auto optData = McrouterOptions::getOptionData();
      auto startupOpts = McrouterStandaloneOptions::getOptionData();
      optData.insert(optData.end(), startupOpts.begin(), startupOpts.end());
      std::string str;
      for (auto& opt : optData) {
        if (optDict.find(opt.name) != optDict.end()) {
          str.append(opt.name + " " + optDict[opt.name] + "\n");
        }
      }
      return str;
    }
  );

  /*
    This is a special case and handled separately below

  {"route", ...
  },

  */

  commands_.emplace("route_handles",
    [this] (const std::vector<folly::StringPiece>& args) {
      if (args.size() != 2) {
        throw std::runtime_error("route_handles: 2 args expected");
      }
      auto requestName = args[0];
      auto key = args[1];

      auto typeId = carbon::getTypeIdByName(
          requestName, typename RouterInfo::RoutableRequests());

      std::string res;
      if (!routeHandlesCommandDispatcher_.dispatch(
              typeId, key, proxyRoute_, res)) {
        throw std::runtime_error(
            folly::sformat("route: unknown request {}", requestName));
      }
      return res;
    }
  );

  commands_.emplace("config_md5_digest",
    [&config] (const std::vector<folly::StringPiece>& args) {
      if (config.getConfigMd5Digest().empty()) {
        throw std::runtime_error("no config md5 digest found!");
      }
      return config.getConfigMd5Digest();
    }
  );

  commands_.emplace("config_sources_info",
    [this] (const std::vector<folly::StringPiece>& args) {
      auto configInfo = proxy_->router().configApi().getConfigSourcesInfo();
      return toPrettySortedJson(configInfo);
    }
  );

  commands_.emplace("preprocessed_config",
    [this] (const std::vector<folly::StringPiece>& args) {
      std::string confFile;
      std::string path;
      if (!proxy_->router().configApi().getConfigFile(confFile, path)) {
        throw std::runtime_error("Can not load config from " + path);
      }
      ProxyConfigBuilder builder(proxy_->router().opts(),
                                 proxy_->router().configApi(),
                                 confFile);
      return toPrettySortedJson(builder.preprocessedConfig());
    }
  );

  commands_.emplace("hostid",
    [] (const std::vector<folly::StringPiece>& args) {
      return folly::to<std::string>(globals::hostid());
    }
  );

  commands_.emplace("verbosity",
    [] (const std::vector<folly::StringPiece>& args) {
      if (args.size() == 1) {
        auto before = FLAGS_v;
        FLAGS_v = folly::to<int>(args[0]);
        return folly::sformat("{} -> {}", before, FLAGS_v);
      } else if (args.empty()) {
        return folly::to<std::string>(FLAGS_v);
      }
      throw std::runtime_error("expected at most 1 argument, got "
            + folly::to<std::string>(args.size()));
    }
  );
}

template <class RouterInfo>
void ServiceInfo<RouterInfo>::ServiceInfoImpl::handleRequest(
    folly::StringPiece key,
    const std::shared_ptr<
        ProxyRequestContextTyped<RouterInfo, ServiceInfoRequest>>& ctx) const {
  auto p = key.find('(');
  auto cmd = key;
  folly::StringPiece argsStr(key.end(), key.end());
  if (p != folly::StringPiece::npos &&
      key.back() == ')') {
    assert(key.size() - p >= 2);
    cmd = folly::StringPiece(key.begin(), key.begin() + p);
    argsStr = folly::StringPiece(key.begin() + p + 1,
                                 key.begin() + key.size() - 1);
  }
  std::vector<folly::StringPiece> args;
  if (!argsStr.empty()) {
    folly::split(',', argsStr, args);
  }

  std::string replyStr;
  try {
    if (cmd == "route") {
      /* Route is a special case since it involves background requests */
      handleRouteCommand(ctx, args);
      return;
    }

    auto it = commands_.find(cmd.str());
    if (it == commands_.end()) {
      throw std::runtime_error("unknown command: " + cmd.str());
    }
    replyStr = it->second(args);
    if (!replyStr.empty() && replyStr.back() == '\n') {
      replyStr = replyStr.substr(0, replyStr.size() - 1);
    }
  } catch (const std::exception& e) {
    replyStr = std::string("ERROR: ") + e.what();
  }
  ReplyT<ServiceInfoRequest> reply(mc_res_found);
  reply.value() = folly::IOBuf(folly::IOBuf::COPY_BUFFER, replyStr);
  ctx->sendReply(std::move(reply));
}

template <class RouterInfo>
void ServiceInfo<RouterInfo>::ServiceInfoImpl::handleRouteCommand(
    const std::shared_ptr<
        ProxyRequestContextTyped<RouterInfo, ServiceInfoRequest>>& ctx,
    const std::vector<folly::StringPiece>& args) const {
  if (args.size() != 2) {
    throw std::runtime_error("route: 2 args expected");
  }
  auto requestName = args[0];
  auto key = args[1];

  auto typeId = carbon::getTypeIdByName(
      requestName, typename RouterInfo::RoutableRequests());

  if (!routeCommandDispatcher_.dispatch(
          typeId, ctx, key, *proxy_, proxyRoute_)) {
    throw std::runtime_error(
        folly::sformat("route: unknown request {}", requestName));
  }
}

template <class RouterInfo>
void ServiceInfo<RouterInfo>::handleRequest(
    folly::StringPiece key,
    const std::shared_ptr<
        ProxyRequestContextTyped<RouterInfo, ServiceInfoRequest>>& ctx) const {
  impl_->handleRequest(key, ctx);
}

} // mcrouter
} // memcache
} // facebook