#include <string>

#include "extensions/filters/network/tcp_proxy/config.h"

#include "test/mocks/server/mocks.h"
#include "test/test_common/utility.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::_;

namespace Envoy {
namespace Extensions {
namespace NetworkFilters {
namespace TcpProxy {

class RouteIpListConfigTest : public testing::TestWithParam<std::string> {};

INSTANTIATE_TEST_SUITE_P(IpList, RouteIpListConfigTest,
                         ::testing::Values(R"EOF("destination_ip_list": [
                                                  {
                                                    "address_prefix": "192.168.1.1",
                                                    "prefix_len": 32
                                                  },
                                                  {
                                                    "address_prefix": "192.168.1.0",
                                                    "prefix_len": 24
                                                  }
                                                ],
                                                "source_ip_list": [
                                                  {
                                                    "address_prefix": "192.168.0.0",
                                                    "prefix_len": 16
                                                  },
                                                  {
                                                    "address_prefix": "192.0.0.0",
                                                    "prefix_len": 8
                                                  },
                                                  {
                                                    "address_prefix": "127.0.0.0",
                                                    "prefix_len": 8
                                                  }
                                                ],)EOF",
                                           R"EOF("destination_ip_list": [
                                                  {
                                                    "address_prefix": "2001:abcd::",
                                                    "prefix_len": 64
                                                  },
                                                  {
                                                    "address_prefix": "2002:ffff::",
                                                    "prefix_len": 32
                                                  }
                                                ],
                                                "source_ip_list": [
                                                  {
                                                    "address_prefix": "ffee::",
                                                    "prefix_len": 128
                                                  },
                                                  {
                                                    "address_prefix": "2001::abcd",
                                                    "prefix_len": 64
                                                  },
                                                  {
                                                    "address_prefix": "1234::5678",
                                                    "prefix_len": 128
                                                  }
                                                ],)EOF"));

TEST_P(RouteIpListConfigTest, DEPRECATED_FEATURE_TEST(TcpProxy)) {
  const std::string json_string = R"EOF(
  {
    "stat_prefix": "my_stat_prefix",
    "cluster": "foobar",
    "deprecated_v1": {
      "routes": [
        {)EOF" + GetParam() +
                                  R"EOF("destination_ports": "1-1024,2048-4096,12345",
          "cluster": "fake_cluster"
        },
        {
          "source_ports": "23457,23459",
          "cluster": "fake_cluster2"
        }
      ]
    }
  }
  )EOF";

  envoy::config::filter::network::tcp_proxy::v2::TcpProxy proto_config;
  TestUtility::loadFromJson(json_string, proto_config);

  NiceMock<Server::Configuration::MockFactoryContext> context;
  ConfigFactory factory;
  Network::FilterFactoryCb cb = factory.createFilterFactoryFromProto(proto_config, context);
  Network::MockConnection connection;
  EXPECT_CALL(connection, addReadFilter(_));
  cb(connection);
}

TEST(ConfigTest, ValidateFail) {
  NiceMock<Server::Configuration::MockFactoryContext> context;
  EXPECT_THROW(ConfigFactory().createFilterFactoryFromProto(
                   envoy::config::filter::network::tcp_proxy::v2::TcpProxy(), context),
               ProtoValidationException);
}

// Test that a minimal TcpProxy v2 config works.
TEST(ConfigTest, ConfigTest) {
  NiceMock<Server::Configuration::MockFactoryContext> context;
  ConfigFactory factory;
  envoy::config::filter::network::tcp_proxy::v2::TcpProxy config =
      *dynamic_cast<envoy::config::filter::network::tcp_proxy::v2::TcpProxy*>(
          factory.createEmptyConfigProto().get());
  config.set_stat_prefix("prefix");
  config.set_cluster("cluster");

  EXPECT_TRUE(factory.isTerminalFilter());
  Network::FilterFactoryCb cb = factory.createFilterFactoryFromProto(config, context);
  Network::MockConnection connection;
  EXPECT_CALL(connection, addReadFilter(_));
  cb(connection);
}

} // namespace TcpProxy
} // namespace NetworkFilters
} // namespace Extensions
} // namespace Envoy
