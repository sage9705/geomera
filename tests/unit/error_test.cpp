#include <gtest/gtest.h>
#include <geomera/error.hpp>

TEST(ErrorTest, SerializationRoundTrip) {
    const std::vector<geomera::ErrorCode> codes = {
        geomera::ErrorCode::IoError,
        geomera::ErrorCode::InvalidMesh,
        geomera::ErrorCode::OperationFailed,
        geomera::ErrorCode::Timeout,
        geomera::ErrorCode::ResourceExhaustion,
        geomera::ErrorCode::InternalError
    };

    for (const auto code : codes) {
        geomera::Error err;
        err.code = code;
        err.message = "Test message for error";
        err.details = {{"facetIndex", 42}, {"file", "test.stl"}};

        const nlohmann::json j = err;
        EXPECT_EQ(j["code"].get<std::string>(), geomera::errorCodeToString(code));
        EXPECT_EQ(j["message"], "Test message for error");
        EXPECT_EQ(j["details"]["facetIndex"], 42);

        const auto deserialized = j.get<geomera::Error>();
        EXPECT_EQ(deserialized.code, err.code);
        EXPECT_EQ(deserialized.message, err.message);
        EXPECT_EQ(deserialized.details, err.details);
    }
}

TEST(WarningTest, SerializationRoundTrip) {
    geomera::Warning warn;
    warn.code = "WARN_DEGENERATE_FACE";
    warn.message = "Degenerate face skipped";
    warn.details = {{"face", 12}};

    const nlohmann::json j = warn;
    EXPECT_EQ(j["code"], "WARN_DEGENERATE_FACE");
    EXPECT_EQ(j["message"], "Degenerate face skipped");

    const auto deserialized = j.get<geomera::Warning>();
    EXPECT_EQ(deserialized, warn);
}

TEST(MetricsTest, SerializationRoundTrip) {
    geomera::OpMetrics metrics;
    metrics.durationMs = 123.45;
    metrics.peakMemoryBytes = 1048576;

    const nlohmann::json j = metrics;
    EXPECT_DOUBLE_EQ(j["durationMs"], 123.45);
    EXPECT_EQ(j["peakMemoryBytes"], 1048576U);

    const auto deserialized = j.get<geomera::OpMetrics>();
    EXPECT_EQ(deserialized, metrics);
}

TEST(ResultTest, SuccessAndFailureSemantics) {
    auto succeed = []() -> geomera::Result<int> {
        return 42;
    };

    auto fail = []() -> geomera::Result<int> {
        geomera::Error err;
        err.code = geomera::ErrorCode::OperationFailed;
        err.message = "Computation divergent";
        return tl::make_unexpected(err);
    };

    const auto resSuccess = succeed();
    ASSERT_TRUE(resSuccess.has_value());
    EXPECT_EQ(resSuccess.value(), 42);

    const auto resFail = fail();
    ASSERT_FALSE(resFail.has_value());
    EXPECT_EQ(resFail.error().code, geomera::ErrorCode::OperationFailed);
    EXPECT_EQ(resFail.error().message, "Computation divergent");
}

TEST(OpOutputTest, SerializationRoundTrip) {
    geomera::OpOutput<int> output;
    output.value = 100;
    output.warnings.push_back({"WARN_1", "Minor issue", {}});
    output.metrics.durationMs = 5.0;
    output.metrics.peakMemoryBytes = 2048;

    const nlohmann::json j = output;
    EXPECT_EQ(j["value"], 100);
    EXPECT_EQ(j["warnings"].size(), 1U);
    EXPECT_DOUBLE_EQ(j["metrics"]["durationMs"], 5.0);

    const auto deserialized = j.get<geomera::OpOutput<int>>();
    EXPECT_EQ(deserialized, output);
}
