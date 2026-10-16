#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>
#include <nlohmann/json.hpp>
#include <tl/expected.hpp>

namespace geomera {

enum class ErrorCode {
    IoError,
    InvalidMesh,
    OperationFailed,
    Timeout,
    ResourceExhaustion,
    InternalError
};

[[nodiscard]] inline constexpr std::string_view errorCodeToString(ErrorCode code) noexcept {
    switch (code) {
        case ErrorCode::IoError:
            return "IoError";
        case ErrorCode::InvalidMesh:
            return "InvalidMesh";
        case ErrorCode::OperationFailed:
            return "OperationFailed";
        case ErrorCode::Timeout:
            return "Timeout";
        case ErrorCode::ResourceExhaustion:
            return "ResourceExhaustion";
        case ErrorCode::InternalError:
            return "InternalError";
    }
    return "UnknownError";
}

struct Error {
    ErrorCode code{ErrorCode::InternalError};
    std::string message;
    nlohmann::json details = nlohmann::json::object();

    [[nodiscard]] bool operator==(const Error& other) const = default;
};

inline void to_json(nlohmann::json& j, const Error& e) {
    j = nlohmann::json{
        {"code", std::string(errorCodeToString(e.code))},
        {"message", e.message},
        {"details", e.details}
    };
}

inline void from_json(const nlohmann::json& j, Error& e) {
    const std::string codeStr = j.at("code").get<std::string>();
    if (codeStr == "IoError") {
        e.code = ErrorCode::IoError;
    } else if (codeStr == "InvalidMesh") {
        e.code = ErrorCode::InvalidMesh;
    } else if (codeStr == "OperationFailed") {
        e.code = ErrorCode::OperationFailed;
    } else if (codeStr == "Timeout") {
        e.code = ErrorCode::Timeout;
    } else if (codeStr == "ResourceExhaustion") {
        e.code = ErrorCode::ResourceExhaustion;
    } else {
        e.code = ErrorCode::InternalError;
    }
    e.message = j.at("message").get<std::string>();
    if (j.contains("details")) {
        e.details = j.at("details");
    }
}

struct Warning {
    std::string code;
    std::string message;
    nlohmann::json details = nlohmann::json::object();

    [[nodiscard]] bool operator==(const Warning& other) const = default;
};

inline void to_json(nlohmann::json& j, const Warning& w) {
    j = nlohmann::json{
        {"code", w.code},
        {"message", w.message},
        {"details", w.details}
    };
}

inline void from_json(const nlohmann::json& j, Warning& w) {
    w.code = j.at("code").get<std::string>();
    w.message = j.at("message").get<std::string>();
    if (j.contains("details")) {
        w.details = j.at("details");
    }
}

struct OpMetrics {
    double durationMs{0.0};
    std::size_t peakMemoryBytes{0};

    [[nodiscard]] bool operator==(const OpMetrics& other) const = default;
};

inline void to_json(nlohmann::json& j, const OpMetrics& m) {
    j = nlohmann::json{
        {"durationMs", m.durationMs},
        {"peakMemoryBytes", m.peakMemoryBytes}
    };
}

inline void from_json(const nlohmann::json& j, OpMetrics& m) {
    m.durationMs = j.value("durationMs", 0.0);
    m.peakMemoryBytes = j.value("peakMemoryBytes", static_cast<std::size_t>(0));
}

template <typename T>
using Result = tl::expected<T, Error>;

template <typename T>
struct OpOutput {
    T value;
    std::vector<Warning> warnings;
    OpMetrics metrics;

    [[nodiscard]] bool operator==(const OpOutput& other) const = default;
};

template <typename T>
void to_json(nlohmann::json& j, const OpOutput<T>& out) {
    j = nlohmann::json{
        {"value", out.value},
        {"warnings", out.warnings},
        {"metrics", out.metrics}
    };
}

template <typename T>
void from_json(const nlohmann::json& j, OpOutput<T>& out) {
    out.value = j.at("value").get<T>();
    if (j.contains("warnings")) {
        out.warnings = j.at("warnings").get<std::vector<Warning>>();
    }
    if (j.contains("metrics")) {
        out.metrics = j.at("metrics").get<OpMetrics>();
    }
}

}
